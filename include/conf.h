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
#define MAX_PASTE_LENGTH      1024 * 1024
#define NUM_COMMANDS          5

// memory
#define DEFAULT_LINE_SIZE     128

// paths
  
#define SECONDARY_FONT_PATH "assets/fonts/JetBrainsMono-Regular.ttf"
#ifdef PROD
#define FONT_PATH "/usr/local/share/fonts/JetBrainsMono-Regular.ttf"
#else 
#define FONT_PATH "assets/fonts/JetBrainsMono-Regular.ttf"
#endif

#endif
