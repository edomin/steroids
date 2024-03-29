#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"

#define ST_OPTS_SHORT_UNSPEC       '\0'
#define ST_OPTS_LONG_INDEX_UNSPEC  -1

typedef enum {
    ST_OA_NO       = 0,
    ST_OA_REQUIRED = 1,
    ST_OA_OPTIONAL = 2,
} st_opt_arg_t;

typedef st_modctx_t *(*st_opts_init_t)(int argc, char **argv,
 st_modctx_t *logger_ctx);
typedef void (*st_opts_quit_t)(st_modctx_t *opts_ctx);
typedef bool (*st_opts_add_option_t)(st_modctx_t *opts_ctx, char short_option,
 const char *long_option, st_opt_arg_t arg, const char *arg_fmt,
 const char *option_descr);
typedef bool (*st_opts_get_str_t)(st_modctx_t *opts_ctx, const char *opt,
 char *dst, size_t dstsize);
typedef bool (*st_opts_get_help_t)(st_modctx_t *opts_ctx, char *dst,
 size_t dstsize, size_t columns);

typedef struct {
    st_opts_init_t       opts_init;
    st_opts_quit_t       opts_quit;
    st_opts_add_option_t opts_add_option;
    st_opts_get_str_t    opts_get_str;
    st_opts_get_help_t   opts_get_help;
} st_opts_funcs_t;
