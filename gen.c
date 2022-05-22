#include "gen.h"

void compile(char* input_file, char* output_file) {

    lexer* lexer = lexer_open(input_file);
    
    body* program;

    parse_program(lexer, &program);

    print_program(program, 0);

    FILE* fp = fopen(output_file, "w");

    gen_code(fp, program);

    fclose(fp);

    lexer_free(lexer);
    parse_free(program);
}

void gen_code(FILE* fp, body* body) {
    if (body == NULL) return;
    gens[body->type](fp, body);
}

void gen_unaryops(FILE* fp, body* body) {
    gen_code(fp, body->child);
    unaryOps* unOp = (unaryOps*)body;
    char* template = "\tcmpl\t$0, %%eax\n\tmovl\t$0, %%eax\n\tsete\t%%al\n";
    switch (unOp->operator) {
        case '-':
            fprintf(fp, "%s", "\tneg\t\t%eax\n");
            break;
        case '~':
            fprintf(fp, "%s", "\tnot\t%eax\n");
            break;
        case '!':
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
    function* func = (function*)body;
    const char* template = "%s:\n\tpush\t%%rbp\n\tmov\t\t%%rsp, %%rbp\n";
    fprintf(fp, template, func->name);

    gen_code(fp, func->child);
    
    list* current = (list*)func->child;

    while(current && current->next && current->next->child) {
        current = current->next;
    }

    if (current == NULL || (current->child == NULL || current->child->type != RETURN)) {
        const char* template_return = "\tmov\t\t%rbp, %rsp\n\tpop\t\t%rbp\n\tmov\t\t$0, %rax\n\tret\n";
        fprintf(fp, "%s", template_return);
    }
}

void gen_program(FILE* fp, body* body) {
    const char* template = "\t.globl\tmain\n";
    fprintf(fp, "%s", template);

    gen_code(fp, body->child);
}

void gen_expression(FILE* fp, body* body) {
    expression* exp = (expression*)body;

    if (exp->child2 == NULL) {
        gen_code(fp, exp->child);
        return;
    }

    gen_code(fp, exp->child);

    char child[24], end[24];
    
    if (strcmp(exp->op, "||") == 0) {
        const char* or_tamplate_1 = "\tcmpl\t$0, %%eax\n\tje\t\t%s\n\tmovl\t$1, %%eax\n\tjmp\t\t%s\n";
        get_clause_names(child, end);

        fprintf(fp, or_tamplate_1, child, end);
        fprintf(fp, "%s:\n", child);

        gen_code(fp, exp->child2);

        fprintf(fp, "%s", "\tcmpl\t$0, %eax\n\tmovl\t$1, %eax\n\tsetne\t%al\n");
        fprintf(fp, "%s:\n", end);

        return;
    }

    if (strcmp(exp->op, "&&") == 0) {
        const char* and_tamplate_1 = "\tcmpl\t$0, %%eax\n\tjne\t\t%s\n\tjmp\t\t%s\n";
        get_clause_names(child, end);

        fprintf(fp, and_tamplate_1, child, end);
        fprintf(fp, "%s:\n", child);

        gen_code(fp, exp->child2);

        fprintf(fp, "%s", "\tcmpl\t$0, %eax\n\tmovl\t$0, %eax\n\tsetne\t%al\n");
        fprintf(fp, "%s:\n", end);

        return;
    }


    fprintf(fp, "%s", "\tpush\t%rax\n");

    gen_code(fp, exp->child2);

    fprintf(fp, "%s", "\tpop\t\t%rcx\n");

    if (strcmp(exp->op, "+") == 0)
        fprintf(fp, "%s", "\taddl\t%ecx, %eax\n");
    if (strcmp(exp->op, "*") == 0)
        fprintf(fp, "%s", "\timul\t%ecx, %eax\n");
    if (strcmp(exp->op, "-") == 0)
        fprintf(fp, "%s", "\tsubl\t%eax, %ecx\n\tmovl\t%ecx, %eax\n");
    if (strcmp(exp->op, "/") == 0)
        fprintf(fp, "%s", "\tmovl\t%eax, %ebx\n\tmovl\t%ecx, %eax\n\tcdq\n\tidivl\t%ebx\n");
    const char* c_template = "\tcmpl\t%%eax, %%ecx\n\tmovl\t$0, %%eax\n\t%s\t%%al\n";
    if (strcmp(exp->op, "==") == 0)
        fprintf(fp, c_template, "sete");
    if (strcmp(exp->op, "!=") == 0)
        fprintf(fp, c_template, "setne");
    if (strcmp(exp->op, ">") == 0)
        fprintf(fp, c_template, "setg");
    if (strcmp(exp->op, "<") == 0)
        fprintf(fp, c_template, "setl");
    if (strcmp(exp->op, ">=") == 0)
        fprintf(fp, c_template, "setge");
    if (strcmp(exp->op, "<=") == 0)
        fprintf(fp, c_template, "setle");
}

void get_clause_names(char* child, char* end) {
    sprintf(child, "_clause%i", clause_count);
    sprintf(end, "_end%i", clause_count);
    clause_count++;
}

void get_if_names(char* child, char* child2, char* end) {
    sprintf(child, "_clause%i", clause_count);
    sprintf(child2, "_eclause%i", clause_count);
    sprintf(end, "_end%i", clause_count);
    clause_count++;
}

void gen_list(FILE* fp, body* b) {
    if (b == NULL || b->child == NULL) return;
    list* l = (list*)b;
    gen_code(fp, l->child);
    gen_list(fp, (body*)l->next);

    const char* template = "\tadd\t\t$%i, %%rsp\n";
    if (l->stack_offset_dif != 0)
        fprintf(fp, template, l->stack_offset_dif);
}

void gen_assign(FILE* fp, body* body) {
    gen_code(fp, body->child);
    assign* ass = (assign*)body;
    const char* template = "\tmov\t\t%%eax,-%i(%%rbp)\n";
    fprintf(fp, template, ass->offset);
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
    get_clause_names(child, end);

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

    char child[24], end[24];
    get_clause_names(child, end);

    gen_code(fp, _for->init);

    fprintf(fp, "%s:\n", child);

    gen_code(fp, _for->condition);

    const char* template = "\tcmpl\t$0, %%eax\n\tje\t\t%s\n";
    fprintf(fp, template, end);

    gen_code(fp, _for->child);
    gen_code(fp, _for->expr);

    fprintf(fp, "\tjmp\t\t%s\n", child);
    fprintf(fp, "%s:\n", end);

}