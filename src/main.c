#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "draw.h"

Editor ed = {
  .mode = NORMAL,
  .s_width = SCREEN_WIDTH,
  .s_height = SCREEN_HEIGHT,
  .conf = {
    .is_menu_open = true,
    .ln_mode = ABSOLUTE,
    .ln_padding = 1,
    .line_height = LINE_HEIGHT,
    .letter_spacing = LETTER_SPACING,
    .padding = {
      .top = LINE_HEIGHT,
      .bottom = LINE_HEIGHT,
      .right = 70,
      .left = 10
    }
  },
  .cursor = {
    .width = 15,
    .height = 35,
  },
};

char dir[100];

int main() {
  Buffer *main_buffer = new_buffer(1); 
  ed.buffer = *main_buffer;
  ed.message = malloc(sizeof(char) * 129);
  Color fg_color = GetColor(0xD9D9D955);
  ed.HOME_DIR  = getenv("HOME");
  if (ed.HOME_DIR == NULL)
    printf("ERRR!\n");
  sprintf(dir, "%s/.local/notes", ed.HOME_DIR);

  InitWindow(ed.s_width, ed.s_height, "Note");
  SetExitKey(KEY_NULL);
  int file_size = 0;
  Font font_SDF = {0};
  loadFontSDF(
      "/usr/local/share/fonts/JetBrainsMonoNF.ttf",
      &file_size,
      128,
      &font_SDF);
  ed.conf.font = font_SDF;

  SetTargetFPS(120);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  while (!WindowShouldClose()) {
    ed.s_width = GetScreenWidth();
    ed.s_height = GetScreenHeight();
    Padding pad = ed.conf.padding;
    if (ed.mode != INSERT) {
      SetExitKey(KEY_Q);
    } else if (ed.mode == INSERT) {
      SetExitKey(KEY_NULL);
    }
    handle_keys(&ed);

    ClearBackground(WHITE);
    for (int l = 0; l < get_max_num_lines(&ed); l++) {
      int yPos = pad.top + l * ed.conf.line_height ;
      DrawLineEx((Vector2){30, yPos}, (Vector2){ed.s_width - 30, yPos}, 3.0f, fg_color);
    }
    DrawTextEx(ed.conf.font,
      ed.mode == NORMAL ? "NORMAL" : "INSERT",
      (Vector2){ed.s_width - ed.cursor.width * 10, 35} ,
      32, 0, BLACK);

    // todo : render body
    size_t y_offset = 0;
    size_t i, x_offset = 0;

    for (
      i = ed.buffer.d_start;
      i < ed.buffer.d_start + ed.buffer.d_length && i < ed.buffer.length;
      i++
    ) {
      // line numbers
      if(ed.conf.ln_mode != NONE) {
        size_t index = ed.buffer.current_line_index;
       DrawTextEx(ed.conf.font,
         TextFormat( "%zu", 
           ed.conf.ln_mode == ABSOLUTE ?  i + 1 :
           index == i ? i + 1 :
           (index < i ? i - index : index - i)
         ),
         (Vector2){
           pad.left,
           pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height
          },
         32, 0, GRAY);
      }

      Line *current_line = ed.buffer.lines[i];
      for(size_t m = 0; m <= current_line->length; m++){
        int cursor_x = pad.left + ed.conf.letter_spacing * x_offset;
        if(ed.conf.ln_mode != NONE) cursor_x += ed.conf.letter_spacing * (ed.conf.ln_padding + 1);
        if(i == ed.buffer.current_line_index && m == ed.cursor.index)
        //cursor
          DrawCursor(
            &ed, 
            cursor_x,
            pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height
          );
        if(m < current_line->length) {
          int char_x = pad.left + ed.conf.letter_spacing * x_offset;
          if(ed.conf.ln_mode != NONE) char_x += ed.conf.letter_spacing * (ed.conf.ln_padding + 1);
          DrawTextCodepoint(ed.conf.font,
            current_line->chars[m],
            (Vector2){
            char_x,
            pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height},
            32,
            m == ed.cursor.index && i == ed.buffer.current_line_index ? GRAY : BLACK
          );

          x_offset++;
          if((m + 1) % get_max_line_length(&ed) == 0) {
            y_offset++;
            x_offset = 0;
          }
        }
      }
        y_offset++;
        x_offset = 0;
      }


    // debugging 
    if(ed.conf.is_debugging) {
      DrawRectangle(170,SCREEN_HEIGHT - 600, 400,600, BLACK);
      /* DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]","isInsertingTitle",ed.conf.isInsertingTitle),
        (Vector2){200, SCREEN_HEIGHT - 570},
        32, 0, GREEN);
      DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "isTakingNote",ed.conf.isTakingNote),
        (Vector2){200, SCREEN_HEIGHT - 540},
        32, 0, GREEN); */
      DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "isMenuOpen",ed.conf.is_menu_open),
        (Vector2){200, SCREEN_HEIGHT - 510},
        32, 0, GREEN);
      DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "isOpeningFile",ed.conf.is_opening_file),
        (Vector2){200, SCREEN_HEIGHT - 480},
        32, 0, GREEN);
      /* DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "isSearching",ed.conf.isSearching),
        (Vector2){200, SCREEN_HEIGHT - 450},
        32, 0, GREEN); */
      /* DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "isNamingFile",ed.conf.isNamingFile),
        (Vector2){200, SCREEN_HEIGHT - 420},
        32, 0, GREEN); */
      DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "current_line_index",ed.buffer.current_line_index),
        (Vector2){200, SCREEN_HEIGHT - 390},
        32, 0, GREEN);
      DrawTextEx(ed.conf.font,
        TextFormat("%-16s [%s]", "query",ed.searchQuery),
        (Vector2){200, SCREEN_HEIGHT - 300},
        32, 0, GREEN);
      DrawTextEx(ed.conf.font,
        TextFormat("%-17s [%d]", "index",ed.cursor.index),
        (Vector2){200, SCREEN_HEIGHT - 330},
        32, 0, GREEN);
      DrawTextEx(ed.conf.font,
        TextFormat("%-16s [%03d]", "buff length", ed.buffer.length),
        (Vector2){200, SCREEN_HEIGHT - 270},
        32, 0, GREEN); 
      DrawTextEx(ed.conf.font,
        TextFormat("%s: [%03d]", "current length",ed.buffer.lines[ed.buffer.current_line_index]->length),
        (Vector2){200, SCREEN_HEIGHT - 240},
        32, 0, GREEN); 
      if(ed.buffer.capacity > ed.buffer.current_line_index + 1)
      DrawTextEx(ed.conf.font,
        TextFormat("%s: [%3d]", "next length", ed.buffer.lines[ed.buffer.current_line_index + 1]->length),
        (Vector2){200, SCREEN_HEIGHT - 210},
        32, 0, GREEN);
      /* DrawTextEx(ed.conf.font,
        TextFormat("note length [%d]",ed.note->length),
        (Vector2){200, SCREEN_HEIGHT - 180},
        32, 0, GREEN); */
    }
    DrawTextEx(ed.conf.font,
      TextFormat("%s",ed.message),
      (Vector2){200, SCREEN_HEIGHT - 60},
      32, 0, BLACK);

    if (ed.conf.is_menu_open) DrawMenu(&ed);
    if(ed.buffer.current_msg_index > -1) DrawCurrentMessage(&ed);

    EndDrawing();
  }
  CloseWindow();
  int i;
  // writeFile(&ed);
  UnloadFontData(ed.conf.font.glyphs, ed.conf.font.glyphCount);
  return 0;
}
