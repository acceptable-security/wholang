typedef enum {
    PUSH,
    POP,
    ADD,
    SUB,
    MOV,
    LEA,
    RET
} instructions_enum_t;

typedef enum {
    EAX,
    ECX,
    EDX,
    EBX,
    ESP,
    EBP,
    ESI,
    EDI
} registers_enum_t;

typedef struct {
    instructions_enum_t instruction;
    int argc;
    int* argv;
} instruction_t;

typedef struct {
    instruction_t** instructions;
    unsigned int instruction_cnt;
    unsigned int instruction_alloc;

    int loc;
    int* vars;
    int var_cnt;
    int var_alloc;

    char** names;
    int name_cnt;
    int name_alloc;

    int u_eax;
    int u_ecx;
    int u_edx;
    int u_ebx;
} compiled_block_t;

#define START_ASM() \
    compiled_block_t* block = (compiled_block_t*) malloc(sizeof(block_t)); \
    block->instruction_alloc = MALLOC_CHUNK; \
    block->instruction_cnt = 0; \
    block->instructions = INIT_LIST(instruction_t); \
    block->vars = (int*) malloc(sizeof(int) * MALLOC_CHUNK); \
    block->var_alloc = MALLOC_CHUNK; \
    block->var_cnt = 0; \
    block->names = INIT_LIST(char); \
    block->name_alloc = MALLOC_CHUNK; \
    block->name_cnt = 0; \

#define CLEAN_ASM() \
    if ( block->instructions ) { \
        for ( int i = 0; i < block->instruction_cnt; i++ ) { \
            free(block->instructions[i]); \
        } \
        free(block->instructions); \
    } \
    free(block); \

#define ASM(INST, ...) \
    { \
        instruction_t* inst = (instruction_t*) malloc(sizeof(instruction_t)); \
        inst->instruction = INST; \
        int tmp[] = (int[]){__VA_ARGS__}; \
        inst->argc = (sizeof(tmp) / sizeof(int)); \
        inst->argv = (int*) malloc(inst->argc * sizeof(int)); \
        for ( int i = 0; i < inst->argc; i++ ) { \
            inst->argv[i] = tmp[i]; \
        } \
        ADD_ARG(block->instructions, block->instruction_cnt, block->instruction_alloc, inst, {}) \
    } \

#define ASM_VAR(NAME, PLC) \
    { \
        block->loc -= PLC; \
        ADD_ARG(block->vars, block->var_cnt, block->var_alloc, block->loc, {}) \
        ADD_ARG(block->names, block->name_cnt, block->name_alloc, NAME, {}) \
    } \
