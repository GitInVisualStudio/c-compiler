#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef LEXER_H
#define LEXER_H

#define INPUT_SIZE 1024
#define bool unsigned char
#define true 1
#define false 0

typedef enum TOKENS {
    O_BRACE=0,
    C_BRACE,
    O_PARENTHESIS,
    C_PARENTHESIS,
    SEMICOLON,
    INT_KEYWORD,
    RETURN_KEYWORD,
    INT_LITERAL,
    MINUS,
    BITWISE_COMPLEMENT,
    N_EQUAL,
    LOGICAL_NEGATION,
    ADDITION,
    MULTIPLICATION,
    DIVISION,
    AND,
    OR, 
    EQUAL,
    LESS_THAN_OR,
    LESS_THAN,
    GREATER_THAN_OR,
    GREATER_THAN,
    ASSIGNMENT,
    IF,
    ELSE,
    WHILE,
    FOR,
    IDENTIFIER,
    EOF_TOKEN,
    TOKEN_LAST=EOF_TOKEN
} TOKENS;

#define TOKENS_LENGTH TOKEN_LAST + 1

static const char* TOKEN_NAMES[] = {
    "O_BRACE",
    "C_BRACE",
    "O_PARENTHESIS",
    "C_PARENTHESIS",
    "SEMICOLON",
    "INT_KEYWORD",
    "RETURN_KEYWORD",
    "INT_LITERAL",
    "MINUS",
    "BITWISE_COMPLEMENT",
    "N_EQUAL",
    "LOGICAL_NEGATION",
    "ADDITION",
    "MULTIPLICATION",
    "DIVISION",
    "AND",
    "OR", 
    "EQUAL",
    "LESS_THAN_OR",
    "GREATER_THAN_OR",
    "GREATER_THAN",
    "LESS_THAN",
    "ASSIGNMENT",
    "IF",
    "ELSE",
    "WHILE",
    "FOR",
    "IDENTIFIER",
    "END OF FILE"
};

/**
 * @brief list of according token symbols to token types
 * if not a static symbol -> NULL and look for condition implementation -> conditions
 */
static const char* TOKEN_SYMBOLS[] = {
    "{",
    "}",
    "(",
    ")",
    ";",
    "int",
    "return",
    NULL,
    "-",
    "~",
    "!=",
    "!",
    "+",
    "*",
    "/",
    "&&",
    "||",
    "==",
    "<=",
    ">=",
    ">",
    "<",
    "=",
    "if",
    "else",
    "while",
    "for",
    NULL,
    NULL
};

typedef struct token {
    TOKENS token_type;
    char value[INPUT_SIZE];
}token;

typedef struct lexer {
    char* filename;
    FILE* fp;
}lexer;

typedef bool (*token_condition)(char input[INPUT_SIZE], int index);

lexer* lexer_open(char* filename);
bool lexer_next(lexer* lexer, token* token);
void lexer_free(lexer* lexer);
bool lexer_peak(lexer* lexer, token* token);

bool __identifiy_token(lexer* lexer, token* token, char input[INPUT_SIZE], token_condition condition);

bool __int_literal(char input[INPUT_SIZE], int index);
bool __identifier(char input[INPUT_SIZE], int index);

#endif