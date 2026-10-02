#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
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

bool is_dir(char *path){
  struct stat st;
  stat(path, &st); 
  return S_ISDIR(st.st_mode);
}

bool key_in(ConfigKey key, ConfigKey *choices, size_t len){
  for(size_t i = 0; i < len; i++){
    if(key == choices[i]) return true;
  }
  return false;
}

void write_new_message(Editor *e, char *msg){
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

  fwrite(msg, sizeof(char), sizeof(char) * strlen(msg), file);
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

String *get_trash_path(Editor *e){
  String *trash_path;
  #ifdef PROD
  trash_path = string("/.local/share/texteditor/trash/");
  add_text_to_str(trash_path, e->HOME_DIR, 0);
  #else
  trash_path = string("trash/");
  #endif
  return trash_path;
}

bool create_file(Editor *e, char *path){
  FILE* file = fopen(path , "r");
  if(file != NULL) {
    new_message(e, "File already exists", ERROR);
    return false;
  }
  file = fopen(path , "w");
  if(file == NULL) {
    new_message(e, "Failed to create file", ERROR);
    return false;
  }else {
    String *trash_path = get_trash_path(e);
    FileAction *act = new_file_action();
    const char *file_name = get_file_name_from_path(path);
    add_str_to_str(trash_path, c_string('/'), trash_path->len);
    add_text_to_str(trash_path, file_name, trash_path->len);
    act->old = trash_path;
    act->new = string(path);
    stack_push(&e->exp->undo_stack, *act);
    stack_flush(&e->exp->redo_stack);
  }
  fclose(file);
  return true;
}

void clear_trash(Editor *e) {
  String *trash_path = get_trash_path(e);
  DIR *dir;
  struct dirent *entry;
  dir = opendir(trash_path->data);
  if(dir == NULL){
    printf("Coundn't open trash directory\n");
    return;
  }
  while((entry = readdir(dir)) != NULL){
    String *path = string(trash_path->data);
    if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
      add_text_to_str(path, "/", path->len);
      add_text_to_str(path, entry->d_name, path->len);
      remove_file(e, path->data);
    }
  }
}

void remove_file(Editor *e, char *path){
  if(is_dir(path)){
    DIR *d = opendir(path);
    if(!d) return;
    struct dirent *entry;
    while((entry = readdir(d)) != NULL){
      if(strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
        String *child_path = string(path);
        add_text_to_str(child_path, "/", child_path->len);
        add_text_to_str(child_path, entry->d_name, child_path->len);
        remove_file(e, child_path->data);
      }
    }
  }
  if(remove(path) != 0) {
    new_message(e, "Failed to delete file", ERROR);
  }
}

bool move_file(char *src, char *dest){
  return rename(src, dest) == 0;
}

void move_to_trash(Editor *e, char *path){
  String *trash_path;
  #ifdef PROD
  trash_path = string("/.local/share/texteditor/trash/");
  add_text_to_str(trash_path, e->HOME_DIR, 0);
  #else
  trash_path = string("trash/");
  #endif
  const char *file_name = get_file_name_from_path(path);
  add_text_to_str(trash_path, file_name, trash_path->len);
  if(move_file(path, trash_path->data)){
    FileAction *act = new_file_action();
    act->old = string(path);
    act->new = trash_path;
    stack_push(&e->exp->undo_stack, *act);
    stack_flush(&e->exp->redo_stack);
  }else {
    new_message(e, "Failed to delete file", ERROR);
  }
}


void read_file(Editor* e, char const * file_path){
	FILE* f = fopen(file_path, "r+");
  if(f == NULL) {
    printf("Coudn't open the file %s path, creating file...\n", file_path);
    f = fopen(file_path, "w+");
  } 
  if(f == NULL) {
    printf("Coudn't create the file %s path\n", file_path);
    return;
  }
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
  fseek(f, 0, SEEK_END);
  long file_size = ftell(f);
  rewind(f);

  if(buff->s->cap < file_size + 1) realloc_str(buff->s, file_size + 1);

  if(e->buffs->len > e->buffs->cap - 1) realloc_editor_buffers(e);

  if(access(file_path, W_OK) != 0) buff->is_readonly = true;
  buff->file_path = strdup(file_path);
  size_t bytes_read = fread(buff->s->data, 1, file_size, f);
  if(bytes_read != (size_t) file_size){
    //todo: error
    return;
  }
  fclose(f);
  buff->s->data[file_size ? (file_size - 1) : 0] = 0;
  buff->s->len = bytes_read ? (bytes_read - 1) : 0;
  buff->is_saved = true;
  update_lines(e);
  update_line_number_padding(e);
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

bool is_font_mono(Font font){
  if(font.glyphCount < 95) return false;
  int first = 0, first_valid_index = 0;
  int idx_A = 'A' - 32;
  int idx_Q = '?' - 32;
  if(
    font.glyphs[idx_A].image.width == font.glyphs[idx_Q].image.width &&
    font.glyphs[idx_A].image.height == font.glyphs[idx_Q].image.height &&
    font.glyphs[idx_A].advanceX == font.glyphs[idx_Q].advanceX
  ){
    return false;
  }
  for(int i = 0; i < font.glyphCount; i++){
    if(font.glyphs[i].advanceX != 0) {
      first = font.glyphs[i].advanceX;
      first_valid_index = i;
      break;
    };
  }
  if(first == 0) return false;
  for(int i = first_valid_index + 1; i < font.glyphCount; i++){
    if(abs(font.glyphs[i].advanceX - first) > 1) return false;
  }
  return true;
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
  if(!is_font_mono(font)) {
    printf("Font is not monospaced\n");
    return false;
  } 
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

void copy_selection_to_clipboard(Editor *e){
  if(!e->conf.is_selecting) return;
  Buffer *buff = e->buffs->data[e->current_buff];
  size_t start = MIN(buff->cursor.index, e->conf.selection_start);
  size_t selection_size = abs(buff->cursor.index - e->conf.selection_start) + 1;
  String *selected = new_str(selection_size + 1);
  memcpy(selected->data, &buff->s->data[start], selection_size);
  selected->len = selection_size;
  add_char_to_str(selected, 0, selection_size);
  int success = copy_to_clipboard(selected->data);
  if(success) new_message(e, "Copied", GOOD);
  else new_message(e, "Failed to copy", ERROR);
}

void paste_from_clipboard(Editor *e, bool is_pre_paste){
  Buffer *curr_buff = e->buffs->data[e->current_buff];
  char *clip_buff = read_from_clipboard();
  if(!clip_buff) return;
  String *clip_str = string(clip_buff);
  free(clip_buff);
  bool has_text = curr_buff->s->len > 0;
  bool at_end = curr_buff->cursor.index >= curr_buff->s->len;
  Line line = curr_buff->lines->data[curr_buff->cur_li];
  bool line_empty = line.end - line.start < 1;
  if(line_empty) is_pre_paste = false;
  size_t insert_index = curr_buff->cursor.index + has_text;
  if(has_text) insert_index -= at_end + line_empty + is_pre_paste;
  update_text_action(&curr_buff->cur_act, insert_index, clip_str, true);
  add_str_to_str(curr_buff->s, clip_str, insert_index);
  move_cursor_right(e, clip_str->len - !has_text -
      ((at_end || line_empty || is_pre_paste) && has_text));
  update_lines(e);
  update_scroll(e, curr_buff->cursor.index, false, false);
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
  buff[read] = 0;
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
  if(!strcmp(key, "status_line_fg"))         return STATUS_LINE_FG;
  if(!strcmp(key, "status_line_bg"))         return STATUS_LINE_BG;
  if(!strcmp(key, "selection_bg"))           return SELECTION_BG;
  if(!strcmp(key, "selection_fg"))           return SELECTION_FG;
  if(!strcmp(key, "search_bg"))              return SEARCH_BG;
  if(!strcmp(key, "search_fg"))              return SEARCH_FG;
  if(!strcmp(key, "scroll_padding"))         return SCROLL_PAD;
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
    new_message(e, TextFormat("Error in value [%s] at: %zu", hex, line_number), ERROR);
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
    case STATUS_LINE_FG:
      e->conf.status_line_fg = color;
      break;
    case STATUS_LINE_BG:
      e->conf.status_line_bg = color;
      break;
    case SELECTION_BG:
      e->conf.selection_bg = color;
      break;
    case SELECTION_FG:
      e->conf.selection_fg = color;
      break;
    case SEARCH_BG:
      e->conf.search_bg = color;
      break;
    case SEARCH_FG:
      e->conf.search_fg = color;
      break;
  }
}

void try_setting_conf_number_value(Editor *e, ConfigKey key_type, char *value, size_t line_number){
  char *endpoint;
  int number = strtoul(value, &endpoint, 10);
  if(endpoint == value || *endpoint != 0 || number < 0) {
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
    case SCROLL_PAD:
      e->conf.scroll_pad = number;
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
    exit(1);
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
  key[j] = 0; 
  while(i < len && (line[i] == '=' || isspace(line[i]) )) i++;
  j = 0;
  while(i < len && !isspace(line[i])) value[j++] = line[i++];
  value[j] = 0; 
  ConfigKey key_type = get_config_key(e, key);
  if(key_type == UNKOWN_KEY){
    new_message(e, TextFormat("Unknown key [%s] at config: %d", key, line_number), ERROR);
  }
  ConfigKey col_keys[] =  {
    BG_COL, TXT_COL, CURSOR_COL, UNDER_CURSOR_COL, LN_COL,
    LINES_COL, LINE_HIGHLIGHT_COL, STATUS_LINE_FG, STATUS_LINE_BG,
    SELECTION_BG, SELECTION_FG, SEARCH_BG, SEARCH_FG
  };
  ConfigKey num_keys[] =  {
    TAB_S, P_RIGHT, P_LEFT, P_BOTTOM, P_TOP, FONT_SIZE,
    SECONDARY_FONT_SIZE, SCROLL_PAD
  };
  ConfigKey font_keys[] =  {
    FONT_PRIMARY, FONT_SECONDARY
  };
  if(key_in(key_type, col_keys, sizeof(col_keys) / sizeof(ConfigKey))){
    try_setting_conf_color_value(e, key_type, value, line_number);
  }else if(key_in(key_type, num_keys, sizeof(num_keys) / sizeof(ConfigKey))){
    try_setting_conf_number_value(e, key_type, value, line_number);
  }else if(key_in(key_type, font_keys, sizeof(font_keys) / sizeof(ConfigKey))) {
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
  if(argc > 2){
    fprintf(stderr, "Too many args\n");
    exit(1);
  }
  if(argc > 1 && !strcmp(argv[1], "-v")){
    printf("texteditor: v%s\n", VERSION);
    exit(0);
  }
  else if(argc == 2){
    String *path = string(argv[1]);
    if(path->data[path->len - 1] == '/'){
      snprintf(e->base_dir, sizeof(e->base_dir), "%s", path->data);
    }else {
      read_file(e, path->data);
    }
  }
}

FileType reduce_type(unsigned char d_type){
  switch(d_type){
    case DT_DIR:
       return FT_DIR;
    case DT_LNK:
       return FT_LNK;
    default:
      return FT_REG;
  }
}

void sort_file_list(Files *files){
  if(!files->len) return;
  bool sorted = false;
  while(!sorted){
    sorted = true;
    File temp = {0};
    for(size_t i = 0; i < files->len - 1; i++){
      if(
        files->data[i].type != FT_DIR &&
        files->data[i + 1].type == FT_DIR
      ){
        sorted = false;
        temp = (File) {
          files->data[i].path,
          files->data[i].type
        };
        files->data[i] = files->data[i + 1];
        files->data[i + 1] = temp;
      }
    }
  }
}

void search_files_in_dir(Editor *e, String *dir_path, bool reset){
  const char *dir_name = get_file_name_from_path(dir_path->data);
  if(
    dir_name[0] == '.' || !strcmp(dir_name, "node_modules") ||
    !strcmp(dir_name, "target") || !strcmp(dir_name, "build")
  ) return;
  DIR *dir;
  struct dirent *entry;
  dir = opendir(dir_path->data);
  if(dir == NULL){
    printf("Coundn't open dir %s\n", dir_path->data);
    return;
  }
  if(reset) e->exp->s_matches->len = 0;
  while((entry = readdir(dir)) != NULL){
    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
    String *path = string(dir_path->data);
    add_text_to_str(path, "/", path->len);
    add_text_to_str(path, entry->d_name, path->len);
    File file = {
      path, reduce_type(entry->d_type)
    };
    if(file.type == FT_DIR){
      search_files_in_dir(e, path, false);
    }else {
      if(str_includes(string(entry->d_name), e->query, false)){
        da_append(e->exp->s_matches, file);
      }
    }
  }
  closedir(dir);
}

void read_dir_files(Editor *e, char *dir_path){
  DIR *dir;
  struct dirent *entry;
  dir = opendir(dir_path);
  if(dir == NULL){
    printf("Coundn't open dir %s\n", dir_path);
    return;
  }
  e->exp->files->len = 0;
  while((entry = readdir(dir)) != NULL){
    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
    String *path = string(dir_path);
    add_text_to_str(path, "/", path->len);
    add_text_to_str(path, entry->d_name, path->len);
    File file = {
      path, reduce_type(entry->d_type)
    };
    da_append(e->exp->files, file);
  }
  udpate_explorer_size(e);
  sort_file_list(e->exp->files);
  closedir(dir);
  e->exp->open_dir = string(dir_path);
}
