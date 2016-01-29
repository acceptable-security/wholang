#include <stdbool.h>
#include "parser.h"

typedef enum {
    PUSH,
    PUSHB,
    PUSHW,
    PUSHL,

    POP,
    POPB,
    POPW,
    POPL,

    ADD,
    ADDB,
    ADDW,
    ADDL,
    ADDSS,
    ADDSD,

    SUB,
    SUBB,
    SUBW,
    SUBL,
    SUBSS,
    SUBSD,

    MUL,
    MULB,
    MULW,
    MULL,
    MULSS,
    MULSD,

    IMUL,
    IMULB,
    IMULW,
    IMULL,

    LEA,
    LEAB,
    LEAW,
    LEAL,

    MOV,
    MOVB,
    MOVW,
    MOVL,
    MOVSS,
    MOVSD,
    MOVZBL,
    MOVZWL,
    MOVSBL,
    MOVSWL,
    MOVS,
    MOVSB,
    MOVSW,
    MOVSL,
    MOVZ,
    MOVZB,
    MOVZW,
    MOVZL,

    NEG,
    NEGB,
    NEGW,
    NEGL,

    SHL,
    SHLB,
    SHLW,
    SHLL,

    SHR,
    SHRB,
    SHRW,
    SHRL,

    AND,
    ANDB,
    ANDW,
    ANDL,

    OR,
    ORB,
    ORW,
    ORL,

    XOR,
    XORB,
    XORW,
    XORL,

    CVTSI2SS,
    CVTSI2SD,
    CVTSS2SI,
    CVTSD2SI,

    CMPL,
    SETE,
    SETLE,
    SETGE,
    SETL,
    SETG,

    JMP,
    JE,
    JNE,
    JG,
    JGE,
    JL,
    JLE,

    RET,
    LEAVE,
    CALL
} x86_instruction_t;

typedef enum {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI,

    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,

    AX,
    BX,
    CX,
    DX,

    AH,
    AL,
    BH,
    BL,
    CH,
    CL,
    DH,
    DL
} x86_register_t;

typedef struct {
    x86_register_t reg;
    int amnt;
} x86_reg_off_t;

typedef struct {
    x86_register_t reg;
} x86_reg_t;

typedef struct {
    x86_register_t reg;
} x86_deref_reg_t;

typedef struct {
    x86_register_t reg;
    int amnt;
} x86_deref_off_t;

typedef struct {
    int number;
} x86_number_t;

typedef struct {
    unsigned long number;
} x86_ulong_t;

typedef struct {
    int index;
} x86_string_t;

typedef enum {
    arg_reg,
    arg_deref_reg,
    arg_reg_off,
    arg_deref_off,
    arg_number,
    arg_label,
    arg_ulong,
    arg_string
} x86_type_arg_t;

typedef struct {
    x86_type_arg_t type;
    void* data;
} x86_arg_t;

typedef struct {
    x86_instruction_t inst;

    x86_arg_t* arg1;
    x86_arg_t* arg2;
} x86_command_t;

typedef enum {
    prim_int,
    prim_uint,
    prim_float,
    prim_double,
    prim_void,
    prim_struct
} x86_primitive_t;

typedef struct {
    unsigned int size;
    x86_primitive_t prim;
    int deref_cnt;
    int struct_ind;

    bool is_static;
    bool is_const;
    bool is_struct;

    bool used;
    int offset;
} x86_type_t;

typedef struct {
    x86_type_t* type;
    char* name;
    int offset;
} x86_struct_arg_t;

typedef struct {
    x86_struct_arg_t** args;
    unsigned int arg_cnt;
    unsigned int arg_alloc;

    char* name;
    unsigned int size;
} x86_struct_t;

typedef struct {
    const char* name;

    void* fn;
    bool global;

    x86_command_t** cmds;
    unsigned int cmd_cnt;
    unsigned int cmd_alloc;
} x86_label_t;

typedef struct {
    x86_label_t* label;
} x86_arg_label_t;

typedef struct {
    int offset;
    int size;
    const char* name;
    x86_type_t* type;
} x86_variable_t;

typedef struct {
    int curr_off;
    x86_label_t* cur_label;
    parser_state_t* parser;
    int curr_labels;

    x86_label_t** labels;
    unsigned int label_cnt;
    unsigned int label_alloc;

    x86_variable_t** variables;
    unsigned int variable_cnt;
    unsigned int variable_alloc;

    char** strings;
    unsigned int string_cnt;
    unsigned int string_alloc;

    x86_struct_t** structs;
    unsigned int struct_cnt;
    unsigned int struct_alloc;

    int error;
} x86_state_t;

#define ASM(INST, OP, OP2) { \
        x86_command_t* cmd = (x86_command_t*) malloc(sizeof(x86_command_t)); \
        cmd->inst = INST; \
        cmd->arg1 = OP; \
        cmd->arg2 = OP2; \
        ADD_ARG(label->cmds, label->cmd_cnt, label->cmd_alloc, cmd, {}) \
    }\

x86_state_t* x86_init();
void x86_clean(x86_state_t* cmp);
int x86_find_label(x86_state_t* cmp, const char* label);
x86_type_t* x86_type_string(x86_state_t* cmp, const char* str);
x86_type_t* x86_type(x86_state_t* cmp, type_t* type);
bool x86_type_equ(x86_state_t* cmp, x86_type_t* type1, x86_type_t* type2);
x86_variable_t* x86_find_variable(x86_state_t* cmp, const char* name);
x86_type_t* x86_compile_fncall(x86_state_t* cmp, expr_call_t* call);
x86_type_t* x86_compile_expression(x86_state_t* cmp, expr_t* expr);
void x86_compile_function(x86_state_t* cmp, function_t* fn);
void _x86_label_remove(x86_label_t* label, int n);
int x86_resolve_string(x86_state_t* cmp, char* str);
void x86_compile_statement(x86_state_t* cmp, stmt_t* stmt);
void x86_label_debug(x86_label_t* label);
void x86_label_optimize(x86_label_t* label);
