/**
 * wholang parser
 * wew
 */

 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include "parser.h"

expr_token_t blank_expr = { "", -1, -1, -1 }; // hurr durr im an idiot please kill me

#define DBG_STMT 3
#define DBG_LEX 2
#define DBG_CLN 1

#define DEBUGGING 0
#define DEBUG(MSG, LVL) if ( DEBUGGING == LVL ) printf(MSG "\n");

parser_state_t* parser_init(const char* src) {
    parser_state_t* parser = (parser_state_t*) malloc(sizeof(parser_state_t));

    static const char* specialToken[] = {
        ">=",
        "<=",
        "!=",
        "==",
        "-=",
        "+=",
        "!=",
        "&&",
        "||",
        "//",
        "/*",
        "*/",
        "->",
        "++",
        "--",
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
        ".",
        ",",
        "=",
        "{",
        "}",
        "<",
        ">",
        "var",
        "fn",
        "include",
        "if",
        "while",
        "for",
        "return",
        "#",
        NULL
    };

    parser->src = src;
    parser->lex = lexer_init(src, specialToken);

    return parser;
}

void expr_debug(expr_t* expr) {
    if ( !expr ) {
        return;
    }

    if ( expr->type == expr_infix ) {
        expr_infix_t* infix = (expr_infix_t*) expr->value;

        printf("(");
        expr_debug(infix->lhs);

        printf(" %s ", infix->op.name);

        expr_debug(infix->rhs);
        printf(")");
    }
    else if ( expr->type == expr_suffix ) {
        expr_suffix_t* suffix = (expr_suffix_t*) expr->value;

        expr_debug(suffix->lhs);
        printf("%s", suffix->op.name);
    }
    else if ( expr->type == expr_prefix ) {
        expr_suffix_t* prefix = (expr_suffix_t*) expr->value;

        printf("%s", prefix->op.name);
        expr_debug(prefix->lhs);
    }
    else if ( expr->type == expr_string ) {
        printf("\"%s\"", expr->value);
    }
    else if ( expr->type == expr_name ) {
        printf("%s", expr->value);
    }
    else if ( expr->type == expr_int ) {
        printf("%d", *(int*)(expr->value));
    }
    else if ( expr->type == expr_expr ) {
        printf("(");
        expr_debug(expr->value);
        printf(")");
    }
    else if ( expr->type == expr_call ) {
        expr_call_t* call = expr->value;

        printf("%s", call->name);
        printf("(");

        for ( int i = 0; i < call->len; i++ ) {
            expr_debug(call->args[i]);

            if ( i < call->len - 1 ) {
                printf(", ");
            }
        }

        printf(")");
    }
}

void expr_clean(expr_t* expr) {
    if ( !expr ) {
        return;
    }

    if ( expr->type == expr_infix ) {
        DEBUG("CLEAN INFIX", DBG_CLN)

        if ( ((expr_infix_t*) expr->value)->lhs != NULL )
            expr_clean(((expr_infix_t*) expr->value)->lhs);

        if ( ((expr_infix_t*) expr->value)->rhs != NULL )
            expr_clean(((expr_infix_t*) expr->value)->rhs);

        free(expr->value);
    }
    else if ( expr->type == expr_prefix ) {
        DEBUG("CLEAN PREFIX", DBG_CLN)

        expr_clean(((expr_prefix_t*) expr->value)->lhs);
        free(expr->value);
    }
    else if ( expr->type == expr_suffix ) {
        DEBUG("CLEAN SUFFIX", DBG_CLN)

        expr_clean(((expr_suffix_t*) expr->value)->lhs);
        free(expr->value);
    }
    else if ( expr->type == expr_expr ) {
        DEBUG("CLEAN EXPRESSION", DBG_CLN)

        expr_clean((expr_t*) expr->value);
    }
    else if ( expr->type == expr_call ) {
        DEBUG("CLEAN CALL", DBG_CLN)

        expr_call_t* call = expr->value;

        for ( int i = 0; i < call->len; i++ ){
            expr_clean(call->args[i]);
        }

        free(call->args);
        free((void*) call->name);
        free(expr->value);
    }
    else {
        DEBUG("CLEAN VALUE", DBG_CLN)

        free(expr->value);
    }

    free(expr);
}

expr_token_t _expr_token_info(char* t, int fix) {
    int j = 0;

    while ( expr_type[fix][j].pres != -1 ) {
        if ( strcmp(expr_type[fix][j].name, t) == 0 ) {
            return expr_type[fix][j];
        }

        j++;
    }

    return blank_expr;
}

expr_t* parser_read_expr(parser_state_t* parser, int pres) {
    expr_t* left = (expr_t*) malloc(sizeof(expr_t));
    left->value = NULL;
    left->type = 0;

    token_t* cur = lexer_cur(parser->lex);

    if ( left == NULL || cur == NULL || parser->lex->error ) {
        goto error;
    }

    if ( cur->type == eof_token ) {
        free(left);
        return NULL;
    }

    if ( cur->type == special_token ) {
        if ( strcmp(cur->value, "(") == 0 ) {
            cur = NULL;

            DEBUG("PARSING EXPR", DBG_LEX)

            MUST_BE(parser->lex, "(", {})

            left->value = parser_read_expr(parser, 0);
            left->type = expr_expr;

            if ( left->value == NULL ) {
                goto error;
            }

            MUST_BE(parser->lex, ")", {
                expr_clean(left->value);
            })
        }
        else {
            expr_token_t info = _expr_token_info(cur->value, 0);

            if ( info.fix == 0 ) {
                DEBUG("PARSING PRFIX", DBG_LEX)

                token_clean(lexer_next(parser->lex));
                cur = NULL;

                expr_t* rhs = parser_read_expr(parser, info.pres);

                if ( rhs == NULL ) {
                    goto error;
                }

                expr_prefix_t* prfx = (expr_prefix_t*) malloc(sizeof(expr_prefix_t));

                if ( prfx == NULL ) {
                    expr_clean(rhs);
                    goto error;
                }

                prfx->op = info;

                if ( rhs->type == expr_infix ) {
                    expr_infix_t* tmp = (expr_infix_t*) rhs->value;

                    while ( tmp->lhs->type == expr_infix) {
                        tmp = tmp->lhs->value;
                    }

                    prfx->lhs = tmp->lhs;

                    expr_t* t_expr = (expr_t*) malloc(sizeof(expr_t));

                    if ( t_expr == NULL ) {
                        expr_clean(rhs);
                        free(prfx);

                        goto error;
                    }

                    t_expr->value = prfx;
                    t_expr->type = expr_prefix;

                    tmp->lhs = t_expr;

                    left->type = expr_infix;
                    left->value = rhs->value;
                }
                else {
                    prfx->lhs = rhs;

                    left->type = expr_prefix;
                    left->value = prfx;
                }
            }
        }
    }
    else if ( cur->type == int_token) {
        DEBUG("PARSING INT", DBG_LEX)

        cur = lexer_next(parser->lex);

        left->value = (int*) malloc(sizeof(int));

        if ( left->value == NULL ) {
            goto error;
        }

        *(int*)left->value = *(int*)cur->value;
        left->type = expr_int;

        token_clean(cur);
        cur = NULL;
    }
    else if ( cur->type == double_token ) {
        DEBUG("PARSING TOKEN", DBG_LEX)

        cur = lexer_next(parser->lex);

        left->value = (double*) malloc(sizeof(double));

        if ( left->value == NULL ) {
            goto error;
        }

        *(double*)left->value = *(double*)cur->value;
        left->type = expr_double;

        token_clean(cur);
        cur = NULL;
    }
    else if ( cur->type == name_token ) {
        cur = lexer_next(parser->lex);
        token_t* next;

        if ( IF_NEXT(parser->lex, "(", next) ) {
            DEBUG("PARSING CALL", DBG_LEX)

            token_clean(next);

            expr_call_t* call = (expr_call_t*) malloc(sizeof(expr_call_t));

            if ( call == NULL ) {
                goto error;
            }

            call->name = strdup(cur->value);

            if ( call->name == NULL ) {
                free(call);
                goto error;
            }

            call->alloc = 8;
            call->len = 0;
            call->args = INIT_LIST(expr_t);

            if ( call->args == NULL ) {
                free(call);
                free((void*) call->name);
                goto error;
            }

            expr_t* expr = parser_read_expr(parser, 0);

            if ( expr == NULL ) {
                free((void*) call->name);
                free(call->args);
                free(call);

                goto error;
            }

            while ( expr != NULL && expr->value != NULL ) {
                ADD_ARG(call->args, call->len, call->alloc, expr, {
                    for ( int i = 0; i < call->len; i++ ) {
                        expr_clean(call->args[i]);
                    }

                    free(call->args);
                    free((void*) call->name);
                    free(call);
                })

                if ( IF_NEXT(parser->lex, ",", next) ) {
                    token_clean(next);

                    expr = parser_read_expr(parser, 0);

                    if ( expr == NULL ) {
                        for ( int i = 0; i < call->len; i++ ) {
                            expr_clean(call->args[i]);
                        }

                        free((void*) call->name);
                        free(call->args);
                        free(call);

                        goto error;
                    }
                }
                else {
                    expr = NULL;
                }
            }

            if ( expr != NULL && expr->value == NULL ) {
                free(expr);
            }

            MUST_BE(parser->lex, ")", {
                for ( int i = 0; i < call->len; i++ ) {
                    expr_clean(call->args[i]);
                }

                free((void*) call->name);
                free(call->args);
                free(call);
            })

            left->type = expr_call;
            left->value = call;
        }
        else if ( IF_NEXT(parser->lex, ".", next) ) {
            DEBUG("PARSING DEREF", DBG_LEX)

            token_clean(next);
        }
        else if ( IF_NEXT(parser->lex, "->", next) ) {
            DEBUG("PARSING DEREF", DBG_LEX)
            token_clean(next);
        }
        else {
            DEBUG("PARSING VARIABLE", DBG_LEX)
            left->type = expr_name;
            left->value = strdup(cur->value);

            if ( left->value == NULL ) {
                goto error;
            }
        }

        token_clean(cur);
        cur = NULL;
    }
    else if ( cur->type == string_token ) {
        DEBUG("PARSING STRING", DBG_LEX)

        left->type = expr_string;
        left->value = strdup(cur->value);

        token_clean(lexer_next(parser->lex));
        cur = NULL;
    }

    token_t* op;
    primary:
    op = lexer_cur(parser->lex);

    if ( op == NULL ) {
        return left;
    }

    if ( op->type == eof_token ) {
        return left;
    }

    if ( op->type == special_token ) {
        expr_token_t info = _expr_token_info(op->value, 1);

        if ( info.pres < 0 ) {
            info = _expr_token_info(op->value, 2);
        }

        if ( info.fix == 1 ) {
            DEBUG("PARSING INFIX", DBG_LEX)
            token_clean(lexer_next(parser->lex));

            expr_t* rhs = parser_read_expr(parser, info.pres);

            if ( rhs == NULL ) {
                expr_clean(left);
                left = NULL;
                goto error;
            }

            expr_infix_t* inf = (expr_infix_t*) malloc(sizeof(expr_infix_t));

            if ( inf == NULL ) {
                expr_clean(left);
                left = NULL;
                expr_clean(rhs);
                goto error;
            }

            if ( rhs->type == expr_infix ) {
                expr_infix_t* tmp = (expr_infix_t*) rhs->value;

                // ok this is really ugly.
                // this pretty much takes RHS
                // and makes INF's RHS into RHS's LHS
                // and RHS's LHS into INF
                // :^) if that makes sense
                if ( tmp->op.pres < info.pres ) {
                    inf->lhs = left;
                    inf->rhs = tmp->lhs;

                    expr_t* tmp2 = (expr_t*) malloc(sizeof(expr_t));

                    if ( tmp2 == NULL ) {
                        free(inf);
                        expr_clean(left);
                        left = NULL;
                        expr_clean(rhs);
                        goto error;
                    }

                    tmp2->type = expr_infix;
                    tmp2->value = inf;

                    tmp->lhs = tmp2;
                    inf->op = info;

                    inf = tmp;

                    free(rhs);
                }
                else {
                    inf->lhs = left;
                    inf->rhs = rhs;
                    inf->op = info;
                }
            }
            else {
                inf->lhs = left;
                inf->rhs = rhs;
                inf->op = info;
            }

            left = (expr_t*) malloc(sizeof(expr_t));

            if ( left == NULL ) {
                expr_clean(inf->lhs);
                expr_clean(inf->rhs);
                free(inf);
                goto error;
            }

            left->type = expr_infix;
            left->value = inf;
        }
        else if ( info.fix == 2 ) {
            DEBUG("PARSING SUFFIX", DBG_LEX)
            token_clean(lexer_next(parser->lex));

            expr_suffix_t* suf = (expr_suffix_t*) malloc(sizeof(expr_suffix_t));

            if ( suf == NULL ) {
                goto error;
            }

            suf->op = info;
            suf->lhs = left;

            left = (expr_t*) malloc(sizeof(expr_t));

            if ( left == NULL ) {
                expr_clean(suf->lhs);
                free(suf);
                goto error;
            }

            left->value = suf;
            left->type = expr_suffix;

            goto primary;
        }
    }

    return left;

    error:

    if ( left != NULL ) {
        free(left);
    }

    if ( cur != NULL ) {
        token_clean(cur);
    }

    return NULL;
}

void stmt_debug(stmt_t* stmt) {
    if ( stmt == NULL) {
        return;
    }

    if ( stmt->type == call_stmt ) {
        expr_debug(stmt->data);
        printf(";\n");
    }
    else if ( stmt->type == vardec_stmt ) {
        printf("var %s : %s = ", ((vardec_stmt_t*) stmt->data)->name,
                                 ((vardec_stmt_t*) stmt->data)->type);
        expr_debug(((vardec_stmt_t*) stmt->data)->value);
        printf(";\n");
    }
    else if ( stmt->type == if_stmt ) {
        printf("if ( ");
        expr_debug(((if_stmt_t*) stmt->data)->expr);
        printf(" ) ");
        block_debug(((if_stmt_t*) stmt->data)->block);
    }
    else if ( stmt->type == for_stmt ) {
        printf("for ( ");
        expr_debug(((for_stmt_t*) stmt->data)->expr[0]);
        printf("; ");
        expr_debug(((for_stmt_t*) stmt->data)->expr[1]);
        printf("; ");
        expr_debug(((for_stmt_t*) stmt->data)->expr[2]);
        printf(" ) ");
        block_debug(((for_stmt_t*) stmt->data)->block);
    }
    else if ( stmt->type == while_stmt ) {
        printf("while ( ");
        expr_debug(((while_stmt_t*) stmt->data)->expr);
        printf(" ) ");
        block_debug(((while_stmt_t*) stmt->data)->block);
    }
    else if ( stmt->type == varset_stmt ) {
        if ( ((varset_stmt_t*) stmt->data)->mod ) {
            printf("%s %c= ", ((varset_stmt_t*) stmt->data)->name, ((varset_stmt_t*) stmt->data)->mod);
        }
        else {
            printf("%s = ", ((varset_stmt_t*) stmt->data)->name);
        }

        expr_debug(((varset_stmt_t*) stmt->data)->value);
        printf(";\n");
    }
    else if ( stmt->type == ret_stmt ) {
        printf("return ");
        expr_debug(stmt->data);
        printf(";\n");
    }
}

void stmt_clean(stmt_t* stmt) {
    if ( stmt == NULL ) {
        return;
    }

    if ( stmt->type == call_stmt ) {
        expr_clean((expr_t*) stmt->data);
    }
    else if ( stmt->type == if_stmt ) {
        expr_clean(((if_stmt_t*) stmt->data)->expr);
        block_clean(((if_stmt_t*) stmt->data)->block);
        free(stmt->data);
    }
    else if ( stmt->type == while_stmt ) {
        expr_clean(((while_stmt_t*) stmt->data)->expr);
        block_clean(((while_stmt_t*) stmt->data)->block);
        free(stmt->data);
    }
    else if ( stmt->type == for_stmt ) {
        expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
        expr_clean(((for_stmt_t*) stmt->data)->expr[1]);
        expr_clean(((for_stmt_t*) stmt->data)->expr[2]);
        block_clean(((for_stmt_t*) stmt->data)->block);
        free(stmt->data);
    }
    else if ( stmt->type == vardec_stmt ) {
        free(((vardec_stmt_t*) stmt->data)->type);
        free(((vardec_stmt_t*) stmt->data)->name);

        if ( ((vardec_stmt_t*) stmt->data)->value ) {
            expr_clean(((vardec_stmt_t*) stmt->data)->value);
        }

        free(stmt->data);
    }
    else if ( stmt->type == varset_stmt ) {
        free(((varset_stmt_t*) stmt->data)->name);
        expr_clean(((varset_stmt_t*) stmt->data)->value);

        free(stmt->data);
    }
    else if ( stmt->type == ret_stmt ) {
        expr_clean(stmt->data);
    }

    free(stmt);
}

stmt_t* parser_read_stmt(parser_state_t* parser) {
    stmt_t* stmt = (stmt_t*) malloc(sizeof(stmt_t));
    token_t* cur = NULL;

    if ( stmt == NULL ) {
        lexer_error(parser->lex, "Failed to allocate space for a statement\n");
        goto error;
    }

    cur = lexer_cur(parser->lex);

    if ( cur == NULL || cur->value == NULL || parser->lex->error) {
        goto error;
    }

    if ( cur->type == eof_token ) {
        return NULL;
    }

    if ( cur->type == special_token ) {
        if ( strcmp(cur->value, "if") == 0 ) {
            token_clean(lexer_next(parser->lex));
            MUST_BE(parser->lex, "(", {})

            stmt->type = if_stmt;
            stmt->data = (if_stmt_t*) malloc(sizeof(if_stmt_t));

            if ( stmt->data == NULL ) {
                lexer_error(parser->lex, "Failed to allocate space for an if statement\n");
                goto error;
            }

            ((if_stmt_t*) stmt->data)->expr = parser_read_expr(parser, 0);

            if ( ((if_stmt_t*) stmt->data)->expr == NULL ) {
                free(stmt->data);
                goto error;
            }

            MUST_BE(parser->lex, ")", {
                expr_clean(((if_stmt_t*) stmt->data)->expr);
                free(stmt->data);
            })

            ((if_stmt_t*) stmt->data)->block = parser_read_block(parser);

            if ( ((if_stmt_t*) stmt->data)->block == NULL ) {
                expr_clean(((if_stmt_t*) stmt->data)->expr);
                free(stmt->data);
                goto error;
            }

            return stmt;
        }
        else if ( strcmp(cur->value, "while") == 0 ) {
            token_clean(lexer_next(parser->lex));
            MUST_BE(parser->lex, "(", {})

            stmt->type = while_stmt;
            stmt->data = (while_stmt_t*) malloc(sizeof(while_stmt_t));

            if ( stmt->data == NULL ) {
                goto error;
            }

            ((while_stmt_t*) stmt->data)->expr = parser_read_expr(parser, 0);

            if ( ((while_stmt_t*) stmt->data)->expr == NULL ) {
                free(stmt->data);
                goto error;
            }

            MUST_BE(parser->lex, ")", {
                expr_clean(((while_stmt_t*) stmt->data)->expr);
                free(stmt->data);
                goto error;
            })

            ((while_stmt_t*) stmt->data)->block = parser_read_block(parser);

            if ( ((while_stmt_t*) stmt->data)->block == NULL ) {
                expr_clean(((while_stmt_t*) stmt->data)->expr);
                free(stmt->data);
                goto error;
            }

            return stmt;
        }
        else if ( strcmp(cur->value, "for") == 0 ) {
            token_clean(lexer_next(parser->lex));
            MUST_BE(parser->lex, "(", {})

            stmt->type = for_stmt;
            stmt->data = (for_stmt_t*) malloc(sizeof(for_stmt_t));

            if ( stmt->data == NULL ) {
                lexer_error(parser->lex, "Failed to allocate space for a for statement\n");
                goto error;
            }

            ((for_stmt_t*) stmt->data)->expr[0] = parser_read_expr(parser, 0);

            if ( ((for_stmt_t*) stmt->data)->expr[0] == NULL ) {
                free(stmt->data);
                goto error;
            }

            MUST_BE(parser->lex, ";", {
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                free(stmt->data);
            })

            ((for_stmt_t*) stmt->data)->expr[1] = parser_read_expr(parser, 0);

            if ( ((for_stmt_t*) stmt->data)->expr[1] == NULL ) {
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                free(stmt->data);
                goto error;
            }

            MUST_BE(parser->lex, ";", {
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[1]);
                free(stmt->data);
            })

            ((for_stmt_t*) stmt->data)->expr[2] = parser_read_expr(parser, 0);

            if ( ((for_stmt_t*) stmt->data)->expr[2] == NULL ) {
                expr_clean(((for_stmt_t*) stmt->data)->expr[1]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                free(stmt->data);
                goto error;
            }

            MUST_BE(parser->lex, ")", {
                expr_clean(((for_stmt_t*) stmt->data)->expr[1]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[2]);
                free(stmt->data);
            })

            ((for_stmt_t*) stmt->data)->block = parser_read_block(parser);

            if ( ((for_stmt_t*) stmt->data)->block == NULL ) {
                expr_clean(((for_stmt_t*) stmt->data)->expr[1]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[0]);
                expr_clean(((for_stmt_t*) stmt->data)->expr[2]);
                free(stmt->data);
                goto error;
            }

            return stmt;
        }
        else if ( strcmp(cur->value, "var") == 0 ) {
            token_clean(lexer_next(parser->lex));

            vardec_stmt_t* dec = (vardec_stmt_t*) malloc(sizeof(vardec_stmt_t));

            if ( dec == NULL ) {
                lexer_error(parser->lex, "Failed to allocate enough space for a variable declaration.\n");
                goto error;
            }

            NEXT_NAME(parser->lex, dec->name)

            MUST_BE(parser->lex, ":", {
                free(dec->name);
                free(dec);
            })

            NEXT_NAME(parser->lex, dec->type)

            token_t* tmp;
            if ( IF_NEXT(parser->lex, "=", tmp) ) {
                token_clean(tmp);
                dec->value = parser_read_expr(parser, 0);
            }

            stmt->data = dec;
            stmt->type = vardec_stmt;

            MUST_BE(parser->lex, ";", {
                if ( dec->value != NULL ) {
                    expr_clean(dec->value);
                }

                free(dec);
            })

            return stmt;
        }
        else if ( strcmp(cur->value, "return") == 0 ) {
            token_clean(lexer_next(parser->lex));

            stmt->type = ret_stmt;
            stmt->data = parser_read_expr(parser, 0);

            MUST_BE(parser->lex, ";", {
                if ( stmt->data != NULL ) {
                    expr_clean(stmt->data);
                }
            })

            return stmt;
        }
        else {
            free(stmt);
            return NULL;
        }
    }
    else if ( cur->type == name_token ) {
        token_t* lookahead = lexer_lookahead(parser->lex);

        if ( lookahead == NULL || parser->lex->error ) {
            goto error;
        }

        if ( lookahead->type != special_token ) {
            lexer_error(parser->lex, "Expected either a ( or an equator\n");
            goto error;
        }

        if ( strcmp((char*) lookahead->value, "(") == 0 ) {
            stmt->type = call_stmt;
            stmt->data = parser_read_expr(parser, 0);

            if ( stmt->data == NULL ) {
                goto error;
            }
        }
        else {
            if ( strcmp((char*) lookahead->value, "=") == 0 || ((((char*) lookahead->value)[1] == '=') &&
                                                                (((char*) lookahead->value)[2] == '\0')) ) {
                lexer_next(parser->lex);
                lexer_next(parser->lex);

                expr_t* value = parser_read_expr(parser, 0);

                if ( value == NULL ) {
                    goto error;
                }

                varset_stmt_t* varset = (varset_stmt_t*) malloc(sizeof(varset_stmt_t));
                varset->value = value;
                varset->name = strdup(cur->value);

                if ( strlen(lookahead->value) > 1 ) {
                    varset->mod = ((char*) lookahead->value)[0];
                }
                else {
                    varset->mod = 0;
                }

                stmt->type = varset_stmt;
                stmt->data = varset;

                token_clean(cur);
                token_clean(lookahead);
            }
            else {
                lexer_error(parser->lex, "Unexpected %s", lookahead->value);
                goto error;
            }
        }

        MUST_BE(parser->lex, ";", {
            if ( stmt->data != NULL ) {
                expr_clean(stmt->data);
            }
        })

        return stmt;
    }

    error:
    if ( stmt != NULL ) {
        free(stmt);
    }

    return NULL;
}

void block_clean(block_t* block) {
    if ( block == NULL ) {
        return;
    }

    for ( int i = 0; i < block->len; i++ ) {
        stmt_clean(block->stmts[i]);
    }

    free(block->stmts);
    free(block);
}

void block_debug(block_t* block) {
    if ( block == NULL ) {
        return;
    }

    printf("{\n");
    for ( int i = 0; i < block->len; i++ ) {
        stmt_debug(block->stmts[i]);
    }
    printf("}\n");
}

block_t* parser_read_block(parser_state_t* parser) {
    block_t* block = (block_t*) malloc(sizeof(block_t));
    block->stmts = NULL;
    stmt_t* stmt = NULL;

    if ( block == NULL ) {
        lexer_error(parser->lex, "Unable to allocate enough space for a block to be read.\n");
        goto error;
    }

    MUST_BE(parser->lex, "{", {})

    block->alloc = MALLOC_CHUNK;
    block->len = 0;
    block->stmts = INIT_LIST(stmt_t);

    stmt = parser_read_stmt(parser);

    while ( stmt != NULL ) {
        ADD_ARG(block->stmts, block->len, block->alloc, stmt, {})
        stmt = parser_read_stmt(parser);
    }

    MUST_BE(parser->lex, "}", {})

    return block;

    error:

    if ( block->stmts != NULL ) {
        for ( int i = 0; i < block->len; i++ ) {
            if ( block->stmts[i] != NULL ) {
                stmt_clean(block->stmts[i]);
            }
        }

        free(block->stmts);
    }

    if ( block != NULL ) {
        free(block);
    }

    if ( stmt != NULL ) {
        stmt_clean(stmt);
    }

    return NULL;
}

void function_debug(function_t* fn) {
    if ( fn == NULL ) {
        return;
    }

    printf("fn %s(", fn->name);

    for ( int i = 0; i < fn->len; i++ ) {
        printf("%s : %s", fn->args[i]->name, fn->args[i]->type);

        if ( i < i - 1 ) {
            printf(", ");
        }
    }

    printf(") : %s ", fn->ret_type);

    block_debug(fn->block);
}

void function_clean(function_t* fn) {
    if ( fn == NULL ) {
        return;
    }

    if ( fn->name ) free((void*) fn->name);
    if ( fn->ret_type) free((void*) fn->ret_type);

    if ( fn->args != NULL ) {
        for ( int i = 0; i < fn->len; i++ ) {
            // This shouldn't ever happen
            // But you can never be too safe :)
            if ( fn->args[i] != NULL ) {
                if ( fn->args[i]->name != NULL ) {
                    free((void*) fn->args[i]->name);
                }

                if ( fn->args[i]->type != NULL ) {
                    free((void*) fn->args[i]->type);
                }

                free(fn->args[i]);
            }
        }

        free(fn->args);
    }

    if ( fn->block != NULL ) {
        block_clean(fn->block);
    }

    free(fn);
}

function_t* parser_read_function(parser_state_t* parser) {
    token_t* cur = lexer_expect_special(parser->lex, "fn");

    if ( cur == NULL || parser->lex->error ) {
        return NULL;
    }

    token_clean(cur);

    function_t* fn = (function_t*) malloc(sizeof(function_t));
    fn->name = NULL;
    fn->ret_type = NULL;
    fn->args = INIT_LIST(function_arg_t);
    fn->alloc = MALLOC_CHUNK;
    fn->len = 0;

    if ( fn == NULL ) {
        lexer_error(parser->lex, "Unable to allocate enough space for a function.\n");
        goto error;
    }

    if ( fn->args == NULL ) {
        free(fn);
        lexer_error(parser->lex, "Unable to allocate enough space for an argument list.\n");
        goto error;
    }

    NEXT_NAME(parser->lex, fn->name)

    MUST_BE(parser->lex, "(", {})

    while ( !(IF_NEXT(parser->lex, ")", cur)) ) {
        char* name;
        char* type;

        NEXT_NAME(parser->lex, name)
        NEXT_NAME(parser->lex, type)

        if ( name == NULL ) {
            if ( type != NULL ) {
                free(type);
            }

            goto error;
        }

        if ( type == NULL ) {
            if ( name != NULL ) {
                free(name);
            }

            goto error;
        }

        function_arg_t* arg = (function_arg_t*) malloc(sizeof(function_arg_t));

        if ( arg == NULL ) {
            lexer_error(parser->lex, "Unable to allocate enough memory for a function argument.\n");
            goto error;
        }

        ADD_ARG(fn->args, fn->len, fn->alloc, arg, {
            if ( arg->name) free((void*) arg->name);
            if ( arg->type) free((void*) arg->type);
            free(arg);
        })

        MUST_BE(parser->lex, ",", {})
    }

    token_clean(cur);

    if ( IF_NEXT(parser->lex, ":", cur) ) {
        token_clean(cur);
        NEXT_NAME(parser->lex, fn->ret_type)

        if ( fn->ret_type == NULL || strcmp(fn->ret_type, "") == 0 ) {
            goto error;
        }
    }
    else {
        fn->ret_type = strdup("void"); // fuggit
    }

    fn->block = parser_read_block(parser);

    if ( fn->block == NULL ) {
        goto error;
    }

    return fn;

    error:
    if ( fn != NULL && fn->args != NULL ) {
        for ( int i = 0; i < fn->len; i++ ) {
            // This shouldn't ever happen
            // But you can never be too safe :)
            if ( fn->args[i] != NULL ) {
                if ( fn->args[i]->name != NULL ) {
                    free((void*) fn->args[i]->name);
                }

                if ( fn->args[i]->type != NULL ) {
                    free((void*) fn->args[i]->type);
                }

                free(fn->args[i]);
            }
        }

        free(fn->args);
    }

    if ( fn->name != NULL ) {
        free((void*) fn->name);
    }

    if ( fn->ret_type != NULL ) {
        free((void*) fn->ret_type);
    }

    if ( fn != NULL ) {
        free(fn);
    }

    // We don't clean up blocks, because the only instance where we go to error around blocks is if blocks are errors

    return NULL;
}

void parser_read(parser_state_t* parser) {
    token_t* cur;

    if ( lexer_matches_special(parser->lex, "fn") ) {
        function_t* fn = parser_read_function(parser);

        if ( fn == NULL ) {
            goto error;
        }

        // do something
        // :^)
    }
    else if ( lexer_matches_special(parser->lex, "var") )  {
        // variable declaration
    }
    else if ( IF_NEXT(parser->lex, "#", cur) ) {
        token_clean(cur);
        cur = NULL;

        if ( IF_NEXT(parser->lex, "include", cur) ) {
            token_clean(cur);
            cur = NULL;

            if ( lexer_matches_special(parser->lex, "\"") ) {
                // include a file
            }
            else if ( lexer_matches_special(parser->lex, "<") ) {
                // include a global file
            }
            else {
                goto error;
            }
        }
    }
    else if ( lexer_matches_special(parser->lex, "//") ) {
        // comment
    }
    else if ( lexer_matches_special(parser->lex, "/*") ) {
        // comment
    }

    error:
    lexer_clean(parser->lex);
    return;
}
