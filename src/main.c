#include "parser.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

int main() {
  FILE *f = fopen("../test.json", "r");
  Parser *p = deserialize(f);
  char *c1[] = {"name"};
  JSON *j1 = (JSON *)json_get_value(p, c1, 1);
  printf("%d\n", j1->type);
  if (j1 != NULL && j1->type == STRING) {
    printf("%s\n", (char *)j1->value);
  }
  char *c2[] = {"version"};
  JSON *j2 = (JSON *)json_get_value(p, c2, 1);
  printf("%d\n", j2->type);
  if (j2 != NULL && j2->type == NUMBER) {
    printf("%d\n", (*(int *)j2->value));
  }

  char *c3[] = {"pi"};
  JSON *j3 = (JSON *)json_get_value(p, c3, 1);
  printf("%d\n", j3->type);
  if (j3 != NULL && j3->type == FLOAT) {
    printf("%f\n", (*(float *)j3->value));
  }

  char *c4[] = {"enabled"};
  JSON *j4 = (JSON *)json_get_value(p, c4, 1);
  printf("%d\n", j4->type);
  if (j4 != NULL && j4->type == BOOL) {
    printf("%s\n", (*(bool *)j4->value) ? "true" : "false");
  }

  char *c5[] = {"owner"};
  JSON *j5 = (JSON *)json_get_value(p, c5, 1);
  printf("%d\n", j5->type);
  if (j5 != NULL && j5->type == NUL) {
    printf("null\n");
  }

  char *c6[] = {"tags"};
  JSON *j6 = (JSON *)json_get_value(p, c6, 1);
  printf("%d\n", j6->type);
  if (j6 != NULL && j6->type == ARRAY) {
  }

  char *c7[] = {"nested", "ratio"};
  JSON *j7 = (JSON *)json_get_value(p, c7, 2);
  printf("%d\n", j7->type);
  if (j7 != NULL && j7->type == FLOAT) {
    printf("%f\n", (*(float *)j7->value));
  }

  free_parser(p);
  fclose(f);
  return 0;
}
