#ifndef CONF_H
#define CONF_H

// raylib
#define GLSL_VERSION          330

// text
#define LINE_HEIGHT           45
#define LETTER_SPACING        14
#define TEXT_COLOR            0xFFFFFFFF
#define CURSOR_COLOR          0xFFFFFFFF
#define UNDER_CURSOR_COLOR    0x000000FF
#define LINE_NUMBERS_COLOR    0x828282FF

// screen
#define SCREEN_HEIGHT         700
#define SCREEN_WIDTH          1300
#define BG_COLOR              0x141415FF
#define DRAW_LINES            false
#define LINES_COLOR           0x545454FF

// editor
#define TAB_SIZE              2
#define MAX_MESSAGES          1024
#define LONG_PRESS_DELAY      0.3f
#define REPEAT_RATE           0.015f
#define FILE_NAME_COLOR       0x828282FF
#define STATUS_LINE_COLOR     0x000000FF
#define IS_SPACES_FOR_TABS    false
#define MAX_PASTE_LENGTH      1024 * 1024
#define ERROR_COLOR           0xFF4C24FF
#define SUCCESS_COLOR         0x00B014FF
#define NUM_COMMANDS          4
#define VIM_MODE              false
#define CAPS_LOCK_AS_ESCAPE   true

// memory
#define DEFAULT_LINE_SIZE     128


#endif
