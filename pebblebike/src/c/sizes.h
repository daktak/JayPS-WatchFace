#ifndef SIZES_H
#define SIZES_H

#define CHAR_WIDTH 35
#define DOT_WIDTH 15
#define CHAR_HEIGHT 51

#ifdef PBL_PLATFORM_CHALK
  #define TOPBAR_HEIGHT 25
  #define SCREEN_W 180
  #define SCREEN_H 180
  // 18
  #define PAGE_OFFSET_X ((180-144)/2)
  // 6
  //#define PAGE_OFFSET_Y ((180-168)/2 - TOPBAR_HEIGHT)
  #define PAGE_OFFSET_Y TOPBAR_HEIGHT
#elif defined(PBL_PLATFORM_EMERY)
  // Emery (Pebble Time 2): 200x228 rectangular, 64-color.
  // Reference layout is 144x168 (basalt); scale offsets/fonts by SCREEN_H/168.
  #define TOPBAR_HEIGHT 24
  #define SCREEN_W 200
  #define SCREEN_H 228
  #define PAGE_OFFSET_X 0
  #define PAGE_OFFSET_Y TOPBAR_HEIGHT
#else
  #define TOPBAR_HEIGHT 18
  #define SCREEN_W 144
  #define SCREEN_H 168
  #define PAGE_OFFSET_X 0
  #define PAGE_OFFSET_Y TOPBAR_HEIGHT
#endif

#define MENU_WIDTH 0

#if defined(PBL_PLATFORM_EMERY)
  #define PAGE_W SCREEN_W
#else
  // Basalt/chalk content area is 144 wide (chalk centers it via PAGE_OFFSET_X).
  #define PAGE_W (144 - MENU_WIDTH)
#endif
#define PAGE_H (SCREEN_H-TOPBAR_HEIGHT)
#define PAGE_SCREEN_CENTER_H (SCREEN_H/2 - PAGE_DATA_TOP_OFFSET_Y)
//#define PAGE_GRECT GRect(PAGE_OFFSET_X, PAGE_OFFSET_Y, PAGE_W, PAGE_H)
#define PAGE_GRECT GRect(0, PAGE_OFFSET_Y, SCREEN_W, PAGE_H)
#define SCREEN_GRECT GRect(0, 0, SCREEN_W, SCREEN_H)

// Font roles. Sizes are baked at build time (name carries px height), so each
// platform selects a size scaled to its screen. Reference baseline: 168px tall.
#if defined(PBL_PLATFORM_EMERY)
  #define PBL_IF_EMERY_ELSE(e, r) (e)
#else
  #define PBL_IF_EMERY_ELSE(e, r) (r)
#endif

#if defined(PBL_PLATFORM_EMERY)
  #define FONT_TOPBAR font_roboto_bold_33
  #define FONT_BIG    font_roboto_bold_72
  #define FONT_FIELD  font_roboto_bold_38
  #define FONT_UNIT   font_roboto_bold_20
#elif defined(PBL_PLATFORM_CHALK)
  #define FONT_TOPBAR font_roboto_bold_16
  #define FONT_BIG    font_roboto_bold_62
  #define FONT_FIELD  fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)
  #define FONT_UNIT   fonts_get_system_font(FONT_KEY_GOTHIC_14)
#else
  #define FONT_TOPBAR fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
  #define FONT_BIG    font_roboto_bold_62
  #define FONT_FIELD  fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)
  #define FONT_UNIT   fonts_get_system_font(FONT_KEY_GOTHIC_14)
#endif

#endif // SIZES_H
