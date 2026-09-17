#include <stdlib.h>
#include "editor.h"

Action* init_action(size_t index){
  Action *action = malloc(sizeof(Action));
  action->index = index;
  action->old = new_str(DEFAULT_LINE_SIZE);
  action->new = new_str(DEFAULT_LINE_SIZE);
  return action;
}

void action_stack_push(ActionStack *stack, Action action){
  if(stack->top >= MAX_STACK_SIZE - 1){
    int shift_size = 20;
    for(int i = 0; i < MAX_STACK_SIZE - shift_size; i++){
      stack->actions[i] = stack->actions[i + shift_size];
    }
    stack->top = MAX_STACK_SIZE - shift_size + 1;
  }
  stack->actions[++stack->top] = action;
}

Action *action_stack_pop(ActionStack *stack){
  if(stack->top > -1) {
    return &stack->actions[stack->top--];
  } 
  return NULL;
}

void undo_action(Editor *e, Action *act){
  Buffer *buff = e->buffs->data[e->current_buff];
  bool is_left = buff->cursor.index >= (act->index + act->old->len);
  bool is_normal = e->conf.is_vim_mode && e->mode == NORMAL;

  str_remove_chars(buff->s, act->index, act->new->len);
  add_str_to_str(buff->s, act->old, act->index);
  update_lines(e);

  if(is_left){
    move_cursor_left(
      e, buff->cursor.index - act->index - act->old->len + is_normal
    );
  } 
  else {
    move_cursor_right(
      e, act->index - buff->cursor.index + act->old->len - is_normal
   );
  } 
  update_lines(e);
}

void redo_action(Editor *e, Action *act){
  Buffer *buff = e->buffs->data[e->current_buff];
  str_remove_chars(buff->s, act->index, act->old->len);
  buff->cursor.index = act->index;
  add_str_to_str(buff->s, act->new, act->index);
  move_cursor_right(e, act->new->len - 1);
}

void undo(Editor *e){
  Buffer *buff = e->buffs->data[e->current_buff];
  Action *last_action = action_stack_pop(&buff->undo_stack);
  if(last_action == NULL) return;
  action_stack_push(&buff->redo_stack, *last_action);
  undo_action(e, last_action);
  update_scroll(e, false, false);
  buff->cursor.last_time_moved = GetTime();
}

void action_stack_flush(ActionStack *stack){
  stack->top = -1;
}

void free_action(Action *action){
  free_str(action->new);
  free_str(action->old);
  free(action);
  action = NULL;
}

void update_action(Action **act, size_t index, char c, bool is_new){
 if(*act == NULL){
   *act = init_action(index);
 }
 if(is_new) {
   add_char_to_str((*act)->new, c, (*act)->new->len);
 }else {
   (*act)->index = index;
   if((*act)->new->len) {
     str_remove_chars((*act)->new, (*act)->new->len - 1, 1);
   }else {
     add_char_to_str((*act)->old, c, (*act)->new->len);
   }
 }
}
