#ifndef __MATRIX_H
#define __MATRIX_H

#include <stdio.h>

#include "main.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void open_matrix_file(struct params *p);
void get_nb_nodes(struct params *p);
void read_matrix(struct params *p);
int matrix_get(struct params *p, int node1, int node2);
void print_matrix(struct params *p);
void free_matrix(struct params *p);
void init_matrix(struct params *p);

#endif
