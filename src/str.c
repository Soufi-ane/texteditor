#include <string.h>
#include <stdlib.h>
#include "str.h"

String *new_str(size_t cap){
  String *str = malloc(sizeof(String));
  str->data = malloc(sizeof(char) * cap);
  str->data[0] = 0;
  str->cap = cap;
  str->len = 0;
  return str;
}

void free_str(String *str){
  if(str != NULL){
    free(str->data);
    free(str);
    str = NULL;
  }
}
void add_char_to_str(String *str, char c, size_t index){
  if(index < 0 || index > str->len || str == NULL) return;
  if(str->cap <= str->len - 1) realloc_str(str, str->cap * 2);
  memmove(&str->data[index + 1], &str->data[index], str->len - index);
  str->data[index] = c;
  str->data[++str->len] = 0;
}

void add_str_to_str(String *dest, String *src, size_t index){
  if(index < 0 || index > dest->len || dest == NULL || src == NULL) return;
  if(src->len >= dest->cap - dest->len) {
    size_t new_cap = dest->cap + src->len;
    realloc_str(dest, new_cap);
  } 
  memmove(&dest->data[index + src->len], &dest->data[index], dest->len - index);
  memcpy(&dest->data[index], src->data, src->len);
  dest->len += src->len;
  dest->data[dest->len] = 0;
}

void add_text_to_str(String *dest, const char *txt, size_t index){
  size_t txt_len = strlen(txt);
  if(index < 0 || index > dest->len || dest == NULL || !txt_len) return;
  if(txt_len >= dest->cap - dest->len) {
    size_t new_cap = dest->cap + txt_len;
    realloc_str(dest, new_cap);
  } 
  memmove(&dest->data[index + txt_len], &dest->data[index], dest->len - index);
  memcpy(&dest->data[index], txt, txt_len);
  dest->len += txt_len;
  dest->data[dest->len] = 0;
}

void str_remove_chars(String *str, size_t index, size_t count){
  if(!str || !str->len) return;
  memmove(
    &str->data[index],
    &str->data[index + count],
    str->len - index - count + 1
  );
  str->len -= count;
}

String *lower_case(String *str){
  String *result = new_str(str->len + 1);
  for(size_t i = 0; i < str->len; i++){
    char c = str->data[i];
    if(c >= 'A' && c <= 'Z'){
      result->data[i] =  c + 32;
    } else result->data[i] = c;
    result->len++;
  }
  return result;
}

void realloc_str(String *str, size_t cap){
  if(!str) return;
  char *temp = realloc(str->data, cap * sizeof(char)); 
  if(!temp) return;
  str->data = temp;
  str->cap = cap;
}

String *c_string(char c){
  String *str = new_str(2);
  str->len = 1;
  str->data[0] = c;
  str->data[1] = 0;
  return str;
}

String *string(const char *text){
  size_t len = strlen(text);
  String *str = new_str(len + 1);
  str->len = len;
  memcpy(str->data, text, len + 1);
  return str;
}

size_t last_index_of(String *str, char c){
  if(!str->len) return 0;
  for(size_t i = str->len - 1; i > 0 ; i--){
    if(str->data[i] == c) return i;
  }
  return 0;
}

Ranges *new_ranges(size_t cap){
  Ranges *rs = malloc(sizeof(Ranges));
  rs->data = malloc(sizeof(Range) * cap);
  rs->cap = cap;
  rs->len = 0;
  return rs;
}
