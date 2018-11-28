#include "main.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void open_matrix_file(struct params *p) {
  p->mat_stream = fopen(p->mat_file, "r");
  if (p->mat_stream == NULL) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }
}

void get_nb_nodes(struct params *p) {
  char *line = NULL;
  char *tmp;
  size_t len = 0;
  int nb_nodes = 1;
  if (getline(&line, &len, p->mat_stream) == -1) {
    fprintf(stderr, "matrix file is empty\n");
    exit(EXIT_FAILURE);
  }
  tmp = line;
  while (*(tmp++))
    if (*tmp == ' ' || *tmp == '\t')
      nb_nodes++;
  if (*tmp == ' ' || *tmp == '\t')
    nb_nodes--;
  free(line);
  rewind(p->mat_stream);
  p->nb_nodes = nb_nodes;
}

void read_matrix(struct params *p) {
  char *tmp;
  char *line = NULL;
  int i, j; // counters
  int n = 0;
  size_t len = 0;

  p->mat = malloc(p->nb_nodes * p->nb_nodes * sizeof(int));

  for (i = 0; i < p->nb_nodes; i++) {
    if (getline(&line, &len, p->mat_stream) == -1) {
      fprintf(stderr, "matrix file is empty\n");
      exit(EXIT_FAILURE);
    }
    tmp = line;
    for (j = 0; j < p->nb_nodes; j++) {
      p->mat[n++] = atoi(strtok(tmp, " \t"));
      tmp = NULL;
    }
  }

  free(line);
}

int matrix_get(struct params *p, int node1, int node2) {
  int r = 0;
  if (!p || !p->mat)
    return r;
  r = p->mat[(node1 - 1) + (node2 - 1) * p->nb_nodes];
  if (!r)
    r = p->mat[(node2 - 1) + (node1 - 1) * p->nb_nodes];
  return r;
}

void print_matrix(struct params *p) {
  int i;
  for (i = 0; i < p->nb_nodes * p->nb_nodes; i++) {
    if (i % p->nb_nodes == 0)
      printf("\n");
    printf(" %3d", p->mat[i]);
  }
  printf("\n");
}

void free_matrix(struct params *p) {
  free(p->mat);
  fclose(p->mat_stream);
}

void init_matrix(struct params *p) {
  open_matrix_file(p);
  get_nb_nodes(p);
  read_matrix(p);
}
