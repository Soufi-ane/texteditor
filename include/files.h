#ifndef FILES_H
#define FILES_H
// #include <raylib.h>

#define MAX_FILE_NAME_LENGTH 100
#define MAX_FILES_NUM 100


void getDirContent(
	NoteLine* files[],
	int* count,
	const char* path
);

void loadFontSDF(
	char* path,
	int* fileSize,
	int size,
	Font* font
);

void writeFile(
	char* name,
	char* day,
	char* month,
	char* year,
	char* title,
	NoteLine** noteText,
	int linesN
);

void readNote(
	NoteLine* note[],
	NoteLine* title,
	char* y,
	char* m,
	char* d,
	int* Nlines,
	char* path
);

#endif

