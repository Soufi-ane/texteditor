#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "io.h"
#include <errno.h>
#include "tinyfiledialogs.h"

const char *get_file_name_from_path(const char *path){
  if(path == NULL) return NULL;
  const char *last_slash = strrchr(path, '/');
  const char *file_name = path;
  if(last_slash && last_slash >= file_name) {
    file_name = last_slash + 1;
  }
  return file_name;
}

void write_new_message(Editor *e, Message *msg){
  #ifdef PROD
  char *file_path = "/usr/local/share/texteditor/messages.log";
  #else
  char *file_path = "assets/messages.log";
  #endif
	FILE* file = fopen(file_path, "a+");
  if(file == NULL) return;

  char* line = NULL;
  size_t size = 0, line_index = 0;
  size_t read;

  fwrite(msg->text, sizeof(char), sizeof(char) * strlen(msg->text), file);
  fputc('\n', file);
	fclose(file);
}

int write_file(Editor* e){
	FILE* file = fopen(e->buffs->data[e->current_buff]->file_path, "w");
  if(file == NULL) return -1;
  Buffer *buff = e->buffs->data[e->current_buff];

  size_t written = fwrite(buff->s->data, sizeof(char), buff->s->len, file);
  fputc('\n', file);

  new_message(e, TextFormat("Saved, %zuB written", written), INFO);

	fclose(file);
  return 0;
}

void read_file(Editor* e, char const * file_path){
  Buffer *buff = e->buffs->data[e->current_buff];
  bool file_buf_exists = false;
  for(size_t i = 0; i < e->buffs->len; i++){
    Buffer *b = e->buffs->data[i];
    if(b->file_path && !strcmp(b->file_path, file_path)){
      file_buf_exists = true;
      e->current_buff = i;
      b->s->len = 0;
      buff = b;
    }
  }
  if((buff->s->len || !buff->is_saved) && !file_buf_exists){
    buff = new_buffer();
    da_append(e->buffs, buff);
    e->current_buff = e->buffs->len - 1;
  }
	FILE* f = fopen(file_path,"r+");
  if(f == NULL) {
    f = fopen(file_path, "w+");
    if(f == NULL) {
      printf("Coudn't create the file %s path\n",file_path);
      return;
    }
  } 
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  rewind(f);

  if(buff->s->cap < file_size + 1) realloc_str(buff->s, file_size + 1);

  if(e->buffs->len > e->buffs->cap - 1) realloc_editor_buffers(e);

  /* if(!e->buffs->data[0]->is_saved || e->buffs->data[0]->num_chars != 0){
    e->current_buff = e->buffs->len++;
  } */

  if(access(file_path, W_OK) != 0) buff->is_readonly = true;
  buff->file_path = strdup(file_path);
  size_t bytes_read = fread(buff->s->data, 1, file_size, f);
  if(bytes_read != (size_t) file_size){
    //todo: error
    return;
  }
  buff->s->data[file_size - 1] = '\0';
  buff->s->len = bytes_read - 1;
  buff->is_saved = true;
  update_lines(e);
  update_line_number_padding(e);
  fclose(f);
}

void try_saving_file(Editor* e){
  Buffer *buff = e->buffs->data[e->current_buff];
  if(buff->is_saved){
    new_message(e, "No changes to be saved", INFO);
    return;
  }
  char config_path[1024];
  #ifdef PROD
  char *messages_path = "/usr/local/share/texteditor/messages.log";
  sprintf(config_path, "%s/.config/texteditor/texteditor.conf", e->HOME_DIR);
  #else
  char *messages_path = "assets/messages.log";
  sprintf(config_path, "assets/texteditor.conf");
  #endif

  if(buff->file_path){
    if(!strcmp(messages_path, buff->file_path)){
      buff->is_readonly = true;
    }
    if(buff->is_readonly){
      new_message(e, "Readonly file!", ERROR);
      return;
    }

    bool is_done = false;
    if(access(buff->file_path, W_OK) == 0 || errno == ENOENT) {
      int err = write_file(e);
      if(!err){
        is_done = true;
        buff->is_saved = true;
      }
    } 
    if (errno == EACCES) {
      new_message(e, "Readonly file!", ERROR);
    }
    if(is_done){
      if(!strcmp(config_path, buff->file_path)){
        try_loading_config(e);
      }
    } 
  }
  else {
    char const * path = 
    tinyfd_saveFileDialog(
      "Save File",
      buff->file_path != NULL ? buff->file_path : "Untitled",
      0, NULL, NULL
    );
    if (path != NULL) {
      buff->file_path = path;
      try_saving_file(e);
    }
  }
}

bool load_font_default(Editor *e, FontData *font_data){
  if(!font_data) return false;
  if(!font_data->is_file_loaded){
    int loaded_size = 0;
    font_data->font_file = LoadFileData(font_data->path, &loaded_size);
    if(loaded_size <= 0 || !font_data->font_file) return false;
    font_data->file_size = loaded_size;
    font_data->is_file_loaded = true;
  }
  if(font_data->font_file == NULL || font_data->font_file == 0) return false;
  Font font = {0};
	font.baseSize = (int) font_data->size;
	font.glyphCount = 95;
	font.glyphs = LoadFontData(font_data->font_file, font_data->file_size, (int) font_data->size, 0, 95, FONT_DEFAULT);
  if (font.glyphs == NULL) return false;
	Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 95, (int) font_data->size, 0, 1);
  if (font.recs == NULL || atlas.data == NULL || atlas.width <= 0 || atlas.height <= 0) {
    UnloadFontData(font.glyphs, 95);
    if(atlas.data) UnloadImage(atlas);
    return false;
  }
	font.texture = LoadTextureFromImage(atlas);
  UnloadImage(atlas);
  if (font.texture.id == 0) {
    UnloadFontData(font.glyphs, 95);
    return false;
  }
	SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
  Font old_font = font_data->font;
  font_data->font = font;
  if(old_font.texture.id != 0){
    UnloadFont(old_font);
  }
  return true;
}

bool str_includes(String *str, String *sub_str){
  str = lower_case(str);
  sub_str = lower_case(sub_str);
  size_t i, j;
  for(i = 0; i < str->len; i++) {
    if(str->data[i] == sub_str->data[0]) {
      for(j = 1; (j < sub_str->len && i < str->len - 1); j++, i++){
        if(sub_str->data[j] != str->data[i + 1]) break;
      }
      if(j >= sub_str->len) return true;
    }
  }
  return false;
}

void copy_selection_to_clipboard(Editor *e){
  if(!e->conf.is_selecting) return;
  Buffer *buff = e->buffs->data[e->current_buff];
  size_t start = MIN(buff->cursor.index, e->conf.selection_start);
  size_t selection_size = abs(buff->cursor.index - e->conf.selection_start) + 1;
  String *selected = new_str(selection_size + 1);
  memcpy(selected->data, &buff->s->data[start], selection_size);
  selected->len = selection_size;
  add_char_to_str(selected, '\0', selection_size);
  int success = copy_to_clipboard(selected->data);
  if(success) new_message(e, "Copied", GOOD);
  else new_message(e, "Failed to copy", ERROR);
}

void paste_from_clipboard(Editor *e){
  Buffer *curr_buff = e->buffs->data[e->current_buff];
  char *clip_buff = read_from_clipboard();
  if(!clip_buff) return;
  String *clip_str = string(clip_buff);
  free(clip_buff);
  bool has_text = curr_buff->s->len > 0;
  bool at_end = curr_buff->cursor.index >= curr_buff->s->len;
  size_t insert_index = curr_buff->cursor.index + has_text;
  if(has_text) insert_index -= at_end;
  update_action(&curr_buff->cur_act, insert_index, clip_str, true);
  add_str_to_str(curr_buff->s, clip_str, insert_index);
  move_cursor_right(e, clip_str->len - !has_text - (at_end && has_text));
  update_lines(e);
  update_scroll(e, false, false);
}

int copy_to_clipboard(const char *text){
  FILE *pipe = popen("xclip -selection clipboard 2>/dev/null || wl-copy 2>/dev/null", "w");
  if(!pipe) return 0;
  fputs(text, pipe);
  int status = pclose(pipe);
  return status == 0;
}

char *read_from_clipboard(){
  char *buff = NULL;
  FILE *pipe = popen("xclip -selection clipboard -o 2>/dev/null || wl-paste 2>/dev/null", "r");
  if(!pipe) return buff;
  buff = malloc(sizeof(char) * MAX_PASTE_LENGTH + 1);
  size_t read = fread(buff, 1, MAX_PASTE_LENGTH, pipe);
  buff[read] = '\0';
  pclose(pipe);
  return buff;
}

ConfigKey get_config_key(Editor *e, char *key){
  if(!strcmp(key, "line_numbers_mode"))      return LN_MODE;
  if(!strcmp(key, "spaces_for_tabs"))        return SPACE_FOR_TAB;
  if(!strcmp(key, "vim_mode"))               return VIM_M;
  if(!strcmp(key, "draw_lines"))             return D_LINES;
  if(!strcmp(key, "background_color"))       return BG_COL;
  if(!strcmp(key, "text_color"))             return TXT_COL;
  if(!strcmp(key, "cursor_color"))           return CURSOR_COL;
  if(!strcmp(key, "under_cursor_color"))     return UNDER_CURSOR_COL;
  if(!strcmp(key, "line_numbers_color"))     return LN_COL;
  if(!strcmp(key, "lines_color"))            return LINES_COL;
  if(!strcmp(key, "tab_size"))               return TAB_S;
  if(!strcmp(key, "caps_lock_as_escape"))    return CAPS_AS_ESCAPE;
  if(!strcmp(key, "padding_top"))            return P_TOP;
  if(!strcmp(key, "padding_bottom"))         return P_BOTTOM;
  if(!strcmp(key, "padding_left"))           return P_LEFT;
  if(!strcmp(key, "padding_right"))          return P_RIGHT;
  if(!strcmp(key, "primary_font_size"))      return FONT_SIZE;
  if(!strcmp(key, "secondary_font_size"))    return SECONDARY_FONT_SIZE;
  if(!strcmp(key, "primary_font"))           return FONT_PRIMARY;
  if(!strcmp(key, "secondary_font"))         return FONT_SECONDARY;
  if(!strcmp(key, "highlight_active_line"))  return LINE_HIGHLIGHT;
  if(!strcmp(key, "active_line_color"))      return LINE_HIGHLIGHT_COL;
  return UNKOWN_KEY;
}

bool try_getting_color_from_hex(char *hex, unsigned int *dest){
  if(strlen(hex) != 6 || hex == NULL) return false;
  unsigned long hex_value = strtoul(hex, NULL, 16);
  *dest = (unsigned int) (hex_value << 8 | 0xFF);
  return true;
}

void try_setting_conf_color_value(Editor *e, ConfigKey key_type, char *hex, size_t line_number){
  unsigned int color;
  bool is_color_valid = try_getting_color_from_hex(++hex, &color);
  if(!is_color_valid){
    new_message( e, TextFormat("Error in value [%s] at: %zu", hex, line_number), ERROR);
    return;
  }
  switch (key_type) {
    case BG_COL:
      e->conf.bg_color = color;
      break;
    case TXT_COL:
      e->conf.text_color = color;
      break;
    case CURSOR_COL:
      e->buffs->data[e->current_buff]->cursor.color = color;
      break;
    case UNDER_CURSOR_COL:
      e->conf.under_cursor_color = color;
      break;
    case LN_COL:
      e->conf.line_numbers_color = color;
      break;
    case LINES_COL:
      e->conf.lines_color = color;
      break;
    case LINE_HIGHLIGHT_COL:
      e->conf.line_highlight_color = color;
      break;
  }
}

void try_setting_conf_number_value(Editor *e, ConfigKey key_type, char *value, size_t line_number){
  char *endpoint;
  int number = strtoul(value, &endpoint, 10);
  if(endpoint == value || *endpoint != '\0' || number < 0) {
    new_message(e, TextFormat("Invalid value [%s] at: %zu", value, line_number), ERROR);
    return;
  }
  switch (key_type) {
    case TAB_S:
      e->conf.tab_size = number;
      break;

    case P_TOP:
      e->conf.padding.top = number;
      break;
    case P_BOTTOM:
      e->conf.padding.bottom = number;
      break;
    case P_LEFT:
      e->conf.padding.left = number;
      break;
    case P_RIGHT:
      e->conf.padding.right = number;
      break;
    case FONT_SIZE:
      if(number != e->conf.font_data.size){
        if(number < 500 && number > 10){
          e->conf.font_data.size = number;
          if(!load_font_default(e, &e->conf.font_data)){
            fprintf(stderr, "Failed to load fonts\n");
            exit(1);
          }
        }else {
          new_message(e, TextFormat("Invalid size for primary font at: %zd", line_number), ERROR);
        } 
      }
      break;
    case SECONDARY_FONT_SIZE:
      if(number != e->conf.font_secondary_data.size){
        if(number <= 45 && number >= 25){
          e->conf.font_secondary_data.size = number;
          if(!load_font_default(e, &e->conf.font_secondary_data)){
            fprintf(stderr, "Failed to load fonts\n");
            exit(1);
          }
        }else {
          new_message(e, TextFormat("Invalid size for secondary font: %zd", line_number), ERROR);
        }
      }
      break;
  }
}

void try_loading_new_font(Editor *e, char *path, size_t n_line, bool is_primary){
  FontData old = e->conf.font_data;
  if(!is_primary) {
    old = e->conf.font_secondary_data;
    e->conf.font_secondary_data.path = strdup(path);
    e->conf.font_secondary_data.is_file_loaded = false;
  } else {
    e->conf.font_data.path = strdup(path);
    e->conf.font_data.is_file_loaded = false;
  }
  if(!load_font_default(e, is_primary ? &e->conf.font_data : &e->conf.font_secondary_data)){
    if(is_primary) e->conf.font_data = old;
    else e->conf.font_secondary_data = old;
    new_message( e, TextFormat("Failed to load font at config: %zu", n_line), ERROR);
  }
}

void try_reading_conf_path(Editor *e, ConfigKey key_type, char *path, size_t n_line){
  if(access(path, F_OK) != 0) {
    new_message(e, TextFormat("Invalid path at config: %zu", n_line), ERROR);
    return;
  }
  switch(key_type){
    case FONT_PRIMARY:
      if(strcmp(e->conf.font_data.path, path) == 0) return;
      try_loading_new_font(e, path, n_line, true);
    break;
    case FONT_SECONDARY:
      if(strcmp(e->conf.font_secondary_data.path, path) == 0) return;
      try_loading_new_font(e, path, n_line, false);
    break;
  }
}

void try_setting_conf_value(Editor *e, ConfigKey key_type, char *value, size_t n_line){
  bool is_invalid = false;
  switch (key_type) {
    case LN_MODE :
      if(!strcmp(value, "relative"))      e->conf.ln_mode = RELATIVE;
      else if(!strcmp(value, "absolute")) e->conf.ln_mode = ABSOLUTE;
      else if(!strcmp(value, "none"))     e->conf.ln_mode = NONE;
      else is_invalid = true;
      break;
    case SPACE_FOR_TAB :
      if(!strcmp(value, "true"))       e->conf.is_spaces_for_tabs = true;
      else if(!strcmp(value, "false")) e->conf.is_spaces_for_tabs = false;
      else is_invalid = true;
      break;
    case VIM_M :
      if(!strcmp(value, "true"))       e->conf.is_vim_mode = true;
      else if(!strcmp(value, "false")) e->conf.is_vim_mode = false;
      else is_invalid = true;
      break;
    case D_LINES:
      if(!strcmp(value, "true"))       e->conf.is_showing_lines = true;
      else if(!strcmp(value, "false")) e->conf.is_showing_lines = false;
      else is_invalid = true;
      break;
    case CAPS_AS_ESCAPE:
      if(!strcmp(value, "true"))       e->conf.caps_lock_as_escape = true;
      else if(!strcmp(value, "false")) e->conf.caps_lock_as_escape = false;
      else is_invalid = true;
      break;
    case LINE_HIGHLIGHT:
      if(!strcmp(value, "true"))       e->conf.is_line_highlight = true;
      else if(!strcmp(value, "false")) e->conf.is_line_highlight = false;
      else is_invalid = true;
      break;
  }
  if(is_invalid){
    new_message(e, TextFormat("Invalid value [%s] at config: %zu", value, n_line), ERROR);
  }
}

void read_config_line(Editor *e, char *line, size_t len, size_t line_number){
  char key[128], value[128];
  size_t i = 0 , j = 0;
  while(isspace(line[i])) i++;
  while(i < len && line[i] != '=' && !isspace(line[i])) key[j++] = line[i++];
  key[j] = '\0'; 
  while(i < len && (line[i] == '=' || isspace(line[i]) )) i++;
  j = 0;
  while(i < len && !isspace(line[i])) value[j++] = line[i++];
  value[j] = '\0'; 
  ConfigKey key_type = get_config_key(e, key);
  if(key_type == UNKOWN_KEY){
    new_message(e, TextFormat("Unknown key [%s] at config: %d", key, line_number), ERROR);
  }
  if(
    key_type == BG_COL || key_type == TXT_COL || key_type == CURSOR_COL ||
    key_type == UNDER_CURSOR_COL || key_type == LN_COL || key_type == LINES_COL ||
    key_type == LINE_HIGHLIGHT_COL
    ){
    try_setting_conf_color_value(e, key_type, value, line_number);
  }else if(
    key_type == TAB_S || key_type == P_RIGHT || key_type == P_LEFT ||
    key_type == P_BOTTOM || key_type == P_TOP || key_type == FONT_SIZE ||
    key_type == SECONDARY_FONT_SIZE
    ){
    try_setting_conf_number_value(e, key_type, value, line_number);
  }else if(key_type == FONT_PRIMARY || key_type == FONT_SECONDARY) {
    try_reading_conf_path(e, key_type, value, line_number);
  }
  else try_setting_conf_value(e, key_type, value, line_number);
}

int try_loading_config(Editor *e){
  if (e->HOME_DIR == NULL)  {
    new_message(e, "$HOME environment variable is not set", ERROR);
    return 0;
  }
  char conf_path[1024];

#ifdef PROD
  sprintf(conf_path, "%s/.config/texteditor/texteditor.conf", e->HOME_DIR);
#else
  sprintf(conf_path, "assets/texteditor.conf");
#endif

  FILE *conf_file = fopen(conf_path, "r");
  if(conf_file == NULL)  {
    new_message(e, TextFormat("Error loading config file [%s]", conf_path), ERROR);
    return 0;
  }

  size_t size, read, line_index = 0;
  char* line = NULL;
	while((read = getline(&line, &size, conf_file)) != -1){
    if(read > 1 && line[0] != '#') {
      read_config_line(e, line, read, line_index + 1);
    }
    line_index++;
  }
  return 1;
}

void handle_cmd_args(Editor *e, int argc, char **argv){
  if(argc > 3){
    fprintf(stderr, "Too many args\n");
    exit(1);
  }
  if(argc > 1 && !strcmp(argv[1], "-v")){
    printf("texteditor: v%s\n", VERSION);
    exit(0);
  }else if(argc > 2 && !strcmp(argv[1], "-f")){
    read_file(e, argv[2]);
  }else if(argc == 2){
    read_file(e, argv[1]);
  }
}
