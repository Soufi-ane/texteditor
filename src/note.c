#include <stdio.h>
#include <stdlib.h>
#include "note.h"
#include "ray.h"
#include "files.h"
#include <string.h>
#include <ctype.h>

double longPressTime = 0.0f;

void remove_char(Editor* e,Line* line){
	if(line->length){
    if(e->cursor.col == line->length - 1){
      line->length--;
      line->chars[line->length] = '\0';
    }else {
      for(int i=e->cursor.col;i<line->length;i++) {
        line->chars[i] = line->chars[i+1];
      }
      line->length--;
      line->chars[line->length] = '\0';
    }
	}
}

void move_to_new_line(Editor* e){
  Line *newLine = createLine(LINE_LENGTH);
  addChar(e,e->currentLine,'\n');
  e->note.body[e->note.linesNum++] = newLine;
  e->currentLine = newLine;
}

void move_cursor_up(Editor* e){
  e->cursor.row--;
  e->currentLine = e->note.body[e->cursor.row];
  if(e->currentLine->length > e->cursor.last_col){
    e->cursor.col = e->cursor.last_col;
  }
  else {
   e->cursor.col = e->currentLine->length 
    - (e->currentLine->length ? 1 : 0);
  }
}

void move_cursor_down(Editor* e){
  e->cursor.row++;
  e->currentLine = e->note.body[e->cursor.row];
  if(e->currentLine->length > e->cursor.last_col){
    e->cursor.col = e->cursor.last_col;
  }
  else {
   e->cursor.col = e->currentLine->length 
    - (e->currentLine->length ? 1 : 0);
  }
}

void move_cursor_left(Editor* e) {
  e->cursor.col--;
  e->cursor.last_col = e->cursor.col;
}
void move_cursor_right(Editor* e) {
  e->cursor.col++;
  e->cursor.last_col = e->cursor.col;
}

void move_to_word_ending(Editor* e,Line* line){
  int i = e->cursor.col; 
  while(isspace(e->currentLine->chars[i+1])){
    move_cursor_right(e);
    i++;
  }
  if(!isalnum(e->currentLine->chars[i+1]) && e->cursor.col < line->length - 1){
    move_cursor_right(e);
    return;
  }
  while(i < line->length && isalnum(e->currentLine->chars[++i])) {
    move_cursor_right(e);
  }
}
void move_to_word_beginning(Editor* e,Line* line){
  int i = e->cursor.col; 
  while(isspace(e->currentLine->chars[i -1])){
    move_cursor_left(e);
    i--;
  }
  if(!isalnum(e->currentLine->chars[i -1]) && e->cursor.col) {
    move_cursor_left(e);
    return;
  }
  while(i > 0 && isalnum(e->currentLine->chars[--i])) {
    move_cursor_left(e);
  }
}

void handle_tab(Editor* e) {
  if(e->mode == INSERT) {
    if(e->isTakingNote ){
      if(e->currentLine->length < 60){
        for(int i = 0;i<TAB_SIZE;++i){
          addChar(e,e->currentLine,' ');
          move_cursor_right(e);
        }
      }else if (e->currentLine->length < 61) {
        addChar(e,e->currentLine,' ');
        move_cursor_right(e);
      }
    }
  }
}

void move_cursor_to_first_char(Editor* e){
  e->cursor.col = 0;
  e->cursor.row = 0;
}

void handle_move_down_files(Editor* e){
  if(e->currentFileIndex < MAX_DISPLAYED_FILES - 1){
    e->currentFileIndex++;
  }
  else if(e->currentFileIndex == MAX_DISPLAYED_FILES - 1) {
    if(e->displayedFilesStart < e->filesCount - MAX_DISPLAYED_FILES) {
      e->displayedFilesStart++;
    }else {
      e->displayedFilesStart = 0;
      e->currentFileIndex = 0;
    }
    refreshDiplayedFiles(e);
  }
  else {
    e->currentFileIndex = 0;
  }
}

void handle_move_up_files(Editor* e){
  if(e->currentFileIndex > 0){
    e->currentFileIndex--;
    if(e->currentFileIndex == e->displayedFilesStart ) {
      refreshDiplayedFiles(e);
    }
  }
  else {
    if(e->displayedFilesStart == 0){
      e->currentFileIndex = MAX_DISPLAYED_FILES - 1;
      e->displayedFilesStart = e->filesCount - MAX_DISPLAYED_FILES;
    }else {
      e->displayedFilesStart--;
    }
    refreshDiplayedFiles(e);
  }
}

void handle_backspace(Editor* e) {
  if(e->mode == INSERT){
    if (e->isSearching) {
      remove_char(e,e->searchQuery);
      find(e->fileNames, e->filesCount, e->searchQuery, e->resultNames, &e->numResults);
    }
    if (e->isNamingFile) remove_char(e,e->currentFileName);
    else if (e->isInsertingTitle){
      // if(e->note.title->length > 0) e->cursor.pos.x -= 11;
      remove_char(e,e->note.title);
      move_cursor_left(e);
    } 
    else {
      if (e->currentLine->length < 1 && e->note.linesNum > 1) {
        e->currentLine = e->note.body[--e->note.linesNum - 1];
        e->cursor.row--;
        e->cursor.col += e->currentLine->length 
          - (e->currentLine->length ? 1 : 0);
      } else if (e->currentLine->length > 0){
        move_cursor_left(e);
      }
      remove_char(e,e->currentLine);
    }
  }
}

void handle_normal_mode_keys(Editor* e, int c){
  switch(c){
    case '?':
      e->isDebugging = !e->isDebugging;
      break;
    case 'n':
      e->mode = INSERT; 
      e->isNamingFile = true;
      e->isMenuOpen = false;
      break;
    case 'a':
      e->mode = INSERT;
      if(e->cursor.col < 61) move_cursor_right(e);
      e->isTakingNote = true;
      break;
    case 's':
      if(!e->currentFileName->length){
        e->isNamingFile = true;
        e->mode = INSERT;
      } else writeFile(e);
      break;
    case 'i':
      e->mode = INSERT;
      // if(e->cursor.col < 61) e->cursor.col++;
      e->isTakingNote = true;
      break;
    /* case 'e':
      e->isMenuOpen = true;
      e->isChoosingDir = false;
      e->isNamingFile = false;
      e->isOpeningFile = false;
      break; */
    case '$':
      e->cursor.last_col = e->cursor.col = e->currentLine->length - 1;
      break;
    case '0':
      e->cursor.last_col = e->cursor.col = 0;
      break;
    case 'b':
      if(e->isTakingNote){
        move_to_word_beginning(e,e->currentLine);
      }
      break;
    case 'e':
      if(e->isTakingNote){
        move_to_word_ending(e,e->currentLine);
      }
      break;
    case 'o':
      if(e->isTakingNote){
        if(e->note.linesNum < LINES_COUNT - 1){
          move_to_new_line(e);
          move_cursor_down(e);
          e->mode = INSERT;
        }
      }
      else {
        e->isOpeningFile = true;
        e->isMenuOpen = false;
      }
      break;
    case '/':
      e->isSearching = true;
      find(e->fileNames, e->filesCount,
          e->searchQuery, e->resultNames,
          &e->numResults);
      break;
    case 'h':
      if(e->isTakingNote) {
        if(e->isInsertingTitle) {}
        else {
          if(e->cursor.col > 0) move_cursor_left(e);
        }
      }
      break;
    case 'l':
      if(e->isTakingNote) {
        if(e->isInsertingTitle) {}
        else {
          if(e->cursor.col < e->currentLine->length - 1) move_cursor_right(e);
        }
      }
      break;
    case 'j':
      if(e->isOpeningFile) handle_move_down_files(e);
      else if(e->isTakingNote) {
        if(e->isInsertingTitle) {
          addChar(e,e->note.title, '\n');
          e->isInsertingTitle = false;
          move_cursor_to_first_char(e);
        }else {
          if(e->cursor.row < e->note.linesNum - 1) move_cursor_down(e);
        }
      }
      break;
    case 'k':
      if(e->isOpeningFile) handle_move_up_files(e);
      else if(e->isTakingNote) {

        if(e->isInsertingTitle) {}
        else {
          if(e->cursor.row > 0) move_cursor_up(e);
        }
      }
      break;
  }
}

void handle_insert_mode_keys(Editor* e,int c){
  if (e->currentLine->length > 61 ) {
    if(e->note.linesNum < LINES_COUNT - 1) {
      Line *newLine;
      newLine = createLine(LINE_LENGTH);
      e->cursor.col = 1;
      e->cursor.row++;
      addChar(e,e->currentLine, '\n');
      e->note.body[e->note.linesNum++] = newLine;
      e->currentLine = newLine;
      addChar(e,e->currentLine, c);
    }
  } else if (c >= 32) {
    if(e->isNamingFile) {
      if(e->currentFileName->length < FILE_NAME_LENGTH - 1){
        addChar(e,e->currentFileName, c);
      }
    }
    else if (e->isInsertingTitle) {
      // todo : word wrap * 2 lines.
      if (e->note.title->length < TITLE_LENGTH - 1) {
        addChar(e,e->note.title, c);
        move_cursor_right(e);
      }
    } else {
      addChar(e,e->currentLine, c);
      move_cursor_right(e);
    }
  }
};

void handle_enter(Editor* e){
  if (e->isOpeningFile) {
    char path[100];
    sprintf(path,
      "%s/.local/notes/%s",
      e->HOME_DIR,
      e->fileNames[e->currentFileIndex]->chars);
    readNote(e, path);
    move_cursor_to_first_char(e);
    e->currentLine = e->note.body[e->note.linesNum - 1];
    // remove \n if it's the last thing.
    if (e->currentLine->chars[e->currentLine->length - 1] == '\n') {
      e->currentLine->length--;
    }
    e->currentFileName = e->fileNames[e->currentFileIndex];
    e->isOpeningFile = false;
    e->isTakingNote = true;
    } 
    else if (e->isNamingFile){
       e->isNamingFile = false;
       e->isTakingNote = true;
       if(e->note.body[0]->length > 0) {
         writeFile(e);
       }
    }
    
    if(e->mode == INSERT){
    if (e->isInsertingTitle ) {
      addChar(e,e->note.title, '\n');
      move_cursor_to_first_char(e);
      e->isInsertingTitle = false;
    } else {
      if (e->note.linesNum < LINES_COUNT - 1) {
        addChar(e,e->currentLine, '\n');
        e->cursor.col = 0;
        e->cursor.row++;
        move_to_new_line(e);
      }
    }
  }
}

// name
void handleKeys(
  Editor* e,
  Line *fileNames[MAX_FILES_NUM]
){
  int c;
  if ((c = GetCharPressed()) >= 8) {
    if (e->mode == NORMAL) handle_normal_mode_keys(e,c);
    else handle_insert_mode_keys(e,c);
  }

//backspace
  if (IsKeyPressed(KEY_BACKSPACE)){
    longPressTime = GetTime();
    handle_backspace(e);
  } 
  if (IsKeyDown(KEY_BACKSPACE)) {
    double now = GetTime();
    if(now - longPressTime > LONG_PRESS_DELAY){
      handle_backspace(e);
      longPressTime = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  if (IsKeyReleased(KEY_BACKSPACE)) longPressTime = 0;

  else if(IsKeyPressed(KEY_TAB)) handle_tab(e);

//escape 
  if (IsKeyPressed(KEY_ESCAPE)) {
    e->mode = NORMAL; 
    if(e->cursor.col > 0) e->cursor.col--;
    emptyLine(e->message);
  } 

// enter 
  if (IsKeyPressed(KEY_ENTER)) handle_enter(e);

// capslock 
  if (IsKeyPressed(KEY_CAPS_LOCK)){
    emptyLine(e->message);
    e->isMenuOpen = false;
    e->isNamingFile = false;
    e->isOpeningFile = false;
    if(e->mode == INSERT){
      e->mode = NORMAL;
      if(e->cursor.col > 0) move_cursor_left(e);
    }
  } 
}

Line* createLine(int n){
	Line* noteLine = (Line*) malloc(sizeof(Line));
	noteLine->size = n;
	noteLine->chars = (char*) malloc(sizeof(char) * n);
	for(int i=0; i<n; i++) noteLine->chars[i] = '\0';
	noteLine->length = 0;
	return noteLine;
}
void deleteLine(Line** line){
	free((*line)->chars);
	free((*line));
	*line = NULL;
}

void emptyLine(Line* line){
	memset(line->chars,0,line->size);
	line->length = 0;
}

void addChar(Editor* e,Line* line,char c){
  //todo: insert char at cursor position
	// line->chars[line->length] = c;
  if(line->length > e->cursor.col){
    for(int i = line->length + 1; i > e->cursor.col;i--) {
      line->chars[i] = line->chars[i-1];
    }
  }
  line->chars[e->cursor.col] = c;
	if(c != '\n') line->length++;
}

void addLine(Editor* e,Line* line,char* text){
	int length = strlen(text);
	// if(length > line->size) {
		// return;
	// }
	int i;
	for(i=0;i< length; i++){
		addChar(e,line, text[i]);
	}
	// addChar(line,'\0');
	// line->length = length ;
}


