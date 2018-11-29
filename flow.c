#include "flow.h"
#include "list.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

struct flow_params {
  int fid;
  double ts;
};

struct flow *flow_new(int fid, double begin) {
  struct flow *f = malloc(sizeof(struct flow));
  f->fid = fid;
  f->begin = begin;
  f->end = begin;
  f->nb_codes[0] = 0;
  f->nb_codes[1] = 0;
  f->nb_codes[2] = 0;
  f->nb_codes[3] = 0;
  f->nb_codes[4] = 0;
  return f;
}

void *list_cb(void *arg) {
  struct flow_params *p = (struct flow_params *)arg;
  return flow_new(p->fid, p->ts);
}

void flow_add(struct params *p, int fid, double ts) {
  struct flow_params fp;
  struct flow *f;
  fp.fid = fid;
  fp.ts = ts;
  if (!p || !p->flow_list)
    return;
  list_get(p->flow_list, fid, list_cb, &fp);
  f = p->flow_list->l[fid];
  f->end = ts;
}

int flow_nb(struct params *p) {
  if (!p || !p->flow_list)
    return 0;
  return p->flow_list->length;
}

void flow_print(struct params *p, int fid) {
  struct flow *f;

  if (!p || !p->flow_list || fid < 0) {
    printf("\n");
    return;
  }

  if ((unsigned int)fid >= p->flow_list->size || !p->flow_list->l[fid]) {
    printf(" /!\\ flow not found.\n");
    return;
  }

  f = p->flow_list->l[fid];
  printf("\n");

  printf(" - fid: %d\n", f->fid);
  printf(" - life start: t=%f\n", f->begin);
  printf(" - end of life: t=%f\n", f->end);
  printf(" - lifetime: %f sec\n", f->end - f->begin);
  printf(" - emited packets (code 0): %d\n", f->nb_codes[0]);
  printf(" - received packets (code 3): %d\n", f->nb_codes[3]);
  printf(" - destroyed packets (code 4): %d\n", f->nb_codes[4]);
  printf(" - loss rate: %f%%\n",
         (float)(100 * f->nb_codes[4]) /
             (f->nb_codes[0] + f->nb_codes[1] + f->nb_codes[2] +
              f->nb_codes[3] + f->nb_codes[4]));
}
