#ifndef FILES_H
#define FILES_H
// #include <raylib.h>

#define MAX_FILE_NAME_LENGTH 100
#define MAX_FILES_NUM 100


void getDirContent(
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

void writeFile(
	char* name,
	char* day,
	char* month,
	char* year,
	char* title,
	Line** noteText,
	int linesN
);

void readNote(
	Line* note[],
	Line* title,
	char* y,
	char* m,
	char* d,
	int* Nlines,
	char* path
);

void find(
	Line* fileNames[],
	int numFiles,
	Line* query,
	Line* resultFiles[],
	int* numResult
);
#endif

