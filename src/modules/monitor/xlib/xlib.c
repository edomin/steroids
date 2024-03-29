#include "xlib.h"

#include <errno.h>
#include <stdio.h>

#include <X11/Xlib.h>

#define ERRMSGBUF_SIZE        128
#define DISPLAY_NAME_SIZE_MAX 128

static st_modsmgr_t      *global_modsmgr;
static st_modsmgr_funcs_t global_modsmgr_funcs;

ST_MODULE_DEF_GET_FUNC(monitor_xlib)
ST_MODULE_DEF_INIT_FUNC(monitor_xlib)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_monitor_xlib_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_monitor_import_functions(st_modctx_t *monitor_ctx,
 st_modctx_t *logger_ctx) {
    st_monitor_xlib_t *module = monitor_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "monitor_xlib: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("monitor_xlib", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("monitor_xlib", logger, info);

    return true;
}

static st_modctx_t *st_monitor_init(st_modctx_t *logger_ctx) {
    st_modctx_t       *monitor_ctx;
    st_monitor_xlib_t *module;

    monitor_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_monitor_xlib_data, sizeof(st_monitor_xlib_t));

    if (!monitor_ctx)
        return NULL;

    monitor_ctx->funcs = &st_monitor_xlib_funcs;

    module = monitor_ctx->data;
    module->logger.ctx = logger_ctx;

    if (!st_monitor_import_functions(monitor_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, monitor_ctx);

        return NULL;
    }

    module->logger.info(module->logger.ctx,
     "monitor_xlib: Monitors mgr initialized.");

    return monitor_ctx;
}

static void st_monitor_quit(st_modctx_t *monitor_ctx) {
    st_monitor_xlib_t *module = monitor_ctx->data;

    module->logger.info(module->logger.ctx,
     "monitor_xlib: Monitors mgr destroyed");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, monitor_ctx);
}

static unsigned st_monitor_get_monitors_count(st_modctx_t *monitor_ctx) {
    st_monitor_xlib_t *module = monitor_ctx->data;
    unsigned           monitors_count;
    Display           *display = XOpenDisplay(NULL);

    if (!display) {
        module->logger.error(module->logger.ctx,
         "monitor_xlib: Unable to open default display");

        return 0;
    }

    monitors_count = (unsigned)ScreenCount(display);
    XCloseDisplay(display);
    return monitors_count;
}

static st_monitor_t *st_monitor_open(st_modctx_t *monitor_ctx, unsigned index) {
    st_monitor_xlib_t *module = monitor_ctx->data;
    char               display_name[DISPLAY_NAME_SIZE_MAX];
    st_monitor_t      *monitor;
    int                ret = snprintf(display_name, DISPLAY_NAME_SIZE_MAX,
     ":0.%u", index);

    if (ret < 0 || ret == DISPLAY_NAME_SIZE_MAX) {
        module->logger.error(module->logger.ctx,
         "monitor_xlib: Unable to construct display name for display with "
         "index %u", index);

        return NULL;
    }

    monitor = malloc(sizeof(st_monitor_t));
    if (!monitor) {
        char errbuf[ERRMSGBUF_SIZE];

        if (strerror_r(errno, errbuf, ERRMSGBUF_SIZE) == 0)
            module->logger.error(module->logger.ctx,
             "monitor_xlib: Unable to allocate memory for monitor structure: "
             "%s", errbuf);

        return NULL;
    }

    monitor->handle = XOpenDisplay(display_name);

    if (!monitor->handle) {
        module->logger.error(module->logger.ctx,
         "monitor_xlib: Unable to open display");
        free(monitor);

        return NULL;
    }

    monitor->root_window = DefaultRootWindow(monitor->handle);
    monitor->index = index;

    return monitor;
}

static void st_monitor_release(st_monitor_t *monitor) {
    XCloseDisplay(monitor->handle);
    free(monitor);
}

static unsigned st_monitor_get_width(st_monitor_t *monitor) {
    int width = XDisplayWidth(monitor->handle, (int)monitor->index);

    return width > 0 ? (unsigned)width : 0u;
}

static unsigned st_monitor_get_height(st_monitor_t *monitor) {
    int height = XDisplayHeight(monitor->handle, (int)monitor->index);

    return height > 0 ? (unsigned)height : 0u;
}

static void *st_monitor_get_handle(st_monitor_t *monitor) {
    return monitor->handle;
}
