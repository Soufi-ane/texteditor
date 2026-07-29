#include "files.h"
#include "draw.h"

const char *get_mode_str(Mode mode){
  switch (mode) {
    case NORMAL :
      return "NORMAL";
    case INSERT :
      return "INSERT";
  }
}

void DrawCmdCursor(Editor* e, int cursor_x, int cursor_y){
  DrawRectangle(
    cursor_x, cursor_y, e->cursor.width,
    e->cursor.height, GetColor(e->cursor.color)
  );
}

void DrawCursor(Editor* e, int x, int y){
  DrawRectangle(
    x, y, e->cursor.width,
    e->cursor.height, GetColor(e->cursor.color)
  );
}

void DrawMenu(Editor * e){
  Padding pad = e->conf.padding;
    
  float menu_w = (float) e->s_width / 2;
  float menu_h = (float) e->s_height / 2;
  Rectangle menu_rec = {
    (float) e->s_width / 2 - menu_w / 2,
    (float) e->s_height / 2 - menu_h / 2,
    menu_w, menu_h
  };
  Rectangle search_underline = {
    menu_rec.x, menu_rec.y + 2 * e->cursor.height,
    menu_rec.width, 2
  };

  DrawRectangleRec(menu_rec, GetColor(e->conf.bg_color));

  DrawRectangleLinesEx(menu_rec, 2, GRAY);

  DrawRectangleRec(search_underline, GRAY);

  float char_w = MeasureTextEx(e->conf.font, "c", 32, 0).x;
  int text_end_x = (float) e->s_width / 4 + 2 * e->conf.letter_spacing
  + (e->conf.letter_spacing * e->cmd_prompt->length);
  int cursor_x = (float) e->s_width / 4 + (2 + e->cmd_prompt->length) * e->conf.letter_spacing;
  if(cursor_x > menu_rec.x + menu_rec.width - 2 * e->conf.letter_spacing){
    cursor_x = menu_rec.x + menu_rec.width - 2 * e->conf.letter_spacing;
  }
  int cursor_y = (float) (e->s_height + 2 * e->cursor.height) / 4;
  int max_displayed = (menu_rec.width - 4 * e->conf.letter_spacing) / e->conf.letter_spacing;

  if(!e->cmd_prompt->length) {
    DrawTextEx(
      e->conf.font, "...",
      (Vector2){
        (float) e->s_width / 4 + 3 * e->cursor.width,
        cursor_y
      },
      32, 0, GRAY
    );
  }
  DrawCmdCursor(e, cursor_x, cursor_y);

  for(
    int i = (e->cmd_prompt->length > max_displayed ? e->cmd_prompt->length - max_displayed : 0);
    i < e->cmd_prompt->length;
    i++
  ){
    DrawChar(
      e, e->cmd_prompt->chars[i],
      e->s_width / 4 + 2 * e->conf.letter_spacing + (e->cmd_prompt->length > max_displayed ?
        i - e->cmd_prompt->length + max_displayed : i) * e->conf.letter_spacing,
      cursor_y, e->conf.text_color
    );
  }

  for(int i = 0; i < e->num_cmds_displayed; i++){
    Cmd current_cmd = default_cmds[e->displayed_cmds[i]];
    float text_width = MeasureTextEx(e->conf.font, current_cmd.text, 32, 0).x;

    Rectangle cmd_box = {
      menu_rec.x + 2, search_underline.y + i * (e->conf.line_height + 10) + 2,
      menu_rec.width - 4, e->conf.line_height * 1.3
    };
    if(cmd_box.y + cmd_box.height < menu_rec.y + menu_rec.height){
      size_t max = (menu_rec.height - (search_underline.y - menu_rec.y)) / (e->conf.line_height * 1.3);
      if(max < e->num_cmds_displayed) e->num_cmds_displayed = max;
    }
    if(i == e->selected_cmd)
      DrawRectangleRec(cmd_box, GetColor(0x333738FF));

    DrawTextEx(
       e->conf.font, current_cmd.text,
      (Vector2){
        menu_rec.x + 2 * e->cursor.width,
        search_underline.y + 15 + i * (e->conf.line_height + 10)
      },
      32, 0, i == e->selected_cmd ? WHITE : GRAY
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
  Message *msg = e->messages[e->buffer.current_msg_index];
  Color color = GetColor(get_msg_color(e, msg->type));
  DrawTextEx(
    e->conf.font, msg->text,
    (Vector2){
      e->conf.padding.left, 
      e->s_height - e->cursor.height 
    },
    32, 0, color
  ); 
}

void DrawStatusLine(Editor *e){
  int sline_y = e->s_height - e->cursor.height;
  DrawRectangle(0, sline_y, e->s_width, e->cursor.height, GetColor(e->conf.status_line_color));
  if(e->buffer.current_msg_index > -1) return;
  DrawTextEx(
    e->conf.font, get_mode_str(e->mode),
    (Vector2){
      e->s_width - e->conf.letter_spacing * 7,
      e->s_height - e->cursor.height
    },
    32, 0, GRAY
  );
  DrawTextEx(
    e->conf.font, e->buffer.file_path ? 
    get_file_name_from_path(e->buffer.file_path) : "Untitled",
    (Vector2){
      e->conf.padding.left,
      e->s_height - e->cursor.height
    },
    32, 0, GetColor(e->conf.file_name_color)
  );
}

void DrawChar(Editor *e, int c, int x_pos, int y_pos, unsigned int color){
  Padding pad = e->conf.padding;
  DrawTextCodepoint(
    e->conf.font, c,
    (Vector2){x_pos, y_pos}, 32,
    GetColor(color)
  );
}

