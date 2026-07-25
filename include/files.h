#ifndef FILES_H
#define FILES_H
#include <raylib.h>
#include "buffer.h"

extern char* menu_icons_names[];

void load_menu_icons(Texture2D* icons);

void loadFontSDF(char* path, int* fileSize, int size, Font* font);

void read_file(Editor* e, char const * file_path);

void find(char* fileNames[], int numFiles, char* query, int result_ids[], int* numResult);

bool str_includes(char* str,char* sub_str);

void try_saving_file(Editor* e);

const char *get_file_name_from_path(const char *path);

#endif

