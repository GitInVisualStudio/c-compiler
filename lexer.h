#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef LEXER_H
#define LEXER_H

#define INPUT_SIZE 1024

#define bool unsigned char
#define true 1
#define false 0

/**
 * TOKEN enum
 * First token starts with value 0
 * Last token is TOKEN_LAST
 */
typedef enum TOKENS {
    O_BRACE=0,
    C_BRACE,
    O_PARENTHESIS,
    C_PARENTHESIS,
    SEMICOLON,
    INT_KEYWORD,
    RETURN_KEYWORD,
    INT_LITERAL,
    ADD_ASSIGN,
    SUB_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
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
    GREATER_THAN_OR,
    GREATER_THAN,
    LESS_THAN,
    ASSIGN_KEYWORD,
    IF_KEYWORD,
    ELSE_KEYWORD,
    WHILE_KEYWORD,
    FOR_KEYWORD,
    CONTINUE_KEYWORD,
    BREAK_KEYWORD,
    COMMA,
    MODULO,
    IDENTIFIER,
    EOF_TOKEN,
    TOKEN_LAST=EOF_TOKEN
} TOKENS;

/**
 * @brief returns the number of different tokens available
 */
#define TOKENS_LENGTH TOKEN_LAST + 1

/**
 * @brief string from of all different tokens 
 */
static const char* TOKEN_NAMES[] = {
    "O_BRACE",
    "C_BRACE",
    "O_PARENTHESIS",
    "C_PARENTHESIS",
    "SEMICOLON",
    "INT_KEYWORD",
    "RETURN_KEYWORD",
    "INT_LITERAL",
    "ADD_ASSIGN",
    "SUB_ASSIGN",
    "MUL_ASSIGN",
    "DIV_ASSIGN",
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
    "ASSIGN_KEYWORD",
    "IF_KEYWORD",
    "ELSE_KEYWORD",
    "WHILE_KEYWORD",
    "FOR_KEYWORD",
    "CONTINUE_KEYWORD",
    "BREAK_KEYWORD",
    "COMMA",
    "MODULO",
    "IDENTIFIER",
    "END OF FILE"
};

/**
 * @brief list of according token symbols to token types
 * if not a static symbol -> NULL and look for condition implementation -> conditions
 * needs to be sorted based on the sub tokens a token can have
 * Example: '+=' would be identified as '+' & '=' if the token '+' comes first in this list
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
    "+=",
    "-=",
    "*=",
    "/=",
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
    "continue",
    "break",
    ",",
    "%",
    NULL,
    NULL
};

/**
 * @brief token struct, contains the type and the string it was identified from 
 */
typedef struct token {
    TOKENS token_type;
    char value[INPUT_SIZE];
} token;

/**
 * @brief saves the file pointer and filename 
 */
typedef struct lexer {
    char* filename;
    FILE* fp;
}lexer;

/**
 * @brief custom implementation to identify the token type
 */
typedef bool (*token_condition)(char input[INPUT_SIZE], int index);

/**
 * @brief creates a new lexer from the filename
 * 
 * @param filename file to be read
 * @return pointer to new allocated struct lexer, NULL if failed
 */
lexer* lexer_open(char* filename);

/**
 * @brief reads the next token from lexer
 * 
 * @param lexer lexer to be read from
 * @param token to store
 * @return true when a token was identified
 * @return false when no token was identified
 */
bool lexer_next(lexer* lexer, token* token);

/**
 * @brief releases allocated memory from lexer
 * 
 * @param lexer 
 */
void lexer_free(lexer* lexer);

/**
 * @brief peaks the next token in lexer, does not increase its reading position
 * 
 * @param lexer lexer to be read from
 * @param token to store
 * @return true when a token was identified
 * @return false when no token was identified
 */
bool lexer_peak(lexer* lexer, token* token);

/**
 * @brief identifies a token from the given input based on the condition function
 * 
 * @param lexer lexer to be read from
 * @param token to store
 * @param input given input
 * @param condition condition function of the token type that is to be identified
 * @return true when a token was identified
 * @return false when no token was identified
 */
bool __identifiy_token(lexer* lexer, token* token, char input[INPUT_SIZE], token_condition condition);

/**
 * @brief identifies wether a given input is a literal int
 * 
 * @param input given input
 * @param index index at the given input string 
 * @return true 
 * @return false 
 */
bool __int_literal(char input[INPUT_SIZE], int index);

/**
 * @brief identifies wether a given input is a identifier
 * 
 * @param input given input
 * @param index index at the given input string
 * @return true 
 * @return false 
 */
bool __identifier(char input[INPUT_SIZE], int index);

#endif