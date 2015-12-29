# wholang
My experiments in lexing, compiling, and assembling.

## Pipeline
The way I've been designing it, the parts of this should at the end look something like

    lexer -> parser -> compiler -> assembler -> linker

## Lexer
Right now the API for the lexer is undocumented, but here's a wonderful copy and paste from the header file.

    lexer_state_t* lexer_init(const char* src, const char* specialTokens[]);
    void lexer_error(lexer_state_t* lex, const char* error, ...);
    void lexer_debug(lexer_state_t* lex);
    void lexer_clean(lexer_state_t* lex);

    token_t* lexer_cur(lexer_state_t* lex);
    token_t* lexer_lookahead(lexer_state_t* lex);
    bool lexer_matches(lexer_state_t* lex, token_types_t tokentype);
    bool lexer_lookaheadmatches(lexer_state_t* lex, token_types_t tokentype);
    token_t* lexer_next(lexer_state_t* lex);
    token_t* lexer_nextif(lexer_state_t* lex, token_types_t tokentype);
    token_t* lexer_nextif_special(lexer_state_t* lex, const char* token);
    token_t* lexer_expect(lexer_state_t* lex, token_types_t tokentype);
    token_t* lexer_expect_special(lexer_state_t* lex, const char* token);
    token_t* lexer_expectmatch(lexer_state_t* lex, const char* end, const char* start, int linenum);

That's helpful, right?

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
