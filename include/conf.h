#ifndef CONF_H
#define CONF_H

// raylib
#define GLSL_VERSION          330

// text
#define LINE_HEIGHT           45
#define LETTER_SPACING        14

// screen
#define SCREEN_HEIGHT         780
#define SCREEN_WIDTH          1300

// editor
#define VERSION               "1.0.0"
#define MAX_MESSAGES          1024
#define LONG_PRESS_DELAY      0.3f
#define REPEAT_RATE           0.015f
#define MAX_PASTE_LENGTH      (1024 * 1024 * 100)
#define NUM_COMMANDS          5
#define CURSOR_BLINK_INTERVAL 0.5
#define CURSOR_BLINK_DURATION 15
#define TIME_BEFORE_BLINK     1

// memory
#define DEFAULT_LINE_SIZE     128

// paths
#ifdef PROD
#define FONT_PATH           "/usr/local/share/fonts/JetBrainsMono-Regular.ttf"
#define SECONDARY_FONT_PATH "/usr/local/share/fonts/JetBrainsMono-Regular.ttf"
#else 
#define FONT_PATH           "assets/fonts/JetBrainsMono-Regular.ttf"
#define SECONDARY_FONT_PATH "assets/fonts/JetBrainsMono-Regular.ttf"
#endif

typedef enum {
  UNKOWN_KEY = 0,
  BG_COL, TXT_COL,
  CURSOR_COL, UNDER_CURSOR_COL,
  FONT_SIZE, SECONDARY_FONT_SIZE,
  FONT_PRIMARY, FONT_SECONDARY, 
  LINE_HIGHLIGHT, LINE_HIGHLIGHT_COL,
  LN_COL, LN_MODE,
  LINES_COL, D_LINES,
  SPACE_FOR_TAB, TAB_S,
  CAPS_AS_ESCAPE,
  P_TOP, P_BOTTOM,
  P_LEFT, P_RIGHT,
  VIM_M
} ConfigKey;


#endif
