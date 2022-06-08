#ifndef FUNCTION_H
#define FUNCTION_H

#include "parser.h"

/**
 * @brief function has an identifier name and an amount of arguments
 * NOTE: argc stores the amount of arguments, but not their type, nor their name
 * we don't need to store the type, because we only support integers 
 * the individual identifiers of the arguments will be stored in the context (scope) of that function, which will be passed extra
 */
typedef struct function {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    char name[INPUT_SIZE];
    int argc;
} function;

void parse_function(lexer* lexer, function** function, context* context);
void parse_declaration(lexer* lexer, body** state, context* context);
void parse_parameter(lexer* lexer, context* context);

void __print_function(body* body, int depth);

#endif