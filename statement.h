#ifndef STATEMENT_H
#define STATEMENT_H

#include "parser.h"

/**
 * @brief continue or break statements
 * needs the label to identifiy the loop
 */
typedef struct statement {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    int label;
} statement;

void parse_continue(lexer* lexer, body** body, context* context);
void parse_break(lexer* lexer, body** body, context* context);
void parse_return(lexer* lexer, body** body, context* context);
void parse_statement(lexer* lexer, body** statement, context* context);

#endif