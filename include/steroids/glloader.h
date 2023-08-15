#ifndef ST_STEROIDS_GLLOADER_H
#define ST_STEROIDS_GLLOADER_H

#include "steroids/module.h"
#include "steroids/types/modules/glloader.h"

static st_modctx_t *st_glloader_init(st_modctx_t *logger_ctx);
static void st_glloader_quit(st_modctx_t *glloader_ctx);

static void *st_glloader_get_proc_address(st_modctx_t *glloader_ctx,
 const char *funcname);

#endif /* ST_STEROIDS_GLLOADER_H */