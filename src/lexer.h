#include <stdbool.h>

typedef enum {
    name_token,
    int_token,
    double_token,
    string_token,
    special_token,
    eof_token
} token_types_t;

typedef struct _token {
    token_types_t type;
    void* value;
    int lineNumber;
    int lineIndex;
} token_t;

typedef struct {
    bool error;

    token_t* current;
    token_t* next;

    const char** specialTokens;
    unsigned int specialTokenLength;

    const char* source;
    unsigned int tokenIndex;
    unsigned int sourceIndex;
    unsigned int lineNumber;
    unsigned int lineIndex;
    unsigned int _sourceLen;
} lexer_state_t;

token_t* token_init(token_types_t type, int lineNumber, int lineIndex, void* data);
void token_debug(token_t* token);
void token_clean(token_t* token);

lexer_state_t* lexer_init(const char* src, const char* specialTokens[]);
void lexer_error(lexer_state_t* lex, const char* error, ...);
void lexer_debug(lexer_state_t* lex);
void lexer_clean(lexer_state_t* lex);

token_t* lexer_cur(lexer_state_t* lex);
token_t* lexer_lookahead(lexer_state_t* lex);
bool lexer_matches(lexer_state_t* lex, token_types_t tokentype);
bool lexer_matches_special(lexer_state_t* lex, const char* special);
bool lexer_lookahead_matches(lexer_state_t* lex, token_types_t tokentype);
bool lexer_lookahead_matches_special(lexer_state_t* lex, const char* special);
token_t* lexer_next(lexer_state_t* lex);
token_t* lexer_nextif(lexer_state_t* lex, token_types_t tokentype);
token_t* lexer_nextif_special(lexer_state_t* lex, const char* token);
token_t* lexer_expect(lexer_state_t* lex, token_types_t tokentype);
token_t* lexer_expect_special(lexer_state_t* lex, const char* token);
token_t* lexer_expectmatch(lexer_state_t* lex, const char* end, const char* start, int linenum);

token_t* _lexer_read_token(lexer_state_t* lex);
void _lexer_tokenize(lexer_state_t* lex);
