#ifndef STR_H
#define STR_H

typedef struct {
  char *data;
  size_t len;
  size_t cap;
} String;

typedef struct {
  size_t start, end;
} Range;

typedef struct {
  Range *data;
  size_t cap;
  size_t len;
} Ranges;

String *new_str(size_t capacity);

void free_str(String *str);

void add_char_to_str(String *str, char c, size_t index);

void str_remove_chars(String *str, size_t index, size_t count);

String *lower_case(String *str);

void realloc_str(String *str, size_t cap);

String *string(const char *text);

String *c_string(char c);

void add_str_to_str(String *dest, String *src, size_t index);

void add_text_to_str(String *dest, const char *txt, size_t index);

size_t last_index_of(String *str, char c);

Ranges *new_ranges(size_t cap);

bool str_includes(String *str, String *sub_str, bool check_case);

#endif
