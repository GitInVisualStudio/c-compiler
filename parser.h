#include "lexer.h"

#ifndef PARSER_H
#define PARSER_H

#define OFFSET_SIZE 8

static int label_count = 0;

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
    IF,
    WHILE,
    FOR,
    CONTINUE,
    BREAK,
    CALL,
    CONTEXT,
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
    "FOR",
    "CONTINUE",
    "BREAK",
    "CALL",
    "CONTEXT"
};

typedef struct function_call {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    struct body** expressions;
    int argc;
} function_call;

typedef struct statement {
    BODY_TYPES type;
    struct body* child;
    int label;
} statement;

typedef struct variable {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int offset;
} variable;

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
    TOKENS op;
} expression;

typedef struct unaryOps {
    BODY_TYPES type;
    struct body* child;
    TOKENS op;
} unaryOps;

typedef struct while_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
    int label;
} while_body;

typedef struct for_body {
    BODY_TYPES type;
    struct body* child;
    struct body* condition;
    struct body* init;
    struct body* expr;
    int label;
} for_body;

/**
 * @brief linked list of bodys
 */
typedef struct list {
    BODY_TYPES type;
    struct body* child;
    struct list* next;
} list;

typedef struct context {
    BODY_TYPES type;
    struct body* child;
    char** variables;
    int stack_offset;
    int stack_offset_dif;
    int labels_count;
    char** function_names;
    int* function_params;
    int functions_length;
} context;

typedef struct function {
    BODY_TYPES type;
    struct body* child;
    char name[INPUT_SIZE];
    int argc;
} function;

typedef struct program {
    BODY_TYPES type;
    struct body* child;
    struct function** funcs;
    int func_length;
} program;

typedef struct body {
    BODY_TYPES type;
    struct body* child;
} body;

typedef void(*parser)(lexer*, body**, context*);

void parse_program(lexer* lexer, body** program);
void parse_function(lexer* lexer, function** function, context* context);
void parse_statement(lexer* lexer, body** statement, context* context);
void parse_expression(lexer* lexer, body** expression, context* context);
void parse_logical_or_exp(lexer* lexer, body** expression, context* context);
void parse_logical_and_exp(lexer* lexer, body** expression, context* context);
void parse_equality_exp(lexer* lexer, body** expression, context* context);
void parse_relational_exp(lexer* lexer, body** expression, context* context);
void parse_additive_exp(lexer* lexer, body** expression, context* context);
void parse_term(lexer* lexer, body** term, context* context);
void parse_factor(lexer* lexer, body** factor, context* context);
void parse_return(lexer* lexer, body** body, context* context);
void parse_declaration(lexer* lexer, body** state, context* context);
void parse_if(lexer* lexer, body** body, context* context);
void parse_list(lexer* lexer, body** body, context* context);
void parse_else(lexer* lexer, body** body, context* context);
void parse_while(lexer* lexer, body** body, context* context);
void parse_for(lexer* lexer, body** body, context* context);
void parse_continue(lexer* lexer, body** body, context* context);
void parse_break(lexer* lexer, body** body, context* context);
void parse_assignment(lexer* lexer, body** body, context* context);
void parse_call(lexer* lexer, body** body, context* context, token* prev);
void parse_parameter(lexer* lexer, context* context);

void cpy_str_arry(char** dest, char** src, int length);

void parse_expressions(lexer* lexer, body** body, context* context, parser p, TOKENS* tokens, int tokens_length);
bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token);

void append_variable(context* context, char* name);
context* copy_context(context* context);
int get_offset(context* context, char* name);

void append_function(context* context, char* name, int argc);
int get_argc(context* context, char* name);

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