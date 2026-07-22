#include <ctype.h>
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "buffer.h"

double longPressTime = 0.0f;

size_t get_max_line_length(Editor *e){ 
  size_t x_padding = e->conf.padding.left + e->conf.padding.right;
  return (e->s_width - x_padding) / e->conf.letter_spacing; 
}

size_t get_max_num_lines(Editor *e){ 
  size_t y_padding = e->conf.padding.top + e->conf.padding.bottom;
  return (e->s_height - y_padding) / e->conf.line_height + 2; 
}

void update_scroll(Editor *e, bool is_up){
  size_t wraps = get_lines_wraps(e, e->buffer.d_start, e->buffer.d_start + e->buffer.d_length);
  size_t max = get_max_num_lines(e) - wraps;
  size_t current_index = e->buffer.current_line_index;
  e->buffer.d_length = e->buffer.length < max ? e->buffer.length : max;

  if(is_up){
    if(current_index < e->buffer.d_start && e->buffer.d_start) e->buffer.d_start--;
  }
  else {
    if(current_index > e->buffer.d_start + e->buffer.d_length - 1) {
      e->buffer.d_start = current_index - max + 1;
    }
  } 
}

void handle_append(Editor *e){
  e->mode = INSERT;
  if(e->cursor.index > 0) move_cursor_right(e);
}

void start_new_line(Editor *e){
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  if(e->buffer.length > e->buffer.capacity - 1){
    size_t new_capacity = e->buffer.capacity + 10;
    e->buffer.lines = realloc(e->buffer.lines, sizeof(Line*) * new_capacity);
    for(size_t i = e->buffer.capacity; i < new_capacity; i++){
      e->buffer.lines[i] = new_line(DEFAULT_LINE_SIZE);
    }
    e->buffer.capacity = new_capacity;
  } 
  if(e->buffer.current_line_index < e->buffer.length - 1){
    for(size_t index = e->buffer.length; index > e->buffer.current_line_index; index--){
      e->buffer.lines[index] = e->buffer.lines[index - 1];
    }
    e->buffer.lines[e->buffer.current_line_index + 1] = new_line(DEFAULT_LINE_SIZE);
  }
  size_t prev = e->buffer.current_line_index;
  if(e->buffer.lines[prev]->length) {
    for(size_t i = e->cursor.index; i < e->buffer.lines[prev]->length; i++){
      add_char_to_line(e, e->buffer.lines[prev + 1], e->buffer.lines[prev]->chars[i], true);
    }
    e->buffer.lines[prev]->length = e->cursor.index;
  }  
  e->cursor.index = 0;
  e->buffer.length++;
  e->buffer.current_line_index++;
  e->buffer.num_chars++;
}

void move_cursor_to_last_line(Editor* e){
  // todo
  int i,j;
  for(j=0, i=e->buffer.length -1; i>0 && j < 1;i--){
    // if(e->note->body[i] == '\n') j++;
  }
  // e->cursor.index = i+2;
  // e->note->displayStart = get_first_diplayed_index(e,false);
}

void add_char_to_line(Editor* e, Line* line, char c, bool append){
  if(line->length >= line->capacity - 1) {
    line->capacity *= 2;
    line->chars = realloc(line->chars, sizeof(char) * line->capacity);
  } 
  if(e->cursor.index < line->length && !append){
    for(size_t i = line->length; i > e->cursor.index; i--) {
      line->chars[i] = line->chars[i - 1];
    }
    line->chars[e->cursor.index] = c;
  } else {
    line->chars[line->length] = c;
  }
  line->length++;
  if(!append){
    e->cursor.index++;
    e->buffer.num_chars++;
  }
  line->chars[line->length] = '\0';
  update_scroll(e, false);
}

void pop_char_from_line(Editor* e, Line *line){
  if(!e->buffer.num_chars) return;
  if(e->buffer.current_line_index && e->cursor.index == 0) {
    Line *prev_line = e->buffer.lines[e->buffer.current_line_index - 1];
    e->cursor.index = prev_line->length;
    for(size_t i = 0; i < line->length; i++){
      add_char_to_line(e, prev_line, line->chars[i], true);
    }
    for(size_t i = e->buffer.current_line_index; i < e->buffer.capacity - 1; i++)
    {
      e->buffer.lines[i] = e->buffer.lines[i + 1];
    }
    e->buffer.lines[e->buffer.capacity - 1] = NULL;
    free_line(line);
    e->buffer.length--;
    e->buffer.capacity--;
    e->buffer.current_line_index--;
  } else if(e->cursor.index != 0){
    for(size_t i = e->cursor.index; i < line->length; i++){
      line->chars[i - 1] = line->chars[i];
    }
    line->chars[--line->length] = '\0';
    e->cursor.index--;
  }
  e->buffer.num_chars--;
  update_scroll(e, true);
}

void update_cursor_position(Editor* e){
  /* size_t max_line_len = get_max_line_length(e);
  size_t max_num_lines = get_max_num_lines(e);
  Line *current_line = e->buffer.lines[e->buffer.current_line_index];
  size_t prev_wraps = get_lines_wraps(e, 0, e->buffer.current_line_index);
  e->cursor.col = current_line->length % max_line_len;
  e->cursor.row = e->buffer.current_line_index + prev_wraps;
  
  if(e->cursor.index != current_line->length){
    size_t row_offset = current_line->length / max_line_len;
    row_offset -= e->cursor.index / max_line_len;
    e->cursor.col = e->cursor.index < max_line_len ? e->cursor.index : e->cursor.index % max_line_len;
    e->cursor.row -= row_offset;
  } */
} 

size_t get_lines_wraps(Editor *e, size_t from, size_t to){
  if(from > to) return 0;
  if(to >= e->buffer.capacity) to = e->buffer.capacity - 1;
  size_t wraps = 0;
  size_t max = get_max_line_length(e);
  for(size_t i = from; i <= to; i++){
    wraps += e->buffer.lines[i]->length / max; 
  }
  return wraps;
}
  
void update_last_col(Editor* e){ e->cursor.last_col = e->cursor.col; }

void move_cursor_left(Editor* e) {
  if(e->cursor.index) {
    e->cursor.index--;
    // todo
    // update_last_col(e);
  }
}

void handle_caps_lock_and_escape(Editor* e){
  e->conf.isMenuOpen = false;
  e->conf.isNamingFile = false;
  e->conf.isOpeningFile = false;
  e->conf.isSearching = false;
  e->searchQuery[0] = '\0';
  if(e->mode == INSERT){
    e->mode = NORMAL;
    if(e->cursor.index) move_cursor_left(e);
  }
}

void move_cursor_right(Editor* e) {
  Line *current_line = e->buffer.lines[e->buffer.current_line_index];
  if(e->cursor.index < current_line->length + (e->mode == INSERT ? 1 : -1)){
    e->cursor.index++;
    // todo 
    // update_last_col(e);
  }
}

void move_cursor_down(Editor* e){
  if(e->buffer.current_line_index >= e->buffer.length - 1) return;
  e->buffer.current_line_index++;
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  if(current->length < e->cursor.index){
    e->cursor.index = current->length ? current->length - 1 : 0;
  }
  update_scroll(e, false);
}

void move_cursor_up(Editor* e){
  if(!e->buffer.current_line_index) return;
  e->buffer.current_line_index--;
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  if(current->length < e->cursor.index){
    e->cursor.index = current->length ? current->length - 1 : 0;
  }
  update_scroll(e, true);
}

void move_to_beginning_of_line(Editor* e) {
  if(!e->cursor.index) return;
  e->cursor.index = 0;
}

void move_to_end_of_line(Editor* e) {
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  if(e->cursor.index == current->length - 1) return;
  e->cursor.index += current->length - e->cursor.index - 1;
}

void move_to_new_line(Editor* e){
  // todo
  // e->note->body[e->cursor.index] = '\n';
  // e->cursor.index++;
  // e->note->length++;
}

void move_to_word_ending(Editor* e){
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  bool next_exists = e->buffer.current_line_index < e->buffer.length - 1;
  if(next_exists && (!current->length || e->cursor.index == current->length - 1)){
    e->buffer.current_line_index++;
    current = e->buffer.lines[e->buffer.current_line_index];
    e->cursor.index = 0;
    if(!current->length) return move_to_word_ending(e);
  }
  size_t i = e->cursor.index; 
  while(isspace(current->chars[i + 1])){
    move_cursor_right(e);
    i++;
  }
  if(!isalnum(current->chars[i + 1]) && i < current->length - 1){
    move_cursor_right(e);
    i++;
  }
  while(i < current->length && (isalnum(current->chars[i + 1]) || current->chars[i + 1] == '_')) {
    move_cursor_right(e);
    i++;
  }
  e->cursor.index = i;
}

void move_to_word_beginning(Editor* e){
  Line *current = e->buffer.lines[e->buffer.current_line_index];
  if(e->cursor.index == 0 && e->buffer.current_line_index) {
    e->buffer.current_line_index--;
    current = e->buffer.lines[e->buffer.current_line_index];
    e->cursor.index = current->length ? current->length - 1 : 0;
  } 
  size_t i = e->cursor.index; 
  while(isspace(current->chars[i - 1])){
    move_cursor_left(e);
    i--;
  }
  if(!isalnum(current->chars[i - 1]) && e->cursor.index > 1) {
    move_cursor_left(e);
    i--;
  }
  while(i > 0 && (isalnum(current->chars[i - 1]) || current->chars[i - 1] == '_')) {
    move_cursor_left(e);
    i--;
  }
  e->cursor.index = i;
}

void remove_current_char(Editor* e){
  Line* current = e->buffer.lines[e->buffer.current_line_index];
  if(!current->length) return;
  if(e->cursor.index < current->length - 1){
    for(int i = e->cursor.index; i <= current->length; i++) {
      current->chars[i] = current->chars[i + 1];
    }
  } else move_cursor_left(e);
  current->length--;
}

void handle_tab(Editor* e) {
  // todo
  /* if(e->mode == INSERT) {
    if(e->conf.isTakingNote ){
      for(int i = 0;i < TAB_SIZE; ++i) addChar(e,' ');
    }
  } */
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
    pop_char_from_line(e, e->buffer.lines[e->buffer.current_line_index]);
  } else {
    move_cursor_left(e);
  } 
}

void handle_normal_mode_keys(Editor* e, int c){
  switch(c){
    case 'G':
      move_cursor_to_last_line(e);
      break;
    case '?':
      e->conf.isDebugging = !e->conf.isDebugging;
      break;
    /* case 'n':
      e->mode = INSERT; 
      e->conf.isNamingFile = true;
      e->conf.isMenuOpen = false;
      break; */
    case 'a':
      handle_append(e);
      break;
    case 's':
      // if(!strlen(e->currentFileName)){
        // e->conf.isNamingFile = true;
        // e->mode = INSERT;
      // } else writeFile(e);
      break;
    case 'i':
      e->mode = INSERT;
      break;
    case 'm':
      // load_menu_icons(e->media.menu_icons);
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
      remove_current_char(e);
      break;
    case 'b':
      move_to_word_beginning(e);
      break;
    case 'e':
      move_to_word_ending(e);
      break;
    case 'o':
      if(e->conf.isMenuOpen){
        e->conf.isOpeningFile = true;
        e->conf.isMenuOpen = false;
      }
      else {
        // todo
        /* if(e->conf.isTakingNote){
          if(e->note->linesNum < LINES_COUNT - 1){
            move_to_new_line(e);
            e->mode = INSERT;
          }
        } */
      }
      break;
    case '/':
      e->conf.isSearching = true;
      // todo
      /* e->conf.isInsertingTitle = false;
      e->conf.isTakingNote = false; */
      e->mode = INSERT;
      break;
    case 'h':
      if(e->mode == NORMAL){
        move_cursor_left(e);
      }
      // todo
      /* if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {}
        else {
          if(e->cursor.col > 0){
            move_cursor_left(e);
            update_last_col(e);
          } 
        }
      } */
      break;
    case 'l':
      if(e->mode == NORMAL){
        move_cursor_right(e);
      }
      // todo
      /* if(e->conf.isTakingNote) {
        if(e->conf.isInsertingTitle) {}
        else {
          move_cursor_right(e);
        } 
      } */
      break;
    case 'j':
      if(e->conf.isOpeningFile) handle_move_down_files(e);
      else move_cursor_down(e);
      break;
    case 'k':
      if(e->conf.isOpeningFile) handle_move_up_files(e);
      else move_cursor_up(e);
      break;
  }
}

void handle_insert_mode_keys(Editor* e,int c){
  if (c >= 32) {
    add_char_to_line(e, e->buffer.lines[e->buffer.current_line_index], c, false);
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
    // todo
    // if(e->conf.isNoteBookMode) readNote(e,path);
    // else readFile(e, path);
    e->currentFileName = e->fileNames[e->conf.currentFileIndex];
    e->conf.isOpeningFile = false;
    // todo
    // e->conf.isTakingNote = true;
  } 
  if(e->mode == INSERT){
    start_new_line(e);
    update_scroll(e, false);
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
  else if (IsKeyDown(KEY_BACKSPACE)) {
    double now = GetTime();
    if(now - longPressTime > LONG_PRESS_DELAY){
      handle_backspace(e);
      longPressTime = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyReleased(KEY_BACKSPACE)) longPressTime = 0;

  else if(IsKeyPressed(KEY_TAB)) handle_tab(e);

// enter 
  else if (IsKeyPressed(KEY_ENTER)) handle_enter(e);

//escape & capslock
  else if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_CAPS_LOCK)) {
    handle_caps_lock_and_escape(e);
  } 
  else if (IsKeyPressed(KEY_UP)) move_cursor_up(e);
  else if (IsKeyPressed(KEY_DOWN)) move_cursor_down(e);
  else if (IsKeyPressed(KEY_LEFT)) move_cursor_left(e);
  else if (IsKeyPressed(KEY_RIGHT)) move_cursor_right(e);
}

// int get_first_diplayed_index(Editor* e,bool isUp){
  // int line_index = get_line_index(e,-1);
  // todo
  // if(isUp && e->cursor.row > 0) return e->note->displayStart;
  // if(isUp) {
    // int i,j;
    // for(j = 0,i = e->cursor.index ; i > -2 && j < 1 ;i--){
      // if(i < 0) continue;
      // todo
      // if(e->note->body[i] == '\n') j++; 
    // }
    // return i+2
  // }
  // int num_hidden_lines = line_index + 2 - LINES_COUNT;
  int i,j;
  // for(i=0,j=0; j < num_hidden_lines; i++){
    // todo
    // if(e->note->body[i] == '\n') j++; 
  // }
  // return i;
// }

Line *new_line(size_t capacity){
  Line *line = malloc(sizeof(Line));
  line->chars = malloc(sizeof(char) * capacity);
  line->capacity = capacity;
  line->length = 0;
  return line;
}

void free_line(Line *line){
  free(line->chars);
  free(line);
  line = NULL;
}

Buffer *new_buffer(size_t capacity){
  Buffer *buff = malloc(sizeof(Buffer));
  buff->lines = malloc(sizeof(Line*) * capacity);
  for(size_t i = 0; i < capacity; i++){
    buff->lines[i] = new_line(DEFAULT_LINE_SIZE);
  }
  buff->capacity = capacity;
  buff->length = 1;
  buff->d_length = 1;
  buff->num_chars = 0;
  buff->current_line_index = 0;
  return buff;
}

