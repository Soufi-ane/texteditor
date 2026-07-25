#include "buffer.h"
#include "files.h"

const char *get_mode_str(Mode mode){
  switch (mode) {
    case NORMAL :
      return "NORMAL";
    case INSERT :
      return "INSERT";
  }
}

void DrawCursor(Editor* e,int x,int y){
  DrawRectangle(x,y, e->cursor.width, e->cursor.height, BLACK);
}

void DrawMenu(Editor * e){
  Padding pad = e->conf.padding;

  char n_text[] = "i : New file        ";
  char o_text[] = "o : Open a file     ";
  // char d_text[] = "d : Select directory";

  float n_width = MeasureTextEx(e->conf.font, n_text, 32, 0).x;
  float o_width = MeasureTextEx(e->conf.font, o_text, 32, 0).x;
  // float d_width = MeasureTextEx(e->conf.font, d_text, 32, 0).x;

  DrawTextEx(e->conf.font,
    n_text,
    (Vector2){
      (float) e->s_width / 2 - n_width / 2,
      (float) e->s_height / 2 - 100
    },
    32, 0, GRAY
  );
  DrawTextEx(e->conf.font,
    o_text,
    (Vector2){
    (float) e->s_width / 2 - o_width / 2,
    (float) e->s_height / 2 - 50
    },
    32, 0, GRAY 
  );
  /* DrawTextEx(e->conf.font,
    d_text,
    (Vector2){
    e->s_width / 2 - d_width / 2,
    e->s_height / 2
    },
    32, 0, GRAY 
  ); */
}

void DrawCurrentMessage(Editor *e) {
  DrawTextEx(
    e->conf.font,
    e->messages[e->buffer.current_msg_index]->text,
    (Vector2){
      e->conf.padding.left, 
      e->s_height - e->cursor.height 
    },
    32, 0, GRAY
  ); 
}

void DrawStatusLine(Editor *e){
  int sline_y = e->s_height - e->cursor.height;
  DrawRectangle(0, sline_y, e->s_width, e->cursor.height, BLACK);
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
    32, 0, GRAY
  );
}

