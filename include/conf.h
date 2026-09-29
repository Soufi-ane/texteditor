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
#define NUM_COMMANDS          6
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
  VIM_M, SCROLL_PAD,
  STATUS_LINE_FG, STATUS_LINE_BG,
  SELECTION_BG, SELECTION_FG,
  SEARCH_BG, SEARCH_FG
} ConfigKey;

typedef struct {
  size_t top;
  size_t right;
  size_t bottom;
  size_t left;
} Padding;

typedef enum {
  NONE,
  RELATIVE,
  ABSOLUTE
} LineNumbers;

typedef struct {
  Font font;
  const char *path;
  float size;
  bool is_file_loaded;
  unsigned char* font_file;
  int file_size;
} FontData ;

typedef struct {
  bool is_opening_file;
  bool is_menu_open;
  bool is_showing_lines ;
  bool is_spaces_for_tabs;
  bool is_selecting;
  bool is_vim_mode;
  bool caps_lock_as_escape;
  bool is_line_highlight;
  size_t selection_start;
  size_t tab_size;
  LineNumbers ln_mode;
  size_t ln_padding;
  Padding padding;
  FontData font_data;
  FontData font_secondary_data;
  unsigned int bg_color;
  unsigned int text_color;
  unsigned int under_cursor_color;
  unsigned int lines_color;
  unsigned int line_numbers_color;
  unsigned int status_line_bg;
  unsigned int status_line_fg;
  unsigned int error_color;
  unsigned int success_color;
  unsigned int selection_bg;
  unsigned int selection_fg;
  unsigned int selected_char_color;
  unsigned int line_highlight_color;
  unsigned int search_bg;
  unsigned int search_fg;
  size_t line_height;
  size_t letter_spacing;
  size_t scroll_pad;
} Config;


#endif
