#include "variable.h"

void parse_assignment(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, IDENTIFIER);

    if (get_offset(context, current.value) == -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable %s not declared previously!", current.value);
        fail_error(error_msg);
    }

    // TODO: parse assignment
    variable* new;
    DEFAULT_BODY(new, struct variable, ASSIGN);
    new->print = __print_variable;
    new->offset = get_offset(context, current.value);
    strcpy(new->name, current.value);
    *body = (struct body*)new;

    lexer_next(lexer, &current);

    switch(current.token_type) {
        case ASSIGN_KEYWORD:
            parse_expression(lexer, &new->child, context);
            break;
        case ADD_ASSIGN:
            parse_op_assign(lexer, new, context, ADDITION);
            break;
        case SUB_ASSIGN:
            parse_op_assign(lexer, new, context, MINUS);
            break;
        case MUL_ASSIGN:
            parse_op_assign(lexer, new, context, MULTIPLICATION);
            break;
        case DIV_ASSIGN:
            parse_op_assign(lexer, new, context, DIVISION);
            break;
    }
}

void parse_op_assign(lexer* lexer, variable* body, context* context, TOKENS op) {
    variable* var;
    DEFAULT_BODY(var, struct variable, VARIABLE);
    var->offset = get_offset(context, body->name);
    var->print = __print_variable;
    strcpy(var->name, body->name);

    expression* expr = (expression*)malloc(sizeof(struct expression));
    expr->type = EXPRESSION;
    expr->op = op;
    expr->print = __print_expression;
    expr->free = __free_expression;
    expr->child2 = (struct body*)var;
    body->child = (struct body*)expr;

    parse_expression(lexer, &expr->child, context);
}

void parse_declaration(lexer* lexer, body** state, context* context) {
    token current;
    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    if (get_offset(context, current.value) != -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable: (%s) already declared!", current.value);
        fail_error(error_msg);
    }

    variable* new;
    DEFAULT_BODY(new, struct variable, DECLARE);
    new->print = __print_variable;
    strcpy(new->name, current.value);

    //TODO: append the variable to the variable list and increase the stack pointer accordingly
    append_variable(context, new->name);
    new->offset = get_offset(context, new->name);
    
    *state = (body*)new;

    lexer_peak(lexer, &current);

    if(current.token_type == ASSIGN_KEYWORD) {

        lexer_next(lexer, &current);

        parse_expression(lexer, &new->child, context);
    }

    check_valid(lexer, &current, SEMICOLON);
}

void __print_variable(body* body, int depth) {
    variable* var = (variable*)body;
    printf("%s: %s(%i)\n", BODY_TYPES_NAMES[body->type], var->name, var->offset);
    PRINT_BODY(body->child, depth + 1);
}