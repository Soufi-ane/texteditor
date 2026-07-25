#include "files.h"
#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ray.h>
#include <dirent.h>
#include "buffer.h"
#include "tinyfiledialogs.h"

char* menu_icons_names[NUM_MENU_ICONS] = {
  "sticky-note.png"
};

void load_menu_icons(Texture2D* icons){
  for(int i = 0 ; i < NUM_MENU_ICONS; i++){
    Image icon_image = LoadImage(TextFormat("assets/img/%s",menu_icons_names[i])); 
    Texture2D icon_texture = LoadTextureFromImage(icon_image);
    icons[i] = icon_texture;
    UnloadImage(icon_image);
  }
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
    // printf("LINE[%s]",line);
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

      line_index++;
      e->buffer.length++;
    }
  free(line);
}

void try_saving_file(Editor* e){
  if(e->buffer.file_path){
    if(access(e->buffer.file_path, W_OK) == 0){
      write_file(e);
    } else {
    }
  }
  else {
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

void find(char* fileNames[], int numFiles, char* query, int result_ids[], int* numResult){
  *numResult = 0;
  for(int i = 0; i < numFiles; i++){
    if(str_includes(fileNames[i],query)) result_ids[*numResult++] = i;
  }
}

bool str_includes(char* str,char* sub_str){
  size_t len1 = strlen(str);
  size_t len2 = strlen(sub_str);
  int i;
  for(i = 0; i < len1; i++) if(str[i] == sub_str[0]) break;
  if(i > len1 - 1) return false; // no match for first char
  for(i = 1; i < len2; i++){
    for(int j = 1; j < len2; j++){
      if(sub_str[i] != str[j]) return false; // doesn't match the full string
    }
  }
  return true;
}


