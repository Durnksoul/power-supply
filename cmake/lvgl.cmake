file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../Middlewares/Third_Party/lvgl/src/*.c")
add_library(lvgl STATIC ${LVGL_SOURCES})
target_include_directories(lvgl SYSTEM PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/../Middlewares/Third_Party/lvgl"
    "${CMAKE_CURRENT_LIST_DIR}/../Core/Inc")
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE LV_KCONFIG_IGNORE)
# Keep the graphics library within the F407 Flash budget, including Debug builds.
target_compile_options(lvgl PRIVATE -Os)
