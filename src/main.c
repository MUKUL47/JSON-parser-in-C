#include "parser.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

int main() {
  FILE *f = fopen("../test.json", "r");
  Parser *p = deserialize(f);
  char *c1[] = {"top_level_values", "number"};
  JSON *j1 = (JSON *)json_get_value(p, c1, 2);
  printf("%d\n", j1->type);
  // if (j1 != NULL && j1->type == STRING) {
  //   printf("%s\n", (char *)j1->value);
  // }

  free_parser(p);
  fclose(f);
  return 0;
}
