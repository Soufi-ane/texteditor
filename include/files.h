#ifndef FILES_H
#define FILES_H
#include <raylib.h>
#include "buffer.h"

extern char* menu_icons_names[];

void load_menu_icons(Texture2D* icons);

void load_font_sdf(char* path, int* fileSize, int size, Font* font);

void read_file(Editor* e, char const * file_path);

bool str_includes(const char* str, char* sub_str, size_t sub_str_length);

void try_saving_file(Editor* e);

const char *get_file_name_from_path(const char *path);

int copy_to_clipboard(const char *text);

void copy_selection_to_clipboard(Editor *e);

void read_from_clipboard(char *buff, size_t max);

void paste_from_clipboard(Editor *e);

int try_loading_config(Editor *e);

void write_new_message(Editor *e, Message *msg);

#endif

