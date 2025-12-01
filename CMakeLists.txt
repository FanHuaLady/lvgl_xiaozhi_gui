cmake_minimum_required(VERSION 3.10)

# For ARM cross-compilation, toolchain file should be provided via CMakePresets.json
# CMAKE_TOOLCHAIN_FILE must be set before project() call
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE must be set via CMakePresets.json")
endif()

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

# 设置交叉编译环境下 pkg-config 使用的路径
set(ENV{PKG_CONFIG_PATH} "${CMAKE_SYSROOT}/usr/lib/pkgconfig")

project(ChatBotApp)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(lvgl)          # LVGL 核心模块
add_subdirectory(gui_app)       # GUI_APP ChatBot 应用模块
add_subdirectory(utils)         # 公共工具模块
add_subdirectory(common)        # common

# Include 配置头文件路径
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR}/conf)

# 定义主程序源文件
set(MAIN_SOURCES main.c)

# 链接库
add_executable(main ${MAIN_SOURCES})

# 链接自定义的文件夹库
target_link_libraries(
    main 
    gui_app
    utils
    common
)

include(${CMAKE_CURRENT_LIST_DIR}/lvgl/tests/FindLibDRM.cmake)
include_directories(${Libdrm_INCLUDE_DIRS})

# Link libraries (DRM driver for display)
target_link_libraries(main lvgl lvgl::examples lvgl::demos lvgl::thorvg ${Libdrm_LIBRARIES} m pthread)

add_custom_target (run COMMAND ${OUTPUT_PATH}/main DEPENDS main)

# Copy configuration files
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_SOURCE_DIR}/utils/system_para.conf
            ${OUTPUT_PATH}/system_para.conf
    COMMENT "Copying system_para.conf to bin directory"
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_SOURCE_DIR}/utils/gaode_adcode.json
            ${OUTPUT_PATH}/gaode_adcode.json
    COMMENT "Copying gaode_adcode.json to bin directory"
)

add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PROJECT_SOURCE_DIR}/utils/cacert.pem
            ${OUTPUT_PATH}/cacert.pem
    COMMENT "Copying cacert.pem to bin directory"
)

# Reference AIChat_demo library
set(AI_CHAT_CLIENT_PATH "${PROJECT_SOURCE_DIR}/../AIChat_demo/Client" CACHE PATH "Path to AIChat_demo Client")
add_subdirectory(${AI_CHAT_CLIENT_PATH} ${CMAKE_BINARY_DIR}/AIChat_build)
target_link_libraries(main AIchat-c-interface AIChatCore)

# Copy AIChat necessary resource files
add_custom_command(
    TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PATH}/third_party/snowboy/resources
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PATH}/third_party/snowboy/resources/models
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PATH}/third_party/audio
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${AI_CHAT_CLIENT_PATH}/third_party/snowboy/resources/common.res
            ${OUTPUT_PATH}/third_party/snowboy/resources/common.res
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${AI_CHAT_CLIENT_PATH}/third_party/snowboy/resources/models/echo.pmdl
            ${OUTPUT_PATH}/third_party/snowboy/resources/models/echo.pmdl
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${AI_CHAT_CLIENT_PATH}/third_party/audio/waked.pcm
            ${OUTPUT_PATH}/third_party/audio/waked.pcm
    COMMENT "Copying AIChat necessary resource files to executable output directory"
)

# Set output directory for executable
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_PATH}"
)

# Custom command to clean up generated files
if(NOT TARGET clean-all)
    add_custom_target(clean-all
        COMMAND find "${CMAKE_BINARY_DIR}" -mindepth 1 -maxdepth 1 -exec rm -rf {} +
        COMMENT "Cleaning all generated files."
    )
endif()
