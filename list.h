#ifndef __LIST_H
#define __LIST_H

#define LIST_INITIAL_SIZE 128

struct list {
  unsigned int size;   // allocated size for the list
  unsigned int length; // number of items in l
  void **l;
};

struct list *new_list();
void free_list(struct list *l, void (*cb)(void *));
void *list_get(struct list *l, unsigned int index, void *(*cb)(void *),
               void *args);

#endif
