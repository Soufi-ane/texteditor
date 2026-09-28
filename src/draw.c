#include <string.h>
#include <ctype.h>
#include <math.h>
#include "io.h"

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

void DrawCursor(Editor* e, int x, int y, unsigned int color, bool is_primary){
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  DrawRectangle(
    x, y, 
    is_primary ? e->buffs->data[e->current_buff]->cursor.width
    : char_size.col,
    is_primary ? e->buffs->data[e->current_buff]->cursor.height
    : char_size.row,
    GetColor(color)
  );
}

void DrawMenu(Editor * e){
  Buffer *buff = e->buffs->data[e->current_buff];
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

  DrawCursor(e, cursor_x, cursor_y, buff->cursor.color, false);
  // DrawCmdCursor(e, cursor_x, cursor_y);

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
      return e->conf.status_line_fg;
  }
}

void DrawCurrentMessage(Editor *e) {
  Buffer *buff = e->buffs->data[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  if(buff->msg_index > e->msgs.len - 1) return;
  Message msg = e->msgs.data[buff->msg_index];
  if(!strlen(msg.text)) return;
  Color color = GetColor(get_msg_color(e, msg.type));
  DrawTextEx(
    e->conf.font_secondary_data.font, msg.text,
    (Vector2){
      e->conf.padding.left, 
      e->s_height - (char_size.row + 4)
    },
    e->conf.font_secondary_data.size, 0, color
  ); 
}

void DrawStatusLine(Editor *e, bool is_blinking){
  Buffer *buff = e->buffs->data[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  Padding pad = e->conf.padding;
  Rectangle status_line = {
    0, e->s_height - char_size.row, e->s_width, char_size.row
  };

  DrawRectangleRec(status_line, GetColor(e->conf.status_line_bg));

  if(buff->msg_index > -1) return;

  if(e->conf.is_vim_mode){
    DrawTextEx(
      e->conf.font_secondary_data.font, get_mode_str(e->mode, e->conf.is_selecting),
      (Vector2){
        e->s_width - (e->conf.letter_spacing + char_size.col) * 7,
        status_line.y
      },
      e->conf.font_secondary_data.size, 
      0, GetColor(e->conf.status_line_fg)
    );
  }

  Line cur_line = buff->lines->data[buff->cur_li];
  int row_span = get_digit_count(buff->num_prev_lines + buff->cur_li + 1);
  int col_span = get_digit_count(buff->cursor.index - cur_line.start + 1);
  int row_col_span = row_span + col_span + 1;
  float row_col_x = e->s_width - (e->conf.letter_spacing + char_size.col) * (8 + row_col_span);

  DrawTextEx(
    e->conf.font_secondary_data.font,
    TextFormat(
      "%d:%d", 
      buff->num_prev_lines + buff->cur_li + 1,
      buff->cursor.index - cur_line.start + 1
    ),
    (Vector2){ row_col_x, status_line.y },
    e->conf.font_secondary_data.size, 
    0, GetColor(e->conf.status_line_fg)
  );

  if(e->is_searching){
    size_t text_end = pad.left + (e->query->len + 1) * char_size.col;
    size_t search_d_start = text_end >= (row_col_x - char_size.col) ? 
      (text_end - row_col_x) / char_size.col + 1 : 0;
    DrawTextCodepoint(
      e->conf.font_data.font, '/',
      (Vector2){pad.left, status_line.y}, e->conf.font_data.size,
      GetColor(e->conf.status_line_fg)
    );
    DrawCursor(
      e, text_end - search_d_start * char_size.col,
      status_line.y,
      is_blinking ? 0x00000000 : buff->cursor.color, false
    );
    DrawTextEx(
      e->conf.font_secondary_data.font, &e->query->data[search_d_start],
      (Vector2){ pad.left + char_size.col , status_line.y },
      e->conf.font_secondary_data.size, 0, GetColor(e->conf.status_line_fg)
    );
  }else {
    DrawTextEx(
      e->conf.font_secondary_data.font, e->buffs->data[e->current_buff]->file_path ? 
      get_file_name_from_path(e->buffs->data[e->current_buff]->file_path) : "Untitled",
      (Vector2){ pad.left, status_line.y },
      e->conf.font_secondary_data.size, 0, GetColor(e->conf.status_line_fg)
    );
  }
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

void udpate_explorer_size(Editor *e){
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  size_t max_num_lines = get_max_num_lines(e);
  e->exp->max_w = e->s_width / 3;
  for(
    size_t i = e->exp->files->d_start;
    (i < e->exp->files->len) && (i - e->exp->files->d_start < max_num_lines);
    i++
  ){
    File file = e->exp->files->data[i];
    size_t text_w = file.name->len * char_size.col;
    if(text_w > e->exp->max_w){
      if(text_w < (2 * e->s_width / 3)){
        e->exp->max_w = text_w + e->conf.padding.left * 2;
      }else e->exp->max_w = 2 * e->s_width / 3;
    }
  }
}

void DrawFileInput(Editor *e, bool is_blinking){
  Buffer *buff = e->buffs->data[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  float input_w = e->s_width / 2;
  float input_h = char_size.row * 2;
  Rectangle input = {
    e->s_width / 2 - input_w / 2,
    e->s_height / 5, input_w, input_h
  };
  DrawRectangleRec(input, GetColor(e->conf.bg_color));
  DrawRectangleLinesEx(input, 2, GetColor(e->conf.text_color));
  size_t text_x = input.x + (e->exp->input->len + 1) * char_size.col;
  size_t d_start = text_x >= (input.x + input.width) - char_size.col * 3 ?
    (text_x - input.x - input_w + char_size.col * 3) / char_size.col : 0;

  float label_w = MeasureTextEx(
    e->conf.font_secondary_data.font, e->exp->label->data, 
    e->conf.font_secondary_data.size, 0
  ).x;

  Rectangle label_box = {
    input.x, input.y - char_size.row * 1.5,
    input_w, char_size.row * 1.5
  };

  DrawRectangleRec(label_box, GetColor(e->conf.bg_color));
  DrawTextEx(
    e->conf.font_secondary_data.font, e->exp->label->data,
    (Vector2){
      e->s_width / 2 - label_w / 2,
      input.y - char_size.row * 1.25
    },
    e->conf.font_secondary_data.size, 0,
    GetColor(e->conf.text_color)
  );

  DrawCursor(
    e, text_x - d_start * char_size.col,
    input.y + char_size.row / 2,
    is_blinking ? 0x00000000 : buff->cursor.color, false
  );
  if(e->exp->input->len){
    DrawTextEx(
      e->conf.font_secondary_data.font, &e->exp->input->data[d_start],
      (Vector2){
        input.x + char_size.col,
        input.y + char_size.row / 2
      },
      e->conf.font_secondary_data.size, 0,
      GetColor(e->conf.text_color)
    );
  }else {
    DrawTextCodepoint(
      e->conf.font_secondary_data.font, e->exp->placeholder->data[0],
      (Vector2){
        input.x + char_size.col,
        input.y + char_size.row / 2
      },
      e->conf.font_secondary_data.size,
      GetColor(is_blinking ? e->conf.line_numbers_color : e->conf.under_cursor_color)
    );
    DrawTextEx(
      e->conf.font_secondary_data.font, &e->exp->placeholder->data[1],
      (Vector2){
        input.x + char_size.col * 2,
        input.y + char_size.row / 2
      },
      e->conf.font_secondary_data.size, 0,
      GetColor(e->conf.line_numbers_color)
    );
  }
}

const char *get_explorer_help_msg(Editor *e){
  if(!e->exp->files->len) return "Empty directory";
  if(e->conf.is_vim_mode) return "Use h,j,k,l to navigate";
  else return "Use arrows to navigate";
}

void DrawExplorerHelp(Editor *e){
  DrawTextEx(
    e->conf.font_secondary_data.font,
    get_explorer_help_msg(e) ,
    (Vector2){ e->conf.padding.left, 3 },
    e->conf.font_secondary_data.size, 0,
    GetColor(e->conf.line_numbers_color)
  );
}

void DrawExplorer(Editor *e, bool is_blinking){
  Padding pad = e->conf.padding;
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  Rectangle explorer = {
    0, pad.top, e->s_width / 3, e->s_height - (char_size.row + pad.top)
  };
  Rectangle separator = {
    e->exp->max_w, pad.top, 2, e->s_height - (char_size.row + pad.top)
  };
  DrawRectangleRec(explorer, GetColor(e->conf.bg_color));
  DrawRectangleRec(separator, GetColor(e->conf.line_numbers_color));
  size_t y_offset = 0;
  size_t max_num_lines = get_max_num_lines(e);
  for(
    size_t i = e->exp->files->d_start;
    (i < e->exp->files->len) && (i - e->exp->files->d_start < max_num_lines);
    i++
  ){
    File file = e->exp->files->data[i];
    String *dis_name = string(file.name->data);
    size_t text_w = file.name->len * char_size.col;
    bool is_too_long = text_w > e->exp->max_w;
    if(is_too_long){
      dis_name->len -= (pad.left * 2 + text_w - e->exp->max_w) / char_size.col + 4;
      dis_name->data[dis_name->len] = 0;
    }
    Rectangle file_rec = {
      0, y_offset * char_size.row + pad.top * 2,
      e->exp->max_w, char_size.row
    };
    DrawRectangleRec(
      file_rec,
      GetColor(i == e->exp->curr_file ? e->conf.line_highlight_color : 0x00000000)
    );

    DrawTextEx(
      e->conf.font_secondary_data.font,
      TextFormat("%s%s", dis_name->data, is_too_long ? "..." : ""),
      (Vector2){ pad.left, file_rec.y },
      e->conf.font_secondary_data.size, 0, GetColor(e->conf.text_color));
    y_offset++;
  }
  if(!e->exp->files->len){
    Rectangle file_rec = {
      0, pad.top * 2,
      e->exp->max_w, char_size.row
    };
    DrawRectangleRec(
      file_rec,
      GetColor(e->conf.line_highlight_color)
    );
  }
  if(e->exp->is_creating_file || e->exp->is_deleting){
    DrawFileInput(e, is_blinking);
  } 
  DrawExplorerHelp(e);
}

void DrawLineNumber(Editor *e, size_t n, size_t y_offset){
  RowCol char_size = get_char_size(e->conf.font_data.size);
  Padding pad = e->conf.padding;
  DrawTextEx(e->conf.font_data.font,
    TextFormat("%zu", n),
    (Vector2){
      pad.left + (e->exp->is_open ? e->exp->max_w : 0),
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
  size_t total_pl = pad.left + (e->exp->is_open ? e->exp->max_w : 0);
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

  if(wheel_delta_y > 0) {
    scroll_down(e, 1);
    if(buff->cur_li > max_num_lines - e->conf.scroll_pad) {
      buff->cursor.index = buff->lines->data[
        max_num_lines - e->conf.scroll_pad
      ].start;
    }
    update_lines(e);
    update_buf_state(e);
  } 
  else if(wheel_delta_y < 0) {
    scroll_up(e, 1);
    if(buff->cur_li < e->conf.scroll_pad){
      buff->cursor.index = buff->lines->data[e->conf.scroll_pad].start;
    } 
    update_lines(e);
    update_buf_state(e);
  } 
  
  size_t cur_gui_index = 0;
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

  if(e->conf.ln_mode != NONE){
    DrawLineNumber(
      e, e->conf.ln_mode == RELATIVE ? 
       cur_gui_index == buff->cur_li ? 1
      : buff->cur_li
      : buff->num_prev_lines + 1,
      y_offset
    );
  } 
  for (
    i = buff->d_start;
    (i <= buff->s->len) && (cur_gui_index <= max_num_lines) ;
    // && (cur_line_index < max_num_lines + 1);
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
        if(!e->exp->is_open && !e->is_searching)
        DrawCursor(
          e, char_pos.x, char_pos.y,
          is_blinking ? 0x00000000 : buff->cursor.color,
          true
        );

      }
    }else if(is_selected(e, char_index) && text[char_index - 1] == '\n') {
      if(!e->exp->is_open && !e->is_searching)
      DrawCursor(e, char_pos.x, char_pos.y, e->conf.selection_color, true);
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
      size_t line_index = cur_gui_index; //get_line_from_index(buff->lines, char_index);
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
      }else if(is_y_big && (line_index == buff->lines->len - 1 || line_index == max_num_lines)){
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
      if(e->conf.ln_mode != NONE) {
        bool is_after = ++cur_gui_index > buff->cur_li;
        size_t line_number = e->conf.ln_mode == RELATIVE 
          ? (
             is_after 
             ? (cur_gui_index - buff->cur_li)
             : (buff->cur_li - cur_gui_index)
            )
          : cur_gui_index + buff->num_prev_lines + 1;
        if(buff->cur_li == cur_gui_index && e->conf.ln_mode == RELATIVE){
          line_number = buff->num_prev_lines + buff->cur_li + 1;
        } 
        DrawLineNumber(e, line_number, y_offset);
      } 
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
    else if(i > 0 && isspace(text[i - 1]) || i == buff->d_start + 1){
      int word_len = 0;
      while(
        word_len < max_len - 1 && 
        i + word_len < buff->s->len &&
        !isspace(text[i + word_len++])
      );
      if((x_offset + word_len - 1) > max_len){
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
      if(!e->exp->is_open && !e->is_searching)
      DrawCursor(
        e, char_pos.x, char_pos.y,
        is_blinking ? 0x00000000 : buff->cursor.color,
        true
      );

    }
    if(e->is_searching){
      for(size_t x = 0; x < e->s_ranges->len; x++){
        Range r = e->s_ranges->data[x];
        if(char_index >= r.start && char_index <= r.end)
        DrawCursor(e, char_pos.x, char_pos.y, 0xFF0000FF, true);
      }
    }
    if(is_selected(e, char_index) && char_index != buff->cursor.index) {
      if(!e->exp->is_open && !e->is_searching)
      DrawCursor(e, char_pos.x , char_pos.y, e->conf.selection_color, true);
    }

    /* if(cur_line_index == buff->cur_li && e->conf.is_line_highlight) {
      Rectangle line_bg = { 
        total_pl, total_pt + total_char_h * y_offset ,
        e->s_width - (total_pl + pad.right),
        total_char_h
      };
      DrawRectangleRec(line_bg, GetColor(e->conf.line_highlight_color));
    } */

    DrawTextEx(
      e->conf.font_data.font, TextFormat("%c", c),
      char_pos, e->conf.font_data.size,
      0.0f,
      GetColor(
        ( char_index == buff->cursor.index && 
         !is_blinking && !e->exp->is_open && !e->is_searching
         ) ? 
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
