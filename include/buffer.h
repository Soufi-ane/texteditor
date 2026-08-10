#ifndef NOTE_H
#define NOTE_H

#include <raylib.h>
#include <stdio.h>
#include "conf.h"

extern double longPressDelay ;

typedef enum {
  INFO,
  GOOD,
  ERROR
} MessageType;

typedef enum {
  UNKOWN_KEY = 0,
  BG_COL, TXT_COL, CURSOR_COL, SPACE_FOR_TAB,
  UNDER_CURSOR_COL, LN_COL, TAB_S, CAPS_AS_ESCAPE,
  D_LINES, LINES_COL, LN_MODE, VIM_M,
  P_TOP, P_BOTTOM, P_LEFT, P_RIGHT,
  FONT_SIZE, SECONDARY_FONT_SIZE,
  FONT_PRIMARY, FONT_SECONDARY
} ConfigKey;

typedef enum{
  OPEN_FILE,
  NEW_FILE,
  HELP,
  OPEN_CONFIG,
  OPEN_MESSAGES
} CmdType;

typedef enum {
  NORMAL,
  INSERT,
} Mode ;

typedef enum {
  NONE,
  RELATIVE,
  ABSOLUTE
} LineNumbers;

typedef struct {
  CmdType type;
  const char *text;
} Cmd;

extern Cmd default_cmds[NUM_COMMANDS];

typedef struct {
  ssize_t index;
  ssize_t last_index;
  float width;
  float height;
  unsigned int color;
} Cursor;

typedef struct {
  ssize_t length;
  ssize_t capacity;
  char *chars;
} Line;

typedef struct {
  char *text;
  MessageType type;
} Message;

typedef struct {
  ssize_t num_chars;
  ssize_t current_line_index;
  int current_msg_index;
  ssize_t length; 
  ssize_t capacity; 
  ssize_t d_start;
  ssize_t d_length;
  Line **lines;
  Cursor cursor;
  char const * file_path;
  bool is_saved;
  bool is_readonly;
} Buffer ;

typedef struct{
  ssize_t row;
  ssize_t col;
} RowCol;

typedef struct {
  ssize_t top;
  ssize_t right;
  ssize_t bottom;
  ssize_t left;
} Padding;

typedef struct {
  Font font;
  const char *path;
  float size;
  bool is_file_loaded;
  unsigned char* font_file;
} FontData ;

typedef struct {
  bool is_opening_file;
  bool is_menu_open;
  bool is_showing_lines ;
  bool is_spaces_for_tabs;
  bool is_selecting;
  bool is_vim_mode;
  bool caps_lock_as_escape;
  RowCol selection_start;
  ssize_t tab_size;
  LineNumbers ln_mode;
  ssize_t ln_padding;
  Padding padding;
  FontData font_data;
  FontData font_secondary_data;
  unsigned int bg_color;
  unsigned int text_color;
  unsigned int under_cursor_color;
  unsigned int lines_color;
  unsigned int line_numbers_color;
  unsigned int file_name_color;
  unsigned int status_line_color;
  unsigned int error_color;
  unsigned int success_color;
  unsigned int selection_color;
  unsigned int selected_char_color;
  ssize_t line_height;
  ssize_t letter_spacing;
} Config;

typedef struct {
  Mode mode;
  Buffer **buffers;
  ssize_t length;
  ssize_t capacity;
  ssize_t current_buff;
  Line *cmd_prompt;
  const char* HOME_DIR;
  Vector2 mouse;
  Config conf;
  Message *messages[MAX_MESSAGES];
  bool is_full_screen;
  bool should_quit;
  ssize_t num_msgs;
  char* currentFileName;
  char* searchQuery;
  int numResults;
  int s_width;
  int s_height;
  ssize_t selected_cmd;
  ssize_t displayed_cmds[NUM_COMMANDS];
  ssize_t num_cmds_displayed;
} Editor;

int get_position(Editor* e);

void move_cursor_down(Editor* e);

char* createchar(int n);

void deletechar(char** line);

void emptychar(char* line);

void addChar(Editor* e, char c);

void addchar(Editor* e, char* line, char* text);

void add_char_to_note_body(Editor* e,char c);

int get_first_diplayed_index(Editor* e,bool isUp);

Line *new_line(ssize_t capacity);

Buffer *new_buffer(ssize_t capacity);

ssize_t get_max_line_length(Editor *e);

ssize_t get_max_num_lines(Editor *e);

void free_line(Line *line);

ssize_t get_lines_wraps(Editor *e, int from, int to, bool include_last);

void move_cursor_right(Editor* e);

void add_char_to_line(Editor* e, Line* line, char c, bool append);

void remove_current_char(Editor* e);

void move_to_end_of_line(Editor* e);

void move_to_beginning_of_line(Editor* e);

void move_to_word_beginning(Editor* e);

void move_to_word_ending(Editor* e);

void start_new_line(Editor *e);

void update_scroll(Editor *e, bool is_up);

void new_message(Editor *e, const char *message, MessageType type);

void handle_keys(Editor* e);

bool is_selected(Editor *e, RowCol row_col);

bool is_selecting_up(Editor *e);

void pop_char_from_line(Editor* e, Line *line);

void pop_char_single_line(Line *line);

void handle_tab(Editor* e, bool is_shift_down);

void filter_cmds_by_prompt(Editor *e);

void to_lower_case(const char *text, char *dest);

void handle_insert_mode_keys(Editor* e,int c);

void handle_mouse_click(Editor *e, int char_x, int char_y, ssize_t char_index,
  ssize_t line_index, bool is_holding);

void handle_click_on_line(Editor *e, int char_x, ssize_t char_index, bool is_holding);

Editor *init_editor();

void free_buffer(Buffer *buff);

void realloc_editor_buffers(Editor *e);

void delete_lines(Buffer *buff, ssize_t from, ssize_t count);

void realloc_line(Line *line, ssize_t cap);

#endif
