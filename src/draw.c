#include <raylib.h>
#include "buffer.h"

void DrawCursor(Editor* e,int x,int y){
  DrawRectangle(x,y, e->cursor.width, e->cursor.height, BLACK);
}
