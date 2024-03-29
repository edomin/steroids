if (${ST_MODULE_GLDEBUG_OPENGL} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "opengl")
set(ST_MODULE_TYPE ${ST_MODULE_GLDEBUG_OPENGL})
set(ST_MODULE_SUBSYSTEM "gldebug")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

set(ST_MODULE_PREREQS [=[
    {
        .subsystem = "gfxctx",
        .name = NULL,
    },
    {
        .subsystem = "glloader",
        .name = NULL,
    },
    {
        .subsystem = "logger",
        .name = NULL,
    },
]=])
set(ST_MODULE_PREREQS_COUNT 3)

configure_file(
    "config.h.in"
    "config.h"
)

bb_add_compile_options(LANG C OPTIONS C_COMPILE_OPTIONS)
bb_add_more_warnings(
    LANG C
    CATEGORIES
        basic
        array
        asciiz
        format
        preprocessor
    OPTIONS C_COMPILE_OPTIONS
)

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_GLDEBUG_OPENGL})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(PkgConfig REQUIRED)
pkg_check_modules(GL REQUIRED gl)

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS})
target_sources(${ST_MODULE_TARGET} PRIVATE
    "opengl.c"
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    ${GL_LINK_LIBRARIES}
)
