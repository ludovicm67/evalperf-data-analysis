#ifndef __FLOW_H
#define __FLOW_H

#include "main.h"

struct flow {
  int fid;
  double begin;
  double end;
  int nb_codes[5]; // indexes representing code from 1 to 4
};

struct flow *flow_new(int fid, double begin);
void flow_add(struct params *p, int fid, double ts);
int flow_nb(struct params *p);
void flow_print(struct params *p, int fid);

#endif
