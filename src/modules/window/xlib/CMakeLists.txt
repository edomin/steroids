if (${ST_MODULE_WINDOW_XLIB} STREQUAL "no")
    return()
endif()

set(ST_MODULE_NAME "xlib")
set(ST_MODULE_TYPE ${ST_MODULE_WINDOW_XLIB})
set(ST_MODULE_SUBSYSTEM "window")
set(ST_MODULE_TARGET st_${ST_MODULE_SUBSYSTEM}_${ST_MODULE_NAME})

set(ST_MODULE_PREREQS [=[
    {
        .subsystem = "events",
        .name = NULL,
    },
    {
        .subsystem = "logger",
        .name = NULL,
    },
    {
        .subsystem = "monitor",
        .name = "xlib",
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

st_add_module(${ST_MODULE_TARGET} ${ST_MODULE_WINDOW_XLIB})
st_process_internal_module(${ST_MODULE_TARGET} ${ST_MODULE_TYPE})

find_package(X11 REQUIRED)

target_compile_options(${ST_MODULE_TARGET} PRIVATE ${C_COMPILE_OPTIONS})
target_sources(${ST_MODULE_TARGET} PRIVATE
    xlib.c
)

bb_set_c_std(${ST_MODULE_TARGET} STD 11 EXTENSIONS)

target_include_directories(${ST_MODULE_TARGET} PRIVATE
    "${CMAKE_FIND_ROOT_PATH}/include"
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/src"
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${ST_MODULE_TARGET} PRIVATE
    dlist
    ${X11_LIBRARIES}
)
