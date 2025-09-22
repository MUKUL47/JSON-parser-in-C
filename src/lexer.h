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
#define SLASH_BACK         92  // /
#define SLASH_FORWARD         47  // /

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

#define UPPER_A        65
#define UPPER_B        66
#define UPPER_C        67
#define UPPER_D        68
#define UPPER_E        69
#define UPPER_F        70
#define UPPER_G        71
#define UPPER_H        72
#define UPPER_I        73
#define UPPER_J        74
#define UPPER_K        75
#define UPPER_L        76
#define UPPER_M        77
#define UPPER_N        78
#define UPPER_O        79
#define UPPER_P        80
#define UPPER_Q        81
#define UPPER_R        82
#define UPPER_S        83
#define UPPER_T        84
#define UPPER_U        85
#define UPPER_V        86
#define UPPER_W        87
#define UPPER_X        88
#define UPPER_Y        89
#define UPPER_Z        90

// Lowercase letters
#define LOWER_A        97
#define LOWER_B        98
#define LOWER_C        99
#define LOWER_D        100
#define LOWER_E        101
#define LOWER_F        102
#define LOWER_G        103
#define LOWER_H        104
#define LOWER_I        105
#define LOWER_J        106
#define LOWER_K        107
#define LOWER_L        108
#define LOWER_M        109
#define LOWER_N        110
#define LOWER_O        111
#define LOWER_P        112
#define LOWER_Q        113
#define LOWER_R        114
#define LOWER_S        115
#define LOWER_T        116
#define LOWER_U        117
#define LOWER_V        118
#define LOWER_W        119
#define LOWER_X        120
#define LOWER_Y        121
#define LOWER_Z        122
typedef struct{
    char *c;
}Token;

typedef struct{
    GList *tokens;  
    GList *raw_tokens; 
    int counter;
}Lexer;

Lexer *new_lexer();
void tokenizer(FILE *f, Lexer *lexer);
void free_lexer(Lexer *lexer);
char *get_token_at(Lexer *lexer, int index);
char *get_raw_token_at(Lexer *lexer, int index);
void free_lexer(Lexer *l);
void tokenizer_object(Lexer *l);
void tokenizer_array(Lexer *l);
void tokenizer_start(Lexer *l);
void tokenizer_string(Lexer *l);
void tokenizer_number(Lexer *l);
void tokenizer_string(Lexer *l);
void tokenizer_consume(Lexer *l, char token);