#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

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

    return 0;
}
