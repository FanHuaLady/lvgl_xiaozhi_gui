cmake_minimum_required(VERSION 3.10)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE must be set via CMakePresets.json")
endif()

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# 设置交叉编译环境下 pkg-config 使用的路径
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")

project(xiaozhi_linux_lvgl)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UI_USE_SIMULATOR "Build the UI with SDL simulator backends" OFF)

if(UI_USE_SIMULATOR)
    add_compile_definitions(
        LV_USE_SIMULATOR=1
        LV_USE_SDL=1
        LV_USE_LINUX_FBDEV=0
        LV_USE_EVDEV=0
    )
else()
    add_compile_definitions(
        LV_USE_SIMULATOR=0
        LV_USE_SDL=0
        LV_USE_LINUX_FBDEV=1
        LV_USE_EVDEV=1
    )
endif()

add_subdirectory(lvgl)          # LVGL 核心模块
add_subdirectory(ui)            # UI ChatBot 应用模块
add_subdirectory(utils)         # 公共工具模块
add_subdirectory(common)        # common

# Include 配置头文件路径
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})

# 定义主程序源文件
set(MAIN_SOURCES main.c)

# 链接库
add_executable(${PROJECT_NAME} ${MAIN_SOURCES})

# 链接自定义的文件夹库
target_link_libraries(
    ${PROJECT_NAME} 
    ui
    utils
    common
)

include(${CMAKE_CURRENT_LIST_DIR}/lvgl/tests/FindLibDRM.cmake)
include_directories(${Libdrm_INCLUDE_DIRS})

# Link libraries (DRM driver for display)
target_link_libraries(${PROJECT_NAME} lvgl lvgl::examples lvgl::demos lvgl::thorvg ${Libdrm_LIBRARIES} m pthread freetype)

# Custom command to clean up generated files
if(NOT TARGET clean-all)
    add_custom_target(clean-all
        COMMAND find "${CMAKE_BINARY_DIR}" -mindepth 1 -maxdepth 1 -exec rm -rf {} +
        COMMENT "Cleaning all generated files."
    )
endif()
