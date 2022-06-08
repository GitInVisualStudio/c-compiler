#include "header/unary.h"

void parse_unary(lexer* lexer, body** body, context* context) {
    token current;

    lexer_next(lexer, &current);
    unaryOps* new;
    DEFAULT_BODY(new, struct unaryOps, UNARY_OPS);
    new->op = current.token_type;
    *body = (struct body*)new;
    parse_factor(lexer, &new->child, context);

    if (new->child == NULL) 
        fail_error("missing factor");
}

void __print_unary(body* body, int depth) {
    printf("%s \'%s\':\n", BODY_TYPES_NAMES[body->type], TOKEN_SYMBOLS[((unaryOps*)body)->op]);
    PRINT_BODY(body->child, depth + 1);
}