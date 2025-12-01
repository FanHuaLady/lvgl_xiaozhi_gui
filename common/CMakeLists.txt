
file(GLOB_RECURSE COMMON_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"  # 查找当前目录及子目录中的所有 .c 文件
)

# 定义 common 模块为一个静态库
add_library(common STATIC ${COMMON_SOURCES})

# 设置 common 的头文件路径
target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/sys_manager
    ${CMAKE_CURRENT_SOURCE_DIR}/gpio_manager
)


find_package(ALSA REQUIRED)
target_link_libraries(common PRIVATE ALSA::ALSA)

find_package(CURL REQUIRED)
target_include_directories(common PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(common PRIVATE CURL::libcurl)