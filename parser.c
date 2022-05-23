#include "parser.h"

void fail_error(char* error) {
    fprintf(stderr, "%s\n", error);
    exit(-1);
}

void fail(token* token, TOKENS expected) {
    fprintf(stderr, "Expected token: %s, got %s!\n", TOKEN_NAMES[expected], TOKEN_NAMES[token->token_type]);
    exit(-1);
}

void check_valid(lexer* lexer, token* token, TOKENS type) {
    lexer_next(lexer, token);
    if (token->token_type != type) fail(token, type);
}

list* copy_list(list* list) {
    struct list* new = (struct list*)calloc(1, sizeof(struct list));
    new->stack_offset = list->stack_offset;
    new->type = LIST;
    new->labels_count = list->labels_count;
    int length = (list->stack_offset/OFFSET_SIZE);
    new->variables = (char**)calloc(sizeof(char*), length);
    for (int i = 0; i < length; i++) {
        int str_length = strlen(list->variables[i]);
        new->variables[i] = (char*)calloc(sizeof(char), str_length + 1);
        strcpy(new->variables[i], list->variables[i]);
    }
    return new;
}

void append_variable(list* list, char* name) {
    list->stack_offset += OFFSET_SIZE;
    int stack_length = (list->stack_offset/OFFSET_SIZE);
    int name_length = strlen(name);
    list->variables = (char**)realloc(list->variables, sizeof(char*) * stack_length);
    list->variables[stack_length - 1] = (char*)calloc(name_length + 1, sizeof(char));
    strcpy(list->variables[stack_length - 1], name);
}

int get_offset(list* list, char* name) {
    int stack_length = (list->stack_offset/OFFSET_SIZE);
    for (int i = 0; i < stack_length; i++) 
        if (strcmp(name, list->variables[i]) == 0)
            return (i + 1) * OFFSET_SIZE;
    return -1;
}

bool contains_variable(list* list, char* name) {
    int stack_length = (list->stack_offset/OFFSET_SIZE);
    for (int i = 0; i < stack_length; i++) 
        if (strcmp(name, list->variables[i]) == 0)
            return true;
    return false;
}

void parse_factor(lexer* lexer, body** factor, list* list) {
    token current;
    lexer_next(lexer, &current);

    if (current.token_type == O_PARENTHESIS) {
        parse_expression(lexer, factor, list);
        check_valid(lexer, &current, C_PARENTHESIS);
        return;
    }

    if (current.token_type == BITWISE_COMPLEMENT || current.token_type == MINUS || current.token_type == LOGICAL_NEGATION) {
        unaryOps* new = (unaryOps*)malloc(sizeof(struct unaryOps));
        new->type = UNARY_OPS;
        new->op = current.token_type;
        *factor = (body*)new;
        parse_factor(lexer, &new->child, list);
        return;
    }

    if (current.token_type == INT_LITERAL) {
        constant* cons = (constant*)malloc(sizeof(struct constant));
        cons->type = CONSTANT;
        cons->value = atoi(current.value);
        cons->child = NULL;
        *factor = (body*)cons;
        return;
    }

    if (current.token_type == IDENTIFIER) {
        
        if (!contains_variable(list, current.value)) {
            char error_msg[INPUT_SIZE * 2];
            sprintf(error_msg, "Variable: (%s) not declared previously!", current.value);
            fail_error(error_msg);
        }

        variable* var = (variable*)malloc(sizeof(struct variable));
        var->child = NULL;
        var->type = VARIABLE;
        var->offset = get_offset(list, current.value);
        strcpy(var->name, current.value);
        *factor = (body*)var;
        return;
    }

    fail(&current, INT_LITERAL);
}

bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token) {
    for (int i = 0; i < tokens_length; i++)
        if (tokens[i] == token)
            return true;
    return false;
}

void parse_expressions(lexer* lexer, body** body, list* list, parser p, TOKENS* tokens, int tokens_length) {
    expression* new;
    p(lexer, (struct body**)&new, list);
    *body = (struct body*)new;

    token current;
    lexer_peak(lexer, &current);
    while(contains_token(tokens, tokens_length, current.token_type)) {
        // go next
        lexer_next(lexer, &current);

        new = (expression*)malloc(sizeof(struct expression));
        new->type = EXPRESSION;
        new->op = current.token_type;
        new->child = *body;
        p(lexer, &new->child2, list);
        *body = (struct body*)new;
        
        lexer_peak(lexer, &current);
    }
}

void parse_term(lexer* lexer, body** term, list* list) {
    parse_expressions(lexer, term, list, parse_factor, (TOKENS[]){MULTIPLICATION, DIVISION}, 2);
}

void parse_additive_exp(lexer* lexer, body** expr, list* list) {
    parse_expressions(lexer, expr, list, parse_term, (TOKENS[]){ADDITION, MINUS}, 2);
}

void parse_relational_exp(lexer* lexer, body** expr, list* list) {
    parse_expressions(lexer, expr, list, parse_additive_exp, (TOKENS[]){GREATER_THAN, GREATER_THAN_OR, LESS_THAN, LESS_THAN_OR}, 4);
}

void parse_equality_exp(lexer* lexer, body** expr, list* list) {
    parse_expressions(lexer, expr, list, parse_relational_exp, (TOKENS[]){EQUAL, N_EQUAL}, 2);
}

void parse_logical_and_exp(lexer* lexer, body** expr, list* list) {
    parse_expressions(lexer, expr, list, parse_equality_exp, (TOKENS[]){AND}, 1);
}

void parse_logical_or_exp(lexer* lexer, body** expr, list* list) {
    parse_expressions(lexer, expr, list, parse_logical_and_exp, (TOKENS[]){OR}, 1);
}

void parse_expression(lexer* lexer, body** expr, list* list) {
    token current;
    lexer_peak(lexer, &current);

    if (current.token_type == IDENTIFIER) {

        //i know this is not fancy :)
        fflush(lexer->fp);
        int offset = ftell(lexer->fp);

        lexer_next(lexer, &current);

        lexer_next(lexer, &current);
        fseek(lexer->fp, offset, SEEK_SET);
        if (current.token_type != ASSIGN_KEYWORD) {            
            parse_logical_or_exp(lexer, expr, list);
            return;
        }

        lexer_next(lexer, &current);

        if (!contains_variable(list, current.value)) {
            char error_msg[INPUT_SIZE * 2];
            sprintf(error_msg, "Variable %s not declared previously!", current.value);
            fail_error(error_msg);
        }

        // TODO: parse assignment
        variable* new = (variable*)malloc(sizeof(struct variable));
        new->type = ASSIGN;
        new->child = NULL;
        new->offset = get_offset(list, current.value);
        strcpy(new->name, current.value);
        *expr = (body*)new;

        check_valid(lexer, &current, ASSIGN_KEYWORD);

        parse_expression(lexer, &new->child, list);
        return;
    }

    parse_logical_or_exp(lexer, expr, list);
}

void parse_return(lexer* lexer, body** b, list* list) {
    body* new = (body*)malloc(sizeof(struct body));
    new->type = RETURN;
    new->child = NULL;
    *b = new;

    token current;

    check_valid(lexer, &current, RETURN_KEYWORD);

    parse_expression(lexer, &new->child, list);

    check_valid(lexer, &current, SEMICOLON);
}

void parse_declaration(lexer* lexer, body** state, list* list) {
    token current;
    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    if (contains_variable(list, current.value)) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable: (%s) already declared!", current.value);
        fail_error(error_msg);
    }

    variable* new = (variable*)malloc(sizeof(struct variable));
    new->child = NULL;
    new->type = DECLARE;
    strcpy(new->name, current.value);

    //TODO: append the variable to the variable list and increase the stack pointer accordingly
    append_variable(list, new->name);
    new->offset = get_offset(list, new->name);
    
    *state = (body*)new;

    lexer_peak(lexer, &current);

    if(current.token_type == ASSIGN_KEYWORD) {

        lexer_next(lexer, &current);

        parse_expression(lexer, &new->child, list);
    }

    check_valid(lexer, &current, SEMICOLON);
}

void parse_else(lexer* lexer, body** body, list* list) {
    if_body** _if = (if_body**)body;

    token current;

    lexer_peak(lexer, &current);

    if (current.token_type == ELSE_KEYWORD) {

        lexer_next(lexer, &current);

        lexer_peak(lexer, &current);

        if (current.token_type == IF_KEYWORD) {
            
            parse_statement(lexer, &(*_if)->else_child, list);
            return;
        }

        parse_list(lexer, &(*_if)->else_child, list);
    }
}

void parse_if(lexer* lexer, body** body, list* list) {
    token current;

    check_valid(lexer, &current, IF_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    if_body* _if = (if_body*)malloc(sizeof(struct if_body));
    _if->type = IF;
    _if->child = NULL;
    _if->else_child = NULL;

    parse_expression(lexer, &_if->condition, list);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &_if->child, list);

    *body = (struct body*)_if;

    parse_else(lexer, (struct body**)&_if, list);
}

void parse_while(lexer* lexer, body** body, list* list) {
    token current;

    check_valid(lexer, &current, WHILE_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    while_body* _while = (while_body*)malloc(sizeof(struct while_body));
    _while->type = WHILE;
    _while->child = NULL;
    _while->condition = NULL;
    _while->label = ++label_count;

    struct list* new_list = copy_list(list);
    new_list->labels_count++;
    _while->child = (struct body*)new_list;

    parse_expression(lexer, &_while->condition, new_list);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_list->child, new_list);

    *body = (struct body*)_while;
}

void parse_for(lexer* lexer, body** body, list* list) {
    token current;

    check_valid(lexer, &current, FOR_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    for_body* _for = (for_body*)malloc(sizeof(struct for_body));
    _for->type = FOR;
    _for->child = NULL;
    _for->expr = NULL;
    _for->condition = NULL;
    _for->label = ++label_count;
    
    struct list* new_list = copy_list(list);
    new_list->labels_count++;
    _for->child = (struct body*)new_list;

    // parse the initiator
    parse_statement(lexer, &_for->init, new_list);

    parse_expression(lexer, &_for->condition, new_list);
    check_valid(lexer, &current, SEMICOLON);

    parse_statement(lexer, &_for->expr, new_list);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_list->child, new_list);

    *body = (struct body*)_for;
}

void parse_continue(lexer* lexer, body** body, list* list) {
    token current;
    check_valid(lexer, &current, CONTINUE_KEYWORD);

    if (list->labels_count == 0) {
        fail_error("Can't continue without a loop!");
    }

    statement* state = (statement*)malloc(sizeof(struct statement));
    state->label = label_count;
    state->child = NULL;
    state->type = CONTINUE;
    *body = (struct body*)state;

    check_valid(lexer, &current, SEMICOLON);
}

void parse_break(lexer* lexer, body** body, list* list) {
    token current;
    check_valid(lexer, &current, BREAK_KEYWORD);

    if (list->labels_count == 0) {
        fail_error("Can't break without a loop!");
    }

    statement* state = (statement*)malloc(sizeof(struct statement));
    state->label = label_count;
    state->child = NULL;
    state->type = BREAK;
    *body = (struct body*)state;

    check_valid(lexer, &current, SEMICOLON);
}

void parse_statement(lexer* lexer, body** state, list* list) {
    token current;

    lexer_peak(lexer, &current);

    switch (current.token_type) {
        case RETURN_KEYWORD:
            parse_return(lexer, state, list);
            break;
        case INT_KEYWORD:
            parse_declaration(lexer, state, list);
            break;
        case CONTINUE_KEYWORD:
            parse_continue(lexer, state, list);
            break;
        case BREAK_KEYWORD:
            parse_break(lexer, state, list);
            break;
        case SEMICOLON:
            lexer_next(lexer, &current);
            break;
        case IF_KEYWORD:
            parse_if(lexer, state, list);
            return;
        case WHILE_KEYWORD:
            parse_while(lexer, state, list);
            return;
        case FOR_KEYWORD:
            parse_for(lexer, state, list);
            return;
        case O_BRACE:
            parse_list(lexer, state, list);
            return;
        default:
            parse_expression(lexer, state, list);
    }

    lexer_peak(lexer, &current);
    if (current.token_type == SEMICOLON) {
        lexer_next(lexer, &current);
        return;
    }
}

void parse_function(lexer* lexer, function** func) {
    function* new = (function*)malloc(sizeof(struct function));
    new->type = FUNCTION;
    *func = new;

    token current;

    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    // copy the function name into the AST 
    strcpy(new->name, current.value);

    check_valid(lexer, &current, O_PARENTHESIS);
    check_valid(lexer, &current, C_PARENTHESIS);
    
    parse_list(lexer, &new->child, NULL);
}

void parse_list(lexer* lexer, body** body, list* _list) {
    token current;

    check_valid(lexer, &current, O_BRACE);

    int start = 0;

    list* l = (list*)calloc(1, sizeof(struct list));
    list *next = NULL;
    list *prev = NULL;

    if (_list != NULL) {
        free(l);
        start = _list->stack_offset;
        l = copy_list(_list);
    }

    _list = l;
    l->type = LIST;
    *body = (struct body*)l;

    lexer_peak(lexer, &current);

    while (current.token_type != C_BRACE) {
        parse_statement(lexer, &l->child, _list);

        next = (list*)calloc(1, sizeof(struct list));
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

    _list->stack_offset_dif = _list->stack_offset - start;
    check_valid(lexer, &current, C_BRACE);
}

void parse_program(lexer* lexer, body** prog) {
    body* new = (body*)malloc(sizeof(struct body));
    new->type = PROGRAM;
    *prog = new;
    parse_function(lexer, (function**)&new->child);
}

void parse_free(body* program) {
    if (program == NULL) return;
    if (program->type == EXPRESSION) {
        expression* exp = (expression*)program;
        parse_free(exp->child2);
    }
    if (program->type == IF) {
        if_body* exp = (if_body*)program;
        parse_free(exp->condition);
        parse_free(exp->else_child);
    }
    if (program->type == LIST) {
        list* l = (list*)program;
        if (l->variables != NULL) {
            for (int i = 0; i < l->stack_offset/OFFSET_SIZE; i++)
                free(l->variables[i]);
            free(l->variables);
        }
        parse_free((body*)l->next);
    }
    if (program->type == FOR) {
        for_body* f = (for_body*)program;
        parse_free((body*)f->condition);
        parse_free((body*)f->init);
        parse_free((body*)f->expr);
    }
    parse_free(program->child);
    free(program);
}

void print_program(body* program, int depth) {
    const char* space = "  ";
    if (program == NULL) return;
    if (program->type != LIST)
        for (int i = 0; i < depth; i++)
            printf("%s", space);
    expression* ex;
    unaryOps* unOp;
    list* current;
    if_body* _if;
    while_body* _while;
    for_body* _for;
    variable* var;
    switch(program->type) {
        case FUNCTION:
            printf("%s (%s):\n", BODY_TYPES_NAMES[program->type], ((function*)program)->name);
            break;
        case EXPRESSION:
            ex = (expression*)program;
            printf("%s \'%s\':\n", BODY_TYPES_NAMES[program->type], TOKEN_SYMBOLS[ex->op]);
            if (ex->child2 != NULL)
                print_program(ex->child2, depth + 1);
            break;
        case CONSTANT:
            printf("%s: %i\n", BODY_TYPES_NAMES[program->type], ((constant*)program)->value);
            break;
        case UNARY_OPS:
            unOp = (unaryOps*)program;
            printf("%s \'%s\':\n", BODY_TYPES_NAMES[program->type], TOKEN_SYMBOLS[unOp->op]);
            break;
        case DECLARE:
            var = (variable*)program;
            printf("%s: %s(%i)\n", BODY_TYPES_NAMES[program->type], var->name, var->offset);
            break;
        case ASSIGN:
            var = (variable*)program;
            printf("%s: %s(%i)\n", BODY_TYPES_NAMES[program->type], var->name, var->offset);
            break;
        case VARIABLE:
            var = (variable*)program;
            printf("%s: %s(%i)\n", BODY_TYPES_NAMES[program->type], var->name, var->offset);
            break;
        case IF:
            _if = (if_body*)program;
            printf("%s%s-CONDITION: \n", space, BODY_TYPES_NAMES[program->type]);
            print_program(_if->condition, depth + 2);
            for (int i = 0; i < depth + 1; i++)
                printf("%s", "  ");
            printf("TRUE:\n");
            break;
        case WHILE:
            _while = (while_body*)program;
            printf("%s%s-CONDITION: \n", space, BODY_TYPES_NAMES[program->type]);
            print_program(_while->condition, depth + 2);
            break;
        case FOR:
            _for = (for_body*)program;
            printf("%s%s-CONDITION: \n", space, BODY_TYPES_NAMES[program->type]);
            print_program(_for->condition, depth + 2);
            print_program(_for->init, depth + 2);
            print_program(_for->expr, depth + 2);
            break;
        default:
            if (program->type != LIST)
                printf("%s:\n", BODY_TYPES_NAMES[program->type]);
            break;

    }
    print_program(program->child, depth + 1);

    if (program->type == LIST) {
        current = (list*)program;
        print_program((body*)current->next, depth);
    }

    if (program->type == IF) {
        _if = (if_body*)program;
        if (_if->else_child == NULL)
            return;
        for (int i = 0; i < depth + 1; i++)
            printf("%s", "  ");
        printf("FALSE:\n");
        print_program((body*)_if->else_child, depth + 1);
    }
}