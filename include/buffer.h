#ifndef NOTE_H
#define NOTE_H

#include <raylib.h>
#include <stddef.h>

#define MAX_FILES_NUM       100
#define LINES_COUNT         20
#define TITLE_SIZE          40
#define FILE_NAME_LENGTH    50
#define MAX_DISPLAYED_FILES 8
#define TAB_SIZE 2
#define DEFAULT_LINE_SIZE 128

#define NUM_MENU_ICONS 1
#define QUERY_LENGTH 35

#define LONG_PRESS_DELAY    0.3f
#define REPEAT_RATE         0.015f

extern double longPressDelay ;

typedef enum {
  NORMAL,
  INSERT,
} Mode ;

typedef enum {
  WARNING,
  INFO,
  ERROR,
} ErrorType ;

typedef struct {
  char year[5];
  char month[3];
  char day[3];
} Date;

typedef struct {
  size_t index;
  size_t col;
  size_t row;
  size_t last_col;
  size_t width;
  size_t height;
} Cursor;

typedef struct {
  size_t length;
  size_t capacity;
  char *chars;
} Line;

typedef struct {
  size_t num_chars;
  size_t current_line_index;
  // TODO : maybe
  // size_t curren_col;
  size_t length; 
  size_t capacity; 
  Line **lines;
  Line file_name;
} Buffer ;

typedef struct {
  Texture2D menu_icons[NUM_MENU_ICONS];
} Media;

typedef struct {
  size_t top;
  size_t right;
  size_t bottom;
  size_t left;
} Padding;

typedef struct {
  bool isChoosingDir;
  bool isOpeningFile;
  bool isDebugging;
  bool isSearching;
  bool isNamingFile;
  bool isMenuOpen;
  size_t displayedFilesStart;
  int filesCount;
  size_t currentFileIndex;
  Padding padding;
  size_t line_height;
  size_t letter_spacing;
} Config;

typedef struct {
  Mode mode;
  Buffer buffer; // TODO: multiple buffers
  const char* HOME_DIR;
  Cursor cursor;
  Config conf;
  Media media;
  char* message;
  char* currentFileName;
  char* fileNames[MAX_FILES_NUM];
  char* displayedNames[MAX_DISPLAYED_FILES];
  char* searchQuery;
  int result_ids[MAX_FILES_NUM];
  int numResults;
  int s_width;
  char* currentchar;
} Editor;

int get_position(Editor* e);

void move_cursor_down(Editor* e);

char* createchar(int n);

void deletechar(char** line);

void emptychar(char* line);

void addChar(Editor* e, char c);

void addchar(Editor* e, char* line, char* text);

void handleKeys(Editor* editor, char *fileNames[MAX_FILES_NUM]);

void add_char_to_note_body(Editor* e,char c);

void getLocalDate(Date* d);

void update_cursor_position(Editor* e);

int get_first_diplayed_index(Editor* e,bool isUp);

Line *new_line(size_t capacity);

Buffer *new_buffer(size_t capacity);

size_t get_max_line_length(Editor *e);

void free_line(Line *line);

size_t get_lines_wraps(Editor *e, size_t from, size_t to);

void move_cursor_right(Editor* e);

void add_char_to_line(Editor* e, Line* line, char c, bool append);

void remove_current_char(Editor* e);

#endif
