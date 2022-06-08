#include "header/list.h"

void parse_list(lexer* lexer, body** body, context* _context) {
    token current;

    context* new_context = copy_context(_context);
    list* l;
    DEFAULT_BODY(l, struct list, LIST);
    l->print = __print_list;
    l->free = __free_list;

    list* next = NULL;
    list* prev = NULL;
    new_context->child = (struct body*)l;
    *body = (struct body*)new_context;

    lexer_peak(lexer, &current);

    // parse only one statement and return
    if (current.token_type != O_BRACE) {
        parse_statement(lexer, &l->child, new_context);

        if (l->child != NULL && l->child->type == DECLARE)
        {
            fail_error("A depended statement may not be a declaration!");
        }

        new_context->stack_offset_dif = new_context->stack_offset - _context->stack_offset;
        return;
    }

    check_valid(lexer, &current, O_BRACE);
    lexer_peak(lexer, &current);

    while (current.token_type != C_BRACE) {
        parse_statement(lexer, &l->child, new_context);

        next = (list*)calloc(1, sizeof(struct list));
        next->print = __print_list;
        next->free = __free_list;

        l->type = LIST;
        l->next = next;
        prev = l;
        l = next;

        lexer_peak(lexer, &current);
    }

    if (prev != l && prev != NULL) {
        free(l);
        prev->next = NULL;
    }

    new_context->stack_offset_dif = new_context->stack_offset - _context->stack_offset;
    check_valid(lexer, &current, C_BRACE);
}

void __free_list(body* body) {
    list* l = (list*)body;
    FREE_BODY((struct body*)l->next);
}

void __print_list(body* body, int depth) {
    PRINT_BODY(body->child, depth + 1);
    list* l = (list*)body;
    PRINT_BODY((struct body*)l->next, depth + 1);
}