#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "io.h"
#include "tinyfiledialogs.h"

#define PAIRS_COUNT 8
#define cur_buf e->buffs->data[e->current_buff]
#define cur_line cur_buf->lines->data[cur_buf->cur_li]

double long_press_time = 0.0f;
int is_g_clicked_before = false;

Cmd default_cmds[NUM_COMMANDS] = {
  { NEW_FILE , "New file" },
  { OPEN_FILE , "Open file" },
  { HELP , "Help!" },
  { OPEN_CONFIG , "Open config" },
  { OPEN_MESSAGES , "Open log messages" },
};

size_t get_max_line_length(Editor *e){ 
  size_t char_width = get_char_size(e->conf.font_data.size).col;
  size_t x_padding = e->conf.padding.left + e->conf.padding.right;
  if(e->conf.ln_mode != NONE) x_padding += e->conf.ln_padding * (e->conf.letter_spacing + char_width);
  return (e->s_width - x_padding) / (e->conf.letter_spacing + char_width); 
}

void update_buf_state(Editor *e){
  cur_buf->msg_index = -1;
  cur_buf->cursor.last_time_moved = GetTime();
  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
}

/* size_t get_num_gui_lines(Buffer *buff){
  size_t num_gui_lines = buff->lines->len ;
  for(size_t i = 0; i < buff->lines->len; i++){
    num_gui_lines += buff->lines->data[i].wraps;
  }
  return num_gui_lines;
} */

/* size_t get_line_from_index(Lines *lines, size_t index){
  for(size_t i = 0; i < lines->len; i++){
    if(index >= lines->data[i].start && index <= lines->data[i].end) return i;
  }
} */

void update_line_number_padding(Editor *e){
  if(!cur_buf->lines) return;
  e->conf.ln_padding = get_digit_count(cur_buf->num_prev_lines + cur_buf->lines->len - 1);
}

size_t get_count_prev_lines(String *str, size_t from){
  size_t count = 0;
  for(size_t i = 0; i < from; i++){
    if(str->data[i] == '\n') count++;
  }
  return count;
}

void update_lines(Editor *e){
  if(!cur_buf || !cur_buf->s || !cur_buf->lines) return;
  cur_buf->lines->len = 0;
  Line line = {
    .start = cur_buf->d_start
  };
  size_t max_len = get_max_line_length(e);
  size_t max_num_lines = get_max_num_lines(e);
  size_t i, x_offset = 0, y_offset = 0, index = 0, total_wraps = 0;
  for (
    i = cur_buf->d_start;
    (i <= cur_buf->s->len) && (cur_buf->lines->len <= max_num_lines);
    i++
  ){
    char c = cur_buf->s->data[i];
    
    if((x_offset) >= max_len) {
      y_offset++;
      x_offset = 0;
      line.wraps++;
      total_wraps++;
    }
    else if(isspace(cur_buf->s->data[i - 1]) || i == cur_buf->d_start + 1){
      int word_len = 0;
      while(
        word_len < max_len - 1 &&
        i + word_len < cur_buf->s->len &&
        !isspace(cur_buf->s->data[i + word_len++])
      );
      if((x_offset + word_len - 1) > max_len){
        x_offset = 0;
        y_offset++;
        line.wraps++;
        total_wraps++;
      }
    }

    if(index + cur_buf->d_start == cur_buf->cursor.index){
      cur_buf->cur_li = cur_buf->lines->len;
      cur_buf->cursor.pos.col = x_offset;
      cur_buf->cursor.pos.row = y_offset;
    }
    if(c == '\n'){
      line.end = i;
      da_append(cur_buf->lines, line);
      line.start = i + 1;
      line.wraps = 0;
      y_offset++;
      x_offset = 0;
      index++;
      continue;
    }
    index++;
    x_offset++;
  }
  line.end = i - 1;
  da_append(cur_buf->lines, line);
  // update_line_number_padding(e);
}

size_t get_max_num_lines(Editor *e){ 
  size_t char_height = get_char_size(e->conf.font_data.size).row;
  size_t y_padding = e->conf.padding.top + e->conf.padding.bottom;
  return (e->s_height - y_padding) / (e->conf.line_height + char_height); 
}

bool is_selecting_up(Editor *e){
  return cur_buf->cursor.index <= e->conf.selection_start;
}

void move_to_matching_pair(Editor *e, char c){
/*   int pairs[PAIRS_COUNT] = { '{', '(', '[', '<', '>', ']', ')', '}' };
  int pair_index = 0;
  int i;
  for(i = 0; i < PAIRS_COUNT; i++) {
    if(pairs[i] == c){ pair_index = i; break; }
  }
  if(i >= PAIRS_COUNT) return;
  int match = match = pairs[PAIRS_COUNT - pair_index - 1];
  bool is_opening = pair_index < (PAIRS_COUNT / 2); 
  int num_opened = 1;
  int line_index = cur_buf->cur_li;
  int char_index = cur_buf->cursor.index;
  int next = 0;
  while(num_opened){ 
    if(is_opening) {
      char_index++;
      if(char_index >= cur_buf->lines[line_index]->length) {
        line_index++;
        char_index = 0;
      } 
      if(line_index >= cur_buf->length) break;
    }else {
      char_index--;
      if(char_index < 0){
        line_index--;
        if(line_index < 0) break;
        char_index = MAX(cur_buf->lines[line_index]->length - 1, 0);
      }
    }
    next = cur_buf->lines[line_index]->chars[char_index];
    if(next == c) {
     num_opened++;
    }
    else if(next == match) {
      num_opened--;
    }
  }
  if(next == match && num_opened == 0) {
    cur_buf->cur_li = line_index;
    cur_buf->cursor.index = char_index;
  }
  cur_buf->cursor.last_time_moved = GetTime(); */
}

void filter_cmds_by_prompt(Editor *e){
  int index = 0;
  e->selected_cmd = 0;
  for(size_t i = 0; i < NUM_COMMANDS; i++){
    if(!e->prompt->len){
      e->displayed_cmds[index++] = i;
      continue;
    }

    bool is_match = str_includes(string(default_cmds[i].text), e->prompt);
    if(is_match){
      e->displayed_cmds[index++] = i;
    }
  }
  e->num_cmds_displayed = index;
}

bool is_selected(Editor *e, size_t index){
  if(!e->conf.is_selecting) return false;
  bool is_left = cur_buf->cursor.index <= e->conf.selection_start;
  if(is_left){
    return index >= cur_buf->cursor.index && index <= e->conf.selection_start;
  }else {
    return index >= e->conf.selection_start && index <= cur_buf->cursor.index;
  }
}

void scroll_up(Editor *e, size_t count){
  size_t max_num_lines = get_max_num_lines(e);
  if(cur_buf->lines->len - 1 <= max_num_lines - e->conf.scroll_pad) return;
  cur_buf->d_start = seek_forward_by_lines(cur_buf, cur_buf->d_start, 1);
}

void scroll_down(Editor *e, size_t count){
  size_t new_start = seek_back_by_lines(cur_buf, cur_buf->d_start, 1);
  cur_buf->d_start = new_start;
}

size_t get_next_line_start(String *str, size_t index){
  size_t i;
  for(i = index; i < str->len; i++){
    if(str->data[i] == '\n') return i + 1;
  }
  return i;
}

size_t seek_forward_by_lines(Buffer *buff, size_t from, size_t count){
  size_t i;
  for(i = from; i < buff->s->len && count > 0; i++){
    if(buff->s->data[i] == '\n') count--;
  } 
  if(i == buff->s->len) i--;
  return i;
}

size_t seek_back_by_lines(Buffer *buff, size_t from, size_t count){
  size_t i;
  for(i = from; i > 0 && count + 1 > 0; i--){
    if(buff->s->data[i] == '\n') count--;
  }
  if(i > 0) i++;
  return i + (i > 0 && buff->s->data[i + 1] != '\n');
}

size_t get_prev_line_start(String *str, size_t index){
  size_t i;
  bool got_previous = false;
  for(i = index; i > 0; i--){
    if(str->data[i] == '\n') {
      if(got_previous) {
        if(str->data[i + 1] == '\n') return i;
        else return i + 1;
      } 
      else got_previous = true;
    }
  }
  return i;
}

 void update_scroll(Editor *e, bool center_line, bool is_up){
  size_t max = get_max_num_lines(e);
  size_t index = cur_buf->cursor.index;
  size_t wraps = get_lines_wraps(e, 0, max);
  size_t max_lines = max - wraps;
  size_t top_pad_start = seek_forward_by_lines(cur_buf, cur_buf->d_start, e->conf.scroll_pad);
  size_t bot_pad_start = seek_forward_by_lines(cur_buf, cur_buf->d_start, max - wraps - e->conf.scroll_pad);
  size_t new_first = cur_buf->d_start;
  if(index < top_pad_start){
    new_first = seek_back_by_lines(cur_buf, index, e->conf.scroll_pad);
  }
  else if(index > bot_pad_start){
    new_first = seek_back_by_lines(cur_buf, index, max - e->conf.scroll_pad);
  } else {
  } 
  if(new_first != cur_buf->d_start) {
    cur_buf->d_start = new_first;
    cur_buf->num_prev_lines = get_count_prev_lines(cur_buf->s, new_first);
    update_line_number_padding(e);
  }
  update_lines(e);
}

int get_digit_count(int number){
  int count = 1;
  for(int i = number; i > 0; i /= 10) count++;
  return count > 1 ? (count - 1) : count;
}

void handle_append(Editor *e){
  e->mode = INSERT;
  e->conf.is_selecting = false;
  if(cur_line.end - cur_line.start > 0) move_cursor_right(e, 1);
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
  update_lines(e);
}

void add_char_to_cur_buf(Editor *e, char c, size_t index){

  String *str = cur_buf->s;
  add_char_to_str(str, c, index);

  cur_buf->cursor.index = index + 1;

  update_action(&cur_buf->cur_act, index, c_string(c), true);
  update_lines(e);
  update_scroll(e, false, false);
  update_buf_state(e);
  cur_buf->is_saved = false;
}

void replace_char(String *str, size_t index, char new_char){
  /* if(index < 0 || index > line->buffs->len - 1 || line == NULL) return;
  line->chars[index] = new_char; */
}

void remove_chars_cur_buf(Editor *e, size_t index, size_t count){
  String *str = cur_buf->s;

  String *deleted = new_str(count + 1);
  memcpy(deleted->data, &cur_buf->s->data[index], count);
  deleted->len += count;
  update_action(&cur_buf->cur_act, index, deleted, false);

  str_remove_chars(str, index, count);
  cur_buf->is_saved = false;
}

size_t get_lines_wraps(Editor *e, size_t from, size_t to){
  if(from > to) return 0;
  if(from < 0) from = 0;
  // if(to >= cur_buf->lines->len) to = cur_buf->lines->len - 1;
  size_t wraps = 0;
  size_t max = get_max_line_length(e);
  for(size_t i = from; i <= to; i++){
    // wraps += cur_buf->lines->data[i].wraps; 
  }
  return wraps;
}
  
void update_last_col(Editor* e){
  cur_buf->cursor.last_col = cur_buf->cursor.pos.col;
}

void move_cursor_right(Editor* e, size_t count) {
  bool is_normal = e->conf.is_vim_mode && e->mode == NORMAL;
  if(cur_buf->cursor.index < cur_buf->s->len - (is_normal ? count : count - 1)){
    if(is_normal && cur_buf->s->data[cur_buf->cursor.index + count] == '\n') {
      cur_buf->cursor.index++;
    } 
    cur_buf->cursor.index += count;
  }
  update_lines(e);
  update_scroll(e, false, false);
  update_last_col(e);
  update_buf_state(e);
}

void move_cursor_left(Editor* e, size_t count) {
  bool is_normal = e->conf.is_vim_mode && e->mode == NORMAL;
  if(cur_buf->cursor.index >= count) {
    if(is_normal && cur_buf->s->data[cur_buf->cursor.index - count] == '\n') {
      cur_buf->cursor.index--;
    } 
    cur_buf->cursor.index -= count;
  } else cur_buf->cursor.index = 0; 
  update_lines(e);
  update_scroll(e, false, true);
  update_last_col(e);
  update_buf_state(e);
}

void handle_caps_lock_and_escape(Editor* e){
  if(e->conf.is_vim_mode){
    if(e->conf.is_selecting) e->conf.is_selecting = false;
    else if(e->mode == INSERT){
      e->mode = NORMAL;
      if(cur_buf->cursor.index != cur_line.start) move_cursor_left(e, 1);
      e->conf.is_menu_open = false;
      e->prompt->len = 0;
      filter_cmds_by_prompt(e);
      e->conf.is_opening_file = false;
      e->conf.is_selecting = false;
    }
  }else {
    e->conf.is_menu_open = !e->conf.is_menu_open;
    e->prompt->len = 0;
    filter_cmds_by_prompt(e);
  }
  if(cur_buf->cur_act != NULL) {
    action_stack_push(&cur_buf->undo_stack, *cur_buf->cur_act);
    action_stack_flush(&cur_buf->redo_stack);
    cur_buf->cur_act = NULL;
  } 
}

void move_cursor_up(Editor* e){
  if(cur_buf->cursor.pos.row > 0){
    size_t max_lines = get_max_num_lines(e);
    Line prev_line = cur_buf->lines->data[cur_buf->cur_li - 1];
    cur_buf->cursor.index = prev_line.start;
    update_lines(e);
    adapte_col_to_cur_line(e);
    update_scroll(e, false, true);
  }
  update_buf_state(e);
}

void move_cursor_down(Editor* e){
  if(cur_buf->cur_li < cur_buf->lines->len - 1){
    size_t max_lines = get_max_num_lines(e);
    size_t wraps = get_lines_wraps(e, 0, cur_buf->cur_li);
    Line curr_line = cur_buf->lines->data[cur_buf->cur_li];
    Line next_line = cur_buf->lines->data[cur_buf->cur_li + 1];
    cur_buf->cursor.index = next_line.start;
    update_lines(e);
    adapte_col_to_cur_line(e);
    update_scroll(e, false, false);
  }
  update_buf_state(e);
}

void adapte_col_to_cur_line(Editor *e){
  size_t len = cur_line.end - cur_line.start;
  if(len >= cur_buf->cursor.last_col + 1){
    cur_buf->cursor.index += cur_buf->cursor.last_col;
  } else cur_buf->cursor.index = cur_line.end - (len ? 1 : 0);
}

void move_to_beginning_of_line(Editor* e) {
  size_t max = get_max_line_length(e);
  Line line = cur_buf->lines->data[cur_buf->cur_li];
  cur_buf->cursor.index = line.start;
  update_lines(e);
  update_last_col(e);
  if(line.end - line.start > max) update_scroll(e, false, true);
  update_buf_state(e);
}

void move_to_end_of_line(Editor* e) {
  size_t max = get_max_line_length(e);
  Line line = cur_buf->lines->data[cur_buf->cur_li];
  cur_buf->cursor.index = line.end - 1;
  update_lines(e);
  update_last_col(e);
  if(line.end - line.start > max) update_scroll(e, false, false);
  update_buf_state(e);
}

void move_to_word_ending(Editor* e){
  size_t i = cur_buf->cursor.index; 
  char *text = cur_buf->s->data;
  size_t last_index = cur_buf->s->len - 1;
  while(isspace(text[i + 1]) && i < last_index ){
    move_cursor_right(e, 1);
    i++;
  }
  if(!isalnum(text[i + 1]) && i < last_index){
    move_cursor_right(e, 1);
    i++;
  }
  while((isalnum(text[i + 1]) || text[i + 1] == '_') && i < last_index) {
    move_cursor_right(e, 1);
    i++;
  }
  // move_cursor_right(e, i - cur_buf->cursor.index);
  cur_buf->cursor.index = i;
  update_last_col(e);
  update_buf_state(e);

  // update_scroll(e, false);
}

void move_to_word_beginning(Editor* e){
  size_t i = cur_buf->cursor.index; 
  char *text = cur_buf->s->data;
  // todo move_cursor with count
  while(isspace(text[i - 1]) && i > 0){
    move_cursor_left(e, 1);
    i--;
  }
  if(!isalnum(text[i - 1]) && i > 0) {
    move_cursor_left(e, 1);
    i--;
  }
  while((isalnum(text[i - 1]) || text[i - 1] == '_') && i > 0) {
    move_cursor_left(e, 1);
    i--;
  }
  cur_buf->cursor.index = i;
  update_last_col(e);

  bool is_shift_down = IsKeyDown(KEY_RIGHT_SHIFT) || IsKeyDown(KEY_LEFT_SHIFT);
  if(!e->conf.is_vim_mode && e->conf.is_selecting && !is_shift_down) e->conf.is_selecting = false;
  // update_scroll(e, true);
  cur_buf->cursor.last_time_moved = GetTime(); 
}

void go_to_next_buffer(Editor *e){
  if(e->buffs->len < 2) return;
  if(e->current_buff >= e->buffs->len - 1) e->current_buff = 0;
  else e->current_buff++;
  e->conf.is_selecting = false;
  update_lines(e);
  // update_line_number_padding(e);
}

void go_to_prev_buffer(Editor *e){
  if(e->buffs->len < 2) return;
  if(e->current_buff < 1) e->current_buff = e->buffs->len - 1;
  else e->current_buff--;
  e->conf.is_selecting = false;
  update_lines(e);
  // update_line_number_padding(e);
}

void handle_tab(Editor* e, bool is_shift_down) {
  if(!e->conf.is_menu_open){
    if(e->mode == INSERT || !e->conf.is_vim_mode){
      if(e->conf.is_spaces_for_tabs) {
        for(int i = 0; i < e->conf.tab_size; ++i)  {
          add_char_to_cur_buf(e, ' ', cur_buf->cursor.index);
          /* update_action(
            &cur_buf->cur_act, ADD_STR,
            (RowCol){ cur_buf->cur_li, index }, ' '
          ); */
        }
      }
      else {
        add_char_to_cur_buf(e, '\t', cur_buf->cursor.index);
        /* update_action(
          &cur_buf->cur_act, ADD_STR,
          (RowCol){ cur_buf->cur_li, index }, '\t'
        ); */
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

void delete_buffer(Editor *e, size_t index){
  if(index < 0 || index > e->buffs->len - 1) return;
  Buffer *to_delete = e->buffs->data[index];

  memmove(
    &e->buffs->data[index],
    &e->buffs->data[index + 1],
    (e->buffs->len - index - 1) * sizeof(Buffer*)
  );
  e->buffs->len--;

  free_buffer(to_delete);

  if(e->buffs->len) {
    if(e->current_buff > 0) e->current_buff--;
    else e->current_buff = e->buffs->len - 1;
    update_lines(e);
    // update_line_number_padding(e);
    update_buf_state(e);
  }else {
    force_quit(e);
  } 
}

void try_closing_current_buffer(Editor *e){
  if(cur_buf->is_readonly || cur_buf->is_saved){
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
  size_t unsaved_index = -1;
  for(int i = 0; i < e->buffs->len; i++){
    if(!e->buffs->data[i]->is_saved && !e->buffs->data[i]->is_readonly){
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

void move_to_last_line(Editor* e){
  size_t i;
  for(i = cur_buf->s->len ; i > 0; i--){
    if(cur_buf->s->data[i] == '\n') break;
  }
  cur_buf->cursor.index = i + 1;
  update_lines(e);
  adapte_col_to_cur_line(e);
  update_scroll(e, true, false);
  update_buf_state(e);
}

void move_to_first_line(Editor *e){
  cur_buf->cursor.index = 0;
  update_lines(e);
  adapte_col_to_cur_line(e);
  update_scroll(e, false, true);
  update_buf_state(e);
}

void handle_delete_selection(Editor *e){
  if(!cur_buf->s->len) return;
  bool is_left = cur_buf->cursor.index <= e->conf.selection_start;
  size_t start = (is_left ? cur_buf->cursor.index : e->conf.selection_start); 
  size_t finish = (is_left ? e->conf.selection_start : cur_buf->cursor.index);
  remove_chars_cur_buf(e, start, finish - start + 1);
  e->conf.is_selecting = false;
  cur_buf->cursor.index = start;

  update_buf_state(e);
  update_scroll(e, false, true);
  // update_line_number_padding(e);
}

void handle_backspace(Editor* e) {
  if(e->mode == INSERT || !e->conf.is_vim_mode) {
    if(e->conf.is_menu_open && e->prompt->len) {
      str_remove_chars(e->prompt, e->prompt->len - 1, 1);
      filter_cmds_by_prompt(e);
    }else if(e->conf.is_selecting) {
      handle_delete_selection(e);
    }
    else if(cur_buf->cursor.index - 1 < cur_buf->s->len && cur_buf->s->len){
      remove_chars_cur_buf(e, cur_buf->cursor.index - 1, 1);
      cur_buf->cursor.index--;
      update_lines(e);
      update_scroll(e, false, true);
      update_buf_state(e);
    } 
  } else {
    move_cursor_left(e, 1);
  } 
}

void handle_normal_mode_keys(Editor* e, int c){
  switch(c){
    case 'G':
      move_to_last_line(e);
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
      if(cur_buf->s->len){
        str_remove_chars(cur_buf->s, cur_buf->cursor.index, 1);
      }
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
        move_cursor_left(e, 1);
      }
      break;
    case 'l':
      if(e->mode == NORMAL){
        move_cursor_right(e, 1);
      }
      break;
    case 'j':
      move_cursor_down(e);
      break;
    case 'k':
      move_cursor_up(e);
      break;
    case '+':
      increase_font_size(e);
      break;
    case '-':
      decrease_font_size(e);
      break;
    case 'f':
      toggle_full_screen(e);
      // update_scroll(e, true);
      break;
    case 'v':
      e->conf.is_selecting = !e->conf.is_selecting;
      e->conf.selection_start = cur_buf->cursor.index;
      break;
    case 'y':
      copy_selection_to_clipboard(e);
      e->conf.is_selecting = false;
      e->conf.is_menu_open = false;
      break;
    case 'p':
      if(e->conf.is_selecting) handle_delete_selection(e);
      paste_from_clipboard(e);
      // update_scroll(e, true);
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
    /* case '%':
      move_to_matching_pair(e, cur_line->chars[cur_buf->cursor.index]);
      break; */
    case 'u':
      undo(e);
      break;
    case '?':
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

void decrease_font_size(Editor *e){
  if(e->conf.font_data.size - 10 > 10){
    int new_size = e->conf.font_data.size - 10;
    e->conf.font_data.size = new_size;
    if(!load_font_default(e, &e->conf.font_data)){
      fprintf(stderr, "Failed to load fonts\n");
      exit(1);
    }
  }
}

void increase_font_size(Editor *e){
  if(e->conf.font_data.size + 10 < 5000){
    int new_size = e->conf.font_data.size + 10;
    e->conf.font_data.size = new_size;
    if(!load_font_default(e,  &e->conf.font_data)){
      fprintf(stderr, "Failed to load fonts\n");
      exit(1);
    }
  }
}

void delete_to_beginning_of_line(Buffer *buff){
  /*
  String *chars = buff->lines[buff->cur_li];
  update_action(
    &buff->cur_act, DELETE_STR, 
    (RowCol){}, 
  );
  line->buffs->len -= buff->cursor.index;
  memmove(&line->chars[0], &line->chars[buff->cursor.index], (size_t) line->buffs->len);
  buff->cursor.index = 0;
  buff->cursor.last_time_moved = GetTime();
  */
}

void handle_ctrl_plus_key(Editor *e, bool is_shift_down){
  if(IsKeyPressed(KEY_F)) {
    toggle_full_screen(e);
    // update_scroll(e, true);
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
    // update_scroll(e, true);
    e->conf.is_menu_open = false;
  }

  if(IsKeyPressed(KEY_U)){
    if(e->mode == INSERT || !e->conf.is_vim_mode){
      delete_to_beginning_of_line(cur_buf);
      update_last_col(e);
    }
  }
  if(IsKeyPressed(KEY_HOME)){
    move_to_first_line(e);
  }
  if(IsKeyPressed(KEY_END)){
    move_to_last_line(e);
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
  if(IsKeyPressed(KEY_MINUS)){
    decrease_font_size(e);
  }
  if(IsKeyPressed(KEY_EQUAL) && is_shift_down){
    increase_font_size(e);
  }
  if(IsKeyPressed(KEY_R)){
    redo(e);
  }
}

void handle_insert_mode_keys(Editor* e,int c){
  if (c >= 32) {
    if(e->conf.is_menu_open) {
      add_char_to_str(e->prompt, c, e->prompt->len);
      filter_cmds_by_prompt(e);
    }else {
      add_char_to_cur_buf(e, c, cur_buf->cursor.index);
      /*
      update_action(
        &cur_buf->cur_act, ADD_STR,
        (RowCol){ cur_buf->cur_li, index }, c
      );
       if(cur_buf->cur_act == NULL){
        cur_buf->cur_act = init_action(
          ADD_STR, (RowCol){ cur_buf->cur_li, index }
        );
      }
      add_char_to_line(cur_buf->cur_act->str, c, cur_buf->cur_act->str->length); */
    }
  }
}

void start_new_file(Editor *e){
  if(e->buffs->len > e->buffs->cap - 1) realloc_editor_buffers(e);
  e->current_buff = e->buffs->len++;
}

void handle_open_file(Editor *e){
  e->conf.is_opening_file = true;
  char const * path = tinyfd_openFileDialog("Select File", "", 0, NULL, NULL, 0);
  if(path != NULL){
    read_file(e, path);
    update_scroll(e, true, true);
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
  update_scroll(e, false, true);
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
  e->prompt->len= 0;
  filter_cmds_by_prompt(e);
}

void handle_enter(Editor* e){
  if(e->mode == INSERT || !e->conf.is_vim_mode){
    if(!e->conf.is_menu_open){

      add_char_to_cur_buf(e, '\n', cur_buf->cursor.index);

      /* if(cur_buf->cur_act == NULL){
        cur_buf->cur_act = init_action(
          ADD_STR, (RowCol){ cur_buf->cur_li - 1, index }
        );
      }
      add_char_to_line(cur_buf->cur_act->str, '\n', cur_buf->cur_act->str->length); */
      // cur_buf->cur_act->str->length--; // not counting new line character
    }else {
      handle_command(e, default_cmds[e->displayed_cmds[e->selected_cmd]]);
    }
  }
}

void handle_mouse_click(Editor *e, size_t index, bool is_holding){

  cur_buf->cursor.index = index;

  if(!is_holding) {
    e->conf.selection_start = cur_buf->cursor.index;
  } 
  update_lines(e);
  update_last_col(e);
   
  /* else if(
    (line_index == cur_buf->d_start && e->mouse.y < char_y){
    cur_buf->cur_li = cur_buf->d_start;
    if(cur_buf->d_start > 0 && is_holding){
      double now = GetTime();
      if(now - last_press_time > HOLD_PRESS_DELAY) {
        cur_buf->d_start--;
        update_scroll(e, true);
        last_press_time = now;
      }
    }
    handle_click_on_line(e, char_x, char_index, is_holding);
  } */
  /* else if(line_index == cur_buf->d_start + cur_buf->d_length - 1 && e->mouse.y > char_y){
      cur_buf->cur_li = cur_buf->d_start + cur_buf->d_length - 1;
      if(cur_buf->d_start + cur_buf->d_length < cur_buf->length && is_holding){
        double now = GetTime();
        if(now - last_press_time > HOLD_PRESS_DELAY) {
          cur_buf->cur_li++;
          update_scroll(e, false);
          last_press_time = now;
        }
      }
    handle_click_on_line(e, char_x, char_index, is_holding);
  } */
  cur_buf->cursor.last_time_moved = GetTime();
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
    e->conf.selection_start = cur_buf->cursor.index;
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
    move_cursor_left(e, 1);
  }
  else if (IsKeyPressed(KEY_RIGHT)) {
    long_press_time = GetTime();
    move_cursor_right(e, 1);
  }

  if (IsKeyDown(KEY_LEFT)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_left(e, 1);
      long_press_time = now - (LONG_PRESS_DELAY - REPEAT_RATE);
    }
  }
  else if (IsKeyDown(KEY_RIGHT)) {
    double now = GetTime();
    if(now - long_press_time > LONG_PRESS_DELAY){
      move_cursor_right(e, 1);
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

Line *new_line(){
  Line *line = malloc(sizeof(Line));
  line->start = 0;
  line->end = 0;
  line->wraps = 0;
  return line;
}

Buffer *new_buffer(){
  Buffer *buff = malloc(sizeof(Buffer));
  buff->s = new_str(DEFAULT_LINE_SIZE);
  buff->d_start = 0;
  // buff->d_len= 1;
  buff->num_chars = 0;
  buff->cur_li = 0;
  buff->msg_index = -1;
  buff->file_path = NULL;
  buff->is_saved = true;
  buff->is_readonly = false;
  buff->cursor = (Cursor) {
    .color = 0xD1D1CFFF
  };
  buff->undo_stack.top = -1;
  buff->redo_stack.top = -1;
  buff->cur_act = NULL;
  buff->lines = malloc(sizeof(Lines));
  buff->lines->data = malloc(sizeof(Line*));
  buff->lines->data[0] = (Line){0};
  buff->lines->cap = 1;
  buff->lines->len = 1;
  return buff;
}

void free_buffer(Buffer *buff){
  free_str(buff->s);
  free(buff);
  buff = NULL;
}

void get_date_time(char *time_buff, size_t size){
  time_t current_time;
  time(&current_time);
  struct tm *local_time = localtime(&current_time);
  strftime(time_buff, size, "%H:%M:%S", local_time);
}

void redo(Editor *e){
  Action *last_action = action_stack_pop(&cur_buf->redo_stack);
  if(last_action == NULL) return;
  action_stack_push(&cur_buf->undo_stack, *last_action);
  redo_action(e, last_action);
  update_scroll(e, false, false);
  cur_buf->cursor.last_time_moved = GetTime();
}

void new_message(Editor *e, const char *message, MessageType type){
  if(e->msgs.len >= MAX_MESSAGES) {
    for(int i = 0; i < e->msgs.len; i++){
      e->msgs.data[i] = e->msgs.data[i + 1];
      e->msgs.len--;
    }
  }
  char display_msg[1024];
  char time_buff[128];
  get_date_time(time_buff, sizeof(time_buff));

  snprintf(display_msg, sizeof(display_msg), "%s - %s", message, time_buff);
  Message msg = {
    .type = type,
    .text = strdup(message)
  };
  write_new_message(e, &msg);
  e->msgs.data[e->msgs.len] = msg;
  e->buffs->data[e->current_buff]->msg_index = e->msgs.len++;
}

/* void lines_append(Lines *lines, Line line){
  if(lines->len >= lines->cap){
    lines->cap *= 2;
    lines->data = realloc(lines->data, sizeof(Line) * lines->cap);
  }
  lines->data[lines->len++] = line;
} */

void editor_append_buf(Editor *e, Buffer *buff){
  /* if(e->buffs->len >= e->buffs->cap){
    e->buffs->cap *= 2;
    e->buffs->datadata = realloc(lines->data, sizeof(Line) * lines->cap);
  }
  lines->data[lines->len++] = line; */
}

void realloc_editor_buffers(Editor *e){
  e->buffs->cap += 5;
  e->buffs->data = realloc(e->buffs->data, sizeof(Buffer*) * e->buffs->cap);
  for(size_t i = e->buffs->len; i < e->buffs->cap; i++){
    e->buffs->data[i] = new_buffer();
  }
}

Editor *init_editor(){
  Editor *e = malloc(sizeof(Editor));
  e->prompt = new_str(DEFAULT_LINE_SIZE);

  e->buffs = malloc(sizeof(Buffers));
  e->buffs->data = malloc(sizeof(Buffer*));
  e->buffs->data[0] = new_buffer();
  e->buffs->len = 1;
  e->buffs->cap = 1;

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
    .lines_color = 0x545454FF,
    .line_numbers_color = 0x828282FF,
    .file_name_color = 0x828282FF,
    .status_line_color = 0x000000FF,
    .error_color = 0xFF4C24FF,
    .success_color = 0x00B014FF,
    .selected_char_color = 0xE8E8E8FF,
    .selection_color = 0x383838FF,
    .line_highlight_color = 0x383737FF,
    .is_menu_open = false,
    .is_vim_mode = true,
    .is_line_highlight = true,
    .is_spaces_for_tabs = true,
    .caps_lock_as_escape = true,
    .tab_size = 2,
    .is_showing_lines = false,
    .ln_mode = NONE,
    .ln_padding = 1,
    // .line_height = 0,
    // .letter_spacing = 0,
    .padding = {
      .top = 45,
      .bottom = 45,
      .right = 70,
      .left = 10
    },
    .scroll_pad = 4
  },
  e->HOME_DIR  = getenv("HOME");

  return e;
}
