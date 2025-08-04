#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>
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

void getDirContent(Editor* e,Line* files[],int* count,const char* path){
	struct dirent* entry;
	DIR* dir = opendir(path);
	int i = 0;
	while((entry = readdir(dir)) != NULL){
		if(entry->d_type == DT_REG){
			files[i] = createLine(LINE_LENGTH);
			addLine(e,files[i++],entry->d_name);
		}
		*count = i;
	}
	closedir(dir);
}
void writeFile(Editor* e){
  char savePath[100];
  sprintf(
    savePath,
    "%s/.local/notes/%s", 
    e->HOME_DIR, 
    e->currentFileName->length > 1 ? e->currentFileName->chars : "untitled");
	FILE* file = fopen(savePath,"w");
  printf("saving file %s ...\n",savePath);
	fprintf(file,"----------------\n");
	fprintf(file,"date: ");
	fprintf(file,e->note.date.day);
	fprintf(file,"-");
	fprintf(file,e->note.date.month);
	fprintf(file,"-");
	fprintf(file,e->note.date.year);
	fprintf(file,"\ntitle: ");
	fprintf(file,e->note.title->chars);
  fprintf(file,"----------------\n\n");
	for(int i=0; i<e->note.linesNum; i++){
		fprintf(file,e->note.body[i]->chars);
	}
  char msg[100];
  sprintf(msg,"File saved: %s",e->currentFileName->chars);
  addLine(e,e->message,msg);
	fclose(file);
}

void readNote(Editor* e, char* path){
	FILE* noteFile = fopen(path,"r");
  if(!noteFile) exit(1);
	char line[100];
	int i = 0;
	int charsN = 0;
	while(fgets(line,sizeof(line),noteFile) != NULL){
    printf("reading line\n");
		e->note.body[i] = createLine(LINE_LENGTH);
		if (i > 3) {
      if(line[0] != '\n'){
        addLine(e,e->note.body[i-4],line);
        printf("[%d] found a line:\n\t[%s]\n",i,line);
        e->note.linesNum++;
      }
    } 
		else if(i == 1) {
			int d = sscanf(
				line,
				"date: %2c-%2c-%4c%n",
				e->note.date.day,e->note.date.month,e->note.date.year,&charsN);
      if(d < 3) {
        printf("sscanf returned %d\n",d);
        return;
      }
		}
		else if(i == 2){
			int d = sscanf(
				line,
				"title: %20[^\n]%n",
				e->note.title->chars, &charsN
			);
      if(d < 1) {
        printf("sscanf returned %d\n",d);
        return;
      }
			e->note.title->length = charsN - 7;
		}
    if(line[0] != '\n') i++;
    else e->note.body[i-1]->length = 0;
	}
	if(e->note.linesNum > 1) e->note.linesNum = i - 4; // 4 for metadata;
  printf("lines : %d\n",e->note.linesNum);
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
	// LoadShader(0,TextFormat("resources/shaders/glsl330/sdf.fs",GLSL_VERSION));
	SetTextureFilter(font->texture,TEXTURE_FILTER_BILINEAR);
}

void find(
	Line* fileNames[],
	int numFiles,
	Line* query,
	Line* resultFiles[],
	int* numResult
){
	int p = 0;
	int scores[numFiles];
	for(int i=0;i<numFiles;i++){
		int score = 0;
		for(int j=0;j<query->length;j++){
			for(int k=0;k<fileNames[i]->length;k++) {
				char qChar = query->chars[j];
				char fChar = fileNames[i]->chars[k];
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
}
