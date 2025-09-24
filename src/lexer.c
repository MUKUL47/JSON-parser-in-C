#include "lexer.h"
#include "glib.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

Lexer *new_lexer() {
  Lexer *l = NULL;
  l = malloc(sizeof(Lexer));
  l->counter = 0;
  l->tokens = NULL;
  l->raw_tokens = NULL;
  return l;
}

Token *new_token(char c) {
  Token *t = NULL;
  t = malloc(sizeof(Token));
  t->c = strdup(&c);
  return t;
}

void raw_tokenizer(FILE *f, Lexer *lexer) {
  assert(f != NULL && lexer != NULL);
  lexer->raw_tokens = NULL;
  int current_char = -1;
  while ((current_char = fgetc(f)) != EOF) {
    char c = (char)current_char;

    lexer->raw_tokens = g_list_append(lexer->raw_tokens, new_token(c));
  }
}

void tokenizer_string(Lexer *l) {
  char *cc = get_raw_token_at(l, l->counter);
  if (cc == NULL) {
    assert(false && "Failed to terminate the string");
  }
  const int current_token = (int)*cc;
  char c = (char)current_token;
  if (current_token == SLASH_BACK) { // check escapse chars
    tokenizer_consume(l, c);
    l->counter++;
    char is_escape = (char)*get_raw_token_at(l, l->counter);
    const int is_escape_char = (int)is_escape;
    if (is_escape_char == DOUBLE_QUOTE || is_escape_char == SLASH_BACK ||
        is_escape_char == SLASH_FORWARD || is_escape_char == LOWER_B ||
        is_escape_char == LOWER_F || is_escape_char == LOWER_N ||
        is_escape_char == LOWER_R || is_escape_char == LOWER_T) {
      l->counter++;
      tokenizer_consume(l, is_escape);
      tokenizer_string(l);
      return;
    } else if (is_escape_char == LOWER_U) { // check HEX
      char unicode_1 = *get_raw_token_at(l, l->counter);
      char unicode_2 = *get_raw_token_at(l, l->counter + 1);
      char unicode_3 = *get_raw_token_at(l, l->counter + 2);
      char unicode_4 = *get_raw_token_at(l, l->counter + 3);
      if (!((

                unicode_1 >= DIGIT_0 && unicode_1 <= DIGIT_9 ||
                unicode_1 >= UPPER_A && unicode_1 <= UPPER_Z ||
                unicode_1 >= LOWER_A && unicode_1 <= LOWER_Z) &&
            (

                unicode_2 >= DIGIT_0 && unicode_2 <= DIGIT_9 ||
                unicode_2 >= UPPER_A && unicode_2 <= UPPER_Z ||
                unicode_2 >= LOWER_A && unicode_2 <= LOWER_Z) &&
            (

                unicode_3 >= DIGIT_0 && unicode_3 <= DIGIT_9 ||
                unicode_3 >= UPPER_A && unicode_3 <= UPPER_Z ||
                unicode_3 >= LOWER_A && unicode_3 <= LOWER_Z) &&
            (

                unicode_4 >= DIGIT_0 && unicode_4 <= DIGIT_9 ||
                unicode_4 >= UPPER_A && unicode_4 <= UPPER_Z ||
                unicode_4 >= LOWER_A && unicode_4 <= LOWER_Z))) {
        assert(false && "incorrect HEX code found");
      }

      tokenizer_consume(l, unicode_1);
      tokenizer_consume(l, unicode_2);
      tokenizer_consume(l, unicode_3);
      tokenizer_consume(l, unicode_4);
      l->counter += 3;
      tokenizer_string(l);
      return;
    }
    assert(false && "Expected escape characters or HEX unicode");
  } else if (current_token == DOUBLE_QUOTE) {
    tokenizer_consume(l, c);
    l->counter++; // string finished
  } else {        // contiue for
    tokenizer_consume(l, c);
    l->counter++;
    tokenizer_string(l);
  }
}

void tokenizer_start(Lexer *lexer) {
  char *cc = get_raw_token_at(lexer, lexer->counter);
  if (cc == NULL) {
    return; // finished tokenizing
  }
  char c = *cc;
  const int current_token = (int)c;
  if (current_token == DOUBLE_QUOTE) {
    tokenizer_consume(lexer, c);
    lexer->counter++;
    tokenizer_string(lexer);
    tokenizer_start(lexer);
  } else if (current_token == CURLY_OPEN || current_token == CURLY_CLOSE ||
             current_token == SQUARE_OPEN || current_token == SQUARE_CLOSE ||
             current_token == COMMA || current_token == SQUARE_OPEN ||
             current_token == COLON ||
             (current_token >= UPPER_A && current_token <= UPPER_Z) ||
             (current_token >= LOWER_A && current_token <= LOWER_Z) ||
             (current_token >= DIGIT_0 && current_token <= DIGIT_9) ||
             current_token == DOT || current_token == MINUS ||
             current_token == PLUS) {
    tokenizer_consume(lexer, c); // consume all valid json tokens
    lexer->counter++;
    tokenizer_start(lexer);
  } else {
    lexer->counter++;
    tokenizer_start(lexer);
  }
}

void tokenizer(FILE *f, Lexer *lexer) {
  assert(f != NULL && lexer != NULL);
  raw_tokenizer(f, lexer);
  tokenizer_start(lexer);
}

char *get_raw_token_at(Lexer *l, int index) {
  assert(l != NULL && l->raw_tokens != NULL);
  Token *t = (Token *)g_list_nth_data(l->raw_tokens, index);
  if (t == NULL) {
    return NULL;
  }
  return t->c;
}

int get_token_at_cast(Lexer *l, int index) {
  char *c = get_token_at(l, index);
  if (c == NULL) {
    return -1;
  }
  return (int)*c;
}

char *get_token_at(Lexer *l, int index) {
  assert(l != NULL && l->tokens != NULL);
  Token *t = (Token *)g_list_nth_data(l->tokens, index);
  if (t == NULL || t->c == NULL) {
    return NULL;
  }
  return strndup(t->c, 1);
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

void tokenizer_consume(Lexer *lexer, char token) {
  assert(lexer != NULL);
  lexer->tokens = g_list_append(lexer->tokens, new_token(token));
}