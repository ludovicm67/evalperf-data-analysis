#ifndef __FLOW_H
#define __FLOW_H

#include "main.h"

struct flow {
  int fid;
  double begin;
  double end;
  int nb_codes[5]; // indexes representing code from 1 to 4
};

struct flow *new_flow(int fid, double begin);
void free_flow(struct flow *f);
void add_flow(struct params *p, int fid, double ts);
int get_nb_flows(struct params *p);
void flow_print(struct params *p, int fid);

#endif
