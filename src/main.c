#include "parser.h"
#include <SDL2/SDL.h>
#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

int main() {
  FILE *f = fopen("../test.sql", "r");
  assert(f != NULL);
  Parser *p = deserialize(f);
}
