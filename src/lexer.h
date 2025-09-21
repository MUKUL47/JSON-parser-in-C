#pragma once
#include <glib.h>
#include <stdio.h>

#define NEWLINE        10  // \n
#define TAB            9   // \t
#define SPACE          32  // ' '
#define CARRIAGE       13  // \r

#define SINGLE_QUOTE   39  // '
#define DOUBLE_QUOTE   34  // "

#define CURLY_OPEN     123 // {
#define CURLY_CLOSE    125 // }
#define SQUARE_OPEN    91  // [
#define SQUARE_CLOSE   93  // ]
#define COMMA          44  // ,
#define COLON          58  // :

#define ASTERISK       42  // *
#define PLUS           43  // +
#define MINUS          45  // -
#define DOT            46  // .
#define SLASH          47  // /

#define DIGIT_0        48
#define DIGIT_1        49
#define DIGIT_2        50
#define DIGIT_3        51
#define DIGIT_4        52
#define DIGIT_5        53
#define DIGIT_6        54
#define DIGIT_7        55
#define DIGIT_8        56
#define DIGIT_9        57

typedef struct{
    char *c;
}Token;

typedef struct{
    GList *tokens;  
}Lexer;

Lexer *new_lexer();
void tokenizer(FILE *f, Lexer *lexer);
void free_lexer(Lexer *lexer);
char *get_token_at(Lexer *lexer, int index);
void free_lexer(Lexer *l);