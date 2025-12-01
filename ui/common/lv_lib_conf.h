#ifndef LV_LIB_CONF_H
#define LV_LIB_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../lvgl/lvgl.h"

/*=========================
   MODULE CONFIGURATION
 =========================*/

#define LV_USE_LIB_ANIMATION 1

/*=========================
   MODULE SPECIFIC CONFIGS
 =========================*/

#if LV_USE_LIB_ANIMATION
  /* Animation helpers enabled */
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // LV_LIB_CONF_H
