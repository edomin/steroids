#pragma once

#include <stddef.h>
#include <stdint.h>

#include "steroids/module.h"

typedef uint32_t (*st_u32hashstrfunc_t)(const char *str);
typedef uint32_t (*st_u32hashbytesfunc_t)(const void *ptr, size_t size);

typedef st_modctx_t *(*st_fnv1a_init_t)(st_modctx_t *logger_ctx);
typedef void (*st_fnv1a_quit_t)(st_modctx_t *so_ctx);
typedef uint32_t (*st_fnv1a_u32hash_string_t)(st_modctx_t *fnv1a_ctx,
 const char *str);
typedef uint32_t (*st_fnv1a_u32hash_bytes_t)(st_modctx_t *fnv1a_ctx,
 const void *ptr, size_t size);
typedef st_u32hashstrfunc_t (*st_fnv1a_get_u32hashstr_func_t)(
 st_modctx_t *fnv1a_ctx);
typedef st_u32hashbytesfunc_t (*st_fnv1a_get_u32hashbytes_func_t)(
 st_modctx_t *fnv1a_ctx);

typedef struct {
    st_fnv1a_init_t                  fnv1a_init;
    st_fnv1a_quit_t                  fnv1a_quit;
    st_fnv1a_u32hash_string_t        fnv1a_u32hash_string;
    st_fnv1a_u32hash_bytes_t         fnv1a_u32hash_bytes;
    st_fnv1a_get_u32hashstr_func_t   fnv1a_get_u32hashstr_func;
    st_fnv1a_get_u32hashbytes_func_t fnv1a_get_u32hashbytes_func;
} st_fnv1a_funcs_t;
