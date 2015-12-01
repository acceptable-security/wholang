#include "parser.h"
#include <stdlib.h>
#include <stdio.h>

void test_lexer() {
    /**
     * NOTE:
     * When creating a special token list, make sure to put tokens which are subsets of larger tokens at the top
     * in order to avoid them being picked up as the smaller token.
     * Hopefully this will be fixed in a later revision.
     */
    const char* specialToken[] = {
        ">=",
        "<=",
        "!=",
        "==",
        "-=",
        "+=",
        "!=",
        "&&",
        "||",
        "&",
        "|",
        "^",
        "!",
        "(",
        ")",
        "-",
        "+",
        "/",
        "*",
        ";",
        ":",
        ",",
        NULL
    };

    lexer_state_t* lex = lexer_init("i123\ntes\n # t(); >= \"1we\\\"wldas\" 323.23", specialToken);
    token_t* tok = lexer_next(lex);

    while ( tok != NULL && tok->type != eof_token ) {
        token_debug(tok);
        token_clean(tok);

        tok = lexer_next(lex);
    }

    lexer_clean(lex);
}

void test_expr() {
    parser_state_t* parser = parser_init("1 + 2");
    expr_t* expr = parser_read_expr(parser, 0);

    expr_debug(expr);

    printf("\n");

    expr_clean(expr);
    lexer_clean(parser->lex);
    free(parser);
}

int main(int argc, char* argv[]) {
    parser_state_t* parser = parser_init("fn test() { void(); var wew = test;}");
    function_t* fn = parser_read_function(parser);
    function_debug(fn);

    function_clean(fn);
    lexer_clean(parser->lex);
    free(parser);

    return 0;
}
