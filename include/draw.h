#ifndef DRAW_H
#define DRAW_H

#include <raylib.h>
#include "editor.h"

void DrawCursor(Editor* e, int x, int y, unsigned int color, bool is_primary);

void DrawMenu(Editor * e);

void DrawCurrentMessage(Editor *e);

void DrawStatusLine(Editor *e, bool is_blinking);

void DrawChar(Editor *e, int c, int x_pos, int y_pos, unsigned int color, float font_size);

int get_max_char_w(Font font);

void DrawLineNumber(Editor *e, size_t i, size_t y_offset);

void DrawLineChars(Editor *e, bool is_blinking, size_t x_offset, size_t y_offset, size_t i);

void DrawEditorLines(Editor *e);

void DrawBufferText(Editor *e, bool is_blinking);

void DrawExplorer(Editor *e, bool is_blinking);

void udpate_explorer_size(Editor *e);

#endif
