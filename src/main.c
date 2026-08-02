#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "draw.h"

Editor ed = {
  .mode = INSERT,
  .s_width = SCREEN_WIDTH,
  .s_height = SCREEN_HEIGHT,
  .num_cmds_displayed = NUM_COMMANDS,
  .conf = {
    .bg_color = BG_COLOR,
    .text_color = TEXT_COLOR,
    .under_cursor_color = UNDER_CURSOR_COLOR,
    .lines_color = LINES_COLOR,
    .line_numbers_color = LINE_NUMBERS_COLOR,
    .file_name_color = FILE_NAME_COLOR,
    .status_line_color = STATUS_LINE_COLOR,
    .error_color = ERROR_COLOR,
    .success_color = SUCCESS_COLOR,
    .is_menu_open = true,
    .is_vim_mode = VIM_MODE,
    .is_spaces_for_tabs = IS_SPACES_FOR_TABS,
    .caps_lock_as_escape = CAPS_LOCK_AS_ESCAPE,
    .tab_size = TAB_SIZE,
    .is_showing_lines = DRAW_LINES,
    .ln_mode = NONE,
    .ln_padding = 1,
    .line_height = LINE_HEIGHT,
    .letter_spacing = LETTER_SPACING,
    .padding = {
      .top = LINE_HEIGHT * 3,
      .bottom = LINE_HEIGHT,
      .right = 70,
      .left = 10
    }
  },
  .cursor = {
    .width = 15,
    .height = 35,
    .color = CURSOR_COLOR
  },
};

int main() {
  Buffer *main_buffer = new_buffer(1); 
  ed.buffer = *main_buffer;
  ed.cmd_prompt = new_line(DEFAULT_LINE_SIZE);
  filter_cmds_by_prompt(&ed);
  ed.HOME_DIR  = getenv("HOME");

  try_loading_config(&ed);

  InitWindow(ed.s_width, ed.s_height, "Text Editor");
  SetExitKey(KEY_NULL);
  int file_size = 0;
  Font font_SDF = {0};
#ifdef PROD
  char *font_path = "/usr/local/share/fonts/JetBrainsMonoNF.ttf";
#else 
  char *font_path = "assets/fonts/JetBrainsMonoNF.ttf";
#endif
  load_font_sdf(font_path, &file_size, 128, &font_SDF);
  ed.conf.font = font_SDF;

  SetTargetFPS(120);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  while (!WindowShouldClose()) {
    if(ed.should_quit) break;
    ed.s_width = GetScreenWidth();
    ed.s_height = GetScreenHeight();
    ed.mouse = GetMousePosition();
    Padding pad = ed.conf.padding;
    if (ed.mode != INSERT) {
      SetExitKey(KEY_Q);
    } else SetExitKey(KEY_NULL);


    ClearBackground(GetColor(ed.conf.bg_color));
    ssize_t max_line_len = get_max_line_length(&ed);
    if(ed.conf.is_showing_lines) {
      for (int l = 0; l < get_max_num_lines(&ed); l++) {
        int yPos = pad.top + l * ed.conf.line_height ;
        DrawLineEx(
          (Vector2){30, yPos}, 
          (Vector2){ed.s_width - 30, yPos}, 3.0f,
          GetColor(ed.conf.lines_color)
        );
      }
    }
    // todo : render body
    ssize_t y_offset = 0;
    ssize_t i, x_offset = 0;

    for (
      i = ed.buffer.d_start;
      i < ed.buffer.d_start + ed.buffer.d_length && i < ed.buffer.length;
      i++
    ) {
      // line numbers
      if(ed.conf.ln_mode != NONE) {
        ssize_t index = ed.buffer.current_line_index;
       DrawTextEx(ed.conf.font,
         TextFormat( "%zu", 
           (ed.conf.ln_mode == ABSOLUTE || index == i) ?  i + 1 :
           (index < i ? i - index : index - i)
         ),
         (Vector2){
           pad.left,
           pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height
          },
         32, 0, GetColor(ed.conf.line_numbers_color));
      }
      int char_x;
      int char_y;
      Line *current_line = ed.buffer.lines[i];
      for(ssize_t m = 0; m <= current_line->length; m++){
        int cursor_x = pad.left + ed.conf.letter_spacing * x_offset;
        if(ed.conf.ln_mode != NONE) cursor_x += ed.conf.letter_spacing * (ed.conf.ln_padding + 1);
        if(i == ed.buffer.current_line_index && m == ed.cursor.index)
        //cursor
          DrawCursor(
            &ed, cursor_x,
            pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height
          );
        if(m < current_line->length) {
          char_x = pad.left + ed.conf.letter_spacing * x_offset;
          char_y = pad.top + (ed.conf.line_height * y_offset) - ed.cursor.height;

          if(ed.conf.ln_mode != NONE) char_x += ed.conf.letter_spacing * (ed.conf.ln_padding + 1);

          char c = current_line->chars[m];
          unsigned int char_color = m == ed.cursor.index &&
            i == ed.buffer.current_line_index ? 
            ed.conf.under_cursor_color : ed.conf.text_color;
          if(c == '\t') {
            for(int i = 0; i < ed.conf.tab_size; i++){
              DrawChar(&ed, ' ',char_x + i * ed.conf.letter_spacing, char_y , char_color);
              x_offset++;
            }
          } else {
            if(is_selected(&ed, (RowCol){i, m})) {
              DrawCursor(&ed, char_x, char_y);
              DrawChar(&ed, c,char_x, char_y , ed.conf.under_cursor_color);
            } else {
              DrawChar(&ed, c,char_x, char_y , char_color);
            } 
          } 

          if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            if(ed.mouse.y >= char_y && ed.mouse.y <= char_y + ed.conf.line_height){
              ed.buffer.current_line_index = i;
              handle_click_on_line(&ed, char_x, m);
            }else if(i == ed.buffer.d_start && ed.mouse.y < char_y){
              ed.buffer.current_line_index = ed.buffer.d_start;
              handle_click_on_line(&ed, char_x, m);
            } else if(i == ed.buffer.d_start + ed.buffer.d_length - 1 && ed.mouse.y > char_y){
              ed.buffer.current_line_index = ed.buffer.d_start + ed.buffer.d_length - 1;
              handle_click_on_line(&ed, char_x, m);
            }
          }
          x_offset++;
          if((m + 1) % max_line_len == 0) {
            y_offset++;
            x_offset = 0;
          }
        }

      }
        y_offset++;
        x_offset = 0;

    }

    handle_keys(&ed);

    DrawStatusLine(&ed);
    if(ed.buffer.current_msg_index > -1) DrawCurrentMessage(&ed);
    if(ed.conf.is_menu_open) DrawMenu(&ed);

    EndDrawing();
  }
  CloseWindow();
  UnloadFontData(ed.conf.font.glyphs, ed.conf.font.glyphCount);
  return 0;
}
