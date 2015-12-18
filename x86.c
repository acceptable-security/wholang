#include "parser.h"
#include <string.h>
#include <stdio.h>
#include "x86.h"

x86_arg_t* x86_reg(x86_register_t reg) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_reg;

    x86_reg_t* r = (x86_reg_t*) malloc(sizeof(x86_reg_t));
    r->reg = reg;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_deref_reg(x86_register_t reg) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_deref_reg;

    x86_deref_reg_t* r = (x86_deref_reg_t*) malloc(sizeof(x86_deref_reg_t));
    r->reg = reg;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_offset_reg(x86_register_t reg, int offset) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_reg_off;

    x86_reg_off_t* r = (x86_reg_off_t*) malloc(sizeof(x86_reg_off_t));
    r->reg = reg;
    r->amnt = offset;
    arg->data = r;

    return arg;
}

x86_arg_t* x86_number(int number) {
    x86_arg_t* arg = (x86_arg_t*) malloc(sizeof(x86_arg_t));
    arg->type = arg_number;

    x86_number_t* r = (x86_number_t*) malloc(sizeof(x86_number_t));
    r->number = number;
    arg->data = r;

    return arg;
}

x86_state_t* x86_init() {
    x86_state_t* cmp = (x86_state_t*) malloc(sizeof(x86_state_t));

    cmp->labels = INIT_LIST(x86_label_t);
    cmp->label_cnt = 0;
    cmp->label_alloc = MALLOC_CHUNK;

    cmp->variables = INIT_LIST(x86_variable_t);
    cmp->variable_cnt = 0;
    cmp->variable_alloc = 0;

    cmp->cur_label = NULL;
    cmp->curr_off = 0;

    return cmp;
}

void x86_command_debug(x86_command_t* cmd) {
    const char* ops[] = {
        "PUSH",
        "PUSHL",
        "POP",
        "POPL",
        "ADD",
        "ADDL",
        "SUB",
        "SUBL",
        "MUL",
        "IMUL",
        "LEA",
        "LEAL",
        "MOV",
        "MOVL",
        "NEGL",
        "NEG",
        "SHL",
        "SHR",
        "AND",
        "ANDL",
        "OR",
        "ORL",
        "XOR",
        "XORL",
        "RET"
    };

    const char* regs[] = {
        "EAX",
        "ECX",
        "EDX",
        "EBX",
        "ESP",
        "EBP",
        "ESI",
        "EDI"
    };

    printf("%s ", ops[cmd->inst]);

    if ( cmd->arg1 != NULL ) {
        if ( cmd->arg1->type == arg_reg ) {
            printf("%%%s", regs[((x86_reg_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_deref_reg ) {
            printf("[%%%s]", regs[((x86_deref_reg_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_reg_off ) {
            printf("%d(%%%s)", ((x86_reg_off_t*) cmd->arg1->data)->amnt, regs[((x86_reg_off_t*) cmd->arg1->data)->reg]);
        }
        else if ( cmd->arg1->type == arg_number ) {
            printf("$%d", ((x86_number_t*) cmd->arg1->data)->number);
        }

        if ( cmd->arg2 != NULL ) {
            printf(", ");

            if ( cmd->arg2->type == arg_reg ) {
                printf("%%%s", regs[((x86_reg_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_deref_reg ) {
                printf("[%%%s]", regs[((x86_deref_reg_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_reg_off ) {
                printf("%d(%%%s)", ((x86_reg_off_t*) cmd->arg2->data)->amnt, regs[((x86_reg_off_t*) cmd->arg2->data)->reg]);
            }
            else if ( cmd->arg2->type == arg_number ) {
                printf("$%d", ((x86_number_t*) cmd->arg2->data)->number);
            }
        }
    }

    printf("\n");
}

void x86_label_debug(x86_label_t* label) {
    printf("%s:\n", label->name);


    for ( int i = 0; i < label->cmd_cnt; i++ ) {
        x86_command_debug(label->cmds[i]);
    }
}

void x86_clean(x86_state_t* cmp) {
    if ( cmp->labels != NULL ) {
        for ( int i = 0; i < cmp->label_cnt; i++ ) {
            x86_label_t* label = cmp->labels[i];

            if ( label == NULL ) continue;

            for ( int j = 0; j < label->cmd_cnt; j++ ) {
                x86_command_t* cmd = label->cmds[j];
                if ( cmd->arg1 ) {
                    free(cmd->arg1->data);
                    free(cmd->arg1);
                }
                if ( cmd->arg2 ) {
                    free(cmd->arg2->data);
                    free(cmd->arg2);
                }
                free(cmd);
            }
            free(label->cmds);
            free((void*) label->name);
            free(label);
        }
        free(cmp->labels);
    }

    if ( cmp->cur_label != NULL ) {
        for ( int i = 0; i < cmp->cur_label->cmd_cnt; i++ ) {
            x86_command_t* cmd = cmp->cur_label->cmds[i];
            if ( cmd->arg1 ) {
                free(cmd->arg1->data);
                free(cmd->arg1);
            }
            if ( cmd->arg2 ) {
                free(cmd->arg2->data);
                free(cmd->arg2);
            }
            free(cmd);
        }
        free(cmp->cur_label->cmds);
        free((void*) cmp->cur_label->name);
        free(cmp->cur_label);
    }

    if ( cmp->variables != NULL ) {
        for ( int i = 0; i < cmp->variable_cnt; i++ ) {
            x86_variable_t* var = cmp->variables[i];

            if ( var == NULL ) continue;

            free((void*) var->name);
            free(var);
        }

        free(cmp->variables);
    }

    free(cmp);
}

int x86_find_label(x86_state_t* cmp, const char* label) {
    for ( int i = 0; i < cmp->label_cnt; i++ ) {
        if ( cmp->labels[i] != NULL && strcmp(cmp->labels[i]->name, label) == 0 ) {
            return i;
        }
    }

    return -1;
}

int x86_sizeof(const char* type) {
    if ( strcmp(type, "uint8_t") == 0 || strcmp(type, "char") == 0 ) {
        return 1;
    }
    else if ( strcmp(type, "int8_t") == 0 ) {
        return 1;
    }
    else if ( strcmp(type, "uint16_t") == 0 || strcmp(type, "short") == 0 ) {
        return 2;
    }
    else if ( strcmp(type, "int16_t") == 0 ) {
        return 2;
    }
    else if ( strcmp(type, "uint32_t") == 0 || strcmp(type, "int") == 0 ) {
        return 4;
    }
    else if ( strcmp(type, "int32_t") == 0 || strcmp(type, "uint") == 0 ) {
        return 4;
    }
    else if ( strcmp(type, "uint64_t") == 0 || strcmp(type, "long") == 0 ) {
        return 8;
    }
    else if ( strcmp(type, "int64_t") == 0 || strcmp(type, "ulong") == 0 ) {
        return 8;
    }
    else if ( strcmp(type, "float") == 0 ) {
        return 4;
    }
    else if ( strcmp(type, "double") == 0 ) {
        return 8;
    }
    else {
        return 0;
    }
}

x86_variable_t* x86_find_variable(x86_state_t* cmp, const char* name) {
    for ( int i = 0; i < cmp->variable_cnt; i++ ) {
        if ( strcmp(cmp->variables[i]->name, name) == 0 ) {
            return cmp->variables[i];
        }
    }

    return NULL;
}

x86_type_t* x86_compile_expression(x86_state_t* cmp, expr_t* expr) {
    x86_label_t* label = cmp->cur_label;

    if ( expr->type == expr_prefix ) {
        expr_prefix_t* prfx = expr->value;
        x86_type_t* type = x86_compile_expression(cmp, prfx->lhs);
        ASM(POP, x86_reg(EAX), NULL)

        if ( prfx->op.token_val == NEG_OP ) {
            ASM(NEGL, x86_reg(EAX), NULL)
        }
        else if ( prfx->op.token_val == OPP_OP ) {
            // THIS IS HARD
        }
        else if ( prfx->op.token_val == DEREF_OP ) {
            ASM(MOV, x86_deref_reg(EAX), x86_reg(EAX))
            // TO TYPE
            type->type = type_ptr;
        }
        else if ( prfx->op.token_val == ADDR_OP ) {
            ASM(LEA, x86_reg(EAX), x86_reg(EAX))
            type->type = type_ptr;
        }
        else {
            printf("UNKOWN PRFX OP\n");
        }

        ASM(PUSH, x86_reg(EAX), NULL)
        return type;
    }
    else if ( expr->type == expr_infix ) {
        expr_infix_t* infx = expr->value;
        x86_type_t* type1 = x86_compile_expression(cmp, infx->lhs);
        ASM(POP, x86_reg(EAX), NULL)
        x86_type_t* type2 = x86_compile_expression(cmp, infx->rhs);
        ASM(POP, x86_reg(EBX), NULL)

        if ( infx->op.token_val == ADD_BIN_OP ) {
            ASM(ADD, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == SUB_BIN_OP ) {
            ASM(SUB, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == MUL_BIN_OP ) {
            ASM(MUL, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == DIV_BIN_OP ) {
            // TO DO
        }
        else if ( infx->op.token_val == SHL_BIN_OP ) {
            ASM(SHL, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == SHR_BIN_OP ) {
            ASM(SHR, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == AND_BIN_OP ) {
            ASM(AND, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == OR_BIN_OP ) {
            ASM(OR, x86_reg(EBX), x86_reg(EAX))
        }
        else if ( infx->op.token_val == XOR_BIN_OP ) {
            ASM(XOR, x86_reg(EBX), x86_reg(EAX))
        }

        ASM(PUSH, x86_reg(EAX), NULL)

        if ( type1->type == type_struct || type2->type == type_struct) {
            free(type1);
            free(type2);
            printf("Unable to do arithmatic on structs");
            goto error;
        }
        else if ( type1->type == type_ptr || type2->type == type_ptr ) {
            free(type1);
            return type2;
        }
        else if ( type1->type == type_i32 || type2->type == type_i32 ) {
            free(type1);
            return type2;
        }
        else if ( type1->type == type_i16 || type2->type == type_i16 ) {
            free(type1);
            return type2;
        }
        else if ( type1->type == type_i8 || type2->type == type_i8 ) {
            free(type1);
            return type2;
        }
        else {
            free(type1);
            free(type2);
            goto error;
        }
    }
    else if ( expr->type == expr_int ) {
        ASM(PUSH, x86_number(*(int*)expr->value), NULL)
        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->size = x86_sizeof("int");
        type->type = type_i32;
        return type;
    }
    else if ( expr->type == expr_name ) {
        x86_variable_t* var = x86_find_variable(cmp, expr->value);

        if ( var == NULL ) {
            printf("VARIABLE NOT FOUND");
            goto error;
        }

        ASM(MOV, x86_offset_reg(EBP, var->offset), x86_reg(EAX))
        ASM(PUSH, x86_reg(EAX), NULL)

        x86_type_t* type = (x86_type_t*) malloc(sizeof(x86_type_t));
        type->size = x86_sizeof("int"); // TODO!!!
        type->type = type_i32;
        return type;
    }

    error:
    return NULL;
}

void x86_compile_statement(x86_state_t* cmp, stmt_t* stmt) {
    x86_label_t* label = cmp->cur_label;

    if ( stmt->type == vardec_stmt ) {
        vardec_stmt_t* dec = stmt->data;

        if ( x86_find_variable(cmp, dec->name) != NULL ) {
            printf("REDEF\n");
            return;
        }

        int size = x86_sizeof(dec->type);

        if ( size == 0 ) {
            printf("UNKNOWN TYPE\n");
            return;
        }

        x86_variable_t* var = (x86_variable_t*) malloc(sizeof(x86_variable_t));
        cmp->curr_off -= size;

        var->name = strdup(dec->name);

        if ( var->name == NULL ) {
            printf("STRDUPFAIL\n");
            free(var);
            goto error;
        }

        var->offset = cmp->curr_off;

        if ( dec->value != NULL ) {
            x86_type_t* type = x86_compile_expression(cmp, dec->value);

            if ( type == NULL ) {
                free((void*) var->name);
                free(var);
                goto error;
            }

            free(type);

            ASM(POP, x86_reg(EAX), NULL)
            ASM(MOV, x86_reg(EAX), x86_offset_reg(EBP, var->offset))
        }

        ADD_ARG(cmp->variables, cmp->variable_cnt, cmp->variable_alloc, var, {
            free(var);
        })
    }
    else if ( stmt->type == ret_stmt ) {
        x86_compile_expression(cmp, (expr_t*) stmt->data);
        ASM(POP, x86_reg(EAX), NULL)
        ASM(RET, NULL, NULL)
    }

    error:
    return;
}

void x86_compile_function(x86_state_t* cmp, function_t* fn) {
    int l = x86_find_label(cmp, fn->name);

    if ( l != -1 ) {
        printf("Unable to compile duplicate function %s\n", fn->name);
        return;
    }

    cmp->cur_label = (x86_label_t*) malloc(sizeof(x86_label_t));
    cmp->cur_label->name = strdup(fn->name);
    cmp->cur_label->cmds = INIT_LIST(x86_command_t);
    cmp->cur_label->cmd_cnt = 0;
    cmp->cur_label->cmd_alloc = MALLOC_CHUNK;

    for ( int i = 0; i < fn->block->len; i++ ) {
        x86_compile_statement(cmp, fn->block->stmts[i]);
    }

    x86_label_optimize(cmp->cur_label);

    ADD_ARG(cmp->labels, cmp->label_cnt, cmp->label_alloc, cmp->cur_label, {})

    cmp->cur_label = NULL;

    error:
    // idf
    return;
}

void _x86_label_remove(x86_label_t* label, int n) {
    x86_command_t* cmd = label->cmds[n];

    for ( int i = n; i < label->cmd_cnt - 1; i++ ) {
        label->cmds[i] = label->cmds[i+1];
    }

    label->cmds[label->cmd_cnt] = NULL;
    label->cmd_cnt -= 1;

    if ( cmd->arg1 ) {
        free(cmd->arg1->data);
        free(cmd->arg1);
    }
    if ( cmd->arg2 ) {
        free(cmd->arg2->data);
        free(cmd->arg2);
    }
    free(cmd);
}

bool _x86_arg_cmp(x86_arg_t* arg1, x86_arg_t* arg2) {
    if ( arg1->type != arg2->type ) {
        return false;
    }

    if ( arg1->type == arg_reg || arg1->type == arg_deref_reg ) {
        x86_reg_t* d1 = arg1->data;
        x86_reg_t* d2 = arg2->data;

        return d1->reg == d2->reg;
    }
    else if ( arg1->type == arg_number ) {
        x86_number_t* n1 = arg1->data;
        x86_number_t* n2 = arg2->data;

        return n1->number == n2->number;
    }
    else if ( arg1->type == arg_reg_off ) {
        x86_reg_off_t* d1 = (x86_reg_off_t*) arg1->data;
        x86_reg_off_t* d2 = (x86_reg_off_t*) arg2->data;

        return d1->reg == d2->reg && d1->amnt == d2->amnt;
    }

    return false;
}

void x86_label_optimize(x86_label_t* label) {
    for ( int i = 0; i < label->cmd_cnt - 1; i++ ) {
        x86_command_t* cmd = label->cmds[i];
        x86_command_t* ahead = label->cmds[i + 1];

        if ( (cmd->inst == PUSH || cmd->inst == PUSHL) && (ahead->inst == POP || ahead->inst == POPL) ) {
            if ( _x86_arg_cmp(cmd->arg1, ahead->arg1) ) {
                _x86_label_remove(label, i+1);
                _x86_label_remove(label, i);
            }
            else {
                cmd->inst = (cmd->inst == PUSHL || ahead->inst == POPL) ? MOVL : MOV;
                cmd->arg2 = ahead->arg1;

                ahead->arg1 = NULL;
                _x86_label_remove(label, i + 1);
            }

            i -= 1;
        }
        else if ( cmd->inst == MOV || cmd->inst == MOVL ) {
            if ( ahead->inst == MOV || ahead->inst == MOV ) {
                if ( _x86_arg_cmp(cmd->arg1, ahead->arg2) && _x86_arg_cmp(cmd->arg2, ahead->arg1) ) {
                    _x86_label_remove(label, i + 1);
                }
            }
            else {
                if ( _x86_arg_cmp(cmd->arg1, cmd->arg2) ) {
                    _x86_label_remove(label, i);
                }
            }
        }
    }
}
