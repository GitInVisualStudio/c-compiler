#include "header/statement.h"

void parse_continue(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, CONTINUE_KEYWORD);

    if (context->labels_count == 0) {
        fail_error("Can't continue without a loop!");
    }

    statement* state;
    DEFAULT_BODY(state, struct statement, CONTINUE);
    state->label = label_count;
    *body = (struct body*)state;

    check_valid(lexer, &current, SEMICOLON);
}

void parse_break(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, BREAK_KEYWORD);

    if (context->labels_count == 0) {
        fail_error("Can't break without a loop!");
    }

    statement* state;
    DEFAULT_BODY(state, struct statement, BREAK);
    state->label = label_count;
    *body = (struct body*)state;

    check_valid(lexer, &current, SEMICOLON);
}

void parse_return(lexer* lexer, body** b, context* context) {
    body* new;
    DEFAULT_BODY(new, struct body, RETURN);
    *b = new;

    token current;

    check_valid(lexer, &current, RETURN_KEYWORD);

    parse_expression(lexer, &new->child, context);

    if (new->child == NULL) 
        fail_error("missing expression");
    

    check_valid(lexer, &current, SEMICOLON);
}

void parse_statement(lexer* lexer, body** state, context* context) {
    token current;

    lexer_peak(lexer, &current);

    switch (current.token_type) {
        case RETURN_KEYWORD:
            parse_return(lexer, state, context);
            break;
        case INT_KEYWORD:
            parse_declaration(lexer, state, context);
            break;
        case CONTINUE_KEYWORD:
            parse_continue(lexer, state, context);
            break;
        case BREAK_KEYWORD:
            parse_break(lexer, state, context);
            break;
        case SEMICOLON:
            lexer_next(lexer, &current);
            return;
        case IF_KEYWORD:
            parse_if(lexer, state, context);
            return;
        case WHILE_KEYWORD:
            parse_while(lexer, state, context);
            return;
        case FOR_KEYWORD:
            parse_for(lexer, state, context);
            return;
        case O_BRACE:
            parse_list(lexer, state, context);
            return;
        default:
            parse_expression(lexer, state, context);
            check_valid(lexer, &current, SEMICOLON);
    }
}