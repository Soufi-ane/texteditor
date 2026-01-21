#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "note.h"
#include "files.h"

double longPressTime = 0.0f;

void move_index_to_last_line(Editor* e){
  int i,j;
  for(j=0, i=e->note->length -1; i>0 && j < 1;i--){
    if(e->note->body[i] == '\n') j++;
  }
  e->cursor.index = i+2;
  e->note->displayStart = get_first_diplayed_index(e,false);
  update_cursor_position(e);
}

void add_char_to_line(Editor* e,char* line,int size,char c){
  size_t len = strlen(line);
  if(len > size -1 || !size || c == '\n' || e->cursor.index > len || e->cursor.index < 0) return;
  if(e->cursor.index == len){
    line[len] = c;
  }else {
    int i;
    for(i=len; i > e->cursor.index; i--) line[i] = line[i-1];
    line[i] = c; 
  }
  line[len+1] = '\0';
  e->cursor.index++;
  update_cursor_position(e);
}

void pop_char_from_line(Editor* e,char* line){
  size_t len = strlen(line);
  if(!len) return;
  line[len-1] = '\0';
  e->cursor.index--;
  update_cursor_position(e);
}

int get_lines_count(Note* note){
  if(!note->length) return 0;
  int count = 1;
  for(int i=0; i < note->length; i++) {
    if(note->body[i] == '\n') count++;
  }
  return count;
}

int get_line_index(Editor* e,int start){
  int i,index = 0;
  int start_index = start > -1 ? start : e->cursor.index;
  for(i=start_index - 1;i > 0 ;i--){
    if(e->note->body[i] == '\n') index++; 
  }
  return index ;
}

void update_cursor_position(Editor* e){
  static int calls =0;
  int i; // index of the start of the line;
  for(i = e->cursor.index; (i>0 && e->note->body[i-1] != '\n'); i--);
  e->cursor.col = (e->cursor.index - i) % 62;
  int line_index = get_line_index(e,-1); 
    
  int num_hidden_lines = line_index + 2 - LINES_COUNT;
  int index_start = get_line_index(e,e->note->displayStart) ;
  if(num_hidden_lines < 0) num_hidden_lines = 0;
  e->cursor.row = line_index - index_start;
}
  
void update_last_col(Editor* e){ e->cursor.last_col = e->cursor.col; }

void move_index_left(Editor* e) {
  if(e->cursor.index) {
    e->cursor.index--;
    update_cursor_position(e);
    update_last_col(e);
  }
}

void handle_caps_lock_and_escape(Editor* e){
  e->conf.isMenuOpen = false;
  e->conf.isNamingFile = false;
  e->conf.isOpeningFile = false;
  if(e->conf.isInsertingTitle){
    e->conf.isTakingNote = true;
    e->cursor.index = 0;
    e->conf.isInsertingTitle = false;
    update_cursor_position(e);
  }
  if(e->mode == INSERT){
    e->mode = NORMAL;
    if(e->note->body[e->cursor.index - 1] != '\n') {
      move_index_left(e);
    }
  }
}

void move_index_right(Editor* e) {
  if(e->cursor.index < e->note->length + (e->mode == INSERT ? 1 : -1)){
    e->cursor.index++;
    update_cursor_position(e);
    update_last_col(e);
  }
}

void move_index_down(Editor* e){
  if(get_line_index(e,-1) < get_lines_count(e->note) - 1){
    int current_line_len = get_line_length(e,-1);
    int chars_left = current_line_len - e->cursor.col - 1;
    chars_left = chars_left < 0 ? 0 : chars_left;
    int next_line_len = get_line_length(e,e->cursor.index + chars_left + 1);
    if(next_line_len > e->cursor.last_col){
      e->cursor.index += chars_left + e->cursor.last_col + 1;
    } else {
      e->cursor.index += next_line_len + chars_left + 1;
    }
    if(e->cursor.index < 0) e->cursor.index = 0;
    e->note->displayStart = get_first_diplayed_index(e,false);
    update_cursor_position(e);
  }
}

void move_index_up(Editor* e){
  int line_index = get_line_index(e,-1);
  if(line_index > 0){
    int prev_line_len = get_line_length(e,e->cursor.index - e->cursor.col - 2 /* don't change*/);
    if(prev_line_len - 1 > e->cursor.last_col){
      e->cursor.index -= e->cursor.col + prev_line_len - e->cursor.last_col;
    } else {
      e->cursor.index -= e->cursor.col + (prev_line_len ? 2 : 1);
    }
    if(e->cursor.index < 0) e->cursor.index = 0;
    e->note->displayStart = get_first_diplayed_index(e,true);
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
  while((j >= 0 && e->note->body[j] != '\n') ||
      (i < e->note->length && e->note->body[i] !='\n'))
  {
    if(e->note->body[i] != '\n' && i < e->note->length) i++;
    if(e->note->body[j] != '\n' && j >= 0) j--;
  }
  int len = i - j;
  return len;
}

void move_to_beginning_of_line(Editor* e) {
  e->cursor.index -= e->cursor.col;
  e->cursor.last_col = 0;
  update_last_col(e);
  update_cursor_position(e);
}

void move_to_end_of_line(Editor* e) {
  int len = get_line_length(e,-1);
  e->cursor.index += len - e->cursor.col - (len > 1 ? 2 : 1) ;
  update_last_col(e);
  update_cursor_position(e);
}

void move_to_new_line(Editor* e){
  e->note->body[e->cursor.index] = '\n';
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
    if(e->cursor.col > strlen(line) - 1 && strlen(line)) {
      move_index_left(e);
    }
  }
}

void handle_tab(Editor* e) {
  if(e->mode == INSERT) {
    if(e->conf.isTakingNote ){
      for(int i = 0;i < TAB_SIZE; ++i) addChar(e,' ');
    }
  }
}

void handle_move_down_files(Editor* e){
  if(e->conf.currentFileIndex < e->conf.filesCount - 1){
    e->conf.currentFileIndex++;
    if(e->conf.currentFileIndex + e->conf.displayedFilesStart > MAX_DISPLAYED_FILES -1){ 
      e->conf.displayedFilesStart++;
    }
  }
  else {
    e->conf.currentFileIndex = 0;
    e->conf.displayedFilesStart = 0;
  }
}

void handle_move_up_files(Editor* e){
  if(e->conf.currentFileIndex > 0){
    e->conf.currentFileIndex--;
    if(e->conf.currentFileIndex - e->conf.displayedFilesStart < 0) {
      e->conf.displayedFilesStart--;
    }
  }
  else {
    e->conf.currentFileIndex = e->conf.filesCount - 1;
    e->conf.displayedFilesStart = e->conf.filesCount - MAX_DISPLAYED_FILES ;
  }
}

void handle_backspace(Editor* e) {
  if(e->mode == INSERT) {
    if(e->conf.isTakingNote) remove_char(e);
    else if(e->conf.isInsertingTitle) pop_char_from_line(e,e->note->title);
  } 
}

void handle_normal_mode_keys(Editor* e, int c){
  switch(c){
    case 'G':
      move_index_to_last_line(e);
      break;
    case 't':
      e->mode = INSERT;
      e->conf.isInsertingTitle = true;
      e->conf.isTakingNote = false;
      e->cursor.index = strlen(e->note->title);
      update_cursor_position(e);
      break;
    case '?':
      e->conf.isDebugging = !e->conf.isDebugging;
      break;
    case 'n':
      e->mode = INSERT; 
      e->conf.isNamingFile = true;
      e->conf.isMenuOpen = false;
      break;
    case 'a':
      e->mode = INSERT;
      if(e->cursor.col < 61 && get_line_length(e,-1)) move_index_right(e);
      e->conf.isTakingNote = true;
      break;
    case 's':
      if(!strlen(e->currentFileName)){
        e->conf.isNamingFile = true;
        e->mode = INSERT;
      } else writeFile(e);
      break;
    case 'i':
      e->mode = INSERT;
      break;
    case 'm':
      load_menu_icons(e->media.menu_icons);
      e->conf.isMenuOpen = true;
      e->conf.isChoosingDir = false;
      e->conf.isNamingFile = false;
      e->conf.isOpeningFile = false;
      break;
    case '$':
      move_to_end_of_line(e);
      break;
    case '0':
      move_to_beginning_of_line(e);
      break;
    case 'x':
      if(e->conf.isTakingNote){
        remove_current_char(e,e->note->body);
      }
      break;
    case 'b':
      if(e->conf.isTakingNote){
        move_to_word_beginning(e,e->note->body);
      }
      break;
    case 'e':
      if(e->conf.isTakingNote){
        move_to_word_ending(e,e->note->body);
      }
      break;
    case 'o':
      if(e->conf.isMenuOpen){
        e->conf.isOpeningFile = true;
        e->conf.isMenuOpen = false;
      }
      else {
        if(e->conf.isTakingNote){
          if(e->note->linesNum < LINES_COUNT - 1){
            move_to_new_line(e);
            e->mode = INSERT;
          }
        }
      }
      break;
    case '/':
      e->conf.isSearching = true;
      /* find(e->fileNames, e->filesCount,
          e->searchQuery, e->resultNames,
          &e->numResults); */
      break;
    case 'h':
      if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {}
        else {
          if(e->cursor.col > 0){
            move_index_left(e);
            update_last_col(e);
          } 
        }
      }
      break;
    case 'l':
      if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {}
        else {
          move_index_right(e);
        } 
      }
      break;
    case 'j':
      if(e->conf.isOpeningFile) handle_move_down_files(e);
      else if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {
          addChar(e,'\n');
          e->conf.isInsertingTitle = false;
        }else {
          move_index_down(e);
        }
      }
      break;
    case 'k':
      if(e->conf.isOpeningFile) handle_move_up_files(e);
      else if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {}
        else {
            move_index_up(e);
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
  if (e->conf.isOpeningFile) {
    char path[100];
    sprintf(path,
      "%s/.local/notes/%s",
      e->HOME_DIR,
      e->fileNames[e->conf.currentFileIndex]);
    for(int i =0 ; i < e->conf.filesCount -1 ; i++){
      printf("[%d] %s\n",i,e->fileNames[i]);
    }
    printf("Opening [%d] %s\n",e->conf.currentFileIndex,e->fileNames[e->conf.currentFileIndex]);
    if(e->conf.isNoteBookMode) readNote(e,path);
    else readFile(e, path);
    e->currentFileName = e->fileNames[e->conf.currentFileIndex];
    e->conf.isOpeningFile = false;
    e->conf.isTakingNote = true;
  } 
  if(e->mode == INSERT){
  if (e->conf.isInsertingTitle ) {
    e->conf.isInsertingTitle = false;
    e->conf.isTakingNote = true;
  } else {
      add_char_to_note_body(e,'\n');
      e->note->displayStart = get_first_diplayed_index(e,false);
      update_cursor_position(e);
    }
  }
}

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

// enter 
  if (IsKeyPressed(KEY_ENTER)) handle_enter(e);

//escape & capslock
  if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_CAPS_LOCK)) {
    handle_caps_lock_and_escape(e);
  } 

}

int get_first_diplayed_index(Editor* e,bool isUp){
  int line_index = get_line_index(e,-1);
  if(isUp && e->cursor.row > 0) return e->note->displayStart;
  if(isUp) {
    int i,j;
    for(j = 0,i = e->cursor.index ; i > -2 && j < 1 ;i--){
      if(i < 0) continue;
      if(e->note->body[i] == '\n') j++; 
    }
    return i+2;
  }
  int num_hidden_lines = line_index + 2 - LINES_COUNT;
  int i,j;
  for(i=0,j=0; j < num_hidden_lines; i++){
    if(e->note->body[i] == '\n') j++; 
  }
  return i;
}

void add_char_to_note_body(Editor* e,char c) {
  // if(e->note->length < LINE_LENGTH * LINES_COUNT) {
    if(e->cursor.col < LINE_LENGTH) {
      int line_len = get_line_length(e,-1); 
      if(line_len > 61){
        int last_pos = e->cursor.index + 62 - e->cursor.index;
      }
    } else {
   } 
    if(e->note->body[e->cursor.index] == '\0'){ 
      e->note->body[e->cursor.index] = c;
      move_index_right(e);
      e->note->length++;
    } else {
      int i;
      for(i = e->note->length; i > e->cursor.index;i--){
        e->note->body[i] = e->note->body[i-1];
      }
      e->note->body[i] = c;
      move_index_right(e);
      e->note->length++;
    }
  // }
  update_cursor_position(e);
  update_last_col(e);
  e->note->body[e->note->length] = '\0';
}

void addChar(Editor* e,char c){
  if(e->conf.isTakingNote) {
    if(e->note->length > e->note->size - 2){
      e->note->body = realloc(e->note->body,e->note->length + LINE_LENGTH);
      e->note->size += LINE_LENGTH;
    }
    add_char_to_note_body(e,c);
  } else if(e->conf.isInsertingTitle){
    add_char_to_line(e,e->note->title,TITLE_SIZE,c);
  }
}

void getLocalDate(Date* d) {
  time_t t = time(NULL);
  struct tm *locaTime = localtime(&t);
  strftime(d->year, 5, "%Y", locaTime);
  strftime(d->month, 3, "%m", locaTime);
  strftime(d->day, 3, "%d", locaTime);
}

