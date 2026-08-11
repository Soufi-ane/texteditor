#include "files.h"
#include <string.h>
#include "draw.h"

Vector2 press_start_pos = {0};

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
    char_size.row, GetColor(e->buffers[e->current_buff]->cursor.color)
  );
}

void DrawCursor(Editor* e, int x, int y, unsigned int color){
  DrawRectangle(
    x, y, e->buffers[e->current_buff]->cursor.width,
    e->buffers[e->current_buff]->cursor.height, GetColor(color)
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
  + ((e->conf.letter_spacing + char_size.col) * e->cmd_prompt->length);
  int cursor_x = (float) e->s_width / 4 + (2 + e->cmd_prompt->length) * (e->conf.letter_spacing + char_size.col);
  if(cursor_x > menu_rec.x + menu_rec.width - 2 * (e->conf.letter_spacing + char_size.col)){
    cursor_x = menu_rec.x + menu_rec.width - 2 * (e->conf.letter_spacing + char_size.col);
  }
  int cursor_y = (float) (e->s_height + 2 * char_size.row) / 4;
  int max_displayed = (menu_rec.width - 4 * (e->conf.letter_spacing + char_size.col))
    / (e->conf.letter_spacing + char_size.col);

  if(!e->cmd_prompt->length) {
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
    int i = (e->cmd_prompt->length > max_displayed ? e->cmd_prompt->length - max_displayed : 0);
    i < e->cmd_prompt->length;
    i++
  ){
    DrawTextCodepoint(
      e->conf.font_secondary_data.font, e->cmd_prompt->chars[i],
      (Vector2){
        e->s_width / 4 + 2 * (e->conf.letter_spacing + char_size.col) +
        (e->cmd_prompt->length > max_displayed ? i - e->cmd_prompt->length + max_displayed : i)
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
  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  Message *msg = e->messages[e->buffers[e->current_buff]->current_msg_index];
  if(!strlen(msg->text)) return;
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
  int sline_y = e->s_height - 45;
  DrawRectangle(0, sline_y, e->s_width, 45, GetColor(e->conf.status_line_color));
  if(e->buffers[e->current_buff]->current_msg_index > -1) return;

  RowCol char_size = get_char_size(e->conf.font_secondary_data.size);
  if(e->conf.is_vim_mode){
    DrawTextEx(
      e->conf.font_secondary_data.font, get_mode_str(e->mode, e->conf.is_selecting),
      (Vector2){
        e->s_width - (e->conf.letter_spacing + char_size.col) * 7,
        e->s_height - 45
      },
      e->conf.font_secondary_data.size, 0, GRAY
    );
  }

  Buffer *buff = e->buffers[e->current_buff];

  int row_span = get_digit_count(buff->current_line_index + 1);
  int col_span = get_digit_count(buff->cursor.index + 1);
  int row_col_span = row_span + col_span + 1;

  DrawTextEx(
    e->conf.font_secondary_data.font,
    TextFormat("%d:%d", buff->current_line_index + 1, buff->cursor.index + 1),
    (Vector2){
      e->s_width - (e->conf.letter_spacing + char_size.col) * (8 + row_col_span),
      e->s_height - 45
    },
    e->conf.font_secondary_data.size, 0, GRAY
  );

  DrawTextEx(
    e->conf.font_secondary_data.font, e->buffers[e->current_buff]->file_path ? 
    get_file_name_from_path(e->buffers[e->current_buff]->file_path) : "Untitled",
    (Vector2){
      e->conf.padding.left,
      e->s_height - 45
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
  Buffer *buff = e->buffers[e->current_buff];
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

void DrawLineNumber(Editor *e, ssize_t i, ssize_t y_offset){
  RowCol char_size = get_char_size(e->conf.font_data.size);
  Padding pad = e->conf.padding;
  ssize_t index = e->buffers[e->current_buff]->current_line_index;
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

void DrawLineChars(Editor *e, bool is_blinking, ssize_t x_offset, ssize_t y_offset, ssize_t i){
  float char_x;
  float char_y;
  Buffer *buff = e->buffers[e->current_buff];
  RowCol char_size = get_char_size(e->conf.font_data.size);
  Padding pad = e->conf.padding;
  ssize_t max_line_len = get_max_line_length(e);
  Line *current_line = buff->lines[i];
  for(ssize_t m = 0; m <= current_line->length; m++){
    float cursor_x = pad.left + (e->conf.letter_spacing + char_size.col) * x_offset;
    if(e->conf.ln_mode != NONE) cursor_x += (e->conf.letter_spacing + char_size.col) * (e->conf.ln_padding + 1);
    if(i == buff->current_line_index && m == buff->cursor.index){
      DrawCursor(
        e, cursor_x,
        pad.top + ((e->conf.line_height + char_size.row) * y_offset),
        is_blinking ? 0x00000000 : buff->cursor.color
      );
    }

    char_x = pad.left + (e->conf.letter_spacing + char_size.col) * x_offset;
    char_y = pad.top + ((e->conf.line_height + char_size.row) * y_offset);
    if(e->conf.ln_mode != NONE) char_x += (e->conf.letter_spacing + char_size.col) * (e->conf.ln_padding + 1);
    if(m < current_line->length) {
      char c = current_line->chars[m];
      unsigned int char_color = (m == buff->cursor.index &&
        i == buff->current_line_index && !is_blinking) ? 
        e->conf.under_cursor_color : e->conf.text_color;
      if(c == '\t') {
        for(int i = 0; i < e->conf.tab_size; i++){
          DrawChar(
            e, ' ',char_x + i * (e->conf.letter_spacing + char_size.col),
            char_y , char_color, e->conf.font_data.size
          );
          x_offset++;
        }
      } else {
        if(is_selected(e, (RowCol){i, m}) && 
            !(i == buff->current_line_index && m == buff->cursor.index)) {
          DrawCursor(e, char_x, char_y, e->conf.selection_color);
          DrawChar(e, c,char_x, char_y , e->conf.selected_char_color, e->conf.font_data.size);
        } else {
          DrawChar(e, c,char_x, char_y , char_color, e->conf.font_data.size);
        } 
      } 

      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        e->conf.is_selecting = false;
        press_start_pos = e->mouse;
        handle_mouse_click(e, char_x, char_y, m, i, false);
      }
      if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        float delta_x = e->mouse.x - press_start_pos.x;
        float delta_y = e->mouse.y - press_start_pos.y;
        float distance_squared = delta_x * delta_x + delta_y * delta_y;
        if(distance_squared > 25.0f) {
          e->conf.is_selecting = true;
          e->mode = NORMAL;
          handle_mouse_click(e, char_x, char_y, m, i, true);
        }
      }

      x_offset++;
      if((m + 1) % max_line_len == 0) {
        y_offset++;
        x_offset = 0;
      }
    }else if(!current_line->length) {
      if(is_selected(e, (RowCol){i, m}) && 
          !(i == buff->current_line_index && m == buff->cursor.index)) {
        DrawCursor(e, char_x, char_y, e->conf.selection_color);
      }
    }
  }
}

