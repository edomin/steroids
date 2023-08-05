#include "ketopt.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#include <safeclib/safe_mem_lib.h>
#include <safeclib/safe_str_lib.h>
#pragma GCC diagnostic pop
#include <safeclib/safe_types.h>

#define LONG_OPT_NUM_TO_INDEX_OFFSET 300
#define SHORTOPT_FMT_SIZE            3
#define SHORT_OPTS_FMT_SIZE          100
#define LONGOPTS_COUNT               64
#define ERR_MSG_BUF_SIZE             1024
#define OPTS_COLUMNS_MAX             1024
#define HELP_COLUMNS_MIN             8

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;
static char               err_msg_buf[ERR_MSG_BUF_SIZE];

typedef enum {
    ST_OT_SHORT,
    ST_OT_LONG,
} st_opttype_t;

ST_MODULE_DEF_GET_FUNC(opts_ketopt)
ST_MODULE_DEF_INIT_FUNC(opts_ketopt)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_opts_ketopt_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_opts_import_functions(st_modctx_t *opts_ctx,
 st_modctx_t *logger_ctx) {
    st_opts_ketopt_t *module = opts_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "opts_ketopt: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("opts_ketopt", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("opts_ketopt", logger, info);
    ST_LOAD_FUNCTION_FROM_CTX("opts_ketopt", logger, warning);

    return true;
}

static st_modctx_t *st_opts_init(int argc, char **argv,
 st_modctx_t *logger_ctx) {
    st_modctx_t      *opts_ctx;
    st_opts_ketopt_t *opts;
    errno_t           err;

    opts_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_opts_ketopt_data, sizeof(st_opts_ketopt_t));

    if (!opts_ctx)
        return NULL;

    opts_ctx->funcs = &st_opts_ketopt_funcs;

    opts = opts_ctx->data;
    opts->logger.ctx = logger_ctx;

    if (!st_opts_import_functions(opts_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, opts_ctx);

        return NULL;
    }

    opts->argc = argc;
    opts->argv = argv;

    err = memset_s(opts->opts, sizeof(st_opt_t) * ST_OPTS_OPTS_MAX, '\0',
     sizeof(st_opt_t) * ST_OPTS_OPTS_MAX);
    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        fprintf(stderr, "Unable to init opts_ketopt: %s\n", err_msg_buf);
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, opts_ctx);

        return NULL;
    }

    opts->opts_count = 0;

    opts->logger.info(opts->logger.ctx, "%s", "opts_ketopt: Opts initialized.");

    return opts_ctx;
}

static void st_opts_quit(st_modctx_t *opts_ctx) {
    st_opts_ketopt_t *opts = opts_ctx->data;

    opts->logger.info(opts->logger.ctx, "%s", "opts_ketopt: Destroying opts.");
    for (unsigned i = 0; i < opts->opts_count; i++) {
        if (opts->opts[i].longopt)
            free(opts->opts[i].longopt);
        if (opts->opts[i].arg_fmt)
            free(opts->opts[i].arg_fmt);
        if (opts->opts[i].opt_descr)
            free(opts->opts[i].opt_descr);
    }

    opts->logger.info(opts->logger.ctx, "%s", "opts_ketopt: Opts destroyed.");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, opts_ctx);
}

static bool st_opts_add_option(st_modctx_t *opts_ctx, char short_option,
 const char *long_option, st_opt_arg_t arg, const char *arg_fmt,
 const char *option_descr) {
    st_opts_ketopt_t *opts = opts_ctx->data;
    st_opt_t         *opt = &opts->opts[opts->opts_count];
    bool              opt_set = false;

    if (!long_option || long_option[0] == '\0' || long_option[1] == '\0') {
        opts->logger.error(opts->logger.ctx,
         "opts_ketopt: Incorrect long option: %s", long_option);
    } else {
        opt->longopt = strdup(long_option);
        if (!opt->longopt) {
            opts->logger.error(opts->logger.ctx,
             "opts_ketopt: Unable to allocate memory for long option \"%s\": "
             "%s", long_option, strerror(errno));
        } else {
            opt->longopt_index = (int)opts->opts_count +
             LONG_OPT_NUM_TO_INDEX_OFFSET;
            opt_set = true;
        }
    }

    if (short_option) {
        if (isalnum(short_option)) {
            opt->shortopt = short_option;
            opt_set = true;
        } else {
            opts->logger.error(opts->logger.ctx,
             "opts_ketopt: Incorrect character for short option \"%c\"",
             short_option);
        }
    }

    if (!opt_set)
        return false;

    opt->arg = arg;

    if (arg_fmt) {
        opt->arg_fmt = strdup(arg_fmt);
        if (!opt->arg_fmt)
            opts->logger.warning(opts->logger.ctx,
             "opts_ketopt: Unable to allocate memory for option argument "
             "format");
    }
    if (option_descr) {
        opt->opt_descr = strdup(option_descr);
        if (!opt->opt_descr)
            opts->logger.warning(opts->logger.ctx,
             "opts_ketopt: Unable to allocate memory for option description");
    }

    opts->opts_count++;

    return true;
}

static bool st_opts_get_str(st_modctx_t *opts_ctx, const char *opt, char *dst,
 size_t dstsize) {
    st_opts_ketopt_t *opts = opts_ctx->data;
    char              short_opts_fmt[SHORT_OPTS_FMT_SIZE] = "";
    ko_longopt_t      longopts[LONGOPTS_COUNT] = {0};
    size_t            longopts_count = 0;
    int               longopt_index = -1;
    char              shortopt = '\0';
    ketopt_t          kopt = KETOPT_INIT;

    if (!dst || !dstsize)
        return false;

    if (!opt || opt[0] == '\0') {
        opts->logger.error(opts->logger.ctx, "opts_ketopt: Empty option");

        return false;
    }

    for (unsigned i = 0; i < opts->opts_count; i++) {
        if (opts->opts[i].shortopt) {
            errno_t err;
            char short_opt_fmt[SHORTOPT_FMT_SIZE] = {opts->opts[i].shortopt,
             '\0', '\0'};

            if (opts->opts[i].arg != ST_OA_NO)
                short_opt_fmt[1] = opts->opts[i].arg == ST_OA_REQUIRED
                 ? ':' : '?';

            err = strncat_s(short_opts_fmt, SHORT_OPTS_FMT_SIZE, short_opt_fmt,
             SHORTOPT_FMT_SIZE);
            if (err) {
                strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
                opts->logger.error(opts->logger.ctx,
                 "opts_ketopt: Unable to construct shortopts format: %s",
                 err_msg_buf);

                return false;
            }
        }

        if (opts->opts[i].longopt) {
            if (strcmp(opts->opts[i].longopt, opt) == 0 ||
             (opt[1] == '\0' && opts->opts[i].shortopt == opt[0])) {
                longopt_index = opts->opts[i].longopt_index;
                shortopt = opts->opts[i].shortopt;
            }

            longopts[longopts_count].name = opts->opts[i].longopt;
            longopts[longopts_count].has_arg = (int)opts->opts[i].arg;
            longopts[longopts_count].val = opts->opts[i].longopt_index;
            longopts_count++;
        }
    }

    while (true) {
        int parse_result = ketopt(&kopt, opts->argc, opts->argv, true,
         short_opts_fmt, longopts);

        if (parse_result == '?' || parse_result == ':')
            continue;

        if (parse_result < 0)
            break;

        if (parse_result == longopt_index ||
         (opt[1] == '\0' && parse_result == opt[0]) ||
         parse_result == shortopt) {
            if (!kopt.arg) {
                dst[0] = '\0';

                return true;
            }

            errno_t err = strcpy_s(dst, dstsize, kopt.arg);
            if (err) {
                strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
                opts->logger.error(opts->logger.ctx,
                 "opts_ketopt: Unable to get option argument: %s", err_msg_buf);

                return false;
            }

            return true;
        }
    }

    return false;
}

static bool st_opts_get_help(st_modctx_t *opts_ctx, char *dst, size_t dstsize,
 size_t columns) {
    st_opts_ketopt_t *module = opts_ctx->data;
    size_t            block_size;
    errno_t           err;
    size_t            opts_columns;
    size_t            descr_columns;

    if (columns <= HELP_COLUMNS_MIN)
        columns = HELP_COLUMNS_MIN;

    opts_columns = columns / 2 - 1;
    descr_columns = columns / 2 - 1;

    if (strcpy_s(dst, dstsize, "Usage:\n") != 0)
        return false;

    err = strcat_s(dst, dstsize, module->argv[0]);
    if (err) {
        strerror_s(err_msg_buf, ERR_MSG_BUF_SIZE, err);
        module->logger.error(module->logger.ctx, "opts_ketopt: strcpy_s: %s",
         err_msg_buf);

        return false;
    }

    if (strcat_s(dst, dstsize, " [OPTS]\n\n") != 0)
        return false;

    for (unsigned i = 0; i < module->opts_count; i++) {
        char        opts[OPTS_COLUMNS_MAX] = {0};
        size_t      opts_len;
        const char *descr = module->opts[i].opt_descr;
        size_t      descr_len = strlen(descr);

        if (module->opts[i].shortopt != ST_OPTS_SHORT_UNSPEC) {
            opts[0] = '-';
            opts[1] = module->opts[i].shortopt;
            opts[2] = '\0';
        } else {
            opts[0] = ' ';
            opts[1] = ' ';
            opts[2] = '\0';
        }

        if (module->opts[i].longopt) {
            opts[2] = module->opts[i].shortopt == ST_OPTS_SHORT_UNSPEC
             ? ' '
             : ',';
            opts[3] = ' ';
            opts[4] = '-';
            opts[5] = '-'; // NOLINT(readability-magic-numbers)
            opts[6] = '\0'; // NOLINT(readability-magic-numbers)

            if (strcat_s(opts, OPTS_COLUMNS_MAX, module->opts[i].longopt) != 0)
                return false;
        }

        if (module->opts[i].arg != ST_OA_NO) {
            if (module->opts[i].arg == ST_OA_OPTIONAL) {
                if (strncat_s(opts, OPTS_COLUMNS_MAX, "[", 1) != 0)
                    return false;
            }

            if (module->opts[i].longopt) {
                if (strncat_s(opts, OPTS_COLUMNS_MAX, "=", 1) != 0)
                    return false;
            } else {
                if (strncat_s(opts, OPTS_COLUMNS_MAX, " ", 1) != 0)
                    return false;
            }

            if (strcat_s(opts, OPTS_COLUMNS_MAX, module->opts[i].arg_fmt) != 0)
                return false;

            if (module->opts[i].arg == ST_OA_OPTIONAL) {
                if (strncat_s(opts, OPTS_COLUMNS_MAX, "]", 1) != 0)
                    return false;
            }
        }

        opts_len = strlen(opts);
        if (strncat_s(dst, dstsize, opts, opts_len) != 0)
            return false;

        if (opts_len > opts_columns) {
            if (strncat_s(dst, dstsize, "\n", 1) != 0)
                return false;
            for (unsigned column = 0; column <= opts_columns; column++) {
                if (strncat_s(dst, dstsize, " ", 1) != 0)
                    return false;
            }
        } else {
            for (unsigned column = 0; column <= opts_columns - opts_len;
             column++) {
                if (strncat_s(dst, dstsize, " ", 1) != 0)
                    return false;
            }
        }

        if (strncat_s(dst, dstsize, " ", 1) != 0)
            return false;


        while (descr_len > 0) {
            size_t      to_copy_len;
            const char *space = NULL;

            if (descr_len > descr_columns) {
                space = descr + descr_columns;

                while (*space != ' ' && space != descr)
                    space--;

                to_copy_len = (space == descr)
                    ? descr_columns
                    : (size_t)(space - descr);
            } else {
                to_copy_len = descr_len;
            }

            if (strncat_s(dst, dstsize, descr, to_copy_len) != 0)
                return false;

            if (to_copy_len < descr_len) {
                if (strncat_s(dst, dstsize, "\n", 1) != 0)
                    return false;
                for (unsigned column = 0; column <= opts_columns;
                 column++) {
                    if (strncat_s(dst, dstsize, " ", 1) != 0)
                        return false;
                }
            }

            if (descr == space)
                to_copy_len++;

            descr_len -= to_copy_len;
            descr += to_copy_len;
        }

        if (strncat_s(dst, dstsize, "\n", 1) != 0)
            return false;
    }

    return true;
}