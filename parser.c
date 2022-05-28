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

void cpy_str_arry(char** dest, char** src, int length) {
    for (int i = 0; i < length; i++) {
        int str_length = strlen(src[i]);
        dest[i] = (char*)calloc(sizeof(char), str_length + 1);
        strcpy(dest[i], src[i]);
    }
}

context* copy_context(context* context) {
    struct context* new = (struct context*)calloc(1, sizeof(struct context));
    new->type = CONTEXT;
    if (context == NULL) {
        return new;        
    }
    new->stack_offset = context->stack_offset;
    new->labels_count = context->labels_count;
    int length = (context->stack_offset/OFFSET_SIZE);
    if (length > 0) {
        new->variables = (char**)calloc(sizeof(char*), length);
        cpy_str_arry(new->variables, context->variables, length);
    }
    int func_length = context->functions_length;
    if (func_length > 0) {
        new->functions_length = context->functions_length;
        new->function_names = (char**)calloc(sizeof(char*), func_length);
        cpy_str_arry(new->function_names, context->function_names, func_length);
        new->function_params = (int*)calloc(sizeof(int), func_length);
        memcpy(new->function_params, context->function_params, sizeof(int) * func_length);
    }
    return new;
}

void append_variable(context* context, char* name) {
    context->stack_offset += OFFSET_SIZE;
    int stack_length = (context->stack_offset/OFFSET_SIZE);
    int name_length = strlen(name);
    context->variables = (char**)realloc(context->variables, sizeof(char*) * stack_length);
    context->variables[stack_length - 1] = (char*)calloc(name_length + 1, sizeof(char));
    strcpy(context->variables[stack_length - 1], name);
}

int get_offset(context* context, char* name) {
    int stack_length = (context->stack_offset/OFFSET_SIZE);
    for (int i = 0; i < stack_length; i++) 
        if (strcmp(name, context->variables[i]) == 0)
            return (i + 1) * OFFSET_SIZE;
    return -1;
}

void append_function(context* context, char* name, int argc) {
    int length = ++context->functions_length;
    context->function_names = (char**)realloc(context->function_names, sizeof(char*) * length);
    context->function_params = (int*)realloc(context->function_params, sizeof(int) * length);
    context->function_params[length - 1] = argc;

    int str_length = strlen(name);
    context->function_names[length - 1] = (char*)calloc(sizeof(char), str_length + 1);
    strcpy(context->function_names[length - 1], name);
}

int get_argc(context* context, char* name) {
    for (int i = 0; i < context->functions_length; i++) {
        if (strcmp(context->function_names[i], name) == 0)
            return context->function_params[i];
    }
    return -1;
}

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
        lexer_next(lexer, &current);
        unaryOps* new = (unaryOps*)malloc(sizeof(struct unaryOps));
        new->type = UNARY_OPS;
        new->op = current.token_type;
        new->child = NULL;
        *factor = (body*)new;
        parse_factor(lexer, &new->child, context);

        if (new->child == NULL) 
            fail_error("missing factor");
        return;
    }

    if (current.token_type == INT_LITERAL) {
        lexer_next(lexer, &current);
        constant* cons = (constant*)malloc(sizeof(struct constant));
        cons->type = CONSTANT;
        cons->value = atoi(current.value);
        cons->child = NULL;
        *factor = (body*)cons;
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

        variable* var = (variable*)malloc(sizeof(struct variable));
        var->child = NULL;
        var->type = VARIABLE;
        var->offset = get_offset(context, current.value);
        strcpy(var->name, current.value);
        *factor = (body*)var;
        return;
    }

    *factor = NULL;
}

bool contains_token(TOKENS* tokens, int tokens_length, TOKENS token) {
    for (int i = 0; i < tokens_length; i++)
        if (tokens[i] == token)
            return true;
    return false;
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

        new = (expression*)malloc(sizeof(struct expression));
        new->type = EXPRESSION;
        new->op = current.token_type;
        new->child = *body;
        new->child2 = NULL;
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

void parse_assignment(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, IDENTIFIER);

    if (get_offset(context, current.value) == -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable %s not declared previously!", current.value);
        fail_error(error_msg);
    }

    // TODO: parse assignment
    variable* new = (variable*)malloc(sizeof(struct variable));
    new->type = ASSIGN;
    new->child = NULL;
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
    variable* add = (variable*)malloc(sizeof(struct variable));
    add->type = VARIABLE;
    add->child = NULL;
    add->offset = get_offset(context, body->name);
    strcpy(add->name, body->name);

    expression* expr = (expression*)malloc(sizeof(struct expression));
    expr->type = EXPRESSION;
    expr->op = op;
    expr->child2 = (struct body*)add;
    body->child = (struct body*)expr;

    parse_expression(lexer, &expr->child, context);
}

void parse_call(lexer* lexer, body** body, context* context, token* prev) {
    token current;
    
    int argc = get_argc(context, prev->value);

    if (argc == -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Function (%s) not defined!", prev->value);
        fail_error(error_msg);
    }

    function_call* call = (function_call*)malloc(sizeof(struct function_call));
    call->type = CALL;
    strcpy(call->name, prev->value);
    call->argc = 0;
    call->child = NULL;
    call->expressions = NULL;

    check_valid(lexer, &current, O_PARENTHESIS);

    lexer_peak(lexer, &current);

    while(current.token_type != C_PARENTHESIS) {
        call->argc++;
        call->expressions = (struct body**)realloc(call->expressions, sizeof(struct body*) * call->argc);
        parse_expression(lexer, &call->expressions[call->argc - 1], context);

        lexer_peak(lexer, &current);
        if(current.token_type == COMMA) {
            lexer_next(lexer, &current);
        }
    }

    if (call->argc != argc) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Wrong amount of arguments! Expected: %i Got: %i", argc, call->argc);
        fail_error(error_msg);
    }

    check_valid(lexer, &current, C_PARENTHESIS);

    *body = (struct body*)call;
}

void parse_expression(lexer* lexer, body** expr, context* context) {
    token current;
    lexer_peak(lexer, &current);

    if (current.token_type == IDENTIFIER) {

        //i know this is not fancy :)
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

void parse_return(lexer* lexer, body** b, context* context) {
    body* new = (body*)malloc(sizeof(struct body));
    new->type = RETURN;
    new->child = NULL;
    *b = new;

    token current;

    check_valid(lexer, &current, RETURN_KEYWORD);

    parse_expression(lexer, &new->child, context);

    if (new->child == NULL) 
        fail_error("missing expression");
    

    check_valid(lexer, &current, SEMICOLON);
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

    variable* new = (variable*)malloc(sizeof(struct variable));
    new->child = NULL;
    new->type = DECLARE;
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

void parse_if(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, IF_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    if_body* _if = (if_body*)malloc(sizeof(struct if_body));
    _if->type = IF;
    _if->child = NULL;
    _if->else_child = NULL;

    parse_expression(lexer, &_if->condition, context);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &_if->child, context);

    *body = (struct body*)_if;

    parse_else(lexer, (struct body**)&_if, context);
}

void parse_while(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, WHILE_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    while_body* _while = (while_body*)malloc(sizeof(struct while_body));
    _while->type = WHILE;
    _while->child = NULL;
    _while->condition = NULL;
    _while->label = ++label_count;

    struct context* new_context = copy_context(context);
    new_context->labels_count++;
    _while->child = (struct body*)new_context;

    parse_expression(lexer, &_while->condition, new_context);

    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_context->child, new_context);

    *body = (struct body*)_while;
}

void parse_for(lexer* lexer, body** body, context* context) {
    token current;

    check_valid(lexer, &current, FOR_KEYWORD);
    check_valid(lexer, &current, O_PARENTHESIS);
    
    for_body* _for = (for_body*)malloc(sizeof(struct for_body));
    _for->type = FOR;
    _for->child = NULL;
    _for->expr = NULL;
    _for->condition = NULL;
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
        struct constant* cons = (struct constant*)malloc(sizeof(struct constant));
        cons->value = 1;
        cons->child = NULL;
        cons->type = CONSTANT;
        _for->condition = (struct body*)cons;
    }
    
    parse_expression(lexer, &_for->expr, new_context);
    check_valid(lexer, &current, C_PARENTHESIS);

    parse_list(lexer, &new_context->child, new_context);

    *body = (struct body*)_for;
}

void parse_continue(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, CONTINUE_KEYWORD);

    if (context->labels_count == 0) {
        fail_error("Can't continue without a loop!");
    }

    statement* state = (statement*)malloc(sizeof(struct statement));
    state->label = label_count;
    state->child = NULL;
    state->type = CONTINUE;
    *body = (struct body*)state;

    check_valid(lexer, &current, SEMICOLON);
}

void parse_break(lexer* lexer, body** body, context* context) {
    token current;
    check_valid(lexer, &current, BREAK_KEYWORD);

    if (context->labels_count == 0) {
        fail_error("Can't break without a loop!");
    }

    statement* state = (statement*)malloc(sizeof(struct statement));
    state->label = label_count;
    state->child = NULL;
    state->type = BREAK;
    *body = (struct body*)state;

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

void parse_parameter(lexer* lexer, context* context) {
    token current;
    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    if (get_offset(context, current.value) != -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "Variable: (%s) already declared!", current.value);
        fail_error(error_msg);
    }

    // only add the variable to the context list of variables
    append_variable(context, current.value);
}

void parse_function(lexer* lexer, function** func, context* context) {
    function* new = (function*)malloc(sizeof(struct function));
    new->type = FUNCTION;
    new->argc = 0;
    *func = new;

    token current;

    check_valid(lexer, &current, INT_KEYWORD);

    check_valid(lexer, &current, IDENTIFIER);

    // copy the function name into the AST 
    strcpy(new->name, current.value);

    check_valid(lexer, &current, O_PARENTHESIS);

    struct context* new_context = copy_context(context);
    new_context->type = CONTEXT;
    new->child = (struct body*)new_context;
    
    lexer_peak(lexer, &current);

    while(current.token_type == INT_KEYWORD) {
        new->argc++;
        if (new->argc > 6) {
            fail_error("No functions supported with more than 6 arguments!");
        }
        parse_parameter(lexer, new_context);

        lexer_peak(lexer, &current);
        if (current.token_type == COMMA) {
            lexer_next(lexer, &current);
            lexer_peak(lexer, &current);
        }
    }

    new_context->stack_offset = OFFSET_SIZE * new->argc;

    check_valid(lexer, &current, C_PARENTHESIS);
    lexer_peak(lexer, &current);

    int context_argc = get_argc(context, new->name);
    if (context_argc != new->argc && context_argc != -1) {
        char error_msg[INPUT_SIZE * 2];
        sprintf(error_msg, "new definition of function with same name: (%s)", new->name);
        fail_error(error_msg);
    }

    append_function(context, new->name, new->argc);

    if (current.token_type == SEMICOLON) {
        new->child = NULL;
        parse_free((body*)new_context);
        lexer_next(lexer, &current);
        return;
    }

    append_function(new_context, new->name, new->argc);

    parse_list(lexer, &new_context->child, new_context);
}

void parse_list(lexer* lexer, body** body, context* _context) {
    token current;

    context* new_context = copy_context(_context);
    list* l = (list*)calloc(1, sizeof(struct list));
    list *next = NULL;
    list *prev = NULL;
    l->type = LIST;
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

void parse_program(lexer* lexer, body** prog) {
    program* new = (program*)malloc(sizeof(struct program));
    new->type = PROGRAM;
    new->func_length = 0;
    new->funcs = NULL;
    new->child = NULL;
    *prog = (body*)new;
    struct context* context = (struct context*)calloc(sizeof(struct context), 1);
    context->type = CONTEXT;
    
    token current;

    do {
        new->func_length++;
        new->funcs = (function**)realloc(new->funcs, sizeof(function*) * new->func_length);

        parse_function(lexer, &new->funcs[new->func_length - 1], context);

        lexer_peak(lexer, &current);
    } while (current.token_type != EOF_TOKEN);

    parse_free((struct body*)context);
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
        parse_free((body*)l->next);
    }
    if (program->type == CONTEXT) {
        context* c = (context*)program;
        //TODO free everything
        for (int i = 0; i < c->stack_offset/OFFSET_SIZE; i++) {
            free(c->variables[i]);
        }

        if (c->stack_offset != 0)
            free(c->variables);

        for (int i = 0; i < c->functions_length; i++) {
            free(c->function_names[i]);
        }
        if (c->functions_length != 0) {
            free(c->function_names);
            free(c->function_params);
        }
    }
    if (program->type == CALL) {
        function_call* call = (function_call*)program;
        for (int i = 0; i < call->argc; i++) {
            parse_free(call->expressions[i]);
        }
        if (call->argc > 0)
            free(call->expressions);
    }
    if (program->type == FOR) {
        for_body* f = (for_body*)program;
        parse_free((body*)f->condition);
        parse_free((body*)f->init);
        parse_free((body*)f->expr);
    }
    if (program->type == PROGRAM) {
        struct program* prog = (struct program*)program;
        for (int i = 0; i < prog->func_length; i++) {
            parse_free((struct body*)prog->funcs[i]);
        }
        if (prog->func_length > 0)
            free(prog->funcs);
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
    function_call* call;
    struct program* prog;
    switch(program->type) {
        case PROGRAM:
            prog = (struct program*)program;
            for (int i = 0; i < prog->func_length; i++)
                print_program((body*)prog->funcs[i], 0);
            return;
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
        case CALL:
            call = (function_call*)program;
            printf("%s (%s):\n", BODY_TYPES_NAMES[program->type], call->name);
            for (int i = 0; i < call->argc; i++) {
                print_program(call->expressions[i], depth + 1);    
            }
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