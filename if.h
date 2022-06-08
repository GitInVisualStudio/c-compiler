#ifndef IF_H
#define IF_H

#include "parser.h"

/**
 * @brief if condition, else body is optional and can be NULL
 */
typedef struct if_body {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct body* else_child;
    struct body* condition;
} if_body;


void parse_if(lexer* lexer, body** body, context* context);
void parse_else(lexer* lexer, body** body, context* context);

void __free_if(body* body);
void __print_if(body* body, int depth);

#endif