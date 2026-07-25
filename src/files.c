#include "files.h"
#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <ray.h>
#include <dirent.h>
#include "buffer.h"

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

	fclose(file);
}

void readFile(Editor* e, char* path){
	FILE* f = fopen(path,"r");
  if(!f) {
    printf("Coudn't open the file %s path\n",path);
    exit(1);
  } 
  printf("Reading %s\n",path);
  char* line = NULL;
  size_t size = 0;
  size_t read;
  // e->note->body = malloc(100);
  // e->note->size = 100;
  int i,index = 0;
	while((read = getline(&line,&size,f)) != -1){
    printf("LINE[%s]",line);
    // if(index + read +1 > e->note->size - 1){
      int new_size = read + index + 128 + 1;
      // e->note->body = realloc(e->note->body,new_size);
      // e->note->size = new_size;
    }
    for(i=0; i < read;i++) {
      // if(line[i] == '\n') e->note->linesNum++;
      // e->note->body[index++] = line[i];
    }
	// }
  // free(line);
  // e->note->length = index;
  // printf("total size : %d\n",e->note->length);
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


