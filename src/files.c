#include <ctype.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "files.h"
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

void write_file(Editor* e){
	FILE* file = fopen(e->buffer.file_path, "w");
  if(file == NULL){
    // TODO : show error
    return;
  }
  for(size_t i = 0; i < e->buffer.length; i++){
    Line *line = e->buffer.lines[i];
    fwrite(line->chars, sizeof(char), sizeof(char) * line->length, file);
    fputc('\n', file);
  }
	fclose(file);
}

void read_file(Editor* e, char const * file_path){
	FILE* f = fopen(file_path,"r");
  if(!f) {
    printf("Coudn't open the file %s path\n",file_path);
    return;
  } 
  char* line = NULL;
  size_t size = 0, line_index = 0;
  size_t read;
  e->buffer = *new_buffer(128);
  e->buffer.file_path = strdup(file_path);
	while((read = getline(&line,&size,f)) != -1){
    if(e->buffer.length > e->buffer.capacity - 1){
      size_t new_capacity = e->buffer.capacity + 10;
      e->buffer.lines = realloc(e->buffer.lines, sizeof(Line*) * new_capacity);
      for(size_t i = e->buffer.capacity; i < new_capacity; i++){
        e->buffer.lines[i] = new_line(DEFAULT_LINE_SIZE);
      }
      e->buffer.capacity = new_capacity;
    }
    for(size_t i = 0; i < read - 1; i++) {
      add_char_to_line(e, e->buffer.lines[line_index], line[i], true);
    }
    e->buffer.num_chars += read;
    line_index++;
    e->buffer.length++;
  }
  if(e->buffer.length > 1) e->buffer.length--;
  free(line);
}

void try_saving_file(Editor* e){
  if(e->buffer.file_path){
    if(access(e->buffer.file_path, W_OK) == 0 || errno == ENOENT) {
      write_file(e);
      new_message(e, "Saved!", GOOD);
    } 
    if (errno == EACCES) {
      new_message(e, "Readonly file!", ERROR);
    }
  }
  if(!e->buffer.file_path) {
    char const * path = 
    tinyfd_saveFileDialog(
     "Save File",
     e->buffer.file_path != NULL ? e->buffer.file_path : "Untitled"
     , 0, NULL, NULL);
    if (path != NULL) {
      e->buffer.file_path = path;
      try_saving_file(e);
    }
  }
}

void loadFontSDF(
		char* path,
		int* fileSize,
		int size,
		Font* font
){
	unsigned char* fontFile = LoadFileData(path,fileSize);
	if(fontFile == NULL || fontFile == 0){
		fontFile = LoadFileData("assets/fonts/JetBrainsMonoNF.ttf",fileSize);
	}
	font->baseSize = size;
	font->glyphCount = 95;
	font->glyphs = LoadFontData(fontFile,*fileSize,size,0,0,FONT_SDF);
	Image atlas = GenImageFontAtlas(font->glyphs,&font->recs,95,size,0,1);
	font->texture = LoadTextureFromImage(atlas);
	UnloadImage(atlas);
	UnloadFileData(fontFile) ;
	SetTextureFilter(font->texture,TEXTURE_FILTER_BILINEAR);
}

bool str_includes(const char* str, char* sub_str, size_t sub_str_length){
  size_t str_len = strlen(str);
  char buff[str_len], query[sub_str_length]; 
  to_lower_case(str, buff);
  to_lower_case(sub_str, query);
  int i, j;
  for(i = 0; i < str_len; i++) {
    if(buff[i] == query[0]) {
      for(j = 1; (j < sub_str_length && i < str_len - 1); j++, i++){
        if(query[j] != buff[i + 1]) break;
      }
      if(j >= sub_str_length) return true;
    }
  }
  return false;
}

void copy_selection_to_clipboard(Editor *e){
  Line *selected = new_line(DEFAULT_LINE_SIZE * 10);
  size_t current_index = e->buffer.current_line_index;
  bool is_up = is_selecting_up(e);
  for(
    int i = (is_up ? current_index : e->conf.selection_start.row);
    i <= (is_up ? e->conf.selection_start.row : current_index);
    i++
  ){
    for(int j = 0; j < e->buffer.lines[i]->length; j++){
      if(is_selected(e, (RowCol){i, j}))
      add_char_to_line(e, selected, e->buffer.lines[i]->chars[j], true);
    }
    if(i < (is_up ? e->conf.selection_start.row : current_index))
    add_char_to_line(e, selected, '\n', true);
  }
  add_char_to_line(e, selected, '\0', true);
  int success = copy_to_clipboard(selected->chars);
  if(success) new_message(e, "Copied!", GOOD);
  else new_message(e, "Failed to copy!", ERROR);
}

void paste_from_clipboard(Editor *e){
  char buff[MAX_PASTE_LENGTH] = {0};
  read_from_clipboard(buff, sizeof(buff));
  for(int i = 0; buff[i] != '\0'; i++){
    if(buff[i] == '\n') {
      start_new_line(e);
    } 
    else {
      add_char_to_line(e, e->buffer.lines[e->buffer.current_line_index], buff[i],false);
    }
  }
}

int copy_to_clipboard(const char *text){
  FILE *pipe = popen("xclip -selection clipboard 2>/dev/null || wl-copy 2>/dev/null", "w");
  if(!pipe) return 0;
  fputs(text, pipe);
  int status = pclose(pipe);
  return status == 0;
}

void read_from_clipboard(char *buff, size_t max){
  FILE *pipe = popen("xclip -selection clipboard -o 2>/dev/null || wl-paste 2>/dev/null", "r");
  if(!pipe) return;
  buff[0] = '\0';
  char line[1024];
  while(fgets(line, sizeof(line), pipe) != NULL){
    size_t len = strlen(buff);
    size_t rest = max - len - 1;
    if(rest == 0) break;
    strncat(buff, line, rest);
  }
  pclose(pipe);
}

ConfigKey get_config_key(Editor *e, char *key){
  if(!strcmp(key, "line_numbers_mode"))    return LN_MODE;
  if(!strcmp(key, "spaces_for_tabs"))      return SPACE_FOR_TAB;
  if(!strcmp(key, "vim_mode"))             return VIM_M;
  if(!strcmp(key, "draw_lines"))           return D_LINES;
  if(!strcmp(key, "background_color"))     return BG_COL;
  if(!strcmp(key, "text_color"))           return TXT_COL;
  if(!strcmp(key, "cursor_color"))         return CURSOR_COL;
  if(!strcmp(key, "under_cursor_color"))   return UNDER_CURSOR_COL;
  if(!strcmp(key, "line_numbers_color"))   return LN_COL;
  if(!strcmp(key, "lines_color"))          return LINES_COL;
  if(!strcmp(key, "tab_size"))             return TAB_S;
  if(!strcmp(key, "caps_lock_as_escape"))  return CAPS_AS_ESCAPE;
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
    new_message(e, TextFormat("Error in value [%s] in config at %zu", hex, line_number), ERROR);
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
      e->cursor.color = color;
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
  }
}

void try_setting_conf_number_value(Editor *e, ConfigKey key_type, char *value, size_t line_number){
  char *endpoint;
  int number = strtoul(value, &endpoint, 10);
  if(endpoint == value || *endpoint != '\0') {
    new_message(e, TextFormat("Invalid value [%s] at config : %zu", value, line_number), ERROR);
  }
  switch (key_type) {
    case TAB_S:
      e->conf.tab_size = number;
      break;
  }
}

void try_setting_conf_value(Editor *e, ConfigKey key_type, char *value){
  switch (key_type) {
    case LN_MODE :
      if(!strcmp(value, "relative"))      e->conf.ln_mode = RELATIVE;
      else if(!strcmp(value, "absolute")) e->conf.ln_mode = ABSOLUTE;
      else if(!strcmp(value, "none"))     e->conf.ln_mode = NONE;
      // else 
      break;
    case SPACE_FOR_TAB :
      if(!strcmp(value, "true"))       e->conf.is_spaces_for_tabs = true;
      else if(!strcmp(value, "false")) e->conf.is_spaces_for_tabs = false;
      // else
      break;
    case VIM_M :
      if(!strcmp(value, "true"))       e->conf.is_vim_mode = true;
      else if(!strcmp(value, "false")) e->conf.is_vim_mode = false;
      // else
      break;
    case D_LINES:
      if(!strcmp(value, "true"))       e->conf.is_showing_lines = true;
      else if(!strcmp(value, "false")) e->conf.is_showing_lines = false;
      // else
      break;
    case CAPS_AS_ESCAPE:
      if(!strcmp(value, "true"))       e->conf.caps_lock_as_escape = true;
      else if(!strcmp(value, "false")) e->conf.caps_lock_as_escape = false;
      // else
      break;
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
    key_type == UNDER_CURSOR_COL || key_type == LN_COL || key_type == LINES_COL
    ){
    try_setting_conf_color_value(e, key_type, value, line_number);
  }else if(key_type == TAB_S){
    try_setting_conf_number_value(e, key_type, value, line_number);
  }
  else try_setting_conf_value(e, key_type, value);
}

int try_loading_config(Editor *e){
  if (e->HOME_DIR == NULL)  {
    new_message(e, "$HOME environment variable is not set", ERROR);
    return 0;
  }
  char conf_path[1024];
  sprintf(conf_path, "%s/.config/texteditor/texteditor.conf", e->HOME_DIR);
  FILE *conf_file = fopen(conf_path, "r");
  if(conf_file == NULL)  {
    new_message(e, "Error loading config file", ERROR);
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

