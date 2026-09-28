#ifndef IO_H
#define IO_H

#include "draw.h"

bool load_font_default(Editor *e, FontData *font_data);

void read_file(Editor* e, char const * file_path);

bool str_includes(String *str, String *sub_str);

void try_saving_file(Editor* e);

const char *get_file_name_from_path(const char *path);

int copy_to_clipboard(const char *text);

void copy_selection_to_clipboard(Editor *e);

char *read_from_clipboard();

void paste_from_clipboard(Editor *e);

int try_loading_config(Editor *e);

void write_new_message(Editor *e, Message *msg);

void handle_cmd_args(Editor *e, int argc, char **argv);

void read_dir_files(Editor *e, char *dir_path);

bool create_file(Editor *e, char *path);

void remove_file(Editor *e, char *path);

void move_to_trash(Editor *e, char *path);

bool move_file(char *src, char *dest);

void clear_trash(Editor *e);

void search_files_in_dir(Editor *e, String *dir_path, bool reset);

#endif
