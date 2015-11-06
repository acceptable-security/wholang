# wholang
My experiments in lexing, compiling, and assembling.

## Pipeline
The way I've been designing it, the parts of this should at the end look something like

    lexer -> parser -> program state -> assembler -> linker

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
Right now this is undeveloped, and I plan on doing this part last, as it will be the past that will least applicable to
other projects. If all goes as well as it could, the end result will be something that can be used to create other
languages too.

## Program State
The program state doesn't really exist right now, but the way it's planned to be is a nice interface inbetween the
assembler and the parser, presenting a nice interface to generate assembly which could also be useful when extending to
other platforms. The theoretical API that's been formed:

    program_state_t program = program_init();
    int globalref = program_create_global_variable(program, size, type); // Create a global variable
    program_set_variable(program, globalref, value); // Set the global variable's data type.
    int func = program_create_function(program); // Create a function
    int varref = program_create_variable(program, func, size, type); // Create a variable in the function
    program_call_function(program, func, program_get_function("default"), variable1); // Call a library function, program_get_function could be replaced by just passing it a function reference.
    program_function_return(program, varref); // Return the variable varref.

As one could see, it is all very up in the air. It is still undecided whether or not the program state should also keep
track of function names and variable names.

## Assembler
For now I think it'd be cleaner and more reliable to just use something like GAS or NASM, just so the language's progress
isn't halted by something that already exists. Of course, in the future, it would certainly be a neat addition

## Linker
See above. Just use LD for now.

## Building
    gcc lexer.c tester.c
