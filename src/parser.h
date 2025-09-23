
#include "lexer.h"
#include <stdio.h>
#include <stdbool.h>
#define STRING_CAP 1000 //realloc later
typedef enum{
    OBJECT = 1,
    ARRAY,
    STRING,
    NUMBER,
    NUL,
    BOOL,
    FLOAT
}JSONType;

typedef  struct{
    JSONType type;
    void *value;
}JSON;

typedef struct{
    JSON *json;
    int counter;
    Lexer *lexer;
}Parser;

typedef struct{
    bool is_float;
    bool is_exponent;
    bool is_negative;
    int counter;
}NonStringGuard;

typedef struct{
    JSONType type;
    char *result;
}ValueResultType;

Parser*  deserialize(FILE *f);
void deserialize_entry(Parser *p);
void deserialize_array(Parser *p, void **lastNodeValue);
void deserialize_object(Parser *p, void **lastNodeValue);
bool is_that_token(int token, Parser *p);
char *get_obj_array_string(Parser *p, int until_tokens[2]);
JSONType identify_json_value_type(char *c);
JSON* alloc_json();
JSON* json_get_value(Parser *p, char** s, int depth);
void free_parser(Parser *p);
void append_json_raw_values(JSON **json, char *c);
void validate_token(Parser *p, u_short target);
char* parse_string(Parser *p);
ValueResultType* parse_non_string(Parser *p);