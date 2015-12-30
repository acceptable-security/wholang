#include "lexer.h"
#include <stdlib.h>

#define IF_NEXT(LEX, NEXT, CUR) (CUR = lexer_nextif_special(LEX, NEXT)) != NULL

#define IF_AHEAD(LEX, NEXT) (lexer_lookahead_matches_special(LEX, NEXT))

#define LEXER_FN_DEST(LEX, DEST, FN) \
            DEST = FN(LEX); \
            if ( DEST == NULL && LEX->error ) { \
                goto error; \
            } \

#define LEXER_FN_DEST_ARGS(LEX, DEST, FN, ARGS...) \
            DEST = FN(LEX, ARGS); \
            if ( DEST == NULL && LEX->error ) { \
                goto error; \
            } \

#define LEXER_FN(LEX, FN) \
            FN(LEX); \
            if ( LEX->error ) { \
                goto error; \
            } \

#define LEXER_FN_ARGS(LEX, FN, ARGS...) \
            FN(LEX, ARGS); \
            if ( LEX->error ) { \
                goto error; \
            } \

#define NEXT_NAME(LEX, DEST) \
            {\
                token_t* tmp; \
                LEXER_FN_DEST(LEX, tmp, lexer_next) \
                if ( tmp->type != name_token ) { \
                    token_clean(tmp); \
                    lexer_error(LEX, "Expected a name token\n"); \
                    goto error; \
                }\
                DEST = strdup(tmp->value); \
                if ( DEST == NULL ) { \
                    token_clean(tmp); \
                    goto error; \
                } \
                token_clean(tmp); \
            }\

#define MALLOC_CHUNK 8

#define ADD_ARG(LIST, LEN, ALCD, ARG, BLK) \
            if ( LEN == ALCD ) { \
                ALCD += MALLOC_CHUNK; \
                typeof(LIST) tmp = realloc(LIST, ALCD * sizeof(typeof(*LIST))); \
                if ( tmp == NULL ) { \
                    BLK \
                    goto error; \
                } \
                LIST = tmp; \
            } \
            LIST[LEN++] = ARG; \

#define INIT_LIST(TYPE) (TYPE**) malloc(sizeof(TYPE*) * MALLOC_CHUNK)

#define MUST_BE(LEX, SPC, CLN) \
    { \
        token_t* _tmp = lexer_expect_special(LEX, SPC); \
        if ( LEX->error ) { \
            if ( _tmp != NULL ) token_clean(_tmp); \
            CLN \
            goto error; \
        } \
        token_clean(_tmp); \
    } \

// Expression Data
typedef enum {
    expr_prefix,    // !var
    expr_infix,     // var * var
    expr_suffix,    // var++
    expr_int,       // int
    expr_double,    // double
    expr_name,      // name
    expr_string,    // string
    expr_call,      // var();
    expr_tetirary,  // var ? var : var
    expr_fn,        // fn() { }
    expr_expr,      // (var)
    expr_cast       // (name) test
} expr_type_t;

typedef struct {

} fndec_expr_t;

typedef struct {
    void* value;
    expr_type_t type;
} expr_t;

typedef struct {
    char* type;
    expr_t* rhs;
} expr_cast_t;

typedef struct {
    const char* name;
    int pres;
    int fix;
    int token_val;
} expr_token_t;

typedef enum {
    ADD_BIN_OP,    // +
    SUB_BIN_OP,    // -
    MUL_BIN_OP,    // *
    DIV_BIN_OP,    // /
    MOD_BIN_OP,    // %
    MOD_OP,        // %

    SHL_BIN_OP,  // <<
    SHR_BIN_OP, // >>
    OR_BIN_OP,     // |
    AND_BIN_OP,    // &
    XOR_BIN_OP,    // ^

    LT_BIN_OP,         // <
    GT_BIN_OP,         // >
    LTE_BIN_OP,        // <=
    GTE_BIN_OP,        // >=
    ET_BIN_OP,         // ==
    // EQ_BIN_OP,         // =
} expr_infix_op_t;

typedef struct {
    expr_t* lhs;
    expr_t* rhs;

    expr_token_t op;
} expr_infix_t;

typedef enum {
    DEREF_OP, // *
    OPP_OP,   // !
    NEG_OP,   // -
    ADDR_OP   // &
} expr_prefix_op_t;

typedef struct {
    expr_t* lhs;

    expr_token_t op;
} expr_prefix_t;

typedef enum {
    PLUSPLUS_OP, // ++
    MINMIN_OP    // --
} expr_suffix_op_t;

typedef struct {
    expr_t* lhs;

    expr_token_t op;
} expr_suffix_t;

typedef struct {
    const char* name;

    expr_t** args;
    unsigned int len;
    unsigned int alloc;
} expr_call_t;

static expr_token_t expr_type[4][128] = {
    // prefix
    {
        { "-", 0, 0, NEG_OP },
        { "!", 0, 0, OPP_OP },
        { "*", 0, 0, DEREF_OP },
        { "&", 0, 0, ADDR_OP },
        { "", -1, 0 }
    },

    // Infix
    {
        { "<",  1, 1, LT_BIN_OP  },
        { ">",  1, 1, GT_BIN_OP  },
        { ">=", 1, 1, GTE_BIN_OP },
        { "<=", 1, 1, LTE_BIN_OP },
        { "==", 1, 1, ET_BIN_OP  },
        // { "=",  1, 1, EQ_BIN_OP  },

        { "+",  2, 1, ADD_BIN_OP },
        { "-",  2, 1, SUB_BIN_OP },
        { "*",  3, 1, MUL_BIN_OP },
        { "/",  3, 1, DIV_BIN_OP },
        { "%",  3, 1, MOD_BIN_OP },

        { "<<", 3, 1, SHL_BIN_OP },
        { ">>", 3, 1, SHR_BIN_OP },
        { "|",  3, 1, OR_BIN_OP  },
        { "&",  3, 1, AND_BIN_OP },
        { "^",  3, 1, XOR_BIN_OP },

        { "",  -1, 1 }
    },

    // Suffix/prefix
    {
        { "++", 1, 2, PLUSPLUS_OP },
        { "--", 1, 2, MINMIN_OP },
        { "",  -1, 2 }
    },
};


// Statement Data
typedef enum {
    call_stmt,   // Call a function
    vardec_stmt, // Declare a variable
    varset_stmt, // Set a variable
    if_stmt,     // If statement
    for_stmt,    // For statement
    while_stmt,  // While statement
    var_stmt,    // Variable Statement
    dec_stmt,    // Declaration Statement
    ret_stmt     // Return Statement
} stmt_type_t;

typedef struct {
    stmt_type_t type;
    void* data;
} stmt_t;

typedef struct {
    stmt_t** stmts;
    unsigned int len;
    unsigned int alloc;
} block_t;

typedef struct {
    expr_t* expr;
    block_t* block;
} if_stmt_t;

typedef struct {
    stmt_t* init;
    expr_t* cond;
    stmt_t* step;

    block_t* block;
} for_stmt_t;

typedef struct {
    expr_t* expr;
    block_t* block;
} while_stmt_t;

typedef struct {
    char* name;
    char* type;

    expr_t* value;
} vardec_stmt_t;

typedef struct {
    expr_t* lhs;
    char mod;
    expr_t* value;
} varset_stmt_t;

typedef struct {
    const char* name;
    const char* type;
} function_arg_t;

typedef struct {
    block_t* block;
    unsigned int stmt_len;

    const char* ret_type;
    const char* name;

    function_arg_t** args;
    unsigned int len;
    unsigned int alloc;
} function_t;

typedef struct {
    function_arg_t** args;
    unsigned int len;
    unsigned int alloc;

    const char* name;
} struct_t;

typedef enum {
    vardec_typedef,
    struct_typedef
} typedef_type_t;

typedef struct {
    const char* name;
    void* data;
    typedef_type_t type;
} typedef_t;

// Parser Information

typedef struct {
    const char* src;
    lexer_state_t* lex;
    bool error;

    function_t** functions;
    unsigned int fn_len;
    unsigned int fn_alloc;

    struct_t** structs;
    unsigned int strc_len;
    unsigned int strc_alloc;

    typedef_t** typedefs;
    unsigned int typedef_len;
    unsigned int typedef_alloc;
} parser_state_t;

parser_state_t* parser_init(const char* src);
void parser_debug(parser_state_t* parser);
void parser_read(parser_state_t* parser);
void parser_clean(parser_state_t* parser);

expr_t* parser_read_expr(parser_state_t* parser, int pres);
void expr_debug(expr_t* expr);
void expr_clean(expr_t* expr);

stmt_t* parser_read_stmt(parser_state_t* parser, bool _inline);
void stmt_debug(stmt_t* stmt);
void stmt_clean(stmt_t* stmt);

block_t* parser_read_block(parser_state_t* parser);
void block_debug(block_t* block);
void block_clean(block_t* block);

function_t* parser_read_function(parser_state_t* parser, bool anonymous);
void function_debug(function_t* fn);
void function_clean(function_t* fn);

struct_t* parser_read_struct(parser_state_t* parser);
void struct_debug(struct_t* strc);
void struct_clean(struct_t* strc);

int parser_typedef(parser_state_t* parser, const char* name, void* data, typedef_type_t type);
