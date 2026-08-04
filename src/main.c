#include <stdlib.h>
#include "files.h"
#include "draw.h"

Editor ed = {
  .mode = INSERT,
  .s_width = SCREEN_WIDTH,
  .s_height = SCREEN_HEIGHT,
  .num_cmds_displayed = NUM_COMMANDS,
  .conf = {
    .font_size = 42,
    .secondary_font_size = 36,
    .bg_color = 0x141415FF,
    .text_color = 0xFFFFFFFF,
    .under_cursor_color = 0X000000FF,
    .selected_char_color = 0XC3C3D5FF,
    .lines_color = 0x545454FF,
    .line_numbers_color = 0x828282FF,
    .file_name_color = 0x828282FF,
    .status_line_color = 0x000000FF,
    .error_color = 0xFF4C24FF,
    .success_color = 0x00B014FF,
    .selection_color = 0x333738FF,
    .is_menu_open = true,
    .is_vim_mode = true,
    .is_spaces_for_tabs = true,
    .caps_lock_as_escape = true,
    .tab_size = 2,
    .is_showing_lines = false,
    .ln_mode = NONE,
    .ln_padding = 1,
    .line_height = 5,
    .letter_spacing = 0,
    .padding = {
      .top = 45,
      .bottom = 45,
      .right = 70,
      .left = 10
    }
  },
  .cursor = {
    .color = 0xD1D1CFFF
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

  load_font_default(&ed, ed.conf.font_size, true);

  load_font_default(&ed, ed.conf.secondary_font_size, false);

  SetTargetFPS(120);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  RowCol char_size = {0};
  Vector2 press_start_pos = {0};

  while (!WindowShouldClose()) {
    if(ed.should_quit) break;
    ed.s_width = GetScreenWidth();
    ed.s_height = GetScreenHeight();
    ed.mouse = GetMousePosition();
    char_size = get_char_size(ed.conf.font_size);
    ed.cursor.height = char_size.row;
    ed.cursor.width = char_size.col;
    Padding pad = ed.conf.padding;

    ClearBackground(GetColor(ed.conf.bg_color));
    ssize_t max_line_len = get_max_line_length(&ed);
    if(ed.conf.is_showing_lines) {
      for (int l = 0; l < get_max_num_lines(&ed); l++) {
        int y_pos = pad.top + ed.cursor.height + l * (ed.conf.line_height + ed.cursor.height);
        int x_pos = ed.conf.padding.left + (ed.conf.ln_padding + 1)
          * (char_size.col + ed.conf.letter_spacing);
        DrawLineEx(
          (Vector2){x_pos, y_pos}, 
          (Vector2){ed.s_width - (ed.conf.padding.right), y_pos}, 3.0f,
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
           pad.top + (ed.conf.line_height + char_size.row) * y_offset
          },
         ed.conf.font_size, 0, GetColor(ed.conf.line_numbers_color));
      }
      float char_x;
      float char_y;
      Line *current_line = ed.buffer.lines[i];
      for(ssize_t m = 0; m <= current_line->length; m++){
        float cursor_x = pad.left + (ed.conf.letter_spacing + char_size.col) * x_offset;
        if(ed.conf.ln_mode != NONE) cursor_x += (ed.conf.letter_spacing + char_size.col) * (ed.conf.ln_padding + 1);
        if(i == ed.buffer.current_line_index && m == ed.cursor.index){
          DrawCursor(
            &ed, cursor_x,
            pad.top + ((ed.conf.line_height + char_size.row) * y_offset),
            ed.cursor.color
          );
        }

        char_x = pad.left + (ed.conf.letter_spacing + char_size.col) * x_offset;
        char_y = pad.top + ((ed.conf.line_height + char_size.row) * y_offset);
        if(ed.conf.ln_mode != NONE) char_x += (ed.conf.letter_spacing + char_size.col) * (ed.conf.ln_padding + 1);
        if(m < current_line->length) {

          char c = current_line->chars[m];
          unsigned int char_color = m == ed.cursor.index &&
            i == ed.buffer.current_line_index ? 
            ed.conf.under_cursor_color : ed.conf.text_color;
          if(c == '\t') {
            for(int i = 0; i < ed.conf.tab_size; i++){
              DrawChar(
                &ed, ' ',char_x + i * (ed.conf.letter_spacing + char_size.col),
                char_y , char_color, ed.conf.font_size
              );
              x_offset++;
            }
          } else {
            if(is_selected(&ed, (RowCol){i, m}) && !(i == ed.buffer.current_line_index && m == ed.cursor.index)) {
              DrawCursor(&ed, char_x, char_y, ed.conf.selection_color);
              DrawChar(&ed, c,char_x, char_y , ed.conf.selected_char_color, ed.conf.font_size);
            } else {
              DrawChar(&ed, c,char_x, char_y , char_color, ed.conf.font_size);
            } 
          } 

          if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            ed.conf.is_selecting = false;
            press_start_pos = ed.mouse;
            handle_mouse_click(&ed, char_x, char_y, m, i, false);
          }
          if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            float delta_x = ed.mouse.x - press_start_pos.x;
            float delta_y = ed.mouse.y - press_start_pos.y;
            float distance_squared = delta_x * delta_x + delta_y * delta_y;
            if(distance_squared > 25.0f) {
              ed.conf.is_selecting = true;
              ed.mode = NORMAL;
              handle_mouse_click(&ed, char_x, char_y, m, i, true);
            }
          }

          x_offset++;
          if((m + 1) % max_line_len == 0) {
            y_offset++;
            x_offset = 0;
          }
        }else if(!current_line->length) {
          if(is_selected(&ed, (RowCol){i, m}) && !(i == ed.buffer.current_line_index && m == ed.cursor.index)) {
            DrawCursor(&ed, char_x, char_y, ed.conf.selection_color);
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
