#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "steroids/module.h"
#include "steroids/types/modules/sprite.h"

#ifndef ST_DRAWQ_T_DEFINED
    typedef struct st_drawq_s st_drawq_t;
#endif

typedef struct {
    const st_sprite_t *sprite;
    float              x;
    float              y;
    float              z;
    float              hscale;
    float              vscale;
    float              angle;
    float              hshear;
    float              vshear;
    float              pivot_x;
    float              pivot_y;
} st_drawrec_t;

typedef st_modctx_t *(*st_drawq_init_t)(st_modctx_t *dynarr_ctx,
 st_modctx_t *logger_ctx, st_modctx_t *sprite_ctx);
typedef void (*st_drawq_quit_t)(st_modctx_t *drawq_ctx);

typedef st_drawq_t *(*st_drawq_create_t)(const st_modctx_t *drawq_ctx);
typedef void (*st_drawq_destroy_t)(st_drawq_t *drawq);
typedef size_t (*st_drawq_len_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_empty_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_export_entry_t)(const st_drawq_t *drawq,
 st_drawrec_t *drawrec, size_t index);
typedef const st_drawrec_t *(*st_drawq_get_all_t)(const st_drawq_t *drawq);
typedef bool (*st_drawq_add_t)(st_drawq_t *drawq, const st_sprite_t *sprite, float x,
 float y, float z, float hscale, float vscale, float angle, float hshear,
 float vshear, float pivot_x, float pivot_y);
typedef bool (*st_drawq_sort_t)(st_drawq_t *drawq);
typedef bool (*st_drawq_clear_t)(st_drawq_t *drawq);

typedef struct {
    st_drawq_init_t         drawq_init;
    st_drawq_quit_t         drawq_quit;
    st_drawq_create_t       drawq_create;
    st_drawq_destroy_t      drawq_destroy;
    st_drawq_len_t          drawq_len;
    st_drawq_empty_t        drawq_empty;
    st_drawq_export_entry_t drawq_export_entry;
    st_drawq_get_all_t      drawq_get_all;
    st_drawq_add_t          drawq_add;
    st_drawq_sort_t         drawq_sort;
    st_drawq_clear_t        drawq_clear;
} st_drawq_funcs_t;
