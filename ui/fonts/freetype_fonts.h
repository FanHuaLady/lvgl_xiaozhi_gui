#ifndef FREETYPE_FONTS_H
#define FREETYPE_FONTS_H

#include "../../lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 FreeType 字体
 * 
 * 这个函数加载并初始化所有需要的 FreeType 字体文件。
 * 注意：字体文件必须存在于指定的路径中
 */
void freetype_fonts_init(void);

/**
 * @brief 获取思源黑体字体（用于中文显示）
 * 
 * @param size 字体大小（单位：像素）
 * @return lv_font_t* 返回字体指针，如果加载失败返回 NULL
 */
lv_font_t* get_font_sourcehansans(uint16_t size);

/**
 * @brief 获取 Montserrat 字体（用于英文显示）
 * 
 * @param size 字体大小（单位：像素）
 * @return lv_font_t* 返回字体指针，如果加载失败返回 NULL
 */
lv_font_t* get_font_montserrat(uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* FREETYPE_FONTS_H */
