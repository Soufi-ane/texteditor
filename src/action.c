#include <stdlib.h>
#include "io.h"
#include "editor.h"

void action_stack_push(Text_AStack *stack, TextAction action){
  if(stack->top >= MAX_STACK_SIZE - 1){
    for(int i = 0; i < MAX_STACK_SIZE - SHIFT_SIZE; i++){
      stack->actions[i] = stack->actions[i + SHIFT_SIZE];
    }
    stack->top = MAX_STACK_SIZE - SHIFT_SIZE + 1;
  }
  stack->actions[++stack->top] = action;
}

FileAction* new_file_action(){
  FileAction *act = malloc(sizeof(FileAction));
  return act;
}

TextAction* init_text_action(size_t index){
  TextAction *act = malloc(sizeof(TextAction));
  act->index = index;
  act->old = new_str(DEFAULT_LINE_SIZE);
  act->new = new_str(DEFAULT_LINE_SIZE);
  return act;
}

void undo_text_action(Editor *e, TextAction *act){
  Buffer *buff = e->buffs->data[e->current_buff];
  bool is_left = buff->cursor.index >= (act->index + act->old->len);
  bool is_normal = e->conf.is_vim_mode && e->mode == NORMAL;

  str_remove_chars(buff->s, act->index, act->new->len);
  add_str_to_str(buff->s, act->old, act->index);
  update_lines(e);

  if(is_left){
    move_cursor_left(
      e, buff->cursor.index - act->index - act->old->len + is_normal - 1
    );
  } 
  else {
    move_cursor_right(
      e, act->index - buff->cursor.index + act->old->len - is_normal
   );
  } 
  update_lines(e);
  buff->is_saved = false;
}

void redo_text_action(Editor *e, TextAction *act){
  Buffer *buff = e->buffs->data[e->current_buff];
  str_remove_chars(buff->s, act->index, act->old->len);
  buff->cursor.index = act->index;
  add_str_to_str(buff->s, act->new, act->index);
  move_cursor_right(e, act->new->len - 1);
  buff->is_saved = false;
}

void undo(Editor *e){
  Buffer *buff = e->buffs->data[e->current_buff];
  if(e->exp->is_open) {
    FileAction *act = stack_pop(&e->exp->undo_stack);
    if(act == NULL) return;
    move_file(act->new->data, act->old->data);
    stack_push(&e->exp->redo_stack, *act);
    read_dir_files(e, e->exp->open_dir->data);
  }else {
    TextAction *act = stack_pop(&buff->undo_stack);
    if(act == NULL) return;
    stack_push(&buff->redo_stack, *act);
    undo_text_action(e, act);
    update_scroll(e, buff->cursor.index, false, false);
  }
  update_buf_state(e);
}

void redo(Editor *e){
  Buffer *buff = e->buffs->data[e->current_buff];
  if(e->exp->is_open){
    FileAction *act = stack_pop(&e->exp->redo_stack);
    if(act == NULL) return;
    move_file(act->old->data, act->new->data);
    stack_push(&e->exp->undo_stack, *act);
    read_dir_files(e, e->exp->open_dir->data);
  }else {
    TextAction *last_action = stack_pop(&buff->redo_stack);
    if(last_action == NULL) return;
    stack_push(&buff->undo_stack, *last_action);
    redo_text_action(e, last_action);
    update_scroll(e, buff->cursor.index, false, false);
  }
  update_buf_state(e);
}

void free_text_action(TextAction *act){
  free_str(act->new);
  free_str(act->old);
  free(act);
  act= NULL;
}

void update_text_action(TextAction **act, size_t index, String *str, bool is_new){
 if(*act == NULL){
   *act = init_text_action(index);
 }
 if(is_new) {
   add_str_to_str((*act)->new, str, (*act)->new->len);
 }else {
   (*act)->index = index;
   if((*act)->new->len) {
     str_remove_chars((*act)->new, (*act)->new->len - str->len, str->len);
   }else {
     add_str_to_str((*act)->old, str, 0);
   }
 }
}
