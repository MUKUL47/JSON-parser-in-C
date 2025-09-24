#include "parser.h"
#include "glib.h"
#include "lexer.h"
#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

bool is_digit(int t) { return t >= DIGIT_0 && t <= DIGIT_9; }

bool is_that_token(int token, Parser *p) {
  return token == get_token_at_cast(p->lexer, p->counter);
}

ValueResultType *parse_non_string(Parser *p) {
  NonStringGuard non_string_guard;
  ValueResultType *resultType = NULL;
  resultType = malloc(sizeof(ValueResultType));
  resultType->result = NULL;
  resultType->type = NUMBER;
  int counter = 0;
  bool finished = false;
  resultType->result =
      malloc(sizeof(char) * STRING_CAP); // TODO : update with realloc
  assert(resultType->result != NULL);
  bool is_started = true;
  resultType->type = 0;
  while (1) {

    const char *current_token_data = get_token_at(p->lexer, p->counter);
    if (current_token_data == NULL) {
      assert(false && "Incorrect number termination");
    }
    int current_token = get_token_at_cast(p->lexer, p->counter);
    const bool is_closing = current_token == CURLY_CLOSE ||
                            current_token == SQUARE_CLOSE ||
                            current_token == COMMA;
    const bool is_token_digit = is_digit(current_token);

    switch (current_token) {

    case CURLY_CLOSE:
    case SQUARE_CLOSE:
    case COMMA: {
      if (!is_started) {
        finished = true;
        break;
      }
    }

    case DIGIT_0: {
      if (is_started) {
        int next = *get_token_at(p->lexer, p->counter + 1);
        if (next != CURLY_CLOSE || next != SQUARE_CLOSE || next != COMMA) {
          assert(false && "Number cannot start with 0");
        }
        resultType->result[counter++] = current_token;
        goto UPDATE;
      }
    }
    case DIGIT_1:
    case DIGIT_2:
    case DIGIT_3:
    case DIGIT_4:
    case DIGIT_5:
    case DIGIT_6:
    case DIGIT_7:
    case DIGIT_8:
    case DIGIT_9: {
      resultType->result[counter++] = (char)current_token;
      goto UPDATE;
    }

    case LOWER_E:
    case UPPER_E: {
      if (!is_started && !non_string_guard.is_exponent &&
          is_digit(*get_token_at(p->lexer, p->counter - 1))) {
        resultType->result[counter++] = current_token;
        non_string_guard.is_exponent = true;
        int next_t = get_token_at_cast(p->lexer, p->counter + 1);
        int next_t1 = get_token_at_cast(p->lexer, p->counter + 2);
        const bool is_first_digit = is_digit(next_t);
        const bool is_first_digit1 = is_digit(next_t1);
        if (is_first_digit) {
          // 2e2
          resultType->result[counter++] = next_t;
          p->counter++;
          goto UPDATE;
        } else if (!is_first_digit && (next_t == PLUS || next_t == MINUS) &&
                   is_first_digit1) {
          // 2e+2
          // 2e-2
          resultType->result[counter++] = next_t;
          resultType->result[counter++] = next_t1;
          p->counter += 2;
          goto UPDATE;
        } else {
          assert(false &&
                 "Invalid exponent closing token found, must be 2e+2 2e-2 2e2");
        }
      }
    }

    case MINUS: {
      if (!non_string_guard.is_negative && is_started) {
        // started with minus
        resultType->result[counter++] = current_token;
        non_string_guard.is_negative = true;
        goto UPDATE;
      }
    }

    case LOWER_F:
    case LOWER_N:
    case LOWER_T: {
      p->counter++;
      resultType->result[counter++] = current_token;
      if (current_token == LOWER_F) {
        resultType->result[counter++] = LOWER_A;
        validate_token(p, LOWER_A);
        resultType->result[counter++] = LOWER_L;
        validate_token(p, LOWER_L);
        resultType->result[counter++] = LOWER_S;
        validate_token(p, LOWER_S);
        resultType->result[counter++] = LOWER_E;
        validate_token(p, LOWER_E);
        resultType->type = BOOL;
        finished = true;
        resultType->type = BOOL;
        break;
      } else if (current_token == LOWER_T) {
        resultType->result[counter++] = LOWER_R;
        validate_token(p, LOWER_R);
        resultType->result[counter++] = LOWER_U;
        validate_token(p, LOWER_U);
        resultType->result[counter++] = LOWER_E;
        validate_token(p, LOWER_E);
        resultType->type = BOOL;
        finished = true;
        resultType->type = BOOL;
        break;
      } else {
        resultType->result[counter++] = LOWER_U;
        validate_token(p, LOWER_U);
        resultType->result[counter++] = LOWER_L;
        validate_token(p, LOWER_L);
        resultType->result[counter++] = LOWER_L;
        validate_token(p, LOWER_L);
        resultType->type = NUL;
        finished = true;
        resultType->type = NUL;
        break;
      }
    }

    case DOT: {
      if (!is_started &&
          is_digit(get_token_at_cast(p->lexer, p->counter - 1)) &&
          !non_string_guard.is_float) {
        non_string_guard.is_float = true;
        resultType->result[counter++] = current_token;
        resultType->type = FLOAT;
        goto UPDATE;
      }
    }

    default:
      assert(false && "Failed to parse number, null, false or true");
    }

    if (finished) {
      break;
    }
  UPDATE:
    p->counter++;
    is_started = false;
  }
  if (resultType->type == 0) {
    resultType->type = NUMBER;
  }
  resultType->result[counter] = '\0';
  return resultType;
}

char *parse_string(Parser *p) {
  char *result = NULL;
  result = malloc(sizeof(char) * STRING_CAP); // TODO: update with realloc
  assert(result != NULL);
  int counter = 0;
  while (1) {
    const char *current_token_data = get_token_at(p->lexer, p->counter);
    if (current_token_data == NULL) {
      assert(false && "Incorrect string termination");
    }
    const int current_token = (int)*current_token_data;
    char c = (char)current_token;
    if (current_token == SLASH_BACK) { // check escapse chars
      char is_escape = *get_token_at(p->lexer, p->counter++);
      const int is_escape_char = (int)is_escape;
      result[counter++] = (char)is_escape_char;
      if (is_escape_char == DOUBLE_QUOTE || is_escape_char == SLASH_BACK ||
          is_escape_char == SLASH_FORWARD || is_escape_char == LOWER_B ||
          is_escape_char == LOWER_F || is_escape_char == LOWER_N ||
          is_escape_char == LOWER_R || is_escape_char == LOWER_T) {
        p->counter++;
      } else if (is_escape_char == LOWER_U) { // check HEX
        char unicode_1 = *get_token_at(p->lexer, p->counter);
        char unicode_2 = *get_token_at(p->lexer, p->counter + 1);
        char unicode_3 = *get_token_at(p->lexer, p->counter + 2);
        char unicode_4 = *get_token_at(p->lexer, p->counter + 3);
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
        result[counter++] = unicode_1;
        result[counter++] = unicode_2;
        result[counter++] = unicode_3;
        result[counter++] = unicode_4;
        p->counter += 3;
      } else {
        assert(false && "Expected escape characters or HEX unicode");
      }
    } else {
      if (current_token == DOUBLE_QUOTE)
        break;
      else {
        p->counter++;
        result[counter++] = (char)current_token;
      }
    }
  }
  result[counter++] = '\0';
  return result;
}

ValueResultType *parse_object_array_value(Parser *p) {
  if (is_that_token(DOUBLE_QUOTE, p)) {
    validate_token(p, DOUBLE_QUOTE);
    ValueResultType *valueResultType = NULL;
    valueResultType = malloc(sizeof(ValueResultType));
    assert(valueResultType != NULL);
    valueResultType->result = parse_string(p);
    valueResultType->type = STRING;
    validate_token(p, DOUBLE_QUOTE);
    return valueResultType;
  } else {
    return parse_non_string(p);
  }
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
    break;
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

void validate_token(Parser *p, u_short target) {
  char *c = get_token_at(p->lexer, p->counter);
  if (target != (u_short)*c) {
    printf("Expected %c but found %c at %d", (char)target, *c, p->counter);
    exit(3);
  }
  p->counter++;
}
void deserialize_entry(Parser *p) {
  u_short current_token = get_token_at_cast(p->lexer, 0);
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
        ValueResultType *valueResult = parse_object_array_value(p);
        json->type = valueResult->type;
        append_json_raw_values(&json, valueResult->result);
        deserialize_array_glist = g_list_append(deserialize_array_glist, json);
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
          parse_string(p);
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
        ValueResultType *valueResult = parse_object_array_value(p);
        json->type = valueResult->type;
        append_json_raw_values(&json, valueResult->result);
        printf("%s\n", valueResult->result);
        g_hash_table_insert(deserialize_object_ghashtable, object_key, json);
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