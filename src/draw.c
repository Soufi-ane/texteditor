#include <raylib.h>
#include "buffer.h"

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
      e->s_height - e->conf.line_height
    },
    32, 0, GRAY
  ); 
}


