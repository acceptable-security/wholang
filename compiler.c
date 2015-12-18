#include "parser.h"
#include "compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define min(a, b) a > b ? a : b

int compile_size(const char* type) {
    if ( strcmp(type, "int") == 0 ) {
        return sizeof(int);
    }
    else if ( strcmp(type, "void") == 0 ) {
        return sizeof(void);
    }
    else if ( strcmp(type, "long") == 0 ) {
        return sizeof(long);
    }
    else {
        // ALL IS LOST
        // PREPARE YOUR ANUS
        // FOR SATAN
        return 0;
    }
}

void compile_expr(expr_t* expr) {

}

void compile_vardec(compiled_block_t* block, vardec_stmt_t* dec) {
    int size = compile_size(dec->type);

    if ( size == 0 ) {
        // VAGGOO GOES POOF
        return;
    }

    for ( int i = 0; i < block->name_cnt; i++ ) {
        if ( strcmp(block->names[i], dec->name) == 0 ) {
            printf("Error: Unable to redefine %s", dec->name);
            return;
        }
    }

    ASM_VAR(dec->name, size)

    if ( dec->value != NULL ) {
        int rest = 0;

        if ( block->u_eax ) {
            rest = 1;
            ASM(PUSH, EAX)
        }
        else {
            block->u_eax = 1;
        }

        compile_expr(dec->value);

        ASM(MOV, EAX, block->loc, EBP)

        if ( rest ) {
            ASM(POP, EAX)
        }
        else {
            block->u_eax = 0;
        }
    }

    return;

    error:
    CLEAN_ASM()
    return;
}

void compile_function(function_t* fn) {
    START_ASM()

    ASM(PUSH, EBP)
    ASM(MOV, ESP, EBP)

    for ( int i = 0; i < min(EDI, fn->len); i++ ) {
        int size = compile_size(fn->args[i]->type);

        if ( i <= EDI ) {
            ASM(MOV, EAX + i, -(size * i), EBP)
            ASM_VAR((char*) fn->args[i]->name, -(size * i))
        }
        else {
            ASM_VAR((char*) fn->args[i]->name, 8 + (size * i))
        }
    }

    for ( int i = 0; i < fn->block->len; i++ ) {
        stmt_t* stmt = fn->block->stmts[i];

        switch ( stmt->type ) {
            case dec_stmt:

                break;
            default:
                // I HATE MY LIFE
                break;
        }
    }

    ASM(POP, EBP)
    ASM(RET)

    return;
    error:
    CLEAN_ASM()
    return;
}
