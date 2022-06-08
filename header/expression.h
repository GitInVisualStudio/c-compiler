#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "parser.h"

/**
 * @brief an expression has 2 children: child (op) child2
 * for example: 1 + 2
 */
typedef struct expression {
    BODY_TYPES type;
    struct body* child;
    free_body free;
    print_body print;
    struct body* child2;
    TOKENS op;
} expression;

void parse_expression(lexer* lexer, body** expression, context* context);
void parse_logical_or_exp(lexer* lexer, body** expression, context* context);
void parse_logical_and_exp(lexer* lexer, body** expression, context* context);
void parse_equality_exp(lexer* lexer, body** expression, context* context);
void parse_relational_exp(lexer* lexer, body** expression, context* context);
void parse_additive_exp(lexer* lexer, body** expression, context* context);
void parse_term(lexer* lexer, body** term, context* context);
void parse_factor(lexer* lexer, body** factor, context* context);

/**
 * @brief generic parser function
 */
typedef void(*parser)(lexer*, body**, context*);

/**
 * @brief parses an expression
 * 
 * because every operator does not have the same priority, we need to parse expressions with higher priority first
 * the structur of each expression stays the same, only their priority is different. 
 * This function is the generic function to parse an expression with unknown prioriy
 * 
 * @param lexer lexer to read from
 * @param body child that needs to be set
 * @param context current context of that expression
 * @param p parser to the next higher priority expression
 * @param tokens tokens of equal priority
 * @param tokens_length 
 */
void parse_expressions(lexer* lexer, body** body, context* context, parser p, TOKENS* tokens, int tokens_length);

void __free_expression(body* body);
void __print_expression(body* body, int depth);

#endif