#ifndef NOTE_H
#define NOTE_H

#include <stdio.h>
#include "conf.h"
#include "str.h"

#define da_append(arr, i)                                                     \
  do {                                                                        \
    if((arr)->len >= (arr)->cap) {                                            \
      (arr)->cap *= 2;                                                        \
      (arr)->data = realloc((arr)->data, (arr)->cap * sizeof(*(arr)->data));  \
    }                                                                         \
    (arr)->data[(arr)->len++] = (i);                                          \
  }while(0)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_STACK_SIZE 1024

typedef enum {
  INFO,
  GOOD,
  ERROR
} MessageType;

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

typedef struct{
  size_t row;
  size_t col;
} RowCol;

/* typedef enum {
  ADD_STR,
  DELETE_STR,
  ADD_LINE,
  DELETE_LINE,
  REPLACE
} ActionType; */

typedef struct {
  // ActionType type;
  size_t index;
  String *new;
  String *old;
  // size_t replace_length;
} Action;

typedef struct {
  Action actions[MAX_STACK_SIZE];
  ssize_t top;
} ActionStack;

typedef struct {
  CmdType type;
  const char *text;
} Cmd;

extern Cmd default_cmds[NUM_COMMANDS];

typedef struct {
  size_t index;
  RowCol pos;
  size_t last_col;
  float width;
  float height;
  double last_time_moved;
  unsigned int color;
} Cursor;

typedef struct {
  size_t start;
  size_t end;
  size_t wraps;
} Line;

typedef struct {
  Line *data;
  size_t len;
  size_t cap;
} Lines;

typedef struct {
  char *text;
  MessageType type;
} Message;

typedef struct {
  size_t num_chars;
  size_t cur_li; // current line index;
  int current_msg_index;
  size_t d_start;
  String *s;
  Lines *lines;
  Cursor cursor;
  ActionStack undo_stack;
  ActionStack redo_stack;
  Action *cur_act;
  char const * file_path;
  bool is_saved;
  bool is_readonly;
} Buffer ;

typedef struct {
  Buffer **data;
  size_t len;
  size_t cap;
} Buffers;

typedef struct {
  size_t top;
  size_t right;
  size_t bottom;
  size_t left;
} Padding;

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
  unsigned int file_name_color;
  unsigned int status_line_color;
  unsigned int error_color;
  unsigned int success_color;
  unsigned int selection_color;
  unsigned int selected_char_color;
  unsigned int line_highlight_color;
  size_t line_height;
  size_t letter_spacing;
  size_t scroll_pad;
} Config;

typedef struct {
  Mode mode;
  Buffers *buffs;
  size_t current_buff;
  String *prompt;
  const char* HOME_DIR;
  Vector2 mouse;
  Config conf;
  Message *messages[MAX_MESSAGES];
  bool is_full_screen;
  bool should_quit;
  size_t num_msgs;
  char* currentFileName;
  char* searchQuery;
  int numResults;
  int s_width;
  int s_height;
  size_t selected_cmd;
  size_t displayed_cmds[NUM_COMMANDS];
  size_t num_cmds_displayed;
} Editor;

int get_position(Editor* e);

void move_cursor_up(Editor* e);

void move_cursor_down(Editor* e);

char* createchar(int n);

void emptychar(char* line);

void addChar(Editor* e, char c);

void addchar(Editor* e, char* line, char* text);

int get_first_diplayed_index(Editor* e,bool isUp);

Buffer *new_buffer();

size_t get_max_line_length(Editor *e);

size_t get_max_num_lines(Editor *e);

size_t get_lines_wraps(Editor *e, size_t from, size_t to);

void move_cursor_right(Editor* e, size_t count);

void add_char_to_cur_buf(Editor *e, char c, size_t index);

void move_to_end_of_line(Editor* e);

void move_to_beginning_of_line(Editor* e);

void move_to_word_beginning(Editor* e);

void move_to_word_ending(Editor* e);

void add_new_line(Buffer *buff, size_t index);

void update_scroll(Editor *e, bool center_line, bool is_up);

void new_message(Editor *e, const char *message, MessageType type);

void handle_keys(Editor* e);

bool is_selected(Editor *e, size_t index);

bool is_selecting_up(Editor *e);

void remove_chars_cur_buf(Editor *e, size_t index, size_t count);

void handle_tab(Editor* e, bool is_shift_down);

void filter_cmds_by_prompt(Editor *e);

void handle_insert_mode_keys(Editor* e,int c);

void handle_mouse_click(Editor *e, size_t index, bool is_holding);

void handle_click_on_line(Editor *e, int char_x, size_t char_index, bool is_holding);

Editor *init_editor();

void free_buffer(Buffer *buff);

void realloc_editor_buffers(Editor *e);

void delete_lines(Buffer *buff, size_t from, size_t count);

int get_digit_count(int number);

void increase_font_size(Editor *e);

void decrease_font_size(Editor *e);

void move_to_matching_pair(Editor *e, char c);

void action_stack_push(ActionStack *stack, Action action);

Action *action_stack_pop(ActionStack *stack);

Action* init_action(size_t index);

void action_delete_new_str(Editor *e, Action *act);

void action_add_old_str(Editor *e, Action *act);

void undo(Editor *e);

void undo_action(Editor *e, Action *act);

void redo(Editor *e);

void redo_action(Editor *e, Action *act);

void free_action(Action *action);

void action_stack_flush(ActionStack *stack);

void update_action(Action **act, size_t index, char c, bool is_new);

void update_lines(Editor *e);

void move_to_first_line(Editor *e);

void adapte_col_to_cur_line(Editor *e);

size_t get_line_from_index(Lines *lines, size_t index);

void scroll_up(Editor *e, size_t count);

void scroll_down(Editor *e, size_t count);

void update_buf_state(Editor *e);

#endif
