#include "files.h"
#include "draw.h"

int main() {

  Editor *e = init_editor();
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
  Vector2 press_start_pos = {0};

  while (!WindowShouldClose()) {
    if(e->should_quit) break;
    e->s_width = GetScreenWidth();
    e->s_height = GetScreenHeight();
    e->mouse = GetMousePosition();
    char_size = get_char_size(e->conf.font_data.size);
    e->buffers[e->current_buff]->cursor.height = char_size.row;
    e->buffers[e->current_buff]->cursor.width = char_size.col;
    Padding pad = e->conf.padding;

    ClearBackground(GetColor(e->conf.bg_color));
    ssize_t max_line_len = get_max_line_length(e);
    if(e->conf.is_showing_lines) {
      for (int l = 0; l < get_max_num_lines(e); l++) {
        int y_pos = pad.top + e->buffers[e->current_buff]->cursor.height + l * (e->conf.line_height + e->buffers[e->current_buff]->cursor.height);
        int x_pos = e->conf.padding.left + (e->conf.ln_padding + 1)
          * (char_size.col + e->conf.letter_spacing);
        DrawLineEx(
          (Vector2){x_pos, y_pos}, 
          (Vector2){e->s_width - (e->conf.padding.right), y_pos}, 3.0f,
          GetColor(e->conf.lines_color)
        );
      }
    }
    // todo : render body
    ssize_t y_offset = 0;
    ssize_t i, x_offset = 0;

    for (
      i = e->buffers[e->current_buff]->d_start;
      i < e->buffers[e->current_buff]->d_start + 
      e->buffers[e->current_buff]->d_length && i < e->buffers[e->current_buff]->length;
      i++
    ) {
      // line numbers
      if(e->conf.ln_mode != NONE) {
        ssize_t index = e->buffers[e->current_buff]->current_line_index;
       DrawTextEx(e->conf.font_data.font,
         TextFormat( "%zu", 
           (e->conf.ln_mode == ABSOLUTE || index == i) ?  i + 1 :
           (index < i ? i - index : index - i)
         ),
         (Vector2){
           pad.left,
           pad.top + (e->conf.line_height + char_size.row) * y_offset
          },
         e->conf.font_data.size, 0, GetColor(e->conf.line_numbers_color));
      }
      float char_x;
      float char_y;
      Line *current_line = e->buffers[e->current_buff]->lines[i];
      for(ssize_t m = 0; m <= current_line->length; m++){
        float cursor_x = pad.left + (e->conf.letter_spacing + char_size.col) * x_offset;
        if(e->conf.ln_mode != NONE) cursor_x += (e->conf.letter_spacing + char_size.col) * (e->conf.ln_padding + 1);
        if(i == e->buffers[e->current_buff]->current_line_index && m == e->buffers[e->current_buff]->cursor.index){
          DrawCursor(
            e, cursor_x,
            pad.top + ((e->conf.line_height + char_size.row) * y_offset),
            e->buffers[e->current_buff]->cursor.color
          );
        }

        char_x = pad.left + (e->conf.letter_spacing + char_size.col) * x_offset;
        char_y = pad.top + ((e->conf.line_height + char_size.row) * y_offset);
        if(e->conf.ln_mode != NONE) char_x += (e->conf.letter_spacing + char_size.col) * (e->conf.ln_padding + 1);
        if(m < current_line->length) {

          char c = current_line->chars[m];
          unsigned int char_color = m == e->buffers[e->current_buff]->cursor.index &&
            i == e->buffers[e->current_buff]->current_line_index ? 
            e->conf.under_cursor_color : e->conf.text_color;
          if(c == '\t') {
            for(int i = 0; i < e->conf.tab_size; i++){
              DrawChar(
                e, ' ',char_x + i * (e->conf.letter_spacing + char_size.col),
                char_y , char_color, e->conf.font_data.size
              );
              x_offset++;
            }
          } else {
            if(is_selected(e, (RowCol){i, m}) && 
                !(i == e->buffers[e->current_buff]->current_line_index && m == e->buffers[e->current_buff]->cursor.index)) {
              DrawCursor(e, char_x, char_y, e->conf.selection_color);
              DrawChar(e, c,char_x, char_y , e->conf.selected_char_color, e->conf.font_data.size);
            } else {
              DrawChar(e, c,char_x, char_y , char_color, e->conf.font_data.size);
            } 
          } 

          if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            e->conf.is_selecting = false;
            press_start_pos = e->mouse;
            handle_mouse_click(e, char_x, char_y, m, i, false);
          }
          if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            float delta_x = e->mouse.x - press_start_pos.x;
            float delta_y = e->mouse.y - press_start_pos.y;
            float distance_squared = delta_x * delta_x + delta_y * delta_y;
            if(distance_squared > 25.0f) {
              e->conf.is_selecting = true;
              e->mode = NORMAL;
              handle_mouse_click(e, char_x, char_y, m, i, true);
            }
          }

          x_offset++;
          if((m + 1) % max_line_len == 0) {
            y_offset++;
            x_offset = 0;
          }
        }else if(!current_line->length) {
          if(is_selected(e, (RowCol){i, m}) && 
              !(i == e->buffers[e->current_buff]->current_line_index && m == e->buffers[e->current_buff]->cursor.index)) {
            DrawCursor(e, char_x, char_y, e->conf.selection_color);
          }
        }
      }
        y_offset++;
        x_offset = 0;
    }

    handle_keys(e);

    DrawStatusLine(e);
    if(e->buffers[e->current_buff]->current_msg_index > -1) DrawCurrentMessage(e);
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
