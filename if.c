#include "if.h"

void parse_if(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, IF_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    if_body* _if;
    DEFAULT_BODY(_if, struct if_body, IF);
    _if->free = __free_if;
    _if->print = __print_if;

    parse_expression(lexer, &_if->condition, context);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &_if->child, context);

    *body = (struct body*)_if;

    parse_else(lexer, (struct body**)&_if, context);
}

void parse_else(lexer* lexer, body** body, context* context) {
    if_body** _if = (if_body**)body;
    token current;
    lexer_peak(lexer, &current);

    if (current.token_type == ELSE_KEYWORD) {

        lexer_next(lexer, &current);
        lexer_peak(lexer, &current);

        if (current.token_type == IF_KEYWORD) {
            parse_statement(lexer, &(*_if)->else_child, context);
            return;
        }

        parse_list(lexer, &(*_if)->else_child, context);
    }
}

void __free_if(body* body) {
    if_body* _if = (if_body*)body;
    FREE_BODY(_if->condition);
    FREE_BODY(_if->else_child);
}

void __print_if(body* body, int depth) {
    if_body* _if = (if_body*)body;
    printf("%s%s-CONDITION: \n", SPACE, BODY_TYPES_NAMES[_if->type]);
    PRINT_BODY(_if->condition, depth + 2);

    for (int i = 0; i < depth + 1; i++)
        printf("%s", "  ");
    printf("TRUE:\n");
    PRINT_BODY(_if->child, depth + 1);

    if (_if->else_child == NULL)
        return;

    for (int i = 0; i < depth + 1; i++)
        printf("%s", "  ");
    printf("FALSE:\n");
    PRINT_BODY(_if->else_child, depth + 1);
}