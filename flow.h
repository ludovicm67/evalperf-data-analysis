#ifndef __FLOW_H
#define __FLOW_H

#include "main.h"

struct flow {
  int fid;
  double begin;
  double end;
  struct flow *next;
};

struct flow *new_flow(int fid, double begin);
void free_flow(struct flow *f);
void add_flow(struct params *p, int fid, double ts);
int get_nb_flows(struct flow *f);

#endif
