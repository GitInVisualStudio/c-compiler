#include "expression.h"

void parse_factor(lexer* lexer, body** factor, context* context) {
    token current;
    lexer_peak(lexer, &current);
    
    if (current.token_type == O_PARENTHESIS) {
        lexer_next(lexer, &current);
        parse_expression(lexer, factor, context);
        check_valid(lexer, &current, C_PARENTHESIS);
        return;
    }

    if (current.token_type == BITWISE_COMPLEMENT || current.token_type == MINUS || current.token_type == LOGICAL_NEGATION) {
        parse_unary(lexer, factor, context);
        return;
    }

    if (current.token_type == INT_LITERAL) {
        parser_constant(lexer, factor, context);
        return;
    }

    if (current.token_type == IDENTIFIER) {
        lexer_next(lexer, &current);
        token next;
        lexer_peak(lexer, &next);
        if (next.token_type == O_PARENTHESIS) {
            parse_call(lexer, factor, context, &current);
            return;
        }

        if (get_offset(context, current.value) == -1) {
            char error_msg[INPUT_SIZE * 2];
            sprintf(error_msg, "Variable: (%s) not declared previously!", current.value);
            fail_error(error_msg);
        }

        variable* var;
        DEFAULT_BODY(var, variable, VARIABLE);
        var->print = __print_variable;
        var->offset = get_offset(context, current.value);
        strcpy(var->name, current.value);
        *factor = (body*)var;
        return;
    }

    *factor = NULL;
}

void parse_expressions(lexer* lexer, body** body, context* context, parser p, TOKENS* tokens, int tokens_length) {
    expression* new;
    p(lexer, (struct body**)&new, context);
    *body = (struct body*)new;

    token current;
    lexer_peak(lexer, &current);
    while(contains_token(tokens, tokens_length, current.token_type)) {
        // go next
        lexer_next(lexer, &current);

        DEFAULT_BODY(new, struct expression, EXPRESSION);
        new->free = __free_expression;
        new->print = __print_expression;
        new->op = current.token_type;
        new->child = *body;
        p(lexer, &new->child2, context);

        if (new->child2 == NULL || new->child == NULL)
            fail_error("missing first expressoin!");

        *body = (struct body*)new;
        
        lexer_peak(lexer, &current);
    }
}

void parse_term(lexer* lexer, body** term, context* context) {
    parse_expressions(lexer, term, context, parse_factor, (TOKENS[]){MULTIPLICATION, DIVISION, MODULO}, 3);
}

void parse_additive_exp(lexer* lexer, body** expr, context* context) {
    parse_expressions(lexer, expr, context, parse_term, (TOKENS[]){ADDITION, MINUS}, 2);
}

void parse_relational_exp(lexer* lexer, body** expr, context* context) {
    parse_expressions(lexer, expr, context, parse_additive_exp, (TOKENS[]){GREATER_THAN, GREATER_THAN_OR, LESS_THAN, LESS_THAN_OR}, 4);
}

void parse_equality_exp(lexer* lexer, body** expr, context* context) {
    parse_expressions(lexer, expr, context, parse_relational_exp, (TOKENS[]){EQUAL, N_EQUAL}, 2);
}

void parse_logical_and_exp(lexer* lexer, body** expr, context* context) {
    parse_expressions(lexer, expr, context, parse_equality_exp, (TOKENS[]){AND}, 1);
}

void parse_logical_or_exp(lexer* lexer, body** expr, context* context) {
    parse_expressions(lexer, expr, context, parse_logical_and_exp, (TOKENS[]){OR}, 1);
}

void parse_expression(lexer* lexer, body** expr, context* context) {
    token current;
    lexer_peak(lexer, &current);

    if (current.token_type == IDENTIFIER) {
        fflush(lexer->fp);
        int offset = ftell(lexer->fp);

        lexer_next(lexer, &current);

        lexer_next(lexer, &current);
        fseek(lexer->fp, offset, SEEK_SET);

        if (contains_token((TOKENS[]){ASSIGN_KEYWORD, ADD_ASSIGN, SUB_ASSIGN, MUL_ASSIGN, DIV_ASSIGN}, 5, current.token_type)) {   
            parse_assignment(lexer, expr, context);         
            return;
        }

        parse_logical_or_exp(lexer, expr, context);
        return;
    }

    parse_logical_or_exp(lexer, expr, context);
}

void __free_expression(body* body) {
    expression* expr = (expression*)body;
    FREE_BODY(expr->child2);
}

void __print_expression(body* body, int depth) {
    expression* ex = (expression*)body;
    printf("%s \'%s\':\n", BODY_TYPES_NAMES[body->type], TOKEN_SYMBOLS[ex->op]);
    PRINT_BODY(ex->child, depth + 1);
    PRINT_BODY(ex->child2, depth + 1);
}