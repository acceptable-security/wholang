# wholang
My experiments in lexing, compiling, and assembling.

## Lexer
libwholex is currently being used.

## Parser
Well. It's a little scary. Here's the API. Uses a weird/hackish Pratt Parser in order to do expression parsing, which
I guess works but is a bit of a hack.

    parser_state_t* parser_init(const char* src);

    expr_t* parser_read_expr(parser_state_t* parser, int pres);
    void expr_debug(expr_t* expr);
    void expr_clean(expr_t* expr);

    stmt_t* parser_read_stmt(parser_state_t* parser);
    void stmt_debug(stmt_t* stmt);
    void stmt_clean(stmt_t* stmt);

    block_t* parser_read_block(parser_state_t* parser);
    void block_debug(block_t* block);
    void block_clean(block_t* block);

    function_t* parser_read_function(parser_state_t* parser);
    void function_debug(function_t* fn);
    void function_clean(function_t* fn);

    void parser_read(parser_state_t* parser);

## Compiler
This is next.

## Assembler
Then this.

## Linker
UUUUUUUUGH

## Building
    gcc lexer.c parser.c tester.c

## Pull Requests
If you some how have managed to obtain so much time you can waste as to want to contribute to this project, please run
valgrind on any builds and make sure that there are no memleaks. This is C after all.
