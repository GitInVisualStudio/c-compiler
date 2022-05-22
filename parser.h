#include "lexer.h"

#ifndef PARSER_H
#define PARSER_H

#define OFFSET_SIZE 8
#define OFFSET_LENGTH(x) x/OFFSET_SIZE

typedef enum BODY_TYPES {
    CONSTANT=0,
    RETURN,
    FUNCTION,
    PROGRAM,
    UNARY_OPS,
    EXPRESSION,
    ASSIGN,
    LIST,
    DECLARE,
    VARIABLE,
    IF_BODY,
    WHILE_BODY,
    FOR_BODY
} BODY_TYPES;

static const char* BODY_TYPES_NAMES[] = {
    "CONSTANT",
    "RETURN",
    "FUNCTION",
    "PROGRAM",
    "UNARY_OPS",
    "EXPRESSION",
    "ASSIGN",
    "LIST",
    "DECLARE",
    "VARIABLE",
    "IF",
    "WHILE",
    "FOR"
};

typedef struct assign {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int offset;
} assign;

typedef struct variable {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int offset;
} variable;

typedef struct declare {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
} declare;

typedef struct if_body {
    BODY_TYPES type;
    struct body* child;
    struct body* else_child;
    struct body* condition;
} if_body;

typedef struct constant {
    BODY_TYPES type;
    struct body* child;
    int value;
} constant;

typedef struct expression {
    BODY_TYPES type;
    struct body* child;
    struct body* child2;
    char op[INPUT_SIZE]; // longest operator is 2 charater long + null determinator
} expression;

typedef struct unaryOps {
    BODY_TYPES type;
    struct body* child;
    unsigned char operator;
} unaryOps;

typedef struct while_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
} while_body;

typedef struct for_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
    struct body* init;
    struct body* expr;
} for_body;

/**
 * @brief linked list of bodys
 */
typedef struct list {
    BODY_TYPES type;
    struct body* child;
    struct list* next;
    char** variables;
    int stack_offset;
    int stack_offset_dif;
} list;

typedef struct function {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
} function;

typedef struct body {
    BODY_TYPES type;
    struct body* child;
} body;

typedef void(*parser)(lexer*, body**, list*);

void parse_program(lexer* lexer, body** program);
void parse_function(lexer* lexer, function** function);
void parse_statement(lexer* lexer, body** statement, list* list);
void parse_expression(lexer* lexer, body** expression, list* list);
void parse_logical_or_exp(lexer* lexer, body** expression, list* list);
void parse_logical_and_exp(lexer* lexer, body** expression, list* list);
void parse_equality_exp(lexer* lexer, body** expression, list* list);
void parse_relational_exp(lexer* lexer, body** expression, list* list);
void parse_additive_exp(lexer* lexer, body** expression, list* list);
void parse_term(lexer* lexer, body** term, list* list);
void parse_factor(lexer* lexer, body** factor, list* list);
void parse_return(lexer* lexer, body** body, list* list);
void parse_declaration(lexer* lexer, body** state, list* list);
void parse_if(lexer* lexer, body** body, list* list);
void parse_list(lexer* lexer, body** body, list* list);
void parse_else(lexer* lexer, body** body, list* list);
void parse_while(lexer* lexer, body** body, list* list);
void parse_for(lexer* lexer, body** body, list* list);

void parse_expressions(lexer* lexer, body** body, list* list, parser p, TOKENS* tokens, int tokens_length);
void append_variable(list* list, char* name);
list* copy_list(list* list);
int get_offset(list* list, char* name);
bool contains_variable(list* list, char* name);
bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token);


/**
 * @brief scans the next token, safes the content into token and checks whether the token equals type, invokes fail if not
 * 
 * @param lexer current lexer
 * @param token token struct to be filled with content
 * @param type expected token type
 */
void check_valid(lexer* lexer, token* token, TOKENS type);

void fail(token* token, TOKENS expected);
void fail_error(char* error);

void print_program(body* program, int depth);

void parse_free(body* program);

#endif