#include <stdio.h>
#include <stdlib.h>
#include "note.h"
#include "ray.h"
#include "files.h"
#include <string.h>
#include <ctype.h>
#include <math.h>

double longPressTime = 0.0f;

int get_lines_count(Note* note){
  if(!note->length) return 0;
  int count = 1;
  for(int i=0; i < note->length; i++) {
    if(note->body[i] == '\n') count++;
  }
  return count;
}

int get_current_line_index(Editor* e){
  int index = 0;
  // int line_wraps = (int) ceil(get_line_length(e,-1) / 62);
  // printf("(%c)\n",e->note->body[e->cursor.index - 1 - e->cursor.col]);
  int curr_length = 0;
  for(int i=e->cursor.index - e->cursor.col; i >= 0; i--){
    curr_length++;
    //todo
    if(e->note->body[i] == '\n' || i == 0){
      index += (int) ceil(get_line_length(e,i + 1) / 62);
      // printf("index += %d\n",(int) ceil(get_line_length(e,i + 1) / 62));
      if(e->note->body[i] == '\n') {
        index++;
        // printf("new line\n");
      }
      curr_length = 0;
    } 
  }
  printf("index ====> %d\n",index);
  return index ;
}

void update_cursor_position(Editor* e){
  int x = e->cursor.col;
  int y = get_current_line_index(e);
  printf("y = %d\n",y);
  int i;
  for(i = e->cursor.index; (i>0 && e->note->body[i-1] != '\n'); i--){}
  x = e->cursor.index - i;
  if(x > 61){
    x = get_line_length(e,-1) % 62;
    // printf("x = %d\n",x);
  } 
  e->cursor.col = x;
  e->cursor.row = y;
  printf("cursor(%d,%d) index : %d\n",x,y,e->cursor.index);
}
  
void update_last_col(Editor* e){ e->cursor.last_col = e->cursor.col; }

void move_index_left(Editor* e) {
  if(e->cursor.index) {
    printf("moving left\n");
    e->cursor.index--;
    update_cursor_position(e);
  }
}

void move_index_right(Editor* e) {
  if(e->cursor.index < e->note->length + (e->mode == INSERT ? 1 : -1)){
    e->cursor.index++;
    update_cursor_position(e);
  }
}

void move_index_down(Editor* e){
  if(get_current_line_index(e) < get_lines_count(e->note) - 1){
    int current_line_len = get_line_length(e,-1);
    int rest_of_chars_in_line = current_line_len - e->cursor.col;
    int next_line_len = get_line_length(e,e->cursor.index + rest_of_chars_in_line + 2);
    // printf("current length : %d\n",current_line_len);
    // printf("next len : %d last_col : %d\n",next_line_len,e->cursor.last_col);
    if(next_line_len > e->cursor.last_col){
      printf("last col : %d rest :%d\n",e->cursor.last_col,rest_of_chars_in_line);
      e->cursor.index += rest_of_chars_in_line + e->cursor.last_col + 1;
    } else {
      printf("this\n");
      e->cursor.index += rest_of_chars_in_line + next_line_len;
    }
    update_cursor_position(e);
  }
}

void move_index_up(Editor* e){
  if(get_current_line_index(e) > 0){
    printf("moving up\n");
    int prev_line_len = get_line_length(e,e->cursor.index - e->cursor.col -2);
    printf("prev len : %d \n",prev_line_len);
    if(prev_line_len - 1 > e->cursor.last_col){
      e->cursor.index -= prev_line_len - e->cursor.last_col + e->cursor.col +1;
    } else {
      e->cursor.index -= e->cursor.col + 2;
    }
    update_cursor_position(e);
  }
}

void remove_char(Editor* e){
	if(e->note->length){
    if(e->cursor.index == e->note->length){
      if(e->cursor.col > 0) {
        move_index_left(e);
      } else {
        e->cursor.index -= 2;
        int len = get_line_length(e,-1);
        e->cursor.index++;
        // e->cursor.row--;
        // e->cursor.col = len - 1;
        printf("-1 length : %d\n",len);
      }
      e->note->length--;
    }else {
      int p = e->cursor.index;
      if(p > 0) {
        for(int i = p - 1; i < e->note->length; i++) {
          e->note->body[i] = e->note->body[i+1];
        }
        move_index_left(e);
        e->note->length--;
      }
    }
    e->note->body[e->note->length] = '\0';
	}
  update_cursor_position(e);
  update_last_col(e);
}

int get_line_length(Editor* e,int start){
  int i,j;
  i = j = start > - 1 ? start : e->cursor.index;
  while((j > 0 && e->note->body[j] != '\n') ||
      (i < e->note->length && e->note->body[i] !='\n'))
  {
    if(e->note->body[i] != '\n' && i < e->note->length) i++;
    if(e->note->body[j] != '\n' && j > 0) j--;
  }
  int len = i - j; 
  return j == 0 ? len : len - 1;
}

void move_cursor_to(Editor* e,int col, int row) {
  e->cursor.col = col;
  e->cursor.row = row;
}


void move_to_beginning_of_line(Editor* e) {
  e->cursor.index -= e->cursor.col;
  e->cursor.last_col = 0;
  update_cursor_position(e);
  update_last_col(e);
}

void move_to_end_of_line(Editor* e) {
  int len = get_line_length(e,-1);
  e->cursor.index += len - e->cursor.col - 1;
  e->cursor.last_col = len -1;
  update_cursor_position(e);
  update_last_col(e);
}

void move_cursor_to_new_line(Editor* e){
  e->cursor.row++;
  e->cursor.col = 0;
}

void move_to_new_line(Editor* e){
  e->note->body[e->cursor.index] = '\n';
  // e->cursor.col = 0;
  // e->cursor.row++;
  e->cursor.index++;
  e->note->length++;
}

void move_to_word_ending(Editor* e,char* line){
  int i = e->cursor.index; 
  while(isspace(e->note->body[i+1])){
    if(e->note->body[i+1] == '\n'){
      int current_line_len = get_line_length(e,-1);
      int rest_of_chars = current_line_len - e->cursor.col;
      int next_line_len = get_line_length(e,e->cursor.index + rest_of_chars + 2) - 1;
      printf("next line is about %d\n",next_line_len);
      printf("found new line at %d\n",i + 1);
      // e->cursor.row++;
      // this is illegal
      // e->cursor.col = -1;
      // printf("moving up to [%d]\n",get_line_length(e,-1));
    } else {
      move_index_right(e);
    }
    i++;
  }
  if(!isalnum(e->note->body[i+1]) && i < e->note->length - 1){
    move_index_right(e);
    i++;
  }
  while(i < e->note->length &&
      (isalnum(e->note->body[i+1]) || e->note->body[i + 1] == '_')) {
    move_index_right(e);
    i++;
  }
  e->cursor.index = i;
  if(e->cursor.col < 0) e->cursor.col = 0;
  update_cursor_position(e);
  update_last_col(e);
}

void move_to_word_beginning(Editor* e,char* line){
  int i = e->cursor.index; 
  while(isspace(e->note->body[i-1])){
    if(e->note->body[i-1] == '\n'){
    } else {
      move_index_left(e);
    }
    i--;
  }
  if(!isalnum(e->note->body[i - 1]) && e->cursor.index > 1) {
    move_index_left(e);
    i--;
  }
  while(i > 0 && (isalnum(e->note->body[i - 1]) || e->note->body[i - 1] == '_')) {
    printf("3 skipping [%c]\n",e->note->body[i - 1]);
    if(e->cursor.col > 0) move_index_left(e);
    i--;
  }
  e->cursor.index = i;
  update_cursor_position(e);
  update_last_col(e);
}


void remove_current_char(Editor* e,char* line){
  if(get_line_length(e,-1)) {
    move_index_right(e);
    remove_char(e);
    // move_index_left(e);
    if(e->cursor.col > strlen(line) - 1 && strlen(line)) {
      move_index_left(e);
    }
  }
}

void handle_tab(Editor* e) {
  if(e->mode == INSERT) {
    if(e->isTakingNote ){
      for(int i = 0;i < TAB_SIZE; ++i){
        addChar(e,' ');
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
      /* remove_char(e,e->searchQuery);
      find(e->fileNames, e->filesCount, e->searchQuery, e->resultNames, &e->numResults); */
    }
    /* if (e->isNamingFile) remove_char(e,e->currentFileName);
    else if (e->isInsertingTitle){
      // if(e->note.title->length > 0) e->cursor.pos.x -= 11;
      remove_char(e,e->note.title);
      move_cursor_left(e); */
    else {
      remove_char(e);
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
      move_cursor_to(e,30,6);
      break;
    case 'a':
      e->mode = INSERT;
      if(e->cursor.col < 61 && get_line_length(e,-1)) move_index_right(e);
      e->isTakingNote = true;
      break;
    case 's':
      if(!strlen(e->currentFileName)){
        e->isNamingFile = true;
        e->mode = INSERT;
        move_cursor_to(e,30,4);
      } else writeFile(e);
      break;
    case 'i':
      e->mode = INSERT;
      break;
    /* case 'e':
      e->isMenuOpen = true;
      e->isChoosingDir = false;
      e->isNamingFile = false;
      e->isOpeningFile = false;
      break; */
    case '$':
      move_to_end_of_line(e);
      break;
    case '0':
      move_to_beginning_of_line(e);
      break;
    case 'x':
      if(e->isTakingNote){
        remove_current_char(e,e->note->body);
      }
      break;
    case 'b':
      if(e->isTakingNote){
        move_to_word_beginning(e,e->note->body);
      }
      break;
    case 'e':
      if(e->isTakingNote){
        move_to_word_ending(e,e->note->body);
      }
      break;
    case 'o':
      if(e->isTakingNote){
        if(e->note->linesNum < LINES_COUNT - 1){
          move_to_new_line(e);
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
      /* find(e->fileNames, e->filesCount,
          e->searchQuery, e->resultNames,
          &e->numResults); */
      break;
    case 'h':
      if(e->isTakingNote) {
        if(e->isInsertingTitle) {}
        else {
          if(e->cursor.col > 0){
            move_index_left(e);
            update_last_col(e);
          } 
        }
      }
      break;
    case 'l':
      if(e->isTakingNote) {
        if(e->isInsertingTitle) {}
        else {
          move_index_right(e);
        } 
      }
      break;
    case 'j':
      if(e->isOpeningFile) handle_move_down_files(e);
      else if(e->isTakingNote) {
        if(e->isInsertingTitle) {
          addChar(e,'\n');
          e->isInsertingTitle = false;
          move_cursor_to_first_char(e);
        }else {
          move_index_down(e);
        }
      }
      // printf("[j] index %d count : %d\n",get_current_line_index(e),get_lines_count(e->note));
      break;
    case 'k':
      if(e->isOpeningFile) handle_move_up_files(e);
      else if(e->isTakingNote) {
        if(e->isInsertingTitle) {}
        else {
          if(e->cursor.row > 0) move_index_up(e);
        }
      }
      break;
  }
}

void handle_insert_mode_keys(Editor* e,int c){
  if (c >= 32) {
    addChar(e,c);
  }
};

void handle_enter(Editor* e){
  if (e->isOpeningFile) {
    char path[100];
    sprintf(path,
      "%s/.local/notes/%s",
      e->HOME_DIR,
      e->fileNames[e->currentFileIndex]);
    readNote(e, path);
    /* if (e->currentLine->chars[e->currentLine->length - 1] == '\n') {
      e->currentLine->length--;
    } */
    e->currentFileName = e->fileNames[e->currentFileIndex];
    e->isOpeningFile = false;
    e->isTakingNote = true;
    } 
    else if (e->isNamingFile){
      //
    }
    if(e->mode == INSERT){
    if (e->isInsertingTitle ) {
      e->isInsertingTitle = false;
      e->isTakingNote = true;
      move_cursor_to_first_char(e);
    } else {
      if (e->cursor.row < LINES_COUNT - 2) {
        add_char_to_note_body(e,'\n');
      }
    }
  }
}

// name
void handleKeys(
  Editor* e,
  char *fileNames[MAX_FILES_NUM]
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
    if(e->cursor.col > 0){
      move_index_left(e);
      update_last_col(e);
    } 
  } 

// enter 
  if (IsKeyPressed(KEY_ENTER)) handle_enter(e);

// capslock 
  if (IsKeyPressed(KEY_CAPS_LOCK)){
    // emptyLine(e->message);
    e->isMenuOpen = false;
    e->isNamingFile = false;
    e->isOpeningFile = false;
    if(e->mode == INSERT){
      e->mode = NORMAL;
      if(e->cursor.col > 0) {
        move_index_left(e);
        update_last_col(e);
      }
    }
  } 
}

/* Line* createLine(int n){
	Line* noteLine = (Line*) malloc(sizeof(Line));
	noteLine->size = n;
	noteLine->chars = (char*) malloc(sizeof(char) * n);
	for(int i=0; i<n; i++) noteLine->chars[i] = '\0';
	noteLine->length = 0;
	return noteLine;
} */
/* void deleteLine(Line** line){
	free((*line)->chars);
	free((*line));
	*line = NULL;
} */

/* void emptyLine(Line* line){
	memset(line->chars,0,line->size);
	line->length = 0;
}
*/

void add_char_to_note_body(Editor* e,char c) {
  if(e->note->length < LINE_LENGTH * LINES_COUNT) {
    if(e->cursor.col < LINE_LENGTH) {
      int line_len = get_line_length(e,-1); 
      if(line_len > 61){
        int last_pos = e->cursor.index + 62 - e->cursor.index;
        printf("last char +1: [%c] at [%d]\n", e->note->body[last_pos+1],last_pos+1);
      }
    } else {
      // e->note->body = realloc(e->note->body,e->note->length + LINE_LENGTH);
      // e->note->body[e->cursor.index++] = '\n';
   } 
    if(e->note->body[e->cursor.index] == '\0'){ 
      e->note->body[e->cursor.index] = c;
      move_index_right(e);
      e->note->length++;
    } else {
      int i;
      for(i = e->note->length; i > e->cursor.index;i--){
        // printf("shifting [%d] to [%d]..\n",i-1,i);
        e->note->body[i] = e->note->body[i-1];
      }
      e->note->body[i] = c;
      move_index_right(e);
      e->note->length++;
    }
  }
  update_cursor_position(e);
  update_last_col(e);
  e->note->body[e->note->length] = '\0';
  printf("[%s]\n",e->note->body);
}

void addChar(Editor* e,char c){
  if(e->isTakingNote) {
    if(e->note->length > e->note->size - 2){
      e->note->body = realloc(e->note->body,e->note->length + LINE_LENGTH);
      e->note->size += LINE_LENGTH;
    }
    add_char_to_note_body(e,c);
  }
}

/* void addLine(Editor* e,Line* line,char* text){
	int len = strlen(text);
	int i;
	for(i=0;i < len; i++){
		line->chars[i] = text[i];
	}
  line->chars[i] = '\0';
	line->length = len;
}
*/

