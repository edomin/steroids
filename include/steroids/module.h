#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ST_MT_internal 0
#define ST_MT_shared   1

#define ST_LOAD_FUNCTION_FROM_CTX(this_mod, mod, function)               \
    module->mod.function = global_modsmgr_funcs.get_function_from_ctx(   \
     global_modsmgr, mod##_ctx, #function);                              \
    if (!module->mod.function) {                                         \
        module->logger.error(module->logger.ctx,                         \
         this_mod ": Unable to load function \"%s\" from module \"%s\"", \
         #function, #mod);                                               \
        return false;                                                    \
    }

#define ST_LOAD_FUNCTION(this_mod, subsystem, name, function)            \
    module->subsystem.function = global_modsmgr_funcs.get_function(      \
     global_modsmgr, #subsystem, name, #function);                       \
    if (!module->subsystem.function) {                                   \
        module->logger.error(module->logger.ctx,                         \
         this_mod ": Unable to load function \"%s\" from module \"%s\"", \
         #function, #subsystem);                                         \
        return false;                                                    \
    }

#define ST_LOAD_GLOBAL_FUNCTION(this_mod, mod, function)                      \
    st_##mod##_##function = global_modsmgr_funcs.get_function(global_modsmgr, \
     #mod, NULL, #function);                                                  \
    if (!st_##mod##_##function) {                                             \
        module->logger.error(module->logger.ctx,                              \
         this_mod ": Unable to load function \"%s\" from module \"%s\"",      \
         #function, #mod);                                                    \
        return false;                                                         \
    }

#define ST_MODULE_DEF_GET_FUNC(modname)                                       \
    void *st_module_##modname##_get_func(const char *func_name) {             \
        size_t entry_index = 0;                                               \
        do {                                                                  \
            if (strcmp(st_module_##modname##_funcs[entry_index].func_name,    \
             func_name) == 0)                                                 \
                return st_module_##modname##_funcs[entry_index].func_pointer; \
        } while (st_module_##modname##_funcs[++entry_index].func_name);       \
        return NULL;                                                          \
    }

#define ST_MODULE_DEF_INIT_FUNC(modname)                                    \
    st_moddata_t *st_module_##modname##_init(st_modsmgr_t *modsmgr,         \
     st_modsmgr_funcs_t *modsmgr_funcs) {                                   \
        global_modsmgr_funcs = *modsmgr_funcs;                              \
        global_modsmgr = modsmgr;                                           \
        return &st_module_##modname##_data;                                 \
    }

typedef struct {
    char *subsystem;
    char *name;
    void *data;
    void *funcs;
} st_modctx_t;

typedef struct {
    const char *subsystem;
    const char *name;
} st_modprerq_t;

typedef void *(*st_getfunc_t)(const char *funcname);

typedef struct {
    const char    *name;
    const char    *type;
    const char    *subsystem;
    st_modprerq_t *prereqs;
    size_t         prereqs_count;
    st_getfunc_t   get_function;
} st_moddata_t;

struct st_modsmgr_funcs_s;

#ifndef ST_MODSMGR_T_DEFINED
    typedef struct st_modsmgr_s st_modsmgr_t;
#endif

typedef st_moddata_t *(*st_modinitfunc_t)(st_modsmgr_t *modsmgr,
 struct st_modsmgr_funcs_s *modsmgr_funcs);

typedef bool (*st_modsmgr_load_module_t)(st_modsmgr_t *modsmgr,
 st_modinitfunc_t modinit_func, bool force);
typedef void (*st_modsmgr_process_deps_t)(st_modsmgr_t *modsmgr);
typedef void (*st_modsmgr_get_module_names_t)(st_modsmgr_t *modsmgr, char **dst,
 size_t mods_count, size_t modname_size, const char *subsystem);
typedef void *(*st_modsmgr_get_function_t)(const st_modsmgr_t *modsmgr,
 const char *subsystem, const char *module_name, const char *func_name);
typedef void *(*st_modsmgr_get_function_from_ctx_t)(const st_modsmgr_t *modsmgr,
 const st_modctx_t *ctx, const char *func_name);
typedef st_modctx_t *(*st_modsmgr_init_module_ctx_t)(st_modsmgr_t *modsmgr,
 const st_moddata_t *module_data, size_t data_size);
typedef void (*st_modsmgr_free_module_ctx_t)(st_modsmgr_t *modsmgr,
 st_modctx_t *modctx);

typedef struct st_modsmgr_funcs_s {
    st_modsmgr_get_module_names_t      get_module_names;
    st_modsmgr_load_module_t           load_module;
    st_modsmgr_process_deps_t          process_deps;
    st_modsmgr_get_function_t          get_function;
    st_modsmgr_get_function_from_ctx_t get_function_from_ctx;
    st_modsmgr_init_module_ctx_t       init_module_ctx;
    st_modsmgr_free_module_ctx_t       free_module_ctx;
} st_modsmgr_funcs_t;

typedef struct {
    const char *func_name;
    void       *func_pointer;
} st_modfuncentry_t;
