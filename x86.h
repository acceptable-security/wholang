#include <stdbool.h>

typedef enum {
    PUSH,
    PUSHL,
    POP,
    POPL,
    ADD,
    ADDL,
    SUB,
    SUBL,
    MUL,
    IMUL,
    LEA,
    LEAL,
    MOV,
    MOVL,
    NEGL,
    NEG,
    SHL,
    SHR,
    AND,
    ANDL,
    OR,
    ORL,
    XOR,
    XORL,
    RET,
    CALL,
    MULL,
    IMULL
} x86_instruction_t;

typedef enum {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI
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

typedef enum {
    arg_reg,
    arg_deref_reg,
    arg_reg_off,
    arg_deref_off,
    arg_number,
    arg_label
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
    type_i32,
    type_i16,
    type_i8,
    type_ptr,
    type_struct
} x86_type_type_t;

typedef struct {
    unsigned int size;
    x86_type_type_t type;
    void* data;
} x86_type_t;

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
    const char* name;
} x86_variable_t;

typedef struct {
    int curr_off;
    x86_label_t* cur_label;

    x86_label_t** labels;
    unsigned int label_cnt;
    unsigned int label_alloc;

    x86_variable_t** variables;
    unsigned int variable_cnt;
    unsigned int variable_alloc;
} x86_state_t;

#define ASM(INST, OP, OP2) { \
        x86_command_t* cmd = (x86_command_t*) malloc(sizeof(x86_command_t)); \
        cmd->inst = INST; \
        cmd->arg1 = OP; \
        cmd->arg2 = OP2; \
        ADD_ARG(label->cmds, label->cmd_cnt, label->cmd_alloc, cmd, {}) \
    }\

x86_state_t* x86_init();
x86_type_t* x86_compile_expression(x86_state_t* cmp, expr_t* expr);
void x86_compile_statement(x86_state_t* cmp, stmt_t* stmt);
void x86_label_debug(x86_label_t* label);
void x86_label_optimize(x86_label_t* label);
