#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../lvgl/lvgl.h"
#include "./common/lv_lib.h"
#include "../common/sys_manager/sys_manager.h"
#include "../common/gpio_manager/gpio_manager.h"

#define UI_SCREEN_WIDTH 320
#define UI_SCREEN_HEIGHT 240

typedef system_para_t ui_system_para_t;

// extern variables
extern ui_system_para_t ui_system_para;

// IMAGES AND IMAGE SETS
LV_IMG_DECLARE(ui_img_question60_png);    // assets/question60.png
LV_IMG_DECLARE(ui_img_think60_png);    // assets/think60.png
LV_IMG_DECLARE(ui_img_hand60_png);    // assets/hand60.png

// FONTS
LV_FONT_DECLARE(ui_font_heiti14);
LV_FONT_DECLARE(ui_font_heiti22);

// UI INIT
void ui_init(void);

// UI INFO MSGBOX
void ui_msgbox_info(const char * title, const char * text);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif