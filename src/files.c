#include <stdio.h>
#include <raylib.h>
#include <ray.h>
#include "note.h"
#include <dirent.h>

void getDirContent(Line* files[],int* count,const char* path){
	struct dirent* entry;
	DIR* dir = opendir(path);
	int i = 0;
	while((entry = readdir(dir)) != NULL){
		if(entry->d_type == DT_REG){
			files[i] = createLine(LINE_LENGTH);
			addLine(files[i++],entry->d_name);
		}
		*count = i;
	}
	closedir(dir);
}
void writeFile(char* name,char* day,char* month,char* year,
		char* title,Line** noteText,int linesN){
	FILE* file = fopen(name,"w");
	fprintf(file,day);
	fprintf(file,month);
	fprintf(file,year);
	fprintf(file," ");
	fprintf(file,title);
	fprintf(file,"\n");
	for(int i=0; i<linesN; i++){
		fprintf(file,noteText[i]->chars);
	}
	fclose(file);
}

void readNote(
	Line* note[],
	Line* title,
	char* y,
	char* m,
	char* d,
	int* Nlines,
	char* path
){
	FILE* noteFile = fopen(path,"r");
	char line[100];
	int i = 0;
	int charsN = 0;
	while(fgets(line,sizeof(line),noteFile) != NULL){
		printf("%s\n",line);
		note[i] = createLine(LINE_LENGTH);
		if(i > 0) addLine(note[i-1],line);
		else {
			sscanf(
				line,
				"%2c%2c%4c %20[^\n]%n",
				d,m,y,
				title->chars,
				&charsN
			);
			title->length = charsN - 9;
			printf("[%s] ==> %d\n",title->chars,title->length);
		}
		i++;
	}
	*Nlines = --i;
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
	printf("============================\n");
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
		// first
		// firstNote
		if(score >= query->length){
			scores[i] = score;
			resultFiles[p++] = fileNames[i];
		}
		printf("----------------------\n");
		printf("[%s] %d\n",fileNames[i]->chars,score);
	}
	*numResult = p;
}





