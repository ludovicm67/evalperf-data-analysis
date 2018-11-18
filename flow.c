#include "flow.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

struct flow *new_flow(int fid, double begin) {
  struct flow *f = malloc(sizeof(struct flow));
  f->fid = fid;
  f->begin = begin;
  f->end = begin;
  f->next = NULL;
  return f;
}

void free_flow(struct flow *f) {
  if (f == NULL)
    return;
  free_flow(f->next);
  free(f);
}

void add_flow(struct params *p, int fid, double ts) {
  struct flow *f;

  // if it is the first flow, initialize it
  if (p->flow_list == NULL) {
    p->flow_list = new_flow(fid, ts);
    return;
  }

  // loop through the list to find if it already exists or not
  for (f = p->flow_list; f->next != NULL; f = f->next) {
    if (f->fid == fid) {
      f->end = ts;
      return;
    }
  }

  // if it's a new flow
  f = new_flow(fid, ts);
  f->next = p->flow_list;
  p->flow_list = f;
}

int get_nb_flows(struct flow *f) {
  int res;
  if (f == NULL)
    return 0;
  res = 0;
  while (f->next) {
    res++;
    f = f->next;
  }
  return res;
}
