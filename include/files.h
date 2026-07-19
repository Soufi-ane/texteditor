#ifndef FILES_H
#define FILES_H
#include <raylib.h>
#include "buffer.h"

#define MAX_FILE_NAME_LENGTH 100

extern char* menu_icons_names[];

void load_menu_icons(Texture2D* icons);

void getDirContent(Editor* e, char* files[], int* count, const char* path);

void loadFontSDF(char* path, int* fileSize, int size, Font* font);

void writeFile(Editor* e);

void readNote(Editor* e, char* path);

void readFile(Editor* e, char* path);

void find(char* fileNames[], int numFiles, char* query, int result_ids[], int* numResult);

bool str_includes(char* str,char* sub_str);

#endif

