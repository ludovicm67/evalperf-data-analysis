#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list *new_list() {
  struct list *l = malloc(sizeof(struct list));
  l->size = LIST_INITIAL_SIZE;
  l->length = 0;
  l->l = malloc(LIST_INITIAL_SIZE * sizeof(void *));
  return l;
}

void free_list(struct list *l, void (*cb)(void *)) {
  unsigned int i;
  if (l) {
    if (cb) {
      for (i = 0; i < l->size; i++) {
        cb(l->l[i]);
      }
    }
    if (l->l)
      free(l->l);
    free(l);
  }
}

void *list_get(struct list *l, unsigned int index, void *(*cb)(void *),
               void *args) {
  unsigned int s;

  if (!l)
    return NULL;

  while (l->size <= index) {
    s = l->size;
    l->size *= 2;
    l->l = realloc(l->l, l->size * sizeof(void *));
    memset(&(l->l[s]), 0, s * sizeof(void *));
  }

  if (!l->l[index]) {
    l->l[index] = cb(args);
    l->length++;
  }

  return l->l[index];
}
