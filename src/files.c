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
  e->buffer.file_path = file_path;
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
  for(i = 0; i < str_len; i++) if(buff[i] == query[0]) break;
  if(i > str_len - 1) return false;
  i++;
  for(j = 1; (j < sub_str_length && i < str_len); j++, i++){
    if(query[j] != buff[i]) return false;
  }
  return j >= sub_str_length;
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


