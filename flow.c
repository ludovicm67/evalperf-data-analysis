#include "flow.h"
#include "list.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

struct flow_params {
  int fid;
  double ts;
};

struct flow *new_flow(int fid, double begin) {
  struct flow *f = malloc(sizeof(struct flow));
  f->fid = fid;
  f->begin = begin;
  f->end = begin;
  return f;
}

void free_flow(struct flow *f) {
  if (f == NULL)
    return;
  free(f);
}

void *list_cb(void *arg) {
  struct flow_params *p = (struct flow_params *)arg;
  return new_flow(p->fid, p->ts);
}

void add_flow(struct params *p, int fid, double ts) {
  struct flow_params fp;
  fp.fid = fid;
  fp.ts = ts;
  if (!p || !p->flow_list)
    return;
  list_get(p->flow_list, fid, list_cb, &fp);
}

int get_nb_flows(struct params *p) {
  if (!p || !p->flow_list)
    return 0;
  return p->flow_list->length;
}
