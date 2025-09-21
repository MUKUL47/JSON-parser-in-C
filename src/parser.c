#include "parser.h"
#include "glib.h"
#include "lexer.h"
#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

bool is_that_token(int token, Parser *p) {
  return token == (u_short)*get_token_at(p->lexer, p->counter);
}

JSON *alloc_json() {
  JSON *json = NULL;
  json = malloc(sizeof(JSON));
  assert(json != NULL);
  return json;
}

void append_json_raw_values(JSON **json, char *c) {
  assert(json != NULL && *json != NULL && c != NULL);
  switch ((*json)->type) {
  case STRING: {
    (*json)->value = c;
    break;
  }
  case NUMBER: {
    int *n = NULL;
    n = malloc(sizeof(int));
    *n = atoi(c);
    (*json)->value = n;
    break;
  }
  case FLOAT: {
    float *n = NULL;
    n = malloc(sizeof(float));
    *n = atof(c);
    (*json)->value = n;
    break;
  }
  case NUL: {
    (*json)->value = NULL;
  }
  case BOOL: {
    bool *b = NULL;
    b = malloc(sizeof(bool));
    *b = strcmp(c, "false") == 0 ? false : true;
    (*json)->value = b;
    break;
  }
  default:
    break;
  }
}

JSONType identify_json_value_type(char *c) {
  if (!strcmp(c, "false") || !strcmp(c, "true")) {
    return BOOL;
  }
  if (!strcmp(c, "null")) {
    return NUL;
  }

  int current = 0;
  int counter = 0;
  u_int8_t has_astrisk_once = 0;
  while ((current = (int)c[counter++]) != (char)'\0') {
    if (current == DOT) {
      has_astrisk_once++;
      continue;
    }
    if (current < DIGIT_0 || current > DIGIT_9 && current != DOT) {
      return STRING;
    }
  }
  if (has_astrisk_once == 1) {
    return FLOAT;
  } else if (has_astrisk_once > 1) {
    return STRING;
  }
  return NUMBER;
}

char *get_obj_array_string(Parser *p, int until_token[2]) {
  int current_array_char = -1;
  char *c = NULL;
  c = malloc(sizeof(char) * STRING_CAP);
  assert(c != NULL);
  int str_counter = 0;
  for (;;) {
    assert(str_counter < STRING_CAP - 1 && "Failed to allocate string");
    current_array_char = *get_token_at(p->lexer, p->counter);
    c[str_counter++] = (char)current_array_char;
    if (current_array_char != until_token[0] &&
        current_array_char != until_token[1]) {
      p->counter++;
    } else {
      break;
    }
  }
  c[str_counter - 1] = '\0';
  return c;
}

void validate_token(Parser *p, u_short target) {
  char *c = get_token_at(p->lexer, p->counter);
  if (target != (u_short)*c) {
    printf("Expected %c but found %c at %d", (char)target, *c, p->counter);
    exit(3);
  }
  p->counter++;
}
void deserialize_entry(Parser *p) {
  u_short current_token = (u_short)*get_token_at(p->lexer, 0);
  if (current_token == SQUARE_OPEN) {
    p->json->type = ARRAY;
    deserialize_array(p, &p->json->value);
    return;
  } else if (current_token == CURLY_OPEN) {
    p->json->type = OBJECT;
    deserialize_object(p, &p->json->value);
    return;
  }
  printf("Expected { or [ but found %c", (char)current_token);
  exit(3);
}

char *deserialize_json_values_inquote(Parser *p, int delimiter) {
  bool is_double_quote = is_that_token(DOUBLE_QUOTE, p);
  if (is_double_quote) {
    validate_token(p, DOUBLE_QUOTE);
  }
  char *c = get_obj_array_string(
      p, is_double_quote
             // if double quote its a string find until another double quote
             ? (int[2]){DOUBLE_QUOTE, DOUBLE_QUOTE}
             // else null bool number find until next or last array item
             : (int[2]){COMMA, delimiter});

  return c;
}

void deserialize_array(Parser *p, void **lastNodeValue) {
  validate_token(p, SQUARE_OPEN);
  GList *deserialize_array_glist = NULL;
  deserialize_array_glist = g_list_alloc();
  assert(deserialize_array_glist != NULL);
  if (!is_that_token(SQUARE_CLOSE, p)) {
    for (;;) { // iterating array
      // for array object recursively call deserialize
      JSON *json = alloc_json();
      json->value = NULL;
      if (is_that_token(SQUARE_OPEN, p)) {
        json->type = ARRAY;
        deserialize_array(p, &json->value);
        deserialize_array_glist = g_list_append(deserialize_array_glist, json);
      } else if (is_that_token(CURLY_OPEN, p)) {
        json->type = OBJECT;
        deserialize_object(p, &json->value);
        deserialize_array_glist = g_list_append(deserialize_array_glist, json);
      } else {
        const bool is_double_quote = is_that_token(DOUBLE_QUOTE, p);
        char *c = deserialize_json_values_inquote(p, SQUARE_CLOSE);
        json->type = identify_json_value_type(c);
        append_json_raw_values(&json, c);
        deserialize_array_glist = g_list_append(deserialize_array_glist, json);
        if (is_double_quote) {
          validate_token(p, DOUBLE_QUOTE);
        }
      }
      if (!is_that_token(COMMA, p)) {
        break;
      }
      validate_token(p, COMMA);
    }
  }
  *lastNodeValue =
      deserialize_array_glist; // assign to last recusive active node
  validate_token(p, SQUARE_CLOSE);
}

void deserialize_object(Parser *p, void **lastNodeValue) {
  validate_token(p, CURLY_OPEN); // {
  GHashTable *deserialize_object_ghashtable = NULL;
  deserialize_object_ghashtable = g_hash_table_new(g_str_hash, g_str_equal);
  if (!is_that_token(CURLY_CLOSE, p)) {
    for (;;) {                         // iterating array
      validate_token(p, DOUBLE_QUOTE); // "
      char *object_key =               // STRING
          get_obj_array_string(p, (int[2]){DOUBLE_QUOTE, DOUBLE_QUOTE});
      validate_token(p, DOUBLE_QUOTE); //"
      validate_token(p, COLON);        //:
      JSON *json = alloc_json();
      if (is_that_token(SQUARE_OPEN, p)) {
        json->type = ARRAY;
        deserialize_array(p, &json->value);
        g_hash_table_insert(deserialize_object_ghashtable, object_key, json);
      } else if (is_that_token(CURLY_OPEN, p)) {
        json->type = OBJECT;
        deserialize_object(p, &json->value);
        g_hash_table_insert(deserialize_object_ghashtable, object_key, json);
      } else {
        const bool is_double_quote = is_that_token(DOUBLE_QUOTE, p);
        char *c = deserialize_json_values_inquote(p, CURLY_CLOSE);
        json->type = identify_json_value_type(c);
        append_json_raw_values(&json, c);
        g_hash_table_insert(deserialize_object_ghashtable, object_key, json);
        if (is_double_quote) {
          validate_token(p, DOUBLE_QUOTE);
        }
      }
      if (!is_that_token(COMMA, p)) {
        break;
      }
      validate_token(p, COMMA);
    }
  }
  *lastNodeValue = deserialize_object_ghashtable;
  validate_token(p, CURLY_CLOSE); //}
}

JSON *json_get_value(Parser *p, char **s, int depth) {
  assert(p->json != NULL && p->json->value != NULL && s != NULL);
  JSON *currentValue = (JSON *)p->json;
  int currentKeyIndex = 0;
  char *currentKey = s[currentKeyIndex];
  while (1) {
    switch (currentValue->type) {
    case OBJECT: {
      GHashTable *hash_table = (GHashTable *)currentValue->value;
      void *v = g_hash_table_lookup(hash_table, currentKey);
      if (v == NULL) {
        return NULL;
      }
      currentValue = (JSON *)v;
      break;
    }
    case ARRAY: {
      GList *list = (GList *)currentValue->value;
      void *v = g_list_nth_data(list, atoi(currentKey));
      if (v == NULL) {
        return NULL;
      }
      currentValue = (JSON *)v;
      break;
    }

    default: {
      return NULL;
    }
    }
    currentKey = s[++currentKeyIndex];
    if (currentKeyIndex == depth) {
      return currentValue;
    }
  }
  return NULL;
}

Parser *deserialize(FILE *f) {
  Parser *p = NULL;
  p = malloc(sizeof(Parser));
  p->json = NULL;
  p->json = alloc_json();
  p->lexer = new_lexer();
  tokenizer(f, p->lexer);
  deserialize_entry(p);
  return p;
}

void free_parser_json(JSON **json) {
  JSON *j = *json;
  assert(j != NULL);
  switch (j->type) {
  case OBJECT: {
    assert(j->value != NULL);
    GHashTable *hash_table = (GHashTable *)j->value;
    GList *keys = g_hash_table_get_keys(hash_table);
    for (GList *iter = keys; iter != NULL; iter = iter->next) {
      JSON *s = (JSON *)g_hash_table_lookup(hash_table, (char *)iter->data);
      free_parser_json(&s);
    }
    g_list_free(keys);
    g_hash_table_destroy(hash_table);
    break;
  }
  case ARRAY: {
    assert(j->value != NULL);
    for (int i = 0; i < g_list_length(j->value); i++) {
      JSON *j = (JSON *)g_list_nth_data(j->value, i);
      free_parser_json(&j);
    }
    g_list_free(j->value);
    break;
  }
  case STRING:
  case NUMBER:
  case BOOL:
  case FLOAT: {
    if (j->value != NULL) {
      free(j->value);
    }
    break;
  }
  default:
    break;
  }
}

void free_parser(Parser *p) {
  assert(p != NULL && p->lexer != NULL);
  free_lexer(p->lexer);
  assert(p->json != NULL);
  free_parser_json(&p->json);
  free(p);
}