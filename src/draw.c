#include "files.h"
#include <string.h>
#include "draw.h"

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
    char_size.row, GetColor(e->cursor.color)
  );
}

void DrawCursor(Editor* e, int x, int y, unsigned int color){
  DrawRectangle(
    x, y, e->cursor.width,
    e->cursor.height, GetColor(color)
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
  Message *msg = e->messages[e->buffer.current_msg_index];
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
  if(e->buffer.current_msg_index > -1) return;

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

  DrawTextEx(
    e->conf.font_secondary_data.font, e->buffer.file_path ? 
    get_file_name_from_path(e->buffer.file_path) : "Untitled",
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

