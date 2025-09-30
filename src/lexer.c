#include "lexer.h"
#include "glib.h"
#include "glibconfig.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

Lexer *new_lexer() {
  Lexer *l = NULL;
  l = malloc(sizeof(Lexer));
  l->counter = 0;
  l->raw_tokens = g_array_new(FALSE, FALSE, sizeof(Token *));
  assert(l->raw_tokens != NULL);
  l->tokens = g_array_new(FALSE, FALSE, sizeof(Token *));
  return l;
}

Token *new_token(char *c) {
  Token *t = NULL;
  t = malloc(sizeof(Token));
  t->c = strdup(c);
  return t;
}

void raw_tokenizer(FILE *f, Lexer *lexer) {
  assert(f != NULL && lexer != NULL);
  int current_char = -1;
  while ((current_char = fgetc(f)) != EOF) {
    char c = (char)current_char;
    Token *t = new_token(&c);
    g_array_append_val(lexer->raw_tokens, t);
  }
}

void tokenizer_string(Lexer *l) {
  char *current_token_at = get_raw_token_at(l, l->counter);
  if (current_token_at == NULL) {
    assert(false && "Failed to terminate the string");
  }
  const int current_token = (int)*current_token_at;
  char c = (char)current_token;
  if (current_token == SLASH_BACK) { // check escapse chars
    tokenizer_consume(l, &c);
    l->counter++;
    char is_escape = (char)*get_raw_token_at(l, l->counter);
    const int is_escape_char = (int)is_escape;
    if (is_escape_char == DOUBLE_QUOTE || is_escape_char == SLASH_BACK ||
        is_escape_char == SLASH_FORWARD || is_escape_char == LOWER_B ||
        is_escape_char == LOWER_F || is_escape_char == LOWER_N ||
        is_escape_char == LOWER_R || is_escape_char == LOWER_T) {
      l->counter++;
      tokenizer_consume(l, &is_escape);
      tokenizer_string(l);
      return;
    } else if (is_escape_char == LOWER_U) { // check HEX
      char *unicode_1 = get_raw_token_at(l, l->counter);
      char *unicode_2 = get_raw_token_at(l, l->counter + 1);
      char *unicode_3 = get_raw_token_at(l, l->counter + 2);
      char *unicode_4 = get_raw_token_at(l, l->counter + 3);
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

void consume_last_valid_token(Lexer *lexer, char **last_char,
                              int *last_char_counter) {
  if (*last_char != NULL) {
    (*last_char)[*last_char_counter] = '\0';
    *last_char_counter = 0;
    tokenizer_consume(lexer, *last_char); // consume all valid json tokens
    free(*last_char);
    *last_char = NULL;
  }
}

void tokenizer_start(Lexer *lexer) {
  int last_char_counter = 0;
  char *last_char = NULL;
  const u_short MOCK_SIZE = 3;
  while (true) {
    char *current_token_at = get_raw_token_at(lexer, lexer->counter);
    if (current_token_at == NULL) {
      return; // finished tokenizing
    }
    char c = *current_token_at;
    const int current_token = (int)c;
    if (current_token == COMMA || current_token == SEMI_COLON ||
        current_token == BRACKET_OPEN || current_token == BRACKET_CLOSE ||
        current_token == SINGLE_QUOTE || current_token == EQUAL_SIGN ||
        current_token == GREATOR_THAN_SIGN || current_token == LESS_THAN_SIGN ||
        current_token == ASTERISK || current_token == PLUS ||
        current_token == MINUS) {
      consume_last_valid_token(lexer, &last_char, &last_char_counter);
      tokenizer_consume(lexer, &c);
    } else if (!(current_token == SPACE || current_token == CARRIAGE ||
                 current_token == TAB || current_token == NEWLINE ||
                 current_token == DOUBLE_QUOTE)) {
      if (last_char == NULL) {
        last_char = malloc(sizeof(char) * MOCK_SIZE);
        assert(last_char != NULL);
      } else {
        const int current_size = last_char_counter + 1;
        if (last_char_counter > 0 && current_size % MOCK_SIZE == 0) {
          last_char =
              realloc(last_char, (current_size + MOCK_SIZE)); // 3+3 6+3 9+3..
          assert(last_char != NULL);
        }
      }
      last_char[last_char_counter++] = current_token;
    } else if (last_char != NULL) {
      consume_last_valid_token(lexer, &last_char, &last_char_counter);
    }
    lexer->counter++;
    // get the lastchar append EOL and consume the token
  }
  consume_last_valid_token(lexer, &last_char, &last_char_counter);
}

void tokenizer(FILE *f, Lexer *lexer) {
  assert(f != NULL && lexer != NULL);
  raw_tokenizer(f, lexer);
  tokenizer_start(lexer);
  for (int i = 0; i < lexer->tokens->len; i++) {
    Token *t = (Token *)g_array_index(lexer->tokens, Token *, i);
    printf("%s\n", t->c);
  }
  exit(0);
  printf("\n");
}

char *get_token_at(Lexer *l, int index) {
  assert(l != NULL && l->tokens != NULL);

  Token *t = (Token *)g_array_index(l->tokens, Token *, index);
  if (t == NULL) {
    return NULL;
  }
  return t->c;
}

char *get_raw_token_at(Lexer *l, int index) {
  assert(l != NULL && l->raw_tokens != NULL);

  Token *t = (Token *)g_array_index(l->raw_tokens, Token *, index);
  if (t == NULL) {
    return NULL;
  }
  return t->c;
}

int get_token_at_cast(Lexer *l, int index) {
  char c = get_token_at(l, index);
  if (!c) {
    return -1;
  }
  return c;
}

void tokenizer_consume(Lexer *lexer, char *token) {
  assert(lexer != NULL);
  Token *t = new_token(token);
  g_array_append_val(lexer->tokens, t);
}