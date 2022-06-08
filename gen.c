#include "header/gen.h"


void compile(char* input_file, char* output_file) {

    lexer* lexer = lexer_open(input_file);
    
    body* program;

    parse_program(lexer, &program);

    PRINT_BODY(program, 0);

    FILE* fp = fopen(output_file, "w");

    gen_code(fp, program);

    fclose(fp);

    lexer_free(lexer);

    FREE_BODY(program);
}

void gen_code(FILE* fp, body* body) {
    if (body == NULL) return;
    gens[body->type](fp, body);
}

void gen_unaryops(FILE* fp, body* body) {
    gen_code(fp, body->child);
    unaryOps* unOp = (unaryOps*)body;
    char* template = "\tcmpl\t$0, %eax\n\tmovl\t$0, %eax\n\tsete\t%al\n";
    switch (unOp->op) {
        case MINUS:
            fprintf(fp, "%s", "\tneg\t\t%eax\n");
            break;
        case BITWISE_COMPLEMENT:
            fprintf(fp, "%s", "\tnot\t%eax\n");
            break;
        case LOGICAL_NEGATION:
            fprintf(fp, "%s", template);
            break;
    }
}

void gen_constant(FILE* fp, body* body) {
    constant* cons = (constant*)body;
    const char* template = "\tmovl\t$%i, %%eax\n";
    fprintf(fp, template, cons->value);
}

void gen_return(FILE* fp, body* body) {
    gen_code(fp, body->child);
    const char* template = "\tmov\t\t%rbp, %rsp\n\tpop\t\t%rbp\n\tret\n";
    fprintf(fp, "%s", template);
}

void gen_function(FILE* fp, body* body) {
    if (body->child == NULL) return;

    function* func = (function*)body;

    const char* template = "%s:\n\tpushq\t%%rbp\n\tmovq\t%%rsp, %%rbp\n";
    fprintf(fp, template, func->name);

    //TODO: convert passed argument to local variables
    //NOTE: we only handle 6 arguments, because that makes it way easier
    const char* regs[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    context* c = (context*)func->child;
    for (int i = 0; i < c->stack_offset/OFFSET_SIZE; i++)
        fprintf(fp, "\tmovl\t%%%s, %%eax\n\tpush\t%%rax\n", regs[i]);

    gen_code(fp, func->child);
}

void gen_program(FILE* fp, body* body) {
    const char* template = "\t.globl\tmain\n";
    fprintf(fp, "%s", template);
    program* prog = (program*)body;
    gen_code(fp, body->child);

    for (int i = 0; i < prog->func_length; i++) {
        gen_code(fp, (struct body*)prog->funcs[i]);
    }
}

void gen_expression(FILE* fp, body* body) {
    expression* exp = (expression*)body;

    if (exp->child2 == NULL) {
        gen_code(fp, exp->child);
        return;
    }

    gen_code(fp, exp->child);

    char child[24], end[24];
    
    // gen AND & OR expressions with short circuit
    if (exp->op == OR || exp->op == AND) {
        get_clause_names(child, end);

        if (exp->op == AND) {
            fprintf(fp, "\tcmpl\t$0, %%eax\n\tjne\t\t%s\n\tjmp\t\t%s\n", child, end);
        } else {
            fprintf(fp, "\tcmpl\t$0, %%eax\n\tje\t\t%s\n\tmovl\t$1, %%eax\n\tjmp\t\t%s\n", child, end);
        }

        fprintf(fp, "%s:\n", child);

        gen_code(fp, exp->child2);

        fprintf(fp, "%s", "\tcmpl\t$0, %eax\n\tmovl\t$1, %eax\n\tsetne\t%al\n");
        fprintf(fp, "%s:\n", end);

        return;
    }


    fprintf(fp, "%s", "\tpush\t%rax\n");

    gen_code(fp, exp->child2);

    fprintf(fp, "%s", "\tpop\t\t%rcx\n");

    const char* c_template = "\tcmpl\t%%eax, %%ecx\n\tmovl\t$0, %%eax\n\t%s\t%%al\n";

    switch (exp->op) {
        case ADDITION:
            fprintf(fp, "%s", "\taddl\t%ecx, %eax\n");
            break;
        case MULTIPLICATION:
            fprintf(fp, "%s", "\timul\t%ecx, %eax\n");
            break;
        case MINUS:
            fprintf(fp, "%s", "\tsubl\t%eax, %ecx\n\tmovl\t%ecx, %eax\n");
            break;
        case DIVISION:
            fprintf(fp, "%s", "\tmovl\t%eax, %ebx\n\tmovl\t%ecx, %eax\n\tcdq\n\tidivl\t%ebx\n");
            break;
        case MODULO:
            fprintf(fp, "%s", "\tmovl\t%eax, %ebx\n\tmovl\t%ecx, %eax\n\tcdq\n\tidivl\t%ebx\n\tmovl\t%edx, %eax\n");
            break;
        case EQUAL:
            fprintf(fp, c_template, "sete");
            break;
        case N_EQUAL:
            fprintf(fp, c_template, "setne");
            break;
        case GREATER_THAN:
            fprintf(fp, c_template, "setg");
            break;
        case LESS_THAN:
            fprintf(fp, c_template, "setl");
            break;
        case GREATER_THAN_OR:
            fprintf(fp, c_template, "setge");
            break;
        case LESS_THAN_OR:
            fprintf(fp, c_template, "setle");
            break;
    }
}

void get_clause_names(char* child, char* end) {
    sprintf(child, "_clause%i", clause_count);
    sprintf(end, "_end%i", clause_count);
    clause_count++;
}

void get_label_names(char* child, char* end, int label) {
    sprintf(child, "_label%i", label);
    sprintf(end, "_lend%i", label);
}


void gen_list(FILE* fp, body* b) {
    if (b == NULL || b->child == NULL) return;
    list* l = (list*)b;
    gen_code(fp, l->child);
    gen_list(fp, (body*)l->next);
}

void gen_context(FILE* fp, body* b) {
    if (b == NULL || b->child == NULL) return;
    context* c = (context*)b;
    gen_code(fp, c->child);

    const char* template = "\tadd\t\t$%i, %%rsp\n";
    
    if (c->stack_offset_dif != 0) {
        fprintf(fp, template, c->stack_offset_dif);
    }
}

void gen_assign(FILE* fp, body* body) {
    gen_code(fp, body->child);
    variable* var = (variable*)body;
    const char* template = "\tmov\t\t%%eax,-%i(%%rbp)\n";
    fprintf(fp, template, var->offset);
}

void gen_declare(FILE* fp, body* body) {
    if (body->child != NULL) {
        gen_code(fp, body->child);
    }
    const char* template = "\tpush\t%rax\n";
    fprintf(fp, "%s", template);
}

void gen_variable(FILE* fp, body* body) {
    variable* var = (variable*)body;
    const char* template = "\tmov\t\t-%i(%%rbp), %%eax\n";
    fprintf(fp, template, var->offset);
}

void gen_if(FILE* fp, body* body) {

    if_body* _if = (if_body*)body;
    
    gen_code(fp, _if->condition);
    char child[24], end[24];
    get_clause_names(child, end);

    const char* template = "\tcmpl\t$0, %%eax\n\tje\t\t%s\n";

    fprintf(fp, template, child);

    gen_code(fp, _if->child);

    fprintf(fp, "\tjmp\t\t%s\n", end);
    fprintf(fp, "%s:\n", child);
    gen_code(fp, _if->else_child);

    fprintf(fp, "%s:\n", end);
}

void gen_while(FILE* fp, body* body) {
    while_body* _while = (while_body*)body;

    char child[24], end[24];
    get_label_names(child, end, _while->label);
    sprintf(child, "_expr%i", _while->label);

    fprintf(fp, "%s:\n", child);

    gen_code(fp, _while->condition);

    const char* template = "\tcmpl\t$0, %%eax\n\tje\t\t%s\n";
    fprintf(fp, template, end);

    gen_code(fp, _while->child);

    fprintf(fp, "\tjmp\t\t%s\n", child);
    fprintf(fp, "%s:\n", end);
}

void gen_for(FILE* fp, body* body) {
    for_body* _for = (for_body*)body;

    char child[24], end[24], expr[24];
    sprintf(expr, "_expr%i", _for->label);
    get_label_names(child, end, _for->label);

    gen_code(fp, _for->init);

    fprintf(fp, "%s:\n", child);

    gen_code(fp, _for->condition);

    const char* template = "\tcmpl\t$0, %%eax\n\tje\t\t%s\n";
    fprintf(fp, template, end);

    gen_code(fp, _for->child);

    fprintf(fp, "%s:\n", expr);
    gen_code(fp, _for->expr);

    fprintf(fp, "\tjmp\t\t%s\n", child);
    fprintf(fp, "%s:\n", end);

    // remove the init variable
    fprintf(fp, "\tadd\t\t$%i, %%rsp\n", 8);
}

void gen_statement(FILE* fp, body* body) {
    statement* state = (statement*)body;
    const char* template = "\tjmp\t\t%s\n";

    char child[24], end[24], expr[24];

    sprintf(expr, "_expr%i", state->label);
    get_label_names(child, end, state->label);

    switch(state->type) {
        case CONTINUE:
            fprintf(fp, template, expr);
            break;
        case BREAK:
            fprintf(fp, template, end);
            break;
    }
}

void gen_call(FILE* fp, body* body) {

    // NOTE: we will only support 6 arguments for our functions due to simplicity :)
    // TODO: implement full x86-64 System V user-space function calling conventions

    const char* regs[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

    function_call* call = (function_call*)body;

    for (int i = 0; i < call->argc; i++) {
        const char* expr_template = "\tmov\t\t%%eax, %%%s\n";
        gen_code(fp, call->expressions[i]);
        fprintf(fp, expr_template, regs[i]);    
    }

    fprintf(fp, "\tcall\t%s\n", call->name);
}