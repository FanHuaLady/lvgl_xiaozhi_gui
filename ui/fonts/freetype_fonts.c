#include "freetype_fonts.h"
#include <stdlib.h>
#include <stdio.h>

/* FreeType 字体缓存池，支持多个不同大小的字体 */
#define FREETYPE_FONT_CACHE_SIZE 10

typedef struct {
    uint16_t size;
    lv_font_t* font;
    const char* path;
} FontCache;

static FontCache sourcehansans_cache[FREETYPE_FONT_CACHE_SIZE];
static FontCache montserrat_cache[FREETYPE_FONT_CACHE_SIZE];
static int sourcehansans_count = 0;
static int montserrat_count = 0;

/**
 * 注意：如果你增加了路径数量，请记得修改数组大小 (例如从 1 改为 2)
 * 或者直接替换掉你不用的路径。
 */
static const char* kFontFallbacks[] = {
    // 1. 优先尝试加载你刚才上传的思源黑体 (使用绝对路径)
    "/root/SourceHanSansSC-Regular.otf",
    // 可以在这里添加更多的字体备选路径
};

#define FONT_FALLBACK_COUNT (sizeof(kFontFallbacks) / sizeof(kFontFallbacks[0]))

/**
 * @brief 尝试从多个路径加载字体
 * 
 * @param size 字体大小
 * @return lv_font_t* 加载成功返回字体指针，失败返回 NULL
 */
static lv_font_t* _load_font_with_fallback(uint16_t size)
{
    lv_font_t* font = NULL;
    
    for (int i = 0; i < FONT_FALLBACK_COUNT; i++) {
        printf("尝试加载字体: %s (大小: %d)\n", kFontFallbacks[i], size);
        
        font = lv_freetype_font_create(kFontFallbacks[i],
                                      LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                      size,
                                      LV_FREETYPE_FONT_STYLE_NORMAL);
        
        if (font != NULL) {
            printf("成功加载字体: %s\n", kFontFallbacks[i]);
            return font;
        } else {
            printf("字体加载失败: %s\n", kFontFallbacks[i]);
        }
    }
    
    printf("警告: 无法加载任何字体文件 (大小: %d)\n", size);
    return NULL;
}

/**
 * @brief 从缓存中获取或创建思源黑体
 */
static lv_font_t* _get_sourcehansans_cached(uint16_t size)
{
    // 检查缓存
    for (int i = 0; i < sourcehansans_count; i++) {
        if (sourcehansans_cache[i].size == size) {
            return sourcehansans_cache[i].font;
        }
    }
    
    // 缓存未命中，需要加载新字体
    if (sourcehansans_count >= FREETYPE_FONT_CACHE_SIZE) {
        printf("警告: 思源黑体缓存已满，无法加载大小为 %d 的字体\n", size);
        return NULL;
    }
    
    lv_font_t* font = _load_font_with_fallback(size);
    if (font != NULL) {
        sourcehansans_cache[sourcehansans_count].size = size;
        sourcehansans_cache[sourcehansans_count].font = font;
        sourcehansans_cache[sourcehansans_count].path = kFontFallbacks[0];
        sourcehansans_count++;
    }
    
    return font;
}

/**
 * @brief 从缓存中获取或创建 Montserrat 字体
 */
static lv_font_t* _get_montserrat_cached(uint16_t size)
{
    // 检查缓存
    for (int i = 0; i < montserrat_count; i++) {
        if (montserrat_cache[i].size == size) {
            return montserrat_cache[i].font;
        }
    }
    
    // 如果没有 Montserrat 字体，使用思源黑体作为后备
    printf("信息: 未找到 Montserrat 字体，将使用思源黑体代替\n");
    return _get_sourcehansans_cached(size);
}

/**
 * @brief 初始化 FreeType 字体系统
 */
void freetype_fonts_init(void)
{
    printf("初始化 FreeType 字体系统...\n");
    
    // 预加载常用字体大小
    printf("预加载字体大小: 14, 20, 24, 32\n");
    
    _get_sourcehansans_cached(14);
    _get_sourcehansans_cached(20);
    _get_sourcehansans_cached(24);
    _get_sourcehansans_cached(32);
    
    printf("FreeType 字体系统初始化完成\n");
}

/**
 * @brief 获取思源黑体字体
 */
lv_font_t* get_font_sourcehansans(uint16_t size)
{
    return _get_sourcehansans_cached(size);
}

/**
 * @brief 获取 Montserrat 字体
 */
lv_font_t* get_font_montserrat(uint16_t size)
{
    return _get_montserrat_cached(size);
}
