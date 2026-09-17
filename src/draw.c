#include <string.h>
#include <ctype.h>
#include <math.h>
#include "files.h"

#define HOLD_PRESS_DELAY 0.02f
Vector2 press_start_pos = {0};
double last_press_time = 0.0f;

const char *get_mode_str(Mode mode, bool is_selecting){
  switch (mode) {
    case NORMAL :
      if(is_selecting) return "VISUAL" ;
      return "NORMAL";
    case INSERT :
      return "INSERT";
  }
}

void DrawCmdCursor(Editor* e, int cursor_x, int cursor_y){
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  DrawRectangle(
    cursor_x, cursor_y, char_size.col,
    char_size.row, GetColor(e->buffs->data[e->current_buff]->cursor.color)
  );
}

void DrawCursor(Editor* e, int x, int y, unsigned int color){
  DrawRectangle(
    x, y, e->buffs->data[e->current_buff]->cursor.width,
    e->buffs->data[e->current_buff]->cursor.height, GetColor(color)
  );
}

void DrawMenu(Editor * e){
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  Padding pad = e->conf.padding;
    
  float menu_w = (float) e->s_width / 2;
  float menu_h = (float) e->s_height / 2;
  Rectangle menu_rec = {
    (float) e->s_width / 2 - menu_w / 2,
    (float) e->s_height / 2 - menu_h / 2,
    menu_w, menu_h
  };
  Rectangle search_underline = {
    menu_rec.x, menu_rec.y + 2 * char_size.row,
    menu_rec.width, 2
  };

  DrawRectangleRec(menu_rec, GetColor(e->conf.bg_color));

  DrawRectangleLinesEx(menu_rec, 2, GRAY);

  DrawRectangleRec(search_underline, GRAY);


  float char_w = MeasureTextEx(e->conf.font_data.font, "c", e->conf.font_secondary_data.size, 0).x;
  int text_end_x = (float) e->s_width / 4 + 2 * (e->conf.letter_spacing + char_size.col)
  + ((e->conf.letter_spacing + char_size.col) * e->prompt->len);
  int cursor_x = (float) e->s_width / 4 + (2 + e->prompt->len) * (e->conf.letter_spacing + char_size.col);
  if(cursor_x > menu_rec.x + menu_rec.width - 2 * (e->conf.letter_spacing + char_size.col)){
    cursor_x = menu_rec.x + menu_rec.width - 2 * (e->conf.letter_spacing + char_size.col);
  }
  int cursor_y = (float) (e->s_height + 2 * char_size.row) / 4;
  int max_displayed = (menu_rec.width - 4 * (e->conf.letter_spacing + char_size.col))
    / (e->conf.letter_spacing + char_size.col);

  if(!e->prompt->len) {
    DrawTextEx(
      e->conf.font_secondary_data.font, "...",
      (Vector2){
        (float) e->s_width / 4 + 3 * char_size.col,
        cursor_y
      },
      e->conf.font_secondary_data.size, 0, GRAY
    );
  }
  DrawCmdCursor(e, cursor_x, cursor_y);

  for(
    int i = (e->prompt->len > max_displayed ? e->prompt->len - max_displayed : 0);
    i < e->prompt->len;
    i++
  ){
    DrawTextCodepoint(
      e->conf.font_secondary_data.font, e->prompt->data[i],
      (Vector2){
        e->s_width / 4 + 2 * (e->conf.letter_spacing + char_size.col) +
        (e->prompt->len > max_displayed ? i - e->prompt->len + max_displayed : i)
        * (e->conf.letter_spacing + char_size.col),
        cursor_y,
      },
      e->conf.font_secondary_data.size,
      GetColor(e->conf.text_color)
    );
  }

  for(int i = 0; i < e->num_cmds_displayed; i++){
    Cmd current_cmd = default_cmds[e->displayed_cmds[i]];
    float text_width = MeasureTextEx(e->conf.font_data.font, current_cmd.text, e->conf.font_secondary_data.size, 0).x;

    Rectangle cmd_box = {
      menu_rec.x + 2, search_underline.y + i * (char_size.row * 1.7) + 2,
      menu_rec.width - 4, char_size.row * 1.7
    };
    if(cmd_box.y + cmd_box.height < menu_rec.y + menu_rec.height){
      size_t max = (menu_rec.height - (search_underline.y - menu_rec.y)) / (char_size.row * 1.7);
      if(max < e->num_cmds_displayed) e->num_cmds_displayed = max;
    }
    if(i == e->selected_cmd)
      DrawRectangleRec(cmd_box, GetColor(0x333738FF));

    Color txt_color = i == e->selected_cmd ? WHITE : GRAY;
    float txt_y_pos = search_underline.y + 15 + i * (char_size.row * 1.7);


    DrawTextEx(
      e->conf.font_secondary_data.font, current_cmd.text,
      (Vector2){ menu_rec.x + 2 * char_size.col, txt_y_pos }, 
      e->conf.font_secondary_data.size, 0, txt_color 
    );
  }
}

unsigned int get_msg_color(Editor *e, MessageType type){
  switch (type) {
    case ERROR:
      return e->conf.error_color;
    case GOOD:
      return e->conf.success_color;
    case INFO:
      return e->conf.file_name_color;
  }
}

void DrawCurrentMessage(Editor *e) {
  Buffer *buff = e->buffs->data[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  if(buff->current_msg_index > e->num_msgs - 1) return;
  Message *msg = e->messages[buff->current_msg_index];
  if(!msg || !strlen(msg->text)) return;
  Color color = GetColor(get_msg_color(e, msg->type));
  DrawTextEx(
    e->conf.font_secondary_data.font, msg->text,
    (Vector2){
      e->conf.padding.left, 
      e->s_height - (char_size.row + 4)
    },
    e->conf.font_secondary_data.size, 0, color
  ); 
}

void DrawStatusLine(Editor *e){
  Buffer *buff = e->buffs->data[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);

  Rectangle status_line = {
    0, e->s_height - char_size.row, e->s_width, char_size.row
  };

  DrawRectangleRec(status_line, GetColor(e->conf.status_line_color));

  if(buff->current_msg_index > -1) return;

  if(e->conf.is_vim_mode){
    DrawTextEx(
      e->conf.font_secondary_data.font, get_mode_str(e->mode, e->conf.is_selecting),
      (Vector2){
        e->s_width - (e->conf.letter_spacing + char_size.col) * 7,
        status_line.y
      },
      e->conf.font_secondary_data.size, 0, GRAY
    );
  }

  Line cur_line = buff->lines->data[buff->cur_li];
  int row_span = get_digit_count(buff->cur_li + 1);
  int col_span = get_digit_count(buff->cursor.index - cur_line.start + 1);
  int row_col_span = row_span + col_span + 1;

  DrawTextEx(
    e->conf.font_secondary_data.font,
    TextFormat(
      "%d:%d", 
      buff->cur_li + 1,
      buff->cursor.index - cur_line.start + 1
    ),
    (Vector2){
      e->s_width - (e->conf.letter_spacing + char_size.col) * (8 + row_col_span),
      status_line.y
    },
    e->conf.font_secondary_data.size, 0, GRAY
  );

  DrawTextEx(
    e->conf.font_secondary_data.font, e->buffs->data[e->current_buff]->file_path ? 
    get_file_name_from_path(e->buffs->data[e->current_buff]->file_path) : "Untitled",
    (Vector2){
      e->conf.padding.left,
      status_line.y
    },
    e->conf.font_secondary_data.size, 0, GetColor(e->conf.file_name_color)
  );
}

RowCol get_char_size(float font_size){
  float char_base_widh = 0.453125;
  return (RowCol) {
    .col = char_base_widh * font_size,
    .row = font_size
  };
}

void DrawChar(Editor *e, int c, int x_pos, int y_pos, unsigned int color, float font_size){
  DrawTextCodepoint(e->conf.font_data.font, c, (Vector2){x_pos, y_pos}, font_size, GetColor(color));
}

void DrawEditorLines(Editor *e){
  Buffer *buff = e->buffs->data[e->current_buff];
  Padding pad = e->conf.padding;
  RowCol char_size = get_char_size(e->conf.font_data.size);

  for (int l = 0; l < get_max_num_lines(e); l++) {
    int y_pos = pad.top + buff->cursor.height + l * (e->conf.line_height + buff->cursor.height);
    int x_pos = e->conf.padding.left + (e->conf.ln_padding + 1) * (char_size.col + e->conf.letter_spacing);
    DrawLineEx(
      (Vector2){x_pos, y_pos}, 
      (Vector2){e->s_width - (e->conf.padding.right), y_pos}, 3.0f,
      GetColor(e->conf.lines_color)
    );
  }
}

void DrawLineNumber(Editor *e, size_t i, size_t y_offset){
  RowCol char_size = get_char_size(e->conf.font_data.size);
  Padding pad = e->conf.padding;
  size_t index = e->buffs->data[e->current_buff]->cur_li;
  DrawTextEx(e->conf.font_data.font,
    TextFormat( "%zu", 
      (e->conf.ln_mode == ABSOLUTE || index == i) ?  i + 1 :
      (index < i ? i - index : index - i)
    ),
    (Vector2){
      pad.left,
      pad.top + (e->conf.line_height + char_size.row) * y_offset
     },
    e->conf.font_data.size, 0, GetColor(e->conf.line_numbers_color));
}

void DrawBufferText(Editor *e, bool is_blinking){
  Buffer *buff = e->buffs->data[e->current_buff];
  Padding pad = e->conf.padding;
  RowCol char_size = get_char_size(e->conf.font_data.size);
  bool has_nums = e->conf.ln_mode != NONE;
  size_t i, index = 0, x_offset = 0, y_offset = 0, num_cariages = 0;
  size_t total_char_w = e->conf.letter_spacing + char_size.col;
  size_t total_char_h = e->conf.line_height + char_size.row;
  size_t total_pl = pad.left;
  if(has_nums) total_pl += (e->conf.ln_padding + 1) * total_char_w;
  size_t total_pt = pad.top;
  size_t max_len = get_max_line_length(e);
  int cursor_x_offset = buff->cursor.index % max_len;
  int cursor_y_offset = buff->cursor.index / max_len;
  Vector2 char_pos = {0};
  bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
  bool mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
  float wheel_delta_y = GetMouseWheelMove();

  size_t max_num_lines = get_max_num_lines(e);
  size_t first_gui_index = get_line_from_index(buff->lines, buff->d_start);

  if(wheel_delta_y > 0) {
    scroll_down(e, 1);
    if(buff->cur_li > first_gui_index + max_num_lines - e->conf.scroll_pad) {
      buff->cursor.index = buff->lines->data[
        first_gui_index + max_num_lines - e->conf.scroll_pad
      ].start;
    }
    update_lines(e);
    update_buf_state(e);
  } 
  else if(wheel_delta_y < 0) {
    scroll_up(e, 1);
    if(buff->cur_li < first_gui_index + e->conf.scroll_pad){
      buff->cursor.index = buff->lines->data[first_gui_index + e->conf.scroll_pad].start;
    } 
    update_lines(e);
    update_buf_state(e);
  } 
  
  size_t total_wraps =
    get_lines_wraps(e, first_gui_index, first_gui_index + max_num_lines);
  size_t cur_line_index = first_gui_index;
  char *text = buff->s->data;

  if(mouse_down && e->conf.is_selecting) {
    if(e->mouse.y < total_pt + total_char_h * e->conf.scroll_pad){
      double now = GetTime();
      if(now - last_press_time > HOLD_PRESS_DELAY) {
        scroll_down(e, 1);
        last_press_time = now;
      }
    }
    else if(e->mouse.y > e->s_height - pad.bottom - e->conf.scroll_pad * total_char_h) {
      double now = GetTime();
      if(now - last_press_time > HOLD_PRESS_DELAY) {
        scroll_up(e, 1);
        last_press_time = now;
      }
    }
  }

  float delta_x = e->mouse.x - press_start_pos.x;
  float delta_y = e->mouse.y - press_start_pos.y;
  bool mouse_dragged = (delta_x * delta_x + delta_y * delta_y) > 25.0f;

  bool matched_char = false;
  size_t closest_char_index = 0;
  float closest_y = 10000.0f;
  size_t closest_x = 10000.0f;

  size_t char_index;

  if(e->conf.ln_mode != NONE) DrawLineNumber(e, first_gui_index, y_offset);
  for (
    i = buff->d_start;
    (i <= buff->s->len) && 
    (cur_line_index < first_gui_index + max_num_lines + 1);
    i++
  ) { 

    // buff = e->buffs->data[e->current_buff];
    char c = text[i];

    char_index = index + buff->d_start;
    char_pos = (Vector2) {
      total_pl + x_offset * total_char_w,
      total_pt + y_offset * total_char_h,
    };

    if(char_index == buff->cursor.index){
      if(text[char_index] == '\n' || char_index == buff->s->len){
        DrawCursor(
          e, char_pos.x, char_pos.y,
          is_blinking ? 0x00000000 : buff->cursor.color
        );

      }
    }else if(is_selected(e, char_index) && text[char_index - 1] == '\n') {
      DrawCursor(e, char_pos.x, char_pos.y, e->conf.selection_color);
    }
    if(i == buff->s->len) break;

    if((mouse_clicked || mouse_down) && !matched_char){
      if(mouse_clicked) {
        e->conf.is_selecting = false;
        press_start_pos = e->mouse;
      }
      e->conf.is_selecting = mouse_dragged;

      bool is_y_match = e->mouse.y >= char_pos.y && e->mouse.y <= (char_pos.y + total_char_h);
      bool is_x_match = e->mouse.x >= char_pos.x && e->mouse.x <= (char_pos.x + total_char_w);
      bool is_y_big   = e->mouse.y > char_pos.y + total_char_h;
      bool is_x_big   = e->mouse.x > char_pos.x + total_char_w;
      bool is_y_small = e->mouse.y < char_pos.y;
      bool is_x_small = e->mouse.x < char_pos.x; 
      float distance_x = fabsf(char_pos.x - e->mouse.x);
      float distance_y = fabsf(char_pos.y - e->mouse.y);
      size_t line_index = get_line_from_index(buff->lines, char_index);
      Line line = buff->lines->data[line_index];

      if(is_x_match && is_y_match){
        matched_char = true;
        handle_mouse_click(e, char_index, mouse_dragged);
      }else if(is_y_match){
        if(is_x_small){
          if(char_index == line.start){
            matched_char = true;
            handle_mouse_click(e, line.start, mouse_dragged);
          }else {
            if(distance_x < closest_x){
               matched_char = true;
              closest_x = distance_x;
              closest_char_index = char_index;
            } 
          }
        }else if (is_x_big){
          if(index == line.end){
            if(line.end - line.start < 1){
              handle_mouse_click(e, line.start, mouse_dragged);
            }else {
              handle_mouse_click(e, line.end - 1, mouse_dragged);
            }
            matched_char = true;
          }else {
            if(distance_x < closest_x){
              closest_x = distance_x;
              closest_char_index = char_index;
            } 
          }
        }
      }else if(is_y_small && line_index == 0){
        if(is_x_match) {
          matched_char = true;
          handle_mouse_click(e, char_index, mouse_dragged);
        }else if(is_x_small || is_x_big){
           if(distance_x < closest_x){
             closest_x = distance_x;
             closest_char_index = char_index;
           } 
        }
      }else if(is_y_big && (line_index == buff->lines->len - 1 || line_index == first_gui_index + max_num_lines)){
        if(is_x_match){
          if(char_index >= line.start + (max_len - 1) * line.wraps) {
            matched_char = true;
            handle_mouse_click(e, char_index, mouse_dragged);
          }
        }else if(is_x_small){
          if(char_index > line.start + max_len * line.wraps - 1) {
            matched_char = true;
            handle_mouse_click(e, line.start + max_len * line.wraps, mouse_dragged);
          }
        }else if(is_x_big && char_index == line.end - 1){
          matched_char = true;
          handle_mouse_click(e, line.end - 1, mouse_dragged);
        }
      } 
    }

    if(c == '\n'){
      y_offset++;
      x_offset = 0;
      index++;
      if(e->conf.ln_mode != NONE) DrawLineNumber(e, ++cur_line_index, y_offset);
      continue;
    }
    if(c == '\r') {
      memmove(&text[i], &text[i + 1], buff->s->len - i);
      num_cariages++;
    }
    else {
      index++;
    }

    if((x_offset) >= max_len) {
      y_offset++;
      x_offset = 0;
    }
    else if(isspace(text[i - 1]) || i == 1){
      int word_len = 0;
      while(!isspace(text[i + word_len++]));
      if((x_offset + word_len - 1) > max_len && word_len < max_len){
        x_offset = 0;
        y_offset++;
      }
    }

    char_pos = (Vector2) {
      total_pl + x_offset * total_char_w,
      total_pt + y_offset * total_char_h,
    };

    char_index = index + buff->d_start - 1;
    if(char_index == buff->cursor.index){
      DrawCursor(
        e, char_pos.x, char_pos.y,
        is_blinking ? 0x00000000 : buff->cursor.color
      );

    }
    if(is_selected(e, char_index) && char_index != buff->cursor.index) {
      DrawCursor(e, char_pos.x , char_pos.y, e->conf.selection_color);
    }

    if(cur_line_index == buff->cur_li && e->conf.is_line_highlight) {
      Rectangle line_bg = { 
        total_pl, total_pt + total_char_h * y_offset ,
        e->s_width - (total_pl + pad.right),
        total_char_h
      };
      DrawRectangleRec(line_bg, GetColor(e->conf.line_highlight_color));
    }

    DrawTextEx(
      e->conf.font_data.font, TextFormat("%c", c),
      char_pos, e->conf.font_data.size,
      0.0f,
      GetColor(
        (char_index == buff->cursor.index && !is_blinking) ? 
        e->conf.under_cursor_color:
        e->conf.text_color
      )
    );

    x_offset++;
  }
  if((mouse_clicked || mouse_down) && !matched_char){
    handle_mouse_click(e, closest_char_index, mouse_dragged);
  }
  buff->s->len -= num_cariages;
}
