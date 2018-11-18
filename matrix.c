#include "main.h"
#include <stdio.h>
#include <stdlib.h>

// get number of nodes by reading the matrix file
int get_nb_nodes(FILE *file) {
  char *line = NULL;
  char *tmp;
  size_t len = 0;
  int nb_nodes = 1;
  if (getline(&line, &len, file) == -1) {
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
  rewind(file);
  return nb_nodes;
}
