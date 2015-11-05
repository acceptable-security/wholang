#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

#define MALLOC_CHUNK 8

token_t* token_init(token_types_t type) {
    token_t* tok = (token_t*) malloc(sizeof(token_t));
    tok->type = type;
    return tok;
}

void token_debug(token_t* token) {
    switch ( token->type ) {
        case name_token:
            printf("NAME_TOKEN %s\n", (char*)token->value);
            break;
        case string_token:
            printf("STRING_TOKEN %s\n", (char*)token->value);
            break;
        case special_token:
            printf("SPECIAL_TOKEN %s\n", (char*)token->value);
            break;
        case int_token:
            printf("INT_TOKEN %d\n", *(int*)token->value);
            break;
        case double_token:
            printf("DOUBLE_TOKEN %f\n", *(double*)token->value);
            break;
        case eof_token:
            printf("EOF\n");
            break;
    }
}

void token_clean(token_t* token) {
    if ( token->type != int_token && token->type != eof_token )
        free(token->value);

    free(token);
}

lexer_state_t* lexer_init(const char* src) {
    lexer_state_t* lex = (lexer_state_t*) malloc(sizeof(lexer_state_t));

    lex->tokenIndex = 0;
    lex->sourceIndex = 0;
    lex->lineNumber = 0;
    lex->lineIndex = 0;
    lex->source = src;
    lex->_sourceLen = strlen(src);

    lex->current = _lexer_read_token(lex);
    lex->next = _lexer_read_token(lex);

    return lex;
}

token_t* lexer_cur(lexer_state_t* lex) {
    return lex->current;
}

token_t* lexer_lookahead(lexer_state_t* lex) {
    return lex->next;
}

bool lexer_matches(lexer_state_t* lex, token_types_t tokentype) {
    return lexer_lookahead(lex)->type == tokentype;
}

bool lexer_lookaheadmatches(lexer_state_t* lex, token_types_t tokentype) {
    return lexer_lookahead(lex)->type == tokentype;
}

token_t* lexer_next(lexer_state_t* lex) {
    token_t* c = lex->current;

    lex->current = lex->next;
    lex->next = _lexer_read_token(lex);

    return c;
}

token_t* lexer_nextif(lexer_state_t* lex, token_types_t tokentype) {
    if ( lexer_matches(lex, tokentype) ) {
        return lexer_next(lex);
    }

    return NULL;
}

token_t* lexer_expect(lexer_state_t* lex, token_types_t tokentype) {
    token_t* tok = lexer_nextif(lex, tokentype);

    if ( tok == NULL ) {
        lexer_error(lex, "Expected token.");
        // WEW
    }

    return tok;
}

void lexer_error(lexer_state_t* lex, const char* error) {
    printf("Error [%d:%d] %s\n", lex->lineNumber, lex->lineIndex, error);
}

void lexer_debug(lexer_state_t* lex) {
    token_debug(lex->current);
    token_debug(lex->next);
}

void lexer_clean(lexer_state_t* lex) {
    token_clean(lex->current);
    token_clean(lex->next);

    free(lex);
}

token_t* _lexer_read_token(lexer_state_t* lex) {
    while ( (lex->source[lex->sourceIndex] == ' '  || lex->source[lex->sourceIndex] == '\t' ||
            lex->source[lex->sourceIndex] == '\n') && lex->sourceIndex < lex->_sourceLen ) {
        if ( lex->source[lex->sourceIndex] == '\n' ) {
            lex->lineNumber++;
            lex->lineIndex = 0;
        }
        else {
            lex->lineIndex++;
        }

        lex->sourceIndex++;
    }

    if ( lex->sourceIndex >= lex->_sourceLen ) {
        return token_init(eof_token);
    }

    char current = lex->source[lex->sourceIndex];

    if ( current >= '0' && current <= '9' ) {
        unsigned int currAlloc = MALLOC_CHUNK;
        unsigned int index = 0;
        char* str = (char*) malloc(currAlloc * sizeof(char));

        while ( (current >= '0' && current <= '9') && (lex->sourceIndex + index) < lex->_sourceLen ) {
            if ( index >= (currAlloc - 1) ) {
                currAlloc += MALLOC_CHUNK;
                char* temp = (char*) realloc(str, currAlloc);

                if ( !temp ) {
                    lex->sourceIndex += index;
                    lex->lineIndex += index;
                    lexer_error(lex, "Ran out of memory.");

                    free(str);
                    return NULL;
                }

                str = temp;
            }

            str[index] = current;
            str[index + 1] = 0;

            current = lex->source[lex->sourceIndex + (++index)];
        }

        if ( current == '.') {
            currAlloc += MALLOC_CHUNK;

            char* temp = (char*) realloc(str, currAlloc);

            if ( !temp ) {
                lex->sourceIndex += index;
                lex->lineIndex += index;
                lexer_error(lex, "Ran out of memory.");

                free(str);
                return NULL;
            }

            str = temp;

            str[index] = '.';
            str[index + 1] = 0;

            current = lex->source[lex->sourceIndex + (++index)];

            while ( (current >= '0' && current <= '9') && (lex->sourceIndex + index) < lex->_sourceLen ) {
                if ( index >= (currAlloc - 1) ) {
                    currAlloc += MALLOC_CHUNK;
                    char* temp = (char*) realloc(str, currAlloc);

                    if ( !temp ) {
                        lex->sourceIndex += index;
                        lex->lineIndex += index;
                        lexer_error(lex, "Ran out of memory.");

                        free(str);
                        return NULL;
                    }

                    str = temp;
                }

                str[index] = current;
                str[index + 1] = 0;

                current = lex->source[lex->sourceIndex + (++index)];
            }

            lex->sourceIndex += index;
            lex->lineIndex += index;

            double* v = (double*) malloc(sizeof(double));
            *v = atof(str);

            token_t* token = token_init(double_token);
            token->value = (void*) v;

            free(str);
            return token;
        }
        else {
            lex->sourceIndex += index;
            lex->lineIndex += index;

            int val = atol(str);

            token_t* token = token_init(int_token);
            token->value = (void*) &val;

            free(str);
            return token;
        }
    }
    else if ( (current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z') ) {
        unsigned int currAlloc = MALLOC_CHUNK;
        char* str = (char*) malloc(currAlloc * sizeof(char));
        str[0] = current;
        str[1] = 0;

        unsigned int index = 1;
        current = lex->source[lex->sourceIndex + index];

        while ( ((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z') ||
                (current >= '0' && current <= '9')) && (lex->sourceIndex + index) < lex->_sourceLen ) {
            if ( index >= (currAlloc - 1) ) {
                currAlloc += MALLOC_CHUNK;
                char* temp = (char*) realloc(str, currAlloc);

                if ( !temp ) {
                    lex->sourceIndex += index;
                    lex->lineIndex += index;
                    lexer_error(lex, "Ran out of memory.");

                    free(str);
                    return NULL;
                }

                str = temp;
            }

            str[index] = current;
            str[index + 1] = 0;

            current = lex->source[lex->sourceIndex + (++index)];
        }

        lex->sourceIndex += index;
        lex->lineIndex += index;

        char* temp = (char*) realloc(str, strlen(str) + 1);

        if ( !temp ) {
            lexer_error(lex, "Ran out of memory.");

            free(str);
            return NULL;
        }

        token_t* token = token_init(name_token);
        token->value = (void*) temp;

        return token;
    }
    else if ( current == '\"' || current == '\'' ) {
        char initial = current;

        lex->sourceIndex += 1;
        lex->lineIndex += 1;

        current = lex->source[lex->sourceIndex];

        unsigned int currAlloc = MALLOC_CHUNK;
        char* str = (char*) malloc(currAlloc * sizeof(char));
        str[0] = current;
        str[1] = 0;

        unsigned int index = 1;
        current = lex->source[lex->sourceIndex + index];

        while ( !(current == initial && lex->source[lex->sourceIndex + index - 1] != '\\') &&
                 (lex->sourceIndex + index) <= lex->_sourceLen ) {
            if ( index >= (currAlloc - 1) ) {
                currAlloc += MALLOC_CHUNK;
                char* temp = (char*) realloc(str, currAlloc);

                if ( !temp ) {
                    lex->sourceIndex += index;
                    lex->lineIndex += index;
                    lexer_error(lex, "Ran out of memory.");

                    free(str);
                    return NULL;
                }

                str = temp;
            }

            if ( lex->source[lex->sourceIndex + index] == '\\' ) {
                if ( lex->source[lex->sourceIndex + index - 1] != '\\' ) {
                    current = lex->source[lex->sourceIndex + (++index)];
                    continue;
                }
            }

            int t = strlen(str);
            str[t] = current;
            str[t + 1] = 0;

            current = lex->source[lex->sourceIndex + (++index)];
        }

        lex->sourceIndex += index;
        lex->lineIndex += index;

        char* temp = (char*) realloc(str, strlen(str) + 1);

        if ( !temp ) {
            lexer_error(lex, "Ran out of memory.");

            free(str);
            return NULL;
        }

        token_t* token = token_init(string_token);
        token->value = (void*) temp;

        lex->sourceIndex++;

        return token;
    }
    else {
        char* special = (char*) malloc(2 * sizeof(char));

        special[0] = current;
        special[1] = 0;

        lex->sourceIndex++;

        token_t* tok = token_init(special_token);
        tok->value = (void*) special;

        return tok;
    }

    return NULL;
}
