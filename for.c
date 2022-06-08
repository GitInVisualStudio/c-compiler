#include "header/for.h"


void parse_for(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, FOR_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    for_body* _for;
    DEFAULT_BODY(_for, struct for_body, FOR);
    _for->free = __free_for;
    _for->print = __print_for;
    _for->label = ++label_count;
    
    struct context* new_context = copy_context(context);
    new_context->labels_count++;
    _for->child = (struct body*)new_context;

    // parse the initiator
    parse_statement(lexer, &_for->init, new_context);

    parse_expression(lexer, &_for->condition, new_context);
    check_valid(lexer, &current, SEMICOLON);

    // empty condition expression -> 1
    if (_for->condition == NULL) {
        struct constant* cons;
        DEFAULT_BODY(cons, struct constant, CONSTANT);
        cons->value = 1;
        _for->condition = (struct body*)cons;
    }
    
    parse_expression(lexer, &_for->expr, new_context);
    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_context->child, new_context);

    *body = (struct body*)_for;
}

void __free_for(body* body) {
    for_body* _for = (for_body*)body;
    FREE_BODY(_for->condition);
    FREE_BODY(_for->expr);
    FREE_BODY(_for->init);
}

void __print_for(body* body, int depth) {
    for_body* _for = (for_body*)body;
    printf("%s%s-CONDITION: \n", SPACE, BODY_TYPES_NAMES[_for->type]);
    PRINT_BODY(_for->condition, depth + 2);
    PRINT_BODY(_for->init, depth + 2);
    PRINT_BODY(_for->expr, depth + 2);
    PRINT_BODY(_for->child, depth + 1);
}