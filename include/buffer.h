#ifndef NOTE_H
#define NOTE_H

#include <raylib.h>
#include <stddef.h>
#include "conf.h"

extern double longPressDelay ;

typedef enum {
  INFO,
  GOOD,
  ERROR
} MessageType;

typedef enum{
  OPEN_FILE,
  NEW_FILE
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
  size_t index;
  size_t last_index;
  size_t width;
  size_t height;
  unsigned int color;
} Cursor;

typedef struct {
  size_t length;
  size_t capacity;
  char *chars;
} Line;

typedef struct {
  const char *text;
  MessageType type;
} Message;

typedef struct {
  size_t num_chars;
  size_t current_line_index;
  int current_msg_index;
  // TODO : maybe
  // size_t curren_col;
  size_t length; 
  size_t capacity; 
  size_t d_start;
  size_t d_length;
  Line **lines;
  char const * file_path;
} Buffer ;

typedef struct{
  size_t row;
  size_t col;
} RowCol;

typedef struct {
  size_t top;
  size_t right;
  size_t bottom;
  size_t left;
} Padding;

typedef struct {
  bool is_opening_file;
  bool is_debugging;
  bool is_menu_open;
  bool is_showing_lines ;
  bool is_spaces_for_tabs;
  bool is_selecting;
  RowCol selection_start;
  size_t tab_size;
  LineNumbers ln_mode;
  size_t ln_padding;
  Padding padding;
  Font font;
  unsigned int bg_color;
  unsigned int text_color;
  unsigned int under_cursor_color;
  unsigned int lines_color;
  unsigned int line_numbers_color;
  unsigned int file_name_color;
  unsigned int status_line_color;
  unsigned int error_color;
  unsigned int success_color;
  size_t line_height;
  size_t letter_spacing;
} Config;

typedef struct {
  Mode mode;
  Buffer buffer; // TODO: multiple buffers
  Line *cmd_prompt;
  const char* HOME_DIR;
  Cursor cursor;
  Config conf;
  Message *messages[MAX_MESSAGES];
  bool is_full_screen;
  size_t num_msgs;
  char* message;
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

void move_cursor_down(Editor* e);

char* createchar(int n);

void deletechar(char** line);

void emptychar(char* line);

void addChar(Editor* e, char c);

void addchar(Editor* e, char* line, char* text);

void add_char_to_note_body(Editor* e,char c);

int get_first_diplayed_index(Editor* e,bool isUp);

Line *new_line(size_t capacity);

Buffer *new_buffer(size_t capacity);

size_t get_max_line_length(Editor *e);

size_t get_max_num_lines(Editor *e);

void free_line(Line *line);

size_t get_lines_wraps(Editor *e, size_t from, size_t to);

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

#endif
