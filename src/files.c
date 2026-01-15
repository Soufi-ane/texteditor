#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <ray.h>
#include "note.h"
#include <dirent.h>

void refreshDiplayedFiles(Editor* e){
  int j = 0;
  for(
      int i = e->displayedFilesStart;
      j < MAX_DISPLAYED_FILES;
      i++,j++)
  {
    e->displayedNames[j] = e->fileNames[i];
  }
}

void getDirContent(Editor* e,char* files[],int* count,const char* path){
	struct dirent* entry;
	DIR* dir = opendir(path);
	int i = 0;
	while((entry = readdir(dir)) != NULL){
		if(entry->d_type == DT_REG){
			files[i] = malloc(sizeof(char) * (LINE_LENGTH));
      memcpy(files[i++],entry->d_name,strlen(entry->d_name) + 1);
		}
		*count = i;
	}
	closedir(dir);
}
void writeFile(Editor* e){
  char savePath[100];
  char date_str[11];

  snprintf(date_str,sizeof(date_str),"%s-%s-%s", e->note->date.day,
  e->note->date.month, e->note->date.year);

  sprintf( savePath, "%s/.local/notes/%s", e->HOME_DIR,
  strlen(e->currentFileName) > 1 ? e->currentFileName : date_str);

	FILE* file = fopen(savePath,"w");
  printf("saving file %s ...\n",savePath);

  if(e->isNoteBookMode){
    fprintf(file,"----------------\n");
    fprintf(file,"date: ");
    fprintf(file,"%s-%s-%s\n",e->note->date.day,e->note->date.month,e->note->date.year);
    fprintf(file,"title: ");
    if(!strlen(e->note->title)) fprintf(file,"Untitled\n");
    else fprintf(file,"%s\n",e->note->title);
    fprintf(file,"----------------\n\n");
  }
  e->note->body[e->note->length] = '\0';
  printf("[[%.*s]]\n",e->note->length,e->note->body);
  fprintf(file,"%s",e->note->body);
  char msg[100];
  sprintf(msg,"File saved: %s",e->currentFileName);
  memcpy(e->message,msg,strlen(msg));
	fclose(file);
}

void readFile(Editor* e, char* path){
	FILE* f = fopen(path,"r");
  if(!f) {
    printf("Coudn't open the file %s path\n");
    exit(1);
  } 
  printf("Reading %s\n",path);
  char* line = NULL;
  size_t size = 0;
  size_t read;
  e->note->body = malloc(100);
  e->note->size = 100;
  int i,index = 0;
	while((read = getline(&line,&size,f)) != -1){
    printf("LINE[%s]",line);
    if(index + read +1 > e->note->size - 1){
      int new_size = read + index + LINE_LENGTH + 1;
      e->note->body = realloc(e->note->body,new_size);
      e->note->size = new_size;
    }
    for(i=0; i < read;i++) {
      if(line[i] == '\n') e->note->linesNum++;
      e->note->body[index++] = line[i];
    }
	}
  free(line);
  e->note->length = index;
  printf("total size : %d\n",e->note->length);
}

void readNote(Editor* e, char* path){
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
      printf("line : [%s][%d]",line,read);
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

/* void find(
	char* fileNames[],
	int numFiles,
	char* query,
	char* resultFiles[],
	int* numResult
){
	int p = 0;
	int scores[numFiles];
	for(int i=0;i<numFiles;i++){
		int score = 0;
		for(int j=0;j<query->length;j++){
			for(int k=0;k<fileNames[i]->length;k++) {
				char qChar = query[j];
				char fChar = fileNames[i][k];
				qChar = qChar < 91 ? qChar + 32 : qChar;
				fChar = fChar < 91 ? fChar + 32 : fChar;
				if(qChar == fChar) score++;
			}
		}
		// firstNote
		if(score >= query->length){
			scores[i] = score;
			resultFiles[p++] = fileNames[i];
		}
	}
	*numResult = p;
} */
