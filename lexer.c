#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"

#define MALLOC_CHUNK 8

const char* token_names[] = {
    "NAME",
    "INT",
    "DOUBLE",
    "STRING",
    "SPECIAL",
    "EOF"
};

token_t* token_init(token_types_t type, int lineNumber, int lineIndex, void* data) {
    token_t* tok = (token_t*) malloc(sizeof(token_t));
    tok->type = type;
    tok->lineNumber = lineNumber;
    tok->lineIndex = lineIndex;
    tok->value = data;

    return tok;
}

void token_debug(token_t* token) {
    switch ( token->type ) {
        case name_token:
            printf("[%d:%d] NAME_TOKEN %s\n", token->lineNumber, token->lineIndex, (char*)token->value);
            break;
        case string_token:
            printf("[%d:%d] STRING_TOKEN %s\n", token->lineNumber, token->lineIndex, (char*)token->value);
            break;
        case special_token:
            printf("[%d:%d] SPECIAL_TOKEN %s\n", token->lineNumber, token->lineIndex, (char*)token->value);
            break;
        case int_token:
            printf("[%d:%d] INT_TOKEN %d\n", token->lineNumber, token->lineIndex, *(int*)token->value);
            break;
        case double_token:
            printf("[%d:%d] DOUBLE_TOKEN %f\n", token->lineNumber, token->lineIndex, *(double*)token->value);
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

lexer_state_t* lexer_init(const char* src, const char* specialTokens[]) {
    lexer_state_t* lex = (lexer_state_t*) malloc(sizeof(lexer_state_t));

    lex->tokenIndex = 0;
    lex->sourceIndex = 0;
    lex->lineNumber = 0;
    lex->lineIndex = 0;
    lex->source = src;
    lex->_sourceLen = strlen(src);

    lex->specialTokens = specialTokens;
    lex->specialTokenLength = 0;

    lex->error = false;

    while ( lex->specialTokens[(lex->specialTokenLength++) + 1] != NULL ) { }

    lex->current = _lexer_read_token(lex);
    lex->next = _lexer_read_token(lex);

    return lex;
}

token_t* lexer_cur(lexer_state_t* lex) {
    if ( lex->error ) return NULL;

    return lex->current;
}

token_t* lexer_lookahead(lexer_state_t* lex) {
    if ( lex->error ) return NULL;

    return lex->next;
}

bool lexer_matches(lexer_state_t* lex, token_types_t tokentype) {
    if ( lex->error ) return NULL;

    return lexer_lookahead(lex)->type == tokentype;
}

bool lexer_lookaheadmatches(lexer_state_t* lex, token_types_t tokentype) {
    if ( lex->error ) return NULL;

    return lexer_lookahead(lex)->type == tokentype;
}

token_t* lexer_next(lexer_state_t* lex) {

    token_t* c = lex->current;

    lex->current = lex->next;

    if ( lex->current != NULL )
        lex->next = _lexer_read_token(lex);
    else
        lex->next = NULL;

    return c;
}

token_t* lexer_nextif(lexer_state_t* lex, token_types_t tokentype) {
    if ( lex->error ) return NULL;

    if ( lexer_matches(lex, tokentype) ) {
        return lexer_next(lex);
    }

    return NULL;
}

token_t* lexer_expect(lexer_state_t* lex, token_types_t tokentype) {
    if ( lex->error ) return NULL;

    token_t* tok = lexer_nextif(lex, tokentype);

    if ( tok == NULL ) {
        lexer_error(lex, "Expected %s token.\n", token_names[tokentype]);
    }

    return tok;
}

void lexer_error(lexer_state_t* lex, const char* error, ...) {
    va_list ap;

    printf("Error [%d:%d] ", lex->lineNumber, lex->lineIndex);
    va_start(ap, error);
    vfprintf(stdout, error, ap);
    va_end (ap);

    lex->error = true;
}

void lexer_debug(lexer_state_t* lex) {
    token_debug(lex->current);
    token_debug(lex->next);
}

void lexer_clean(lexer_state_t* lex) {
    if ( lex->current != NULL )
        token_clean(lex->current);

    if ( lex->next != NULL )
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
        return token_init(eof_token, lex->lineNumber, lex->lineIndex, NULL);
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
                    lexer_error(lex, "Ran out of memory.\n");

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
                lexer_error(lex, "Ran out of memory.\n");

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
                        lexer_error(lex, "Ran out of memory.\n");

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

            token_t* token = token_init(double_token, lex->lineNumber, lex->lineIndex, (void*) v);

            free(str);
            return token;
        }
        else {
            lex->sourceIndex += index;
            lex->lineIndex += index;

            int val = atol(str);

            token_t* token = token_init(int_token, lex->lineNumber, lex->lineIndex, (void*) &val);

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
                    lexer_error(lex, "Ran out of memory.\n");

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
            lexer_error(lex, "Ran out of memory.\n");

            free(str);
            return NULL;
        }

        bool special = false;

        for ( int i = 0; i < lex->specialTokenLength; i++ ) {
            if ( strcmp(temp, lex->specialTokens[i]) == 0 ) {
                special = true;
                break;
            }
        }

        token_t* token = token_init((special ? special_token : name_token), lex->lineNumber, lex->lineIndex, (void*) temp);

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
                    lexer_error(lex, "Ran out of memory.\n");

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
            lexer_error(lex, "Ran out of memory.\n");

            free(str);
            return NULL;
        }

        token_t* token = token_init(string_token, lex->lineNumber, lex->lineIndex, (void*) temp);

        lex->sourceIndex++;

        return token;
    }
    else {
        for ( int i = 0; i < lex->specialTokenLength; i++ ) {
            bool good = true;

            if ( lex->specialTokens[i][0] == current ) {
                for ( int j = lex->sourceIndex; j < strlen(lex->specialTokens[i]); j++ ) {
                    if ( lex->specialTokens[i][j - lex->sourceIndex] != lex->source[j] ) {
                        good = false;
                        break;
                    }
                }
            }
            else {
                good = false;
            }

            if ( good ) {
                char* copy = (char*) malloc((strlen(lex->specialTokens[i]) + 1) * sizeof(char));;
                strcpy(copy, lex->specialTokens[i]);

                lex->sourceIndex += strlen(copy);
                lex->lineIndex += strlen(copy);

                token_t* tok = token_init(special_token, lex->lineNumber, lex->lineIndex, (void*) copy);

                return tok;
            }
        }

        lexer_error(lex, "Unexpected token %c\n", current);
    }

    return NULL;
}
