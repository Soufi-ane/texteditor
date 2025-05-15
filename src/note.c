#include <stdio.h>
#include <stdlib.h>
#include "note.h"
#include <string.h>


Line* createLine(int n){
	Line* noteLine = (Line*) malloc(sizeof(Line));
	noteLine->size = n;
	noteLine->chars = (char*) malloc(sizeof(char) * n);
	for(int i=0; i<n; i++) noteLine->chars[i] = '\0';
	noteLine->length = 0;
	return noteLine;
}
void deleteLine(Line** line){
	free((*line)->chars);
	free((*line));
	*line = NULL;
}

void emptyLine(Line* line,int n){
	memset(line->chars,0,n);
	line->size = n;
	line->length = 0;
}

void addChar(Line* line,char c){
	if(line->length > line->size - 1) {
		line->size = line->size + 100;
		char* newChars = (char*) realloc(line->chars,line->size * sizeof(char));
		for(int i=0;i< line->length; i++){
			newChars[i] = line->chars[i];
		}
		line->chars = newChars;
	}
	line->chars[line->length] = c;
	line->length++;
}

void addLine(Line* line,char* text){
	int length = strlen(text);
	// if(length > line->size) {
		// return;
	// }
	int i;
	for(i=0;i< length; i++){
		addChar(line, text[i]);
	}
	// addChar(line,'\0');
	// line->length = length ;
}


void popChar(Line* line){
	if(line->length){
		line->length--;
		line->chars[line->length] = '\0';
	}
}

