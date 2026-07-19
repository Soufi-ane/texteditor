#include "files.h"
#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <ray.h>
#include "buffer.h"
#include <dirent.h>

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

void getDirContent(Editor* e,char* files[],int* count,const char* path){
	struct dirent* entry;
	DIR* dir = opendir(path);
	int i = 0;
	while((entry = readdir(dir)) != NULL){
		if(entry->d_type == DT_REG){
			files[i] = malloc(sizeof(char) * 128);
      memcpy(files[i++],entry->d_name,strlen(entry->d_name) + 1);
		}
		*count = i;
	}
	closedir(dir);
}
void writeFile(Editor* e){
  char savePath[100];
  char date_str[11];

  /* snprintf(date_str,sizeof(date_str),"%s-%s-%s", e->note->date.day,
  e->note->date.month, e->note->date.year); */

  sprintf( savePath, "%s/.local/notes/%s", e->HOME_DIR,
  strlen(e->currentFileName) > 1 ? e->currentFileName : date_str);

	FILE* file = fopen(savePath,"w");
  printf("saving file %s ...\n",savePath);

  /* if(e->conf.isNoteBookMode){
    fprintf(file,"----------------\n");
    fprintf(file,"date: ");
    fprintf(file,"%s-%s-%s\n",e->note->date.day,e->note->date.month,e->note->date.year);
    fprintf(file,"title: ");
    if(!strlen(e->note->title)) fprintf(file,"Untitled\n");
    else fprintf(file,"%s\n",e->note->title);
    fprintf(file,"----------------\n\n");
  } */
  // e->note->body[e->note->length] = '\0';
  // printf("[[%.*s]]\n",e->note->length,e->note->body);
  // fprintf(file,"%s",e->note->body);
  char msg[100];
  sprintf(msg,"File saved: %s",e->currentFileName);
  memcpy(e->message,msg,strlen(msg));
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

/* void readNote(Editor* e, char* path){
	FILE* noteFile = fopen(path,"r");
  if(!noteFile) exit(1);
	int i = 0;
  int position = 0;
  char* line = NULL;
  size_t len;
  size_t read;
  size_t current_length = 0;
	while((read = getline(&line,&len,noteFile)) != -1){
    printf("i = %d\n",i);
    if(i>4) {
      printf("line : [%s][%zu]",line,read);
      current_length += read;
      if(i == 5){ 
        e->note->body = malloc(read + 1);
      } else {
        e->note->body = realloc(e->note->body,current_length + 1);
      }
      for(int n = 0; n < read;position++) {
        if(line[n] == '\n') e->note->linesNum++;
        e->note->body[position] = line[n++];
      }
    }
    else {
      if(i == 1){
        printf("[1] : %s",line);
        int d = sscanf(
          line,
          "date: %2c-%2c-%4c",
          e->note->date.day,e->note->date.month,e->note->date.year);
        if(d < 3) {
          printf("[date] sscanf returned %d\n",d);
          return;
        }
      } 
      else if(i == 2){
        int read_chars;
        int d = sscanf(
          line,
          "title: %20[^\n]%n",
          e->note->title, &read_chars
        );
        if(d < 1) {
          printf("sscanf returned %d\n",d);
          return;
        }
        e->note->title[read_chars - 7] = '\0';
      }
    } 
    i++;
	}
  e->note->length = position;
  printf("total size : %d\n",e->note->length);
} */

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


