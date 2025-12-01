set(UI_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ui.c
    ${CMAKE_CURRENT_SOURCE_DIR}/inter_process_comms.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pages/ui_ChatBotPage/ui_ChatBotPage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common/animation/lv_lib_animation.c
)

file(GLOB UI_FONT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/fonts/*.c")
file(GLOB UI_IMAGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/images/*.c")

set(UI_SOURCES
    ${UI_CORE_SOURCES}
    ${UI_FONT_SOURCES}
    ${UI_IMAGE_SOURCES}
)

add_library(ui STATIC ${UI_SOURCES})

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/animation
    ${CMAKE_CURRENT_SOURCE_DIR}/pages/ui_ChatBotPage
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts
    ${CMAKE_CURRENT_SOURCE_DIR}/images
)

find_package(json-c REQUIRED)
target_link_libraries(ui PUBLIC json-c)
