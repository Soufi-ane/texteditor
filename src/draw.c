#include <raylib.h>
#include "note.h"

void DrawCursor(Editor* e,int x,int y){
  DrawRectangle(x,y, e->cursor.width, e->cursor.height, BLACK);
}
