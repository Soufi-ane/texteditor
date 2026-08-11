#ifndef DRAW_H
#define DRAW_H
#include <raylib.h>
#include "buffer.h"

void DrawCursor(Editor* e, int x, int y, unsigned int color);

void DrawCmdCursor(Editor* e, int cursor_x, int cursor_y);

void DrawMenu(Editor * e);

void DrawCurrentMessage(Editor *e);

void DrawStatusLine(Editor *e);

void DrawChar(Editor *e, int c, int x_pos, int y_pos, unsigned int color, float font_size);

RowCol get_char_size(float font_size);

void DrawLineNumber(Editor *e, ssize_t i, ssize_t y_offset);

void DrawLineChars(Editor *e, bool is_blinking, ssize_t x_offset, ssize_t y_offset, ssize_t i);

void DrawEditorLines(Editor *e);

#endif
