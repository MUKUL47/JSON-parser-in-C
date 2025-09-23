#include "parser.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

int main() {
  FILE *f = fopen("../test.json", "r");
  Parser *p = deserialize(f);
  char *c1[] = {"v"};
  JSON *j1 = (JSON *)json_get_value(p, c1, 1);
  printf("%b\n", j1);
  if (j1 != NULL && j1->type == NUMBER) {
    printf("%f\n", *(float *)j1->value);
  }

  return 0;
}
