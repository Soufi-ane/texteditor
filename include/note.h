#ifndef NOTE_H
#define NOTE_H
#include <raylib.h>

#define MAX_FILES_NUM       100
#define LINES_COUNT         20
#define LINE_LENGTH         100
#define TITLE_LENGTH        40
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
	char* chars;
	int length;
	int size;
} Line;

typedef struct {
  char year[5];
  char month[3];
  char day[3];
} Date;

typedef struct {
  int col;
  int last_col;
  int row;
  int width;
  int height;
} Cursor;

typedef struct {
  int linesNum;
  Line *title;
  Date date;
  Line *body[LINES_COUNT - 1];
} Note;

typedef struct {
  Mode mode;
  Note note;
  const char* HOME_DIR;
  Cursor cursor;
  Line* message;
  Line* currentFileName;
  Line* fileNames[MAX_FILES_NUM];
  Line* displayedNames[MAX_DISPLAYED_FILES];
  Line* searchQuery;
  Line* resultNames[MAX_FILES_NUM];
  Line* currentLine;
  bool isChoosingDir;
  bool isOpeningFile;
  bool isDebugging;
  bool isTakingNote;
  bool isSearching;
  bool isNamingFile;
  bool isMenuOpen;
  bool isInsertingTitle;
  int displayedFilesStart;
  int filesCount;
  int numResults;
  int currentFileIndex;
} Editor;

Line* createLine(int n);
void deleteLine(Line** line);

void emptyLine(Line* line);

void addChar(
  Editor* e,
	Line* line,
	char c
);

void addLine(
  Editor* e,
	Line* line,
	char* text
);

void remove_char(Editor* e,Line* line);

void handleKeys(
  Editor* editor,
  Line *fileNames[MAX_FILES_NUM]
);


#endif
