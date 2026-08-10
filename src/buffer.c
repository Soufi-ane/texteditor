#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "files.h"
#include "draw.h"
#include "tinyfiledialogs.h"

#define HOLD_PRESS_DELAY 0.02f

double long_press_time = 0.0f;
double last_press_time = 0.0f;
int is_g_clicked_before = false;

Cmd default_cmds[NUM_COMMANDS] = {
  { NEW_FILE , "New file" },
  { OPEN_FILE , "Open file" },
  { HELP , "Help!" },
  { OPEN_CONFIG , "Open config" },
  { OPEN_MESSAGES , "Open log messages" },
};

ssize_t get_max_line_length(Editor *e){ 
  ssize_t char_width = get_char_size(e->conf.font_data.size).col;
  ssize_t x_padding = e->conf.padding.left + e->conf.padding.right;
  if(e->conf.ln_mode != NONE) x_padding += e->conf.ln_padding * (e->conf.letter_spacing + char_width);
  return (e->s_width - x_padding) / (e->conf.letter_spacing + char_width); 
}

ssize_t get_max_num_lines(Editor *e){ 
  ssize_t char_height = get_char_size(e->conf.font_data.size).row;
  ssize_t y_padding = e->conf.padding.top + e->conf.padding.bottom;
  return (e->s_height - y_padding) / (e->conf.line_height + char_height); 
}

bool is_selecting_up(Editor *e){
  return e->buffers[e->current_buff]->current_line_index <= e->conf.selection_start.row;
}

void filter_cmds_by_prompt(Editor *e){
  int index = 0;
  e->selected_cmd = 0;
  for(ssize_t i = 0; i < NUM_COMMANDS; i++){
    if(!e->cmd_prompt->length){
      e->displayed_cmds[index++] = i;
      continue;
    }
    bool is_match = str_includes(default_cmds[i].text, e->cmd_prompt->chars, e->cmd_prompt->length);
    if(is_match){
      e->displayed_cmds[index++] = i;
    }
  }
  e->num_cmds_displayed = index;
}

bool is_selected(Editor *e, RowCol row_col){
  if(!e->conf.is_selecting) return false;
  Buffer *buff = e->buffers[e->current_buff];
  bool is_at_start = row_col.row == e->conf.selection_start.row;
  bool is_at_current = row_col.row == buff->current_line_index;
  bool is_left = buff->cursor.index <= e->conf.selection_start.col;

  if(is_at_start && is_at_current){
    return is_left ? 
      row_col.col <= e->conf.selection_start.col && row_col.col >= buff->cursor.index
      : row_col.col >= e->conf.selection_start.col && row_col.col <= buff->cursor.index;
  }
  if(
    row_col.row >= e->conf.selection_start.row &&
    row_col.row <= buff->current_line_index
  ){
    if(is_at_start) return row_col.col >= e->conf.selection_start.col;
    if(is_at_current) return row_col.col <= buff->cursor.index;
    return true;
  }
  if(
    row_col.row >= buff->current_line_index &&
    row_col.row <= e->conf.selection_start.row
  ){
    if(is_at_start) return row_col.col <= e->conf.selection_start.col;
    if(is_at_current) return row_col.col >= buff->cursor.index;
    return true;
  }
  return false;
}

void update_scroll(Editor *e, bool is_up){
  ssize_t wraps = get_lines_wraps(e, e->buffers[e->current_buff]->d_start, e->buffers[e->current_buff]->d_start + e->buffers[e->current_buff]->d_length, true);
  ssize_t max = get_max_num_lines(e) - wraps;
  ssize_t current_index = e->buffers[e->current_buff]->current_line_index;
  e->buffers[e->current_buff]->d_length = e->buffers[e->current_buff]->length < max ? e->buffers[e->current_buff]->length : max;

  if(is_up){
    if(current_index < e->buffers[e->current_buff]->d_start && e->buffers[e->current_buff]->d_start) e->buffers[e->current_buff]->d_start--;
  }
  else {
    if(current_index > e->buffers[e->current_buff]->d_start + e->buffers[e->current_buff]->d_length - 1) {
      e->buffers[e->current_buff]->d_start = current_index - max + 1;
    }
  } 
}

int get_digit_count(int number){
  int count = 1;
  for(int i = number; i > 0; i /= 10) count++;
  return count > 1 ? (count - 1) : count;
}

void update_line_number_padding(Editor *e){
  e->conf.ln_padding = get_digit_count(e->buffers[e->current_buff]->length);
}

void handle_append(Editor *e){
  e->mode = INSERT;
  e->conf.is_selecting = false;
  if(e->buffers[e->current_buff]->cursor.index > 0) move_cursor_right(e);
}

void toggle_full_screen(Editor *e){
  int monitor = GetCurrentMonitor();
  int m_width  = GetMonitorWidth(monitor);
  int m_height = GetMonitorHeight(monitor);
  if(e->is_full_screen) {
    e->s_width = SCREEN_WIDTH;
    e->s_height = SCREEN_HEIGHT;
    SetWindowPosition(
      m_width / 2 - SCREEN_WIDTH / 2,
      m_height / 2 - SCREEN_HEIGHT / 2
    );
  }
  else {
    e->s_width = m_width;
    e->s_height = m_height;
    SetWindowPosition(0, 0);
  }
  SetWindowSize(e->s_width, e->s_height);
  e->is_full_screen = !e->is_full_screen;
}

void realloc_buffer(Buffer *buff, ssize_t new_capacity){
  buff->lines = realloc(buff->lines, sizeof(Line*) * new_capacity);
  for(ssize_t i = buff->capacity; i < new_capacity; i++){
    buff->lines[i] = new_line(DEFAULT_LINE_SIZE);
  }
  buff->capacity = new_capacity;
}

void start_new_line(Editor *e){
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  if(e->buffers[e->current_buff]->length > e->buffers[e->current_buff]->capacity - 2){
    ssize_t new_capacity = e->buffers[e->current_buff]->capacity + 10;
    realloc_buffer(e->buffers[e->current_buff], new_capacity);
  } 
  if(e->buffers[e->current_buff]->current_line_index < e->buffers[e->current_buff]->length - 1){
    for(ssize_t index = e->buffers[e->current_buff]->length; index > e->buffers[e->current_buff]->current_line_index; index--){
      e->buffers[e->current_buff]->lines[index] = e->buffers[e->current_buff]->lines[index - 1];
    }
    e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index + 1] = new_line(DEFAULT_LINE_SIZE);
  }
  ssize_t prev = e->buffers[e->current_buff]->current_line_index;
  if(e->buffers[e->current_buff]->lines[prev]->length) {
    for(ssize_t i = e->buffers[e->current_buff]->cursor.index; i < e->buffers[e->current_buff]->lines[prev]->length; i++){
      add_char_to_line(e, e->buffers[e->current_buff]->lines[prev + 1], e->buffers[e->current_buff]->lines[prev]->chars[i], true);
    }
    e->buffers[e->current_buff]->lines[prev]->length = e->buffers[e->current_buff]->cursor.index;
  }  
  e->buffers[e->current_buff]->cursor.index = 0;
  e->buffers[e->current_buff]->length++;
  e->buffers[e->current_buff]->current_line_index++;
  e->buffers[e->current_buff]->num_chars++;
  e->buffers[e->current_buff]->is_saved = false;
}

void add_char_to_line(Editor* e, Line* line, char c, bool append){
  if(line->length >= line->capacity - 1) {
    line->capacity *= 2;
    line->chars = realloc(line->chars, sizeof(char) * line->capacity);
  } 
  if(e->buffers[e->current_buff]->cursor.index < line->length && !append){
    for(ssize_t i = line->length; i > e->buffers[e->current_buff]->cursor.index; i--) {
      line->chars[i] = line->chars[i - 1];
    }
    line->chars[e->buffers[e->current_buff]->cursor.index] = c;
  } else {
    line->chars[line->length] = c;
  }
  line->length++;
  if(!append){
    e->buffers[e->current_buff]->cursor.index++;
    e->buffers[e->current_buff]->num_chars++;
  }
  line->chars[line->length] = '\0';
  update_scroll(e, false);
  update_line_number_padding(e);
  e->buffers[e->current_buff]->current_msg_index = - 1;
  if(!e->conf.is_menu_open) e->buffers[e->current_buff]->is_saved = false;
}

void pop_char_single_line(Line *line){
  if(!line->length) return;
  line->chars[line->length - 1] = '\0';
  line->length--;
}

void remove_char_from_line(Editor* e, Line *line){
  bool is_removed = false;
  Buffer *current_buffer = e->buffers[e->current_buff];
  if(!current_buffer->num_chars) return;
  if(!current_buffer->cursor.index) {
    if(current_buffer->current_line_index){
      Line *prev_line = current_buffer->lines[current_buffer->current_line_index - 1];
      current_buffer->cursor.index = prev_line->length;
      while(prev_line->capacity - prev_line->length < line->length){
        realloc_line(prev_line, prev_line->capacity * 2);
      }
      memcpy(&prev_line->chars[prev_line->length], line->chars, (size_t) line->length);
      current_buffer->cursor.index = prev_line->length;
      prev_line->length += line->length;

      delete_lines(current_buffer, current_buffer->current_line_index, 1);
      current_buffer->current_line_index--;
      is_removed = true;
    }  
  } 
  else if(current_buffer->cursor.index < line->length){
    memmove(
      &line->chars[current_buffer->cursor.index - 1],
      &line->chars[current_buffer->cursor.index],
      line->length - current_buffer->cursor.index
    );
    line->length--;
    current_buffer->cursor.index--;
    is_removed = true;
  }
  else {
    line->chars[--line->length] = '\0';
    current_buffer->cursor.index--;
    is_removed = true;
  }
  if(is_removed) current_buffer->num_chars--;
  update_scroll(e, true);
  update_line_number_padding(e);
  current_buffer->is_saved = false;
}

ssize_t get_lines_wraps(Editor *e, int from, int to, bool include_last){
  if(from > to) return 0;
  if(from < 0) from = 0;
  if(to >= e->buffers[e->current_buff]->length) to = e->buffers[e->current_buff]->length - 1;
  ssize_t wraps = 0;
  ssize_t max = get_max_line_length(e);
  for(ssize_t i = from; (include_last ? (i <= to) : (i < to)); i++){
    wraps += e->buffers[e->current_buff]->lines[i]->length / max; 
  }
  return wraps;
}
  
void update_last_index(Editor* e){ e->buffers[e->current_buff]->cursor.last_index = e->buffers[e->current_buff]->cursor.index; }

void move_cursor_left(Editor* e) {
  ssize_t *current_index = &e->buffers[e->current_buff]->current_line_index;
  if(e->buffers[e->current_buff]->cursor.index) {
    e->buffers[e->current_buff]->cursor.index--;
    update_last_index(e);
  }else {
    if(*current_index) {
      (*current_index)--;
      e->buffers[e->current_buff]->cursor.index = e->buffers[e->current_buff]->lines[*current_index]->length - 1;
    } 
  }
  e->buffers[e->current_buff]->current_msg_index = -1;

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void handle_caps_lock_and_escape(Editor* e){
  if(e->conf.is_vim_mode){
    if(e->conf.is_selecting) e->conf.is_selecting = false;
    else if(e->mode == INSERT){
      e->mode = NORMAL;
      if(e->buffers[e->current_buff]->cursor.index) move_cursor_left(e);
      e->conf.is_menu_open = false;
      e->cmd_prompt->length = 0;
      filter_cmds_by_prompt(e);
      e->conf.is_opening_file = false;
      e->conf.is_selecting = false;
    }
  }else {
    e->conf.is_menu_open = !e->conf.is_menu_open;
    e->cmd_prompt->length = 0;
    filter_cmds_by_prompt(e);
  }
}

void move_cursor_right(Editor* e) {
  ssize_t *current_index = &e->buffers[e->current_buff]->current_line_index;
  Line *current_line = e->buffers[e->current_buff]->lines[*current_index];
  if(e->buffers[e->current_buff]->cursor.index < current_line->length -
    (!e->conf.is_selecting && (e->mode == INSERT || !e->conf.is_vim_mode) ? 0 : 1)){
    e->buffers[e->current_buff]->cursor.index++;
    update_last_index(e);
  }else {
    if(*current_index < e->buffers[e->current_buff]->length -1 /*todo /*/) {
      (*current_index)++;
      e->buffers[e->current_buff]->cursor.index = 0;
    } 
  }
  e->buffers[e->current_buff]->current_msg_index = -1;

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void move_cursor_down(Editor* e){
  if(e->buffers[e->current_buff]->current_line_index >= e->buffers[e->current_buff]->length - 1) return;
  e->buffers[e->current_buff]->current_line_index++;
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];

  if(!current->length) e->buffers[e->current_buff]->cursor.index = 0;

  else if(current->length - 1 < e->buffers[e->current_buff]->cursor.last_index){
    e->buffers[e->current_buff]->cursor.index = current->length ? current->length - 1 : 0;
  } else e->buffers[e->current_buff]->cursor.index = e->buffers[e->current_buff]->cursor.last_index;

  update_scroll(e, false);
  update_line_number_padding(e);
  e->buffers[e->current_buff]->current_msg_index = -1;

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void adapte_index_to_current_line(Editor *e){
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  if(!current->length) e->buffers[e->current_buff]->cursor.index = 0;

  else if(current->length - 1 < e->buffers[e->current_buff]->cursor.last_index){
    e->buffers[e->current_buff]->cursor.index = current->length ? current->length - 1 : 0;
  } else e->buffers[e->current_buff]->cursor.index = e->buffers[e->current_buff]->cursor.last_index;
}

void move_cursor_up(Editor* e){
  if(!e->buffers[e->current_buff]->current_line_index) return;
  e->buffers[e->current_buff]->current_line_index--;
  adapte_index_to_current_line(e);
  update_scroll(e, true);
  update_line_number_padding(e);
  e->buffers[e->current_buff]->current_msg_index = -1;

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void move_to_beginning_of_line(Editor* e) {
  if(!e->buffers[e->current_buff]->cursor.index) return;
  e->buffers[e->current_buff]->cursor.index = 0;
  update_last_index(e);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void move_to_end_of_line(Editor* e) {
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  if(e->buffers[e->current_buff]->cursor.index == current->length - 1) return;
  e->buffers[e->current_buff]->cursor.index += current->length - e->buffers[e->current_buff]->cursor.index - 1;
  update_last_index(e);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void move_to_new_line(Editor* e){
  // todo
  // e->note->body[e->buffers[e->current_buff]->cursor.index] = '\n';
  // e->buffers[e->current_buff]->cursor.index++;
  // e->note->length++;
}

void move_to_word_ending(Editor* e){
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  bool next_exists = e->buffers[e->current_buff]->current_line_index < e->buffers[e->current_buff]->length - 1;
  if(next_exists && (!current->length || e->buffers[e->current_buff]->cursor.index == current->length - 1)){
    e->buffers[e->current_buff]->current_line_index++;
    current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
    e->buffers[e->current_buff]->cursor.index = 0;
    if(!current->length) {
      move_to_word_ending(e);
      return;
    }
  }
  ssize_t i = e->buffers[e->current_buff]->cursor.index; 
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
  e->buffers[e->current_buff]->cursor.index = i;
  update_last_index(e);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void move_to_word_beginning(Editor* e){
  Line *current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  if(e->buffers[e->current_buff]->cursor.index == 0 && e->buffers[e->current_buff]->current_line_index) {
    e->buffers[e->current_buff]->current_line_index--;
    current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
    e->buffers[e->current_buff]->cursor.index = current->length ? current->length - 1 : 0;
  } 
  ssize_t i = e->buffers[e->current_buff]->cursor.index; 
  while(isspace(current->chars[i - 1])){
    move_cursor_left(e);
    i--;
  }
  if(!isalnum(current->chars[i - 1]) && e->buffers[e->current_buff]->cursor.index > 1) {
    move_cursor_left(e);
    i--;
  }
  while(i > 0 && (isalnum(current->chars[i - 1]) || current->chars[i - 1] == '_')) {
    move_cursor_left(e);
    i--;
  }
  e->buffers[e->current_buff]->cursor.index = i;
  update_last_index(e);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

void remove_current_char(Editor* e){
  Line* current = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  if(!current->length) return;
  if(e->buffers[e->current_buff]->cursor.index < current->length - 1){
    for(int i = e->buffers[e->current_buff]->cursor.index; i <= current->length; i++) {
      current->chars[i] = current->chars[i + 1];
    }
  } else move_cursor_left(e);
  current->length--;
  e->buffers[e->current_buff]->is_saved = false;
}

void go_to_next_buffer(Editor *e){
  if(e->length < 2) return;
  if(e->current_buff >= e->length - 1) e->current_buff = 0;
  else e->current_buff++;
  e->conf.is_selecting = false;
  update_line_number_padding(e);
}

void go_to_prev_buffer(Editor *e){
  if(e->length < 2) return;
  if(e->current_buff < 1) e->current_buff = e->length - 1;
  else e->current_buff--;
  e->conf.is_selecting = false;
  update_line_number_padding(e);
}

void handle_tab(Editor* e, bool is_shift_down) {
  if(!e->conf.is_menu_open){
    if(e->mode == INSERT){
      if(e->conf.is_spaces_for_tabs) {
        for(int i = 0; i < e->conf.tab_size; ++i)  {
          add_char_to_line(
            e, e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index],
            ' ', false
          );
        }
      }
      else {
        add_char_to_line(
          e, e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index],
          '\t', false
        );
      }
    }else if(e->conf.is_vim_mode){
      if(is_shift_down) go_to_prev_buffer(e);
      else go_to_next_buffer(e);
    }
  }else {
    if(is_shift_down){
      if(e->selected_cmd > 0) e->selected_cmd--;
      else e->selected_cmd = e->num_cmds_displayed - 1;
    } else {
      if(e->selected_cmd < e->num_cmds_displayed - 1) e->selected_cmd++;
      else e->selected_cmd = 0;
    }
  }
}

void force_quit(Editor *e){
  e->should_quit = true;
}

void delete_buffer(Editor *e, ssize_t index){
  if(index < 0 || index > e->length - 1) return;
  Buffer *to_delete = e->buffers[index];
  if(e->length == 1 && (e->buffers[0]->is_saved || e->buffers[0]->is_readonly)){
    force_quit(e);
  }
  else if(e->capacity < 2){
    e->buffers[index] = new_buffer(1);
  }else {
    for(ssize_t i = index; i < e->capacity; i++){
      e->buffers[i] = e->buffers[i + 1];
    }
    if(e->length > 1) e->length--;
    e->capacity--;
    if(e->current_buff > 0) e->current_buff--;
    else if(e->current_buff < e->length - 1) e->current_buff++;
  }
  free_buffer(to_delete);
}

void try_closing_current_buffer(Editor *e){
  Buffer *buff = e->buffers[e->current_buff];
  if(buff->is_readonly || buff->is_saved){
    delete_buffer(e, e->current_buff);
  }else {
    if(e->conf.is_vim_mode){
      new_message(e, "Unsaved buffer, 's' to save / 'Q' to force quit", ERROR);
    }else {
      new_message(e, "Unsaved buffer, ctrl+s to save / ctrl+X to force quit", ERROR);
    }
  }
}

void force_close_current_buffer(Editor *e){
  delete_buffer(e, e->current_buff);
}

void try_quitting(Editor *e){
  ssize_t unsaved_index = -1;
  for(int i = 0; i < e->length; i++){
    if(!e->buffers[i]->is_saved && !e->buffers[i]->is_readonly){
      unsaved_index = i;
      break;
    }
  }
  if(unsaved_index == -1){
    e->should_quit = true;
  }else {
    e->current_buff = unsaved_index;
    if(e->conf.is_vim_mode){
      new_message(e, "Unsaved buffer, 's' to save / 'Q' to force quit", ERROR);
    }else {
      new_message(e, "Unsaved buffer, ctrl+s to save / ctrl+X to force quit", ERROR);
    }
  }
}

void delete_chars(Line *line, ssize_t from, ssize_t count){
  if(from + count > line->capacity || from < 0 || count < 1) return;
  for(int i = from; i < line->capacity - count; i++){
    line->chars[i] = line->chars[i + count];
  }
  line->length -= count;
}

void delete_lines(Buffer *buff, ssize_t from, ssize_t count){
  if((from + count) > buff->length || from < 0 || count < 1 || from >= buff->length) return;
  
  for(int i = from; i < from + count; i++) free_line(&buff->lines[i]);

  for(int i = from; i < buff->capacity - count; i++){
    buff->lines[i] = buff->lines[i + count];
  }
  buff->length -= count;
  if(!buff->length) {
    buff->length = 1;
    buff->cursor.index = 0;
  } 

  for(int i = buff->capacity - count; i < buff->capacity; i++){
    buff->lines[i] = new_line(DEFAULT_LINE_SIZE);
  }
}

void move_cursor_to_last_line(Editor* e){
  e->buffers[e->current_buff]->current_line_index = e->buffers[e->current_buff]->length - 1;
  e->buffers[e->current_buff]->d_start = e->buffers[e->current_buff]->current_line_index - get_max_num_lines(e);
  if(e->buffers[e->current_buff]->d_start < 0) e->buffers[e->current_buff]->d_start = 0;
  update_scroll(e, false);
  adapte_index_to_current_line(e);
}

void move_to_first_line(Editor *e){
  e->buffers[e->current_buff]->current_line_index = 0;
  e->buffers[e->current_buff]->d_start = 0;
  update_scroll(e, true);
  adapte_index_to_current_line(e);
}

void handle_delete_selection(Editor *e){
  Buffer *buff = e->buffers[e->current_buff];
  bool is_up = is_selecting_up(e); 
  ssize_t start = (is_up ? buff->current_line_index : e->conf.selection_start.row); 
  ssize_t finish = (is_up ? e->conf.selection_start.row : buff->current_line_index);
  bool is_left = buff->cursor.index <= e->conf.selection_start.col;

  if(e->conf.selection_start.col >= buff->lines[e->conf.selection_start.row]->length){
    e->conf.selection_start.col = buff->lines[e->conf.selection_start.row]->length - 1;
  }

  Line *start_line = buff->lines[start];
  Line *end_line = buff->lines[finish];

  if(start != finish){
    if(is_up){
     start_line->length = buff->cursor.index;
     if(end_line->length) end_line->length -= e->conf.selection_start.col + 1;
     memmove(
       &end_line->chars[0],
       &end_line->chars[e->conf.selection_start.col + 1],
       end_line->length
     );
    }
    else {
     start_line->length = e->conf.selection_start.col;
     if(end_line->length) end_line->length -= buff->cursor.index + 1;
     memmove(
       &end_line->chars[0],
       &end_line->chars[buff->cursor.index + 1],
       (size_t) end_line->length
     );
    } 
    memcpy(
      &start_line->chars[start_line->length],
      &end_line->chars[0],
      (size_t) end_line->length
    );
    start_line->length += end_line->length;
    ssize_t from = start + (start_line->length ? 1 : 0);
    ssize_t count = finish - start;
    delete_lines(buff, from , count);
    buff->current_line_index = finish - count;
    if(buff->current_line_index < 0) buff->current_line_index = 0;
    if(!e->conf.selection_start.col) buff->cursor.index = 0;
  }else {
    ssize_t num_chars = abs(buff->cursor.index - e->conf.selection_start.col) + 1;
    if(!start_line->length) {
      if(buff->current_line_index) {
        buff->current_line_index--;
        delete_lines(buff, start, 1);
        buff->cursor.index = 0;
      }
    } else {
      ssize_t start_index = is_left ? buff->cursor.index : e->conf.selection_start.col;
      delete_chars(start_line, start_index , num_chars);
      if(!is_left) buff->cursor.index -= num_chars ? (num_chars - 1) : 0;
    }
  }
  e->conf.is_selecting = false;
  update_line_number_padding(e);
}

void handle_backspace(Editor* e) {
  if(e->mode == INSERT || !e->conf.is_vim_mode) {
    if(e->conf.is_menu_open) {
      pop_char_single_line(e->cmd_prompt);
      filter_cmds_by_prompt(e);
    }else if(e->conf.is_selecting) {
      handle_delete_selection(e);
    }
    else {
      Buffer *current_buffer = e->buffers[e->current_buff];
      remove_char_from_line(e, current_buffer->lines[current_buffer->current_line_index]);
    } 
  } else {
    move_cursor_left(e);
  } 
}

void handle_normal_mode_keys(Editor* e, int c){
  switch(c){
    case 'G':
      move_cursor_to_last_line(e);
      break;
    case 'a':
      handle_append(e);
      break;
    case 'i':
      e->mode = INSERT;
      e->conf.is_selecting = false;
      e->conf.is_menu_open = false;
      break;
    case 'm':
      e->conf.is_menu_open = true;
      e->conf.is_opening_file = false;
      e->mode = INSERT;
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
      if(e->conf.is_menu_open){
      }
      break;
    case 's':
      try_saving_file(e);
      break;
    case '/':
      e->mode = INSERT;
      break;
    case 'h':
      if(e->mode == NORMAL){
        move_cursor_left(e);
      }
      break;
    case 'l':
      if(e->mode == NORMAL){
        move_cursor_right(e);
      }
      break;
    case 'j':
      move_cursor_down(e);
      break;
    case 'k':
      move_cursor_up(e);
      break;
    case '+':
      if(e->conf.font_data.size + 10 < 500){
        int new_size = e->conf.font_data.size + 10;
        e->conf.font_data.size = new_size;
        if(!load_font_default(e,  &e->conf.font_data)){
          fprintf(stderr, "Failed to load fonts\n");
          exit(1);
        }
      }
      break;
    case '-':
      if(e->conf.font_data.size - 10 > 10){
        int new_size = e->conf.font_data.size - 10;
        e->conf.font_data.size = new_size;
        if(!load_font_default(e, &e->conf.font_data)){
          fprintf(stderr, "Failed to load fonts\n");
          exit(1);
        }
      }
      break;
      break;
    case 'f':
      toggle_full_screen(e);
      update_scroll(e, true);
      break;
    case 'v':
      e->conf.is_selecting = !e->conf.is_selecting;
      e->conf.selection_start.row = e->buffers[e->current_buff]->current_line_index;
      e->conf.selection_start.col = e->buffers[e->current_buff]->cursor.index;
      break;
    case 'y':
      copy_selection_to_clipboard(e);
      e->conf.is_selecting = false;
      e->conf.is_menu_open = false;
      break;
    case 'p':
      if(e->conf.is_selecting) handle_delete_selection(e);
      paste_from_clipboard(e);
      update_scroll(e, true);
      e->conf.is_menu_open = false;
      break;
    case 'd':
      if(e->conf.is_selecting && e->conf.is_vim_mode) handle_delete_selection(e);
      break;
    case 'q':
      try_closing_current_buffer(e);
      break;
    case 'Q':
      force_close_current_buffer(e);
      break;
  }
  if(c == 'g'){
    if(is_g_clicked_before) {
      move_to_first_line(e);
      is_g_clicked_before = false;
    }else {
      is_g_clicked_before = true;
      //todo : move to line number
    } 
  }
}

void delete_to_beginning_of_line(Buffer *buff){
  Line *current = buff->lines[buff->current_line_index];
  current->length -= buff->cursor.index;
  memmove(&current->chars[0], &current->chars[buff->cursor.index], (size_t) current->length);
  buff->cursor.index = 0;
}

void handle_ctrl_plus_key(Editor *e, bool is_shift_down){
  if(IsKeyPressed(KEY_F)) {
    toggle_full_screen(e);
    update_scroll(e, true);
  }
  if(IsKeyPressed(KEY_S)) try_saving_file(e);

  if(IsKeyPressed(KEY_X)) {
    if(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)){
      force_close_current_buffer(e);
    }else {
      try_closing_current_buffer(e);
    }
  }

  if(IsKeyPressed(KEY_C)) {
    copy_selection_to_clipboard(e);
    e->conf.is_selecting = false;
    e->conf.is_menu_open = false;
  }

  if(IsKeyPressed(KEY_V)) {
    paste_from_clipboard(e);
    update_scroll(e, true);
    e->conf.is_menu_open = false;
  }

  if(IsKeyPressed(KEY_U)){
    if(e->mode == INSERT || !e->conf.is_vim_mode){
      delete_to_beginning_of_line(e->buffers[e->current_buff]);
      update_last_index(e);
    }
  }
  if(IsKeyPressed(KEY_HOME)){
    move_to_first_line(e);
  }
  if(IsKeyPressed(KEY_END)){
    move_cursor_to_last_line(e);
  }
  if(IsKeyPressed(KEY_N)){
    go_to_next_buffer(e);
  }
  if(IsKeyPressed(KEY_P)){
    go_to_prev_buffer(e);
  }
  if(IsKeyPressed(KEY_LEFT)){
    move_to_word_beginning(e);
  }
  if(IsKeyPressed(KEY_RIGHT)){
    move_to_word_ending(e);
  }
}

void handle_insert_mode_keys(Editor* e,int c){
  if (c >= 32) {
    add_char_to_line(e, 
      e->conf.is_menu_open ? e->cmd_prompt:
      e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index],
      c, e->conf.is_menu_open
    );
    if(e->conf.is_menu_open) filter_cmds_by_prompt(e);
  }
}

void start_new_file(Editor *e){
  if(e->length > e->capacity - 1) realloc_editor_buffers(e);
  e->current_buff = e->length++;
}

void to_lower_case(const char *text, char *dest){
  for(int i = 0; i < strlen(text); i++){
    char c = text[i];
    if(c >= 'A' && c <= 'Z'){
      dest[i] =  c + 32;
    } else dest[i] = c;
  }
}


void handle_open_file(Editor *e){
  e->conf.is_opening_file = true;
  char const * path = tinyfd_openFileDialog("Select File", "", 0, NULL, NULL, 0);
  if(path != NULL){
    read_file(e, path);
    update_scroll(e, true);
  }
  e->mode = NORMAL;
}

void open_config_file(Editor *e){
  char path[128];
  #ifdef PROD
  sprintf(path, "%s/.config/texteditor/texteditor.conf", e->HOME_DIR);
  #else
  sprintf(path, "assets/texteditor.conf");
  #endif
  read_file(e, path);
  e->mode = NORMAL;
}

void open_messages_file(Editor *e){
  #ifdef PROD
  char *file_path = "/usr/local/share/texteditor/messages.log";
  #else
  char *file_path = "assets/messages.log";
  #endif
  read_file(e, file_path);
  e->mode = NORMAL;
}

void handle_command(Editor *e, Cmd cmd){
  if(!e->num_cmds_displayed) return;
  switch (cmd.type) {
    case OPEN_FILE:
      handle_open_file(e);
      break;
    case NEW_FILE:
      start_new_file(e);
      break;
    case HELP:
      #ifdef PROD
      read_file(e, "/usr/local/share/texteditor/help.txt");
      #else
      read_file(e, "assets/help.txt");
      #endif
      e->mode = NORMAL;
      break;
    case OPEN_CONFIG:
      open_config_file(e);
      break;
    case OPEN_MESSAGES:
      open_messages_file(e);
      break;
  }
  e->conf.is_menu_open = false;
  e->selected_cmd = 0;
  e->cmd_prompt->length = 0;
  filter_cmds_by_prompt(e);
}

void handle_enter(Editor* e){
  if(e->mode == INSERT || !e->conf.is_vim_mode){
    if(!e->conf.is_menu_open){
      start_new_line(e);
      update_scroll(e, false);
      update_line_number_padding(e);
    }else {
      handle_command(e, default_cmds[e->displayed_cmds[e->selected_cmd]]);
    }
  }
}

void handle_mouse_click(
  Editor *e, int char_x, int char_y, ssize_t char_index,
  ssize_t line_index, bool is_holding
){

  ssize_t char_height = get_char_size(e->conf.font_data.size).row + e->conf.line_height;
  if(e->mouse.y >= char_y && e->mouse.y <= char_y + char_height){
    e->buffers[e->current_buff]->current_line_index = line_index;
    handle_click_on_line(e, char_x, char_index, is_holding);
  }else if(line_index == e->buffers[e->current_buff]->d_start && e->mouse.y < char_y){
    e->buffers[e->current_buff]->current_line_index = e->buffers[e->current_buff]->d_start;
    if(e->buffers[e->current_buff]->d_start > 0 && is_holding){
      double now = GetTime();
      if(now - last_press_time > HOLD_PRESS_DELAY) {
        e->buffers[e->current_buff]->d_start--;
        update_scroll(e, true);
        last_press_time = now;
      }
    }
    handle_click_on_line(e, char_x, char_index, is_holding);
  } else if(line_index == e->buffers[e->current_buff]->d_start + e->buffers[e->current_buff]->d_length - 1 && e->mouse.y > char_y){
      e->buffers[e->current_buff]->current_line_index = e->buffers[e->current_buff]->d_start + e->buffers[e->current_buff]->d_length - 1;
      if(e->buffers[e->current_buff]->d_start + e->buffers[e->current_buff]->d_length < e->buffers[e->current_buff]->length && is_holding){
        double now = GetTime();
        if(now - last_press_time > HOLD_PRESS_DELAY) {
          e->buffers[e->current_buff]->current_line_index++;
          update_scroll(e, false);
          last_press_time = now;
        }
      }
    handle_click_on_line(e, char_x, char_index, is_holding);
  }
}

void handle_click_on_line(Editor *e, int char_x, ssize_t char_index, bool is_holding){
  Line *current_line = e->buffers[e->current_buff]->lines[e->buffers[e->current_buff]->current_line_index];
  ssize_t char_width = get_char_size(e->conf.font_data.size).col + e->conf.letter_spacing;

  if(e->mouse.x >= char_x && e->mouse.x <= char_x + char_width){
    e->buffers[e->current_buff]->cursor.index = char_index;
  } else if(char_index== 0 && e->mouse.x < char_x){
    e->buffers[e->current_buff]->cursor.index = 0;
  } else if(char_index == current_line->length - 1 && e->mouse.x > char_x){
    e->buffers[e->current_buff]->cursor.index = current_line->length - is_holding;
  }

  if(!is_holding) {
    e->conf.selection_start.row = e->buffers[e->current_buff]->current_line_index;
    e->conf.selection_start.col = e->buffers[e->current_buff]->cursor.index;
  }
}

void handle_keys(Editor* e){
  int c;
  if ((c = GetCharPressed()) >= 8) {
    if (e->mode == NORMAL && e->conf.is_vim_mode) handle_normal_mode_keys(e,c);
    else handle_insert_mode_keys(e, c);
  }
  bool is_ctrl_down = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);


  if(!e->conf.is_vim_mode && is_shift_down && !e->conf.is_selecting) {
    e->conf.is_selecting = true;
    e->conf.selection_start.row = e->buffers[e->current_buff]->current_line_index;
    e->conf.selection_start.col = e->buffers[e->current_buff]->cursor.index;
  } 

  if(is_ctrl_down) handle_ctrl_plus_key(e, is_shift_down);

//backspace
  if (IsKeyPressed(KEY_BACKSPACE)){
    long_press_time = GetTime();
    handle_backspace(e);
  } 
  else if (IsKeyDown(KEY_BACKSPACE)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      handle_backspace(e);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyReleased(KEY_BACKSPACE)) long_press_time = 0;

  else if(IsKeyPressed(KEY_TAB)) {
     handle_tab(e, is_shift_down);
  }

// enter 
  else if (IsKeyPressed(KEY_ENTER)) handle_enter(e);

//escape & capslock
  else if (IsKeyPressed(KEY_ESCAPE) || (IsKeyPressed(KEY_CAPS_LOCK) && e->conf.caps_lock_as_escape)) {
    handle_caps_lock_and_escape(e);
  } 
  else if (IsKeyPressed(KEY_UP)) {
    long_press_time = GetTime();
    move_cursor_up(e);
  }
  else if (IsKeyPressed(KEY_DOWN)) {
    long_press_time = GetTime();
    move_cursor_down(e);
  }
  else if (IsKeyPressed(KEY_LEFT)) {
    long_press_time = GetTime();
    move_cursor_left(e);
  }
  else if (IsKeyPressed(KEY_RIGHT)) {
    long_press_time = GetTime();
    move_cursor_right(e);
  }

  if (IsKeyDown(KEY_LEFT)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_left(e);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyDown(KEY_RIGHT)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_right(e);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyDown(KEY_UP)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_up(e);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyDown(KEY_DOWN)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_down(e);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
}

// int get_first_diplayed_index(Editor* e,bool isUp){
  // int line_index = get_line_index(e,-1);
  // todo
  // if(isUp && e->buffers[e->current_buff]->cursor.row > 0) return e->note->displayStart;
  // if(isUp) {
    // int i,j;
    // for(j = 0,i = e->buffers[e->current_buff]->cursor.index ; i > -2 && j < 1 ;i--){
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

Line *new_line(ssize_t cap){
  Line *line = malloc(sizeof(Line));
  line->chars = malloc(sizeof(char) * cap);
  line->capacity = cap;
  line->length = 0;
  return line;
}

void realloc_line(Line *line, ssize_t cap){
  line->chars = realloc(line->chars, cap * sizeof(char));
  line->capacity = cap;
}

void free_line(Line **line){
  if(*line != NULL){
    free((*line)->chars);
    free(*line);
    *line = NULL;
  }
}

Buffer *new_buffer(ssize_t capacity){
  Buffer *buff = malloc(sizeof(Buffer));
  buff->lines = malloc(sizeof(Line*) * capacity);
  for(ssize_t i = 0; i < capacity; i++){
    buff->lines[i] = new_line(DEFAULT_LINE_SIZE);
  }
  buff->capacity = capacity;
  buff->length = 1;
  buff->d_start = 0;
  buff->d_length = 1;
  buff->num_chars = 0;
  buff->current_line_index = 0;
  buff->current_msg_index = -1;
  buff->file_path = NULL;
  buff->is_saved = true;
  buff->is_readonly = false;
  buff->cursor = (Cursor) {
    .color = 0xD1D1CFFF
  };
  return buff;
}

void free_buffer(Buffer *buff){
  for(ssize_t i = 0; i < buff->capacity; i++) {
    free_line(&buff->lines[i]);
  }
  free(buff);
  buff = NULL;
}

void get_date_time(char *time_buff, size_t size){
  time_t current_time;
  time(&current_time);
  struct tm *local_time = localtime(&current_time);
  strftime(time_buff, size, "%H:%M:%S", local_time);
}

void new_message(Editor *e, const char *message, MessageType type){
  if(e->num_msgs >= MAX_MESSAGES) {
    for(int i = 0; i < e->num_msgs; i++){
      e->messages[i] = e->messages[i + 1];
      e->num_msgs--;
    }
  }
  char display_msg[1024];
  char time_buff[128];
  get_date_time(time_buff, sizeof(time_buff));

  snprintf(display_msg, sizeof(display_msg), "%s - %s", message, time_buff);
  Message *msg = malloc(sizeof(Message));

  msg->type = type;
  msg->text = strdup(display_msg);
  write_new_message(e, msg);
  e->messages[e->num_msgs] = msg;
  e->buffers[e->current_buff]->current_msg_index = e->num_msgs++;
}

void realloc_editor_buffers(Editor *e){
  e->capacity += 5;
  e->buffers = realloc(e->buffers, sizeof(Buffer*) * e->capacity);
  for(ssize_t i = e->length; i < e->capacity; i++){
    e->buffers[i] = new_buffer(1);
  }
}

Editor *init_editor(){
  Editor *e = malloc(sizeof(Editor));
  e->cmd_prompt = new_line(DEFAULT_LINE_SIZE);

  e->buffers = malloc(sizeof(Buffer*));
  e->length = 1;
  e->capacity = 1;

  for(int i = 0; i < e->capacity; i++){
    e->buffers[i] = new_buffer(1);
  }

  e->mode = NORMAL;
  e->s_width = SCREEN_WIDTH;
  e->s_height = SCREEN_HEIGHT;
  e->num_cmds_displayed = NUM_COMMANDS;
  e->conf = (Config) {
    .font_data = {
      .size = 42,
      .path = FONT_PATH
    },
    .font_secondary_data= {
      .size = 36,
      .path = SECONDARY_FONT_PATH
    },
    .bg_color = 0x141415FF,
    .text_color = 0xFFFFFFFF,
    .under_cursor_color = 0X000000FF,
    .selected_char_color = 0XC3C3D5FF,
    .lines_color = 0x545454FF,
    .line_numbers_color = 0x828282FF,
    .file_name_color = 0x828282FF,
    .status_line_color = 0x000000FF,
    .error_color = 0xFF4C24FF,
    .success_color = 0x00B014FF,
    .selection_color = 0x333738FF,
    .is_menu_open = false,
    .is_vim_mode = true,
    .is_spaces_for_tabs = true,
    .caps_lock_as_escape = true,
    .tab_size = 2,
    .is_showing_lines = false,
    .ln_mode = NONE,
    .ln_padding = 1,
    .line_height = 5,
    .letter_spacing = 0,
    .padding = {
      .top = 45,
      .bottom = 45,
      .right = 70,
      .left = 10
    }
  },
  e->HOME_DIR  = getenv("HOME");

  return e;
}

