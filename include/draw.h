#ifndef DRAW_H
#define DRAW_H
#include <raylib.h>
#include "buffer.h"

void DrawCursor(Editor* e,int x,int y);

void DrawMenu(Editor * e);

void DrawCurrentMessage(Editor *e);

void DrawStatusLine(Editor *e);

void DrawChar(Editor *e, int c, int x_pos, int y_pos, unsigned int color);

#endif
