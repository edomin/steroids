#include "dynarr.h"

#include <stdio.h>

#define METATABLE_NAME "dynarr_ctx"

static st_modsmgr_t                   *global_modsmgr;
static st_modsmgr_funcs_t              global_modsmgr_funcs;

static st_dynarr_init_t                st_dynarr_init;
static st_dynarr_quit_t                st_dynarr_quit;

static st_lua_get_state_t              st_lua_get_state;
static st_lua_create_userdata_t        st_lua_create_userdata;
static st_lua_create_metatable_t       st_lua_create_metatable;
static st_lua_create_module_t          st_lua_create_module;
static st_lua_set_metatable_t          st_lua_set_metatable;
static st_lua_set_cfunction_to_field_t st_lua_set_cfunction_to_field;
static st_lua_set_copy_to_field_t      st_lua_set_copy_to_field;
static st_lua_get_named_userdata_t     st_lua_get_named_userdata;
static st_lua_pop_t                    st_lua_pop;

static void st_luabind_bind_all(st_modctx_t *luabind_ctx);

ST_MODULE_DEF_GET_FUNC(luabind_dynarr)
ST_MODULE_DEF_INIT_FUNC(luabind_dynarr)

#ifdef ST_MODULE_TYPE_shared
st_moddata_t *st_module_init(st_modsmgr_t *modsmgr,
 st_modsmgr_funcs_t *modsmgr_funcs) {
    return st_module_luabind_dynarr_init(modsmgr, modsmgr_funcs);
}
#endif

static bool st_luabind_import_functions(st_modctx_t *luabind_ctx,
 st_modctx_t *lua_ctx, st_modctx_t *logger_ctx) {
    st_luabind_dynarr_t *module = luabind_ctx->data;

    module->logger.error = global_modsmgr_funcs.get_function_from_ctx(
     global_modsmgr, logger_ctx, "error");
    if (!module->logger.error) {
        fprintf(stderr,
         "luabind_dynarr: Unable to load function \"error\" from module "
         "\"logger\"\n");

        return false;
    }

    ST_LOAD_FUNCTION_FROM_CTX("luabind_dynarr", logger, debug);
    ST_LOAD_FUNCTION_FROM_CTX("luabind_dynarr", logger, info);

    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", dynarr, init);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", dynarr, quit);

    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, get_state);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, create_userdata);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, create_metatable);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, create_module);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, set_metatable);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, set_cfunction_to_field);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, set_copy_to_field);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, get_named_userdata);
    ST_LOAD_GLOBAL_FUNCTION("luabind_dynarr", lua, pop);

    return true;
}

static st_modctx_t *st_luabind_init(st_modctx_t *logger_ctx,
 st_modctx_t *lua_ctx) {
    st_modctx_t         *luabind_ctx;
    st_luabind_dynarr_t *luabind;

    luabind_ctx = global_modsmgr_funcs.init_module_ctx(global_modsmgr,
     &st_module_luabind_dynarr_data, sizeof(st_luabind_dynarr_t));

    if (!luabind_ctx)
        return NULL;

    luabind_ctx->funcs = &st_luabind_dynarr_funcs;

    luabind = luabind_ctx->data;
    luabind->lua.ctx = lua_ctx;
    luabind->logger.ctx = logger_ctx;

    if (!st_luabind_import_functions(luabind_ctx, lua_ctx, logger_ctx)) {
        global_modsmgr_funcs.free_module_ctx(global_modsmgr, luabind_ctx);

        return NULL;
    }

    st_luabind_bind_all(luabind_ctx);

    luabind->logger.info(luabind->logger.ctx,
     "luabind_dynarr: Dynarr funcs binding initialized");

    return luabind_ctx;
}

static void st_luabind_quit(st_modctx_t *luabind_ctx) {
    st_luabind_dynarr_t *module = luabind_ctx->data;

    module->logger.info(module->logger.ctx,
     "luabind_events: Dynarr funcs binding destroyed.");
    global_modsmgr_funcs.free_module_ctx(global_modsmgr, luabind_ctx);
}

static int st_dynarr_init_bind(st_luastate_t *lua_state) {
    void        *userdata = st_lua_create_userdata(lua_state,
     sizeof(st_modctx_t *));
    st_modctx_t *logger_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, "logger_ctx");

    *(st_modctx_t **)userdata = st_dynarr_init(logger_ctx);
    st_lua_set_metatable(lua_state, METATABLE_NAME);

    return 1;
}

static int st_dynarr_quit_bind(st_luastate_t *lua_state) {
    st_modctx_t *dynarr_ctx = *(st_modctx_t **)st_lua_get_named_userdata(
     lua_state, 1, METATABLE_NAME);

    st_dynarr_quit(dynarr_ctx);

    return 0;
}

static void st_luabind_bind_all(st_modctx_t *luabind_ctx) {
    st_luabind_dynarr_t *module = luabind_ctx->data;
    st_luastate_t       *lua_state = st_lua_get_state(module->lua.ctx);

    st_lua_create_module(lua_state, "Dynarr");
    st_lua_set_cfunction_to_field(lua_state, "new_ctx", st_dynarr_init_bind);

    st_lua_pop(lua_state, 3);

    st_lua_create_metatable(lua_state, METATABLE_NAME);

    st_lua_set_cfunction_to_field(lua_state, "__gc", st_dynarr_quit_bind);
    st_lua_set_cfunction_to_field(lua_state, "destroy", st_dynarr_quit_bind);
    st_lua_set_copy_to_field(lua_state, "__index", -1);

    st_lua_pop(lua_state, 1);
}
