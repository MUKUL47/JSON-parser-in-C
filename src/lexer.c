#include "lexer.h"
#include "glib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

Lexer *new_lexer() {
  Lexer *l = NULL;
  l = malloc(sizeof(Lexer));
  l->tokens = NULL;
  return l;
}

Token *new_token(char *c) {
  Token *t = NULL;
  t = malloc(sizeof(Token));
  t->c = strdup(c);
  return t;
}

void tokenizer(FILE *f, Lexer *lexer) {
  assert(f != NULL && lexer != NULL);
  int current_char = fgetc(f);
  int last_char = -1;
  bool is_under_quote = false;
  while (current_char != EOF) {
    char c = ((char)current_char);
    bool is_quote = c == DOUBLE_QUOTE;
    if (!is_under_quote && is_quote && last_char != DOUBLE_QUOTE) {
      is_under_quote = true;
    } else if (is_under_quote && is_quote) {
      is_under_quote = false;
    }
    if (!is_under_quote &&
            (current_char != CARRIAGE && current_char != NEWLINE &&
             current_char != TAB && current_char != SINGLE_QUOTE &&
             current_char != SPACE) ||
        is_under_quote) {
      lexer->tokens = g_list_append(lexer->tokens, new_token(&c));
    }
    last_char = c;
    current_char = fgetc(f);
  }
}

char *get_token_at(Lexer *l, int index) {
  assert(l != NULL && l->tokens != NULL);
  Token *t = (Token *)g_list_nth_data(l->tokens, index);
  assert(t != NULL && t->c != NULL);
  return t->c;
}

void free_lexer(Lexer *lexer) {
  assert(lexer != NULL && lexer->tokens != NULL);
  for (int i = 0; i < g_list_length(lexer->tokens); i++) {
    Token *t = (Token *)g_list_nth_data(lexer->tokens, i);
    if (t != NULL) {
      free(t->c);
      free(t);
    }
  }
  g_list_free(lexer->tokens);
}