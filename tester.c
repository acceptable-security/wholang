#include "lexer.h"

int main(int argc, char* argv[]) {
    lexer_state_t* lex = lexer_init("i123 test(); \"1we\\\"wldas\" 323.23");
    token_t* tok;

    while ( (tok = lexer_next(lex)) && (tok->type != eof_token) ) {
        token_debug(tok);
        token_clean(tok);
    }

    lexer_clean(lex);

    return 0;
}
