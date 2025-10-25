/**
 * @file    parser.c
 * @brief   Syntaktická analýza (rekurzivní sestup + precedenční pro výrazy)
 * a generování AST pro jazyk IFJ25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "lex_scanner.h"
#include "parser.h"
#include "ast.h"

#ifdef PARSE_TRACE
#define TRACEF(...)                   \
    do                                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
    } while (0)
#else
#define TRACEF(...) \
    do              \
    {               \
    } while (0)
#endif

extern Token ifj_get_token(void);

// Parser state
int ifj_error_code = 0;
static Token ifj_cur;
static bool ifj_seen_main = false;

static void next(void)
{
    // Před načtením nového tokenu uvolníme starý
    free_token_lexeme(&ifj_cur);

    ifj_cur = ifj_get_token();
    TRACEF("[TOK l%03d] %-12s %s\n",
           ifj_get_line(),
           convert(ifj_cur.type),
           (ifj_cur.lexeme ? ifj_cur.lexeme : ""));
}

static bool expect(TokenType t, const char *ctx)
{
    if (ifj_cur.type != t)
    {
        fprintf(stderr, "[SYNTAX] l%d: expected %s ('%s'), got %s\n", ifj_get_line(), convert(t), ctx ? ctx : "", convert(ifj_cur.type));
        ifj_error_code = 2;
        return false;
    }
    next();
    return true;
}

static void skip_eol_star(void)
{
    while (ifj_cur.type == EOL)
        next();
}

enum EOL_POLICY
{
    EOL_ZERO_OR_MORE,
    EOL_ONE_EXACT
};

static bool consume_eol(enum EOL_POLICY p)
{
    if (p == EOL_ZERO_OR_MORE)
    {
        skip_eol_star();
        return true;
    }
    if (ifj_cur.type != EOL)
    {
        fprintf(stderr,
                "[SYNTAX] l%d: expected end-of-line, got %s\n",
                ifj_get_line(), convert(ifj_cur.type));
        ifj_error_code = 2;
        return false;
    }
    do
    {
        next();
    } while (ifj_cur.type == EOL);
    return true;
}

typedef enum
{
    PREC_LOWEST = 0,
    PREC_EQ_OP,  // == !=
    PREC_IS_OP,  // is
    PREC_REL_OP, // < > <= >=
    PREC_ADD_OP, // + -
    PREC_MUL_OP  // * /
} IfjPrec;

// Forward deklarace
static bool parse_prolog(void);
static AstNode *parse_class(void);
static bool parse_func_list(AstNodeProgram *prog);
static AstNode *parse_funcdef(void);
static AstNodeBlock *parse_block(void);
static AstNode *parse_stmt_list(AstNodeBlock *block);
static AstNode *parse_stmt(void);
static AstNode *parse_vardecl_stmt(void);
static AstNode *parse_assign_or_call_stmt(void);
static AstNode *parse_if_stmt(void);
static AstNode *parse_while_stmt(void);
static AstNode *parse_return_stmt(void);
static AstNodeList *parse_call_args_terms(void);
static AstNode *parse_expr(void);
static AstNode *parse_expr_bp(IfjPrec minbp);
static AstNode *parse_primary(void);
static int lbp_of(TokenType t);

// Public entry
AstNode *ifj_parse_program(void)
{
    ifj_seen_main = false;
    ifj_error_code = 0;
    next();

    if (!parse_prolog())
        return NULL;

    AstNode *program_node = parse_class();
    if (program_node == NULL)
        return NULL;

    if (!ifj_seen_main)
    {
        fprintf(stderr, "[SEMANTIC] missing static main() without params (error 3)\n");
        ifj_error_code = 3;
        free_ast_node(program_node);
        return NULL;
    }

    return program_node;
}

static bool can_start_expr(TokenType t)
{
    switch (t)
    {
    case INT:
    case FLOAT:
    case STRING:
    case KEYWORD_null:
    case IDENTIFIER_LOCAL:
    case IDENTIFIER_GLOBAL:
    case L_ROUND:
    case IFJ_READ_STR:
    case IFJ_READ_NUM:
    case IFJ_WRITE:
    case IFJ_FLOOR:
    case IFJ_STR:
    case IFJ_LENGTH:
    case IFJ_SUBSTRING:
    case IFJ_STRCMP:
    case IFJ_ORD:
    case IFJ_CHR:
        return true;
    default:
        return false;
    }
}

static bool parse_prolog(void)
{
    skip_eol_star();

    if (ifj_cur.type != KEYWORD_import)
    {
        fprintf(stderr, "[SYNTAX] missing prolog 'import' (error 2)\n");
        ifj_error_code = 2;
        return false;
    }
    next();

    if (ifj_cur.type != STRING || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "ifj25") != 0)
    {
        fprintf(stderr, "[SYNTAX] expected string literal \"ifj25\" after import (error 2)\n");
        ifj_error_code = 2;
        return false;
    }
    next();

    if (!expect(KEYWORD_for, "for"))
    {
        return false;
    }

    if (ifj_cur.type == KEYWORD_Ifj)
    {
        next();
    }
    else if (ifj_cur.type == IDENTIFIER_LOCAL && ifj_cur.lexeme && strcmp(ifj_cur.lexeme, "Ifj") == 0)
    {
        next();
    }
    else
    {
        fprintf(stderr, "[SYNTAX] expected 'Ifj' after 'for' (error 2)\n");
        ifj_error_code = 2;
        return false;
    }

    return consume_eol(EOL_ONE_EXACT);
}

static bool accept(TokenType t)
{
    if (ifj_cur.type == t)
    {
        next();
        return true;
    }
    return false;
}


static AstNodeList *parse_call_args_terms(void)
{
    if (!expect(L_ROUND, "("))
        return NULL;

    AstNodeList *args = create_list();

    if (ifj_cur.type != R_ROUND)
    {
        AstNode *arg_expr = parse_expr();
        if (!arg_expr)
        {
            free_ast_node((AstNode *)args);
            return NULL;
        }
        add_to_list(args, arg_expr);

        while (accept(COMMA))
        {
            arg_expr = parse_expr();
            if (!arg_expr)
            {
                free_ast_node((AstNode *)args);
                return NULL;
            }
            add_to_list(args, arg_expr);
        }
    }

    if (!expect(R_ROUND, ")"))
    {
        free_ast_node((AstNode *)args);
        return NULL;
    }

    return args;
}

static AstNode *parse_class(void)
{
    if (!expect(KEYWORD_class, "class"))
        return NULL;

    if (ifj_cur.type != IDENTIFIER_LOCAL || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "Program") != 0)
    {
        fprintf(stderr, "[SYNTAX] expected class name 'Program' (error 2)\n");
        ifj_error_code = 2;
        return NULL;
    }
    next();

    if (!expect(L_CURLY, "{"))
        return NULL;
    consume_eol(EOL_ZERO_OR_MORE);

    AstNodeProgram *prog = create_program();

    if (!parse_func_list(prog))
    {
        free_ast_node((AstNode *)prog);
        return NULL;
    }

    if (!expect(R_CURLY, "}"))
    {
        free_ast_node((AstNode *)prog);
        return NULL;
    }
    consume_eol(EOL_ZERO_OR_MORE);
    return (AstNode *)prog;
}

static bool parse_func_list(AstNodeProgram *prog)
{
    while (ifj_cur.type == KEYWORD_static)
    {
        AstNodeFuncDef *func = (AstNodeFuncDef *)parse_funcdef();
        if (!func)
            return false;

        add_func_to_program(prog, func);

        consume_eol(EOL_ZERO_OR_MORE);
    }
    return true;
}

static AstNode *parse_funcdef(void)
{
    if (!expect(KEYWORD_static, "static"))
        return NULL;

    if (ifj_cur.type != IDENTIFIER_LOCAL)
    {
        fprintf(stderr, "[SYNTAX] expected function identifier after 'static'\n");
        ifj_error_code = 2;
        return NULL;
    }
    Token fname_id = ifj_cur;
    ifj_cur.lexeme = NULL;
    next();

    skip_eol_star();

    // 1) GETTER
    if (ifj_cur.type == L_CURLY)
    {
        AstNodeBlock *body = parse_block();
        if (!body)
        {
            free_token_lexeme(&fname_id);
            return NULL;
        }
        return (AstNode *)create_func_def(fname_id, create_list(), body);
    }

    // 2) FUNKCE
    if (accept(L_ROUND))
    {
        AstNodeList *params = create_list();
        int arity = 0;

        if (ifj_cur.type != R_ROUND)
        {
            do
            {
                if (ifj_cur.type != IDENTIFIER_LOCAL)
                {
                    fprintf(stderr, "[SYNTAX] expected parameter name\n");
                    ifj_error_code = 2;
                    free_ast_node((AstNode *)params);
                    free_token_lexeme(&fname_id);
                    return NULL;
                }

                add_to_list(params, create_variable(ifj_cur));
                ifj_cur.lexeme = NULL;

                arity++;
                next();
            } while (accept(COMMA));

            if (!expect(R_ROUND, ")"))
            {
                free_ast_node((AstNode *)params);
                free_token_lexeme(&fname_id);
                return NULL;
            }
        }
        else
        {
            next(); // sežer R_ROUND
        }

        if (fname_id.lexeme && strcmp(fname_id.lexeme, "main") == 0 && arity == 0)
            ifj_seen_main = true;

        AstNodeBlock *body = parse_block();
        if (!body)
        {
            free_ast_node((AstNode *)params);
            free_token_lexeme(&fname_id);
            return NULL;
        }
        return (AstNode *)create_func_def(fname_id, params, body);
    }

    // 3) SETTER
    if (accept(ASSIGN))
    {
        if (!expect(L_ROUND, "("))
        {
            free_token_lexeme(&fname_id);
            return NULL;
        }

        if (ifj_cur.type != IDENTIFIER_LOCAL)
        {
            fprintf(stderr, "[SYNTAX] l%d: expected parameter name in setter definition\n", ifj_get_line());
            ifj_error_code = 2;
            free_token_lexeme(&fname_id);
            return NULL;
        }

        Token param_id = ifj_cur;
        ifj_cur.lexeme = NULL;
        next();

        if (!expect(R_ROUND, ")"))
        {
            free_token_lexeme(&fname_id);
            free_token_lexeme(&param_id);
            return NULL;
        }

        AstNodeBlock *body = parse_block();
        if (!body)
        {
            free_token_lexeme(&fname_id);
            free_token_lexeme(&param_id);
            return NULL;
        }

        AstNodeList *params = create_list();
        add_to_list(params, create_variable(param_id));

        return (AstNode *)create_func_def(fname_id, params, body);
    }

    // Pokud to nebylo nic z výše uvedeného, je to chyba
    fprintf(stderr, "[SYNTAX] l%d: expected '(', '{' or '=' after function name '%s'\n",
            ifj_get_line(), fname_id.lexeme ? fname_id.lexeme : "<unknown>");
    ifj_error_code = 2;
    free_token_lexeme(&fname_id);
    return NULL;
}

static AstNodeBlock *parse_block(void)
{
    int line = ifj_get_line();
    if (!expect(L_CURLY, "{"))
        return NULL;

    AstNodeBlock *block = create_block(line);

    if (ifj_cur.type == R_CURLY)
    {
        next();
        return block;
    }

    consume_eol(EOL_ZERO_OR_MORE);

    if (!parse_stmt_list(block))
    {
        free_ast_node((AstNode *)block);
        return NULL;
    }

    if (!expect(R_CURLY, "}"))
    {
        free_ast_node((AstNode *)block);
        return NULL;
    }

    return block;
}

static AstNode *parse_stmt_list(AstNodeBlock *block)
{
    while (1)
    {
        skip_eol_star();

        if (ifj_cur.type == R_CURLY || ifj_cur.type == T_EOF)
            return (AstNode *)block;

        AstNode *stmt = parse_stmt();
        if (!stmt)
            return NULL;

        add_stmt_to_block(block, stmt);

        skip_eol_star();
    }
}

static AstNode *parse_stmt(void)
{
    switch (ifj_cur.type)
    {
    case KEYWORD_var:
        return parse_vardecl_stmt();
    case KEYWORD_if:
        return parse_if_stmt();
    case KEYWORD_while:
        return parse_while_stmt();
    case KEYWORD_return:
        return parse_return_stmt();
    case L_CURLY:
        return (AstNode *)parse_block();

    case IDENTIFIER_LOCAL:
    case IDENTIFIER_GLOBAL:
        return parse_assign_or_call_stmt();

    case IFJ_WRITE:
    {
        Token func_id = ifj_cur;
        ifj_cur.lexeme = NULL;
        next();
        AstNodeList *args = parse_call_args_terms();
        if (!args)
        {
            free_token_lexeme(&func_id);
            return NULL;
        }
        return create_func_call(func_id, args);
    }

    case KEYWORD_class:
    case KEYWORD_else:
    case KEYWORD_is:
    case KEYWORD_null:
    case KEYWORD_Ifj:
    case KEYWORD_static:
    case KEYWORD_import:
    case KEYWORD_for:
    case KEYWORD_Num:
    case KEYWORD_String:
    case KEYWORD_Null:
    case INT:
    case FLOAT:
    case STRING:
    case PROLOG:
    case TIMES:
    case DIVIDE:
    case PLUS:
    case MINUS:
    case LESSER:
    case GREATER:
    case LESSER_EQUAL:
    case GREATER_EQUAL:
    case EQUAL:
    case NOT_EQUAL:
    case ASSIGN:
    case L_ROUND:
    case R_ROUND:
    case R_CURLY:
    case IFJ_READ_STR:
    case IFJ_READ_NUM:
    case IFJ_FLOOR:
    case IFJ_STR:
    case IFJ_LENGTH:
    case IFJ_SUBSTRING:
    case IFJ_STRCMP:
    case IFJ_ORD:
    case IFJ_CHR:
    case ERROR:
    case COMMA:
    case EOL:
    case T_EOF:
    default:
        fprintf(stderr, "[SYNTAX] l%d: expected statement (id/if/while/return/var), got %s\n", ifj_get_line(), convert(ifj_cur.type));
        ifj_error_code = 2;
        return NULL;
    }
}

static AstNode *parse_vardecl_stmt(void)
{
    int line = ifj_get_line();
    next(); // sežer 'var'
    if (ifj_cur.type != IDENTIFIER_LOCAL)
    {
        fprintf(stderr, "[SYNTAX] l%d: expected identifier after 'var', got %s\n", line, convert(ifj_cur.type));
        ifj_error_code = 2;
        return NULL;
    }

    Token var_id = ifj_cur;
    ifj_cur.lexeme = NULL;
    next();

    return create_vardecl(var_id);
}

static AstNode *parse_assign_or_call_stmt(void)
{
    int line = ifj_get_line();

    if (!(ifj_cur.type == IDENTIFIER_LOCAL || ifj_cur.type == IDENTIFIER_GLOBAL))
    {
        fprintf(stderr, "[SYNTAX] l%d: expected identifier at start of assignment\n", ifj_get_line());
        ifj_error_code = 2;
        return NULL;
    }

    Token target_id = ifj_cur;
    ifj_cur.lexeme = NULL;
    next();

    if (!expect(ASSIGN, "="))
    {
        free_token_lexeme(&target_id);
        return NULL;
    }

    AstNode *rvalue = parse_expr();
    if (!rvalue)
    {
        free_token_lexeme(&target_id);
        return NULL;
    }

    AstNodeVariable *lvalue = (AstNodeVariable *)create_variable(target_id);

    return create_assign_stmt(lvalue, rvalue, line);
}

static AstNode *parse_if_stmt(void)
{
    int line = ifj_get_line();
    next(); // přeskoč 'if'

    if (!expect(L_ROUND, "("))
        return NULL;

    AstNode *condition = parse_expr();
    if (condition == NULL)
        return NULL;

    if (!expect(R_ROUND, ")"))
    {
        free_ast_node(condition);
        return NULL;
    }

    if (ifj_cur.type == EOL)
    {
        fprintf(stderr, "[SYNTAX] l%d: newline not allowed between ')' and '{' in if\n", line);
        ifj_error_code = 2;
        free_ast_node(condition);
        return NULL;
    }

    AstNodeBlock *then_block = parse_block();
    if (then_block == NULL)
    {
        free_ast_node(condition);
        return NULL;
    }

    AstNodeBlock *else_block = NULL;
    consume_eol(EOL_ZERO_OR_MORE);

    if (ifj_cur.type == KEYWORD_else)
    {
        next();
        else_block = parse_block();
        if (else_block == NULL)
        {
            free_ast_node(condition);
            free_ast_node((AstNode *)then_block);
            return NULL;
        }
    }

    return create_if_stmt(condition, then_block, else_block, line);
}

static AstNode *parse_while_stmt(void)
{
    int line = ifj_get_line();
    next(); // sežer 'while'

    if (!expect(L_ROUND, "("))
        return NULL;

    AstNode *condition = parse_expr();
    if (!condition)
        return NULL;

    if (!expect(R_ROUND, ")"))
    {
        free_ast_node(condition);
        return NULL;
    }

    if (ifj_cur.type == EOL)
    {
        fprintf(stderr, "[SYNTAX] newline not allowed between ')' and '{' in while\n");
        ifj_error_code = 2;
        free_ast_node(condition);
        return NULL;
    }

    AstNodeBlock *body = parse_block();
    if (!body)
    {
        free_ast_node(condition);
        return NULL;
    }

    return create_while_stmt(condition, body, line);
}

static AstNode *parse_return_stmt(void)
{
    int line = ifj_get_line();
    next(); // sežer 'return'

    AstNode *expr = NULL;
    if (can_start_expr(ifj_cur.type))
    {
        expr = parse_expr();
        if (!expr)
            return NULL;
    }

    return create_return_stmt(expr, line);
}

static int lbp_of(TokenType t)
{
    switch (t)
    {
    case TIMES:
    case DIVIDE:
        return PREC_MUL_OP;
    case PLUS:
    case MINUS:
        return PREC_ADD_OP;
    case LESSER:
    case GREATER:
    case LESSER_EQUAL:
    case GREATER_EQUAL:
        return PREC_REL_OP;
    case KEYWORD_is:
        return PREC_IS_OP;
    case EQUAL:
    case NOT_EQUAL:
        return PREC_EQ_OP;
    default:
        return -1;
    }
}

static AstNode *parse_primary(void)
{
    switch (ifj_cur.type)
    {
    case INT:
    case FLOAT:
    case STRING:
    case KEYWORD_null:
    {
        Token t = ifj_cur;
        ifj_cur.lexeme = NULL;
        if (t.type == STRING)
            ifj_cur.value.string_val = NULL;
        next();
        return create_literal(t);
    }

    case IFJ_READ_STR:
    case IFJ_READ_NUM:
    case IFJ_WRITE:
    case IFJ_FLOOR:
    case IFJ_STR:
    case IFJ_LENGTH:
    case IFJ_SUBSTRING:
    case IFJ_STRCMP:
    case IFJ_ORD:
    case IFJ_CHR:
    {
        Token func_id = ifj_cur;
        ifj_cur.lexeme = NULL;
        next();
        AstNodeList *args = parse_call_args_terms();
        if (!args)
        {
            free_token_lexeme(&func_id);
            return NULL;
        }
        return create_func_call(func_id, args);
    }

    case IDENTIFIER_LOCAL:
    case IDENTIFIER_GLOBAL:
    {
        Token id_token = ifj_cur;
        ifj_cur.lexeme = NULL;
        next();

        if (ifj_cur.type == L_ROUND)
        {
            AstNodeList *args = parse_call_args_terms();
            if (!args)
            {
                free_token_lexeme(&id_token);
                return NULL;
            }
            return create_func_call(id_token, args);
        }
        return create_variable(id_token);
    }

    case L_ROUND:
    {
        next();
        AstNode *expr = parse_expr();
        if (!expr)
            return NULL;
        if (!expect(R_ROUND, ")"))
        {
            free_ast_node(expr);
            return NULL;
        }
        return expr;
    }

    default:
        fprintf(stderr, "[SYNTAX] l%d: expected term in expression, got %s\n", ifj_get_line(), convert(ifj_cur.type));
        ifj_error_code = 2;
        return NULL;
    }
}

static AstNode *parse_expr(void) { return parse_expr_bp(PREC_LOWEST); }

static AstNode *parse_expr_bp(IfjPrec minbp)
{
    int line = ifj_get_line();

    AstNode *left = parse_primary();
    if (!left)
        return NULL;

    while (1)
    {
        int lbp = lbp_of(ifj_cur.type);
        if (lbp < 0 || lbp < (int)minbp)
            break;

        TokenType op = ifj_cur.type;
        next();
        IfjPrec rbp = (IfjPrec)(lbp + 1);

        if (op == KEYWORD_is)
        {
            if (!(ifj_cur.type == KEYWORD_Num || ifj_cur.type == KEYWORD_String || ifj_cur.type == KEYWORD_Null))
            {
                fprintf(stderr, "[SYNTAX] l%d: right side of 'is' must be type keyword (Num/String/Null)\n", ifj_get_line());
                ifj_error_code = 2;
                free_ast_node(left);
                return NULL;
            }

            Token type_token = ifj_cur;
            type_token.lexeme = NULL;
            next();

            AstNode *right = create_literal(type_token);
            left = create_binary_expr(op, left, right, line);
            continue;
        }

        AstNode *right = parse_expr_bp(rbp);
        if (!right)
        {
            free_ast_node(left);
            return NULL;
        }

        left = create_binary_expr(op, left, right, line);
    }
    return left;
}