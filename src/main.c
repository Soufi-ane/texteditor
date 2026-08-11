#include "files.h"
#include "draw.h"

int main(int argc, char **argv) {

  Editor *e = init_editor();
  handle_cmd_args(e, argc, argv);

  filter_cmds_by_prompt(e);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(e->s_width, e->s_height, "Text Editor");
  SetExitKey(KEY_NULL);

  try_loading_config(e);
  if(
    !load_font_default(e, &e->conf.font_data) || 
    !load_font_default(e, &e->conf.font_secondary_data)
  ){
    fprintf(stderr, "Failed to load fonts\n");
    return 1;
  }

  SetTargetFPS(120);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  RowCol char_size = {0};

  double last_blink_time = 0;
  bool is_blinking = false;

  while (!WindowShouldClose()) {
    if(e->should_quit) break;
    e->s_width = GetScreenWidth();
    e->s_height = GetScreenHeight();
    e->mouse = GetMousePosition();
    char_size = get_char_size(e->conf.font_data.size);
    e->buffers[e->current_buff]->cursor.height = char_size.row;
    e->buffers[e->current_buff]->cursor.width = char_size.col;
    Padding pad = e->conf.padding;
    Buffer *buff = e->buffers[e->current_buff];

    double now = GetTime();
    if(
        now - buff->cursor.last_time_moved > CURSOR_BLINK_DURATION ||
        now - buff->cursor.last_time_moved < TIME_BEFORE_BLINK
    ){
      is_blinking = false;
    }else {
      if(now - last_blink_time > CURSOR_BLINK_INTERVAL) {
        last_blink_time = now;
        is_blinking = !is_blinking;
      }
    }

    ClearBackground(GetColor(e->conf.bg_color));

    if(e->conf.is_showing_lines) DrawEditorLines(e);

    ssize_t y_offset = 0;
    ssize_t i, x_offset = 0;
    ssize_t display_start = buff->d_start;
    ssize_t display_end   = display_start + buff->d_length;
    ssize_t total_length  = buff->length;

    for (i = display_start; i < display_end && i < total_length; i++) {

      if(e->conf.ln_mode != NONE) DrawLineNumber(e, i, y_offset);

      DrawLineChars(e, is_blinking, x_offset, y_offset, i);

      y_offset++;
      x_offset = 0;

    }

    handle_keys(e);
    DrawStatusLine(e);
    if(buff->current_msg_index > -1) DrawCurrentMessage(e);
    if(e->conf.is_menu_open) DrawMenu(e);

    EndDrawing();
  }
  CloseWindow();
  UnloadFontData(e->conf.font_data.font.glyphs, e->conf.font_data.font.glyphCount);
  UnloadFontData(e->conf.font_secondary_data.font.glyphs, e->conf.font_secondary_data.font.glyphCount);
  UnloadFileData(e->conf.font_data.font_file);
  UnloadFileData(e->conf.font_secondary_data.font_file);
  return 0;
}
