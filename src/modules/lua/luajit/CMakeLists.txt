if (${ST_MODULE_LUA_LUAJIT} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "luajit")
set(ST_MODULE_TYPE ${ST_MODULE_LUA_LUAJIT})
set(ST_MODULE_SUBSYSTEM "lua")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

set(ST_MODULE_PREREQS [=[
    {
        .subsystem = "logger",
        .name = NULL,
    },
    {
        .subsystem = "opts",
        .name = NULL,
    },
]=])
set(ST_MODULE_PREREQS_COUNT 2)

configure_file(
    "config.h.in"
    "config.h"
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES
        basic
        alloc
        array
        asciiz
        format
        preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_LUA_LUAJIT})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT REQUIRED luajit)

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS})
target_sources(${ST_MODULE_TARGET} PRIVATE
    luajit.c
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LUAJIT_INCLUDE_DIRS}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    slist
    ${LUAJIT_STATIC_LDFLAGS}
)
