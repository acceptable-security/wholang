#include "parser.h"
#include "x86.h"
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

int run(int argc, char* argv[]) {
    if ( argc != 2 ) {
        printf("Usage: %s filename\n", argv[0]);
        return -1;
    }

    char* buffer = 0;
    long length;
    FILE* file = fopen(argv[1], "r");

    if ( file ) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc((length + 1) * sizeof(char));

        if ( buffer ) {
            fread(buffer, 1, length, file);
        }
        else {
            fclose(file);
            printf("Unable to allocate enough space for %s\n", argv[1]);
            return -1;
        }

        buffer[length] = 0;

        fclose(file);
    }
    else  {
        printf("Unable to open %s\n", argv[1]);
        return -1;
    }

    parser_state_t* parser = parser_init(buffer);
    parser_read(parser);
    parser_debug(parser);
    parser_clean(parser);

    free(buffer);

    return 0;
}

int main(int argc, char* argv[]) {
    parser_state_t* parser = parser_init("fn func() { var test : int = 1 + 2; var butt : int = test + 2; }");
    function_t* stmt = parser_read_function(parser, false);

    function_debug(stmt);

    printf("\n");

    x86_state_t* st = x86_init();
    x86_compile_function(st, stmt);
    x86_label_debug(st->labels[0]);
    x86_clean(st);

    function_clean(stmt);
    parser_clean(parser);
}
