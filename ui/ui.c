#include "ui.h"
#include "./pages/ui_ChatBotPage/ui_ChatBotPage.h"
///////////////////// VARIABLES ////////////////////

///////////////////// TEST LVGL SETTINGS ////////////////////

#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// Function ////////////////////

static void msgbox_close_click_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = lv_event_get_target(e);
    bool * mbox_exist = lv_event_get_user_data(e);
    *mbox_exist = false;
}

void ui_msgbox_info(const char * title, const char * text)
{
    static lv_obj_t * current_mbox;
    static bool mbox_exist = false;
    if(mbox_exist)
    {
        lv_msgbox_close(current_mbox);
        mbox_exist = false;
    }
    // 创建新消息框
    current_mbox = lv_msgbox_create(NULL);
    mbox_exist = true;
    lv_msgbox_add_title(current_mbox, title);
    lv_msgbox_add_text(current_mbox, text);
    lv_obj_t * close_btn = lv_msgbox_add_close_button(current_mbox);
    lv_obj_add_event_cb(close_btn, msgbox_close_click_event_cb, LV_EVENT_PRESSED, &mbox_exist);
}

static void _gpios_init(void)
{
    // GPIO
    gpio_init(LED_BLUE, OUT_DIRECTION);
    gpio_init(MOTOR1_INA, OUT_DIRECTION);
    gpio_init(MOTOR1_INB, OUT_DIRECTION);
    gpio_init(MOTOR2_INA, OUT_DIRECTION);
    gpio_init(MOTOR2_INB, OUT_DIRECTION);
    // set default value
    gpio_set_value(LED_BLUE, 0);
    gpio_set_value(MOTOR1_INA, 0);
    gpio_set_value(MOTOR1_INB, 0);
    gpio_set_value(MOTOR2_INA, 0);
    gpio_set_value(MOTOR2_INB, 0);
}

///////////////////// timer //////////////////////

// 1s timer
static void _maintimer_cb(lv_timer_t *timer)
{
    (void)timer;
    static bool led_on;
    led_on = !led_on;
    gpio_set_value(LED_BLUE, led_on ? 1 : 0);
}

///////////////////// SCREENS ////////////////////

void ui_init(void)
{
    _gpios_init();
    lv_disp_t * dispp = lv_display_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    ui_ChatBotPage_init();
    lv_timer_create(_maintimer_cb, 1000, NULL);
}