#include <ijvm.h>
#include <frame.h>
#include <stdlib.h>

frame* new_frame(void)
{
  frame* frm = malloc(sizeof *frm);
  frm->top = -1;
  frm->frm_size = 0;
  frm->elements = malloc(sizeof(word_t));
  frm->locals = malloc(sizeof(word_t) * 100);
  frm->local_size = 100;
  for (int i = 0; i < frm->local_size; i++)
  {
    frm->locals[i] = NULL;
  }
  return frm;
}

void destroy_frame(frame *frm)
{
  free(frm->elements);
  free(frm->locals);
  free(frm);
}

word_t* get_elements(frame *frm)
{
  return frm->elements;
}

int get_frm_size(frame *frm)
{
  return frm->frm_size;
}

void push(frame *frm, word_t data)
{
  printf("%d\n", frm->frm_size);
  realloc(frm->elements, frm->frm_size + 1);
  frm->elements[frm->top + 1] = data;
  frm->top++;
  frm->frm_size++;
}

word_t pop(frame *frm)
{
  word_t temp = frm->elements[frm->top];
  if (top == -1)
  {
    printf("%s\n", "Warning: popping empty frame");
  }
  else
  {
    frm->elements[frm->top] = NULL;
    frm->top--;
    realloc(frm->elements, frm->frm_size);
    frm->frm_size--;
  }
  return temp;
}

word_t top(frame *frm)
{
  return frm->elements[frm->top];
}

word_t* get_locals(frame *frm)
{
  return frm->locals;
}

void set_local(frame *frm, int index, word_t data)
{

  while (frm->local_size < index)
  {
    realloc(frm->locals, frm->local_size + 1);
    frm->local_size += 1;
  }
  frm->locals[index] = data;
}
