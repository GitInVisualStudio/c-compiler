#include "context.h"
#include "lexer.h"
#include "list.h"
#include "while.h"
#include "for.h"
#include "program.h"
#include "function.h"
#include "if.h"
#include "function_call.h"
#include "statement.h"
#include "expression.h"
#include "variable.h"
#include "constant.h"
#include "unary.h"

#ifndef PARSER_H
#define PARSER_H

/**
 * @brief offset size of the stack per push
 */
#define OFFSET_SIZE 8

#define SPACE "  "

/**
 * @brief used to tell what labels each element has -> needs to be done to tell continue & break where to go
 * 
 */
extern int label_count;

/**
 * @brief All different body types an AST can have, start at 0
 * 
 */
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

/**
 * @brief string representation of each body type
 * 
 */
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

/**
 * @brief delegate function to free & print each body type
 */
typedef struct body body;
typedef void (*free_body)(body*);
typedef void (*print_body)(body*, int);

// define context type, necessary for linking
typedef struct context context;

/**
 * @brief generic body type to represent an AST (abstract syntax tree)
 * stores the type and any further children of the AST in child
 */
struct body {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
};


#define FREE_BODY __free_body
#define PRINT_BODY __print_body

void __free_body(body* body);
void __print_body(body* body, int depth);


/**
 * generic free method
 */
#define DEFAULT_PRINT __default_print_body
void __default_print_body(struct body* body, int depth);
/**
 * default is no implementation at all
 */
#define DEFAULT_FREE NULL

/**
 * @brief allocates corresponding memory to body pointer and init it with default values
 */
#define DEFAULT_BODY(var, struct, id) \
    do {\
        var = (struct*)calloc(sizeof(struct), 1);\
        var->type = id;\
        var->free = DEFAULT_FREE;\
        var->print = DEFAULT_PRINT;\
    } while(0)\

/**
 * @brief copies the strings of src into dest, dest needs to be allocated
 * 
 * @param dest string array
 * @param src string array
 * @param length how many strings to copy
 */
void cpy_str_arry(char** dest, char** src, int length);

/**
 * @brief generic parser function
 */
typedef void(*parser)(lexer*, body**, context*);

/**
 * @brief returns whether token is in tokens
 * 
 * @param tokens 
 * @param tokens_length 
 * @param token 
 * @return true 
 * @return false 
 */
bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token);

/**
 * @brief scans the next token, safes the content into token and checks whether the token equals type, invokes fail if not
 * 
 * @param lexer current lexer
 * @param token token struct to be filled with content
 * @param type expected token type
 */
void check_valid(lexer* lexer, token* token, TOKENS type);

/**
 * @brief prints the error msg and exits the program
 * 
 * @param token 
 * @param expected 
 */
void fail(token* token, TOKENS expected);
void fail_error(char* error);

#endif