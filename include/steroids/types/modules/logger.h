#pragma once

#include <stdbool.h>

#include "steroids/module.h"

#define ST_EV_LOG_MSG_SIZE 1000

typedef enum {
    ST_LL_NONE    = 0x0,
    ST_LL_ERROR   = 0x8,
    ST_LL_WARNING = 0x10,
    ST_LL_INFO    = 0x40,
    ST_LL_DEBUG   = 0x80,
    ST_LL_ALL     = 0xFF,
} st_loglvl_t;

typedef void (*st_logcbk_t)(const char *msg, void *userdata);
typedef void (*st_logger_generic_msg_t)(const st_modctx_t *logger_ctx,
 const char* format, ...);

typedef st_modctx_t *(*st_logger_init_t)(st_modctx_t *events_ctx);
typedef void (*st_logger_quit_t)(st_modctx_t *logger_ctx);

typedef bool (*st_logger_enable_events_t)(st_modctx_t *logger_ctx,
 st_modctx_t *events_ctx);
typedef bool (*st_logger_set_stdout_levels_t)(st_modctx_t *logger_ctx,
 st_loglvl_t levels);
typedef bool (*st_logger_set_stderr_levels_t)(st_modctx_t *logger_ctx,
 st_loglvl_t levels);
typedef bool (*st_logger_set_log_file_t)(st_modctx_t *logger_ctx,
 const char *filename, st_loglvl_t levels);
typedef bool (*st_logger_set_callback_t)(st_modctx_t *logger_ctx,
 st_logcbk_t callback, void *userdata, st_loglvl_t levels);
typedef st_logger_generic_msg_t st_logger_debug_t;
typedef st_logger_generic_msg_t st_logger_info_t;
typedef st_logger_generic_msg_t st_logger_warning_t;
typedef st_logger_generic_msg_t st_logger_error_t;
typedef void (*st_logger_set_postmortem_msg_t)(st_modctx_t *logger_ctx,
 const char *msg);

typedef struct {
    st_logger_init_t               logger_init;
    st_logger_quit_t               logger_quit;
    st_logger_enable_events_t      logger_enable_events;
    st_logger_set_stdout_levels_t  logger_set_stdout_levels;
    st_logger_set_stderr_levels_t  logger_set_stderr_levels;
    st_logger_set_log_file_t       logger_set_log_file;
    st_logger_set_callback_t       logger_set_callback;
    st_logger_debug_t              logger_debug;
    st_logger_info_t               logger_info;
    st_logger_warning_t            logger_warning;
    st_logger_error_t              logger_error;
    st_logger_set_postmortem_msg_t logger_set_postmortem_msg;
} st_logger_funcs_t;
