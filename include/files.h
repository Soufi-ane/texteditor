#ifndef FILES_H
#define FILES_H
#include <raylib.h>
#include "note.h"

#define MAX_FILE_NAME_LENGTH 100


void refreshDiplayedFiles(Editor* e);

void getDirContent(
  Editor* e,
	Line* files[],
	int* count,
	const char* path
);

void loadFontSDF(
	char* path,
	int* fileSize,
	int size,
	Font* font
);

void writeFile(Editor* e);

void readNote(Editor* e, char* path);

void find(
	Line* fileNames[],
	int numFiles,
	Line* query,
	Line* resultFiles[],
	int* numResult
);
#endif

