#ifndef NOTE_H
#define NOTE_H
#include <raylib.h>

#define MAX_FILES_NUM       100
#define LINES_COUNT         20
#define LINE_LENGTH         62
#define TITLE_SIZE          40
#define FILE_NAME_LENGTH    50
#define MAX_DISPLAYED_FILES 8
#define TAB_SIZE 2

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
  int index;
  int col;
  int last_col;
  int row;
  int width;
  int height;
} Cursor;

typedef struct {
  int linesNum;
  char* title;
  Date date;
  char* body;
  int length;
  int size;
  int displayStart; 
} Note;

typedef struct {
  Mode mode;
  Note* note;
  const char* HOME_DIR;
  Cursor cursor;
  char* message;
  char* currentFileName;
  char* fileNames[MAX_FILES_NUM];
  char* displayedNames[MAX_DISPLAYED_FILES];
  char* searchQuery;
  char* resultNames[MAX_FILES_NUM];
  char* currentchar;
  bool isChoosingDir;
  bool isOpeningFile;
  bool isDebugging;
  bool isTakingNote;
  bool isSearching;
  bool isNamingFile;
  bool isMenuOpen;
  bool isInsertingTitle;
  bool isNoteBookMode;
  int displayedFilesStart;
  int filesCount;
  int numResults;
  int currentFileIndex;
} Editor;

int get_position(Editor* e);

void move_index_down(Editor* e);

int get_line_length(Editor* e,int start);

char* createchar(int n);

void deletechar(char** line);

void emptychar(char* line);

void addChar(Editor* e, char c);

void addchar(Editor* e, char* line, char* text);

void remove_char(Editor* e);

void handleKeys(Editor* editor, char *fileNames[MAX_FILES_NUM]);

void add_char_to_note_body(Editor* e,char c);

void getLocalDate(Date* d);

void update_cursor_position(Editor* e);

int get_first_diplayed_index(Editor* e,bool isUp);
#endif
