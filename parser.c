/**
 * @file    parser.c
 * @brief   Syntaktická analýza (rekurzivní sestup + precedenční pro výrazy)
 *          a generování AST pro jazyk IFJ25
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>

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

// === Exit kódy ==================================================
#define ERR_SYN 2
#define ERR_SEM 3

// Globální proměnná ponechána kvůli kompatibilitě linkování (jinak se nepoužije)
int ifj_error_code = 0;

// Vararg ukončovací funkce (čisté C11, s _Noreturn)
_Noreturn static void die_syn(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "[SYNTAX] ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(ERR_SYN);
}
_Noreturn static void die_sem(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "[SEMANTIC] ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(ERR_SEM);
}

// Alias makra
#define DIE_SYN(...) die_syn(__VA_ARGS__)
#define DIE_SEM(...) die_sem(__VA_ARGS__)

// Stav parseru
static Token ifj_cur;
static bool ifj_seen_main = false;

// Lexer API
extern Token ifj_get_token(void);

// Dopředné deklarace utilit
static void next(void);
static bool expect(TokenType t, const char *ctx);
static void skip_eol_star(void);

enum EOL_POLICY
{
    EOL_ZERO_OR_MORE,
    EOL_ONE_EXACT
};
static bool consume_eol(enum EOL_POLICY p);

// Precedence pro precedenční parser výrazů
typedef enum
{
    PREC_LOWEST = 0,
    PREC_EQ_OP,  // == !=
    PREC_IS_OP,  // is
    PREC_REL_OP, // < > <= >=
    PREC_ADD_OP, // + -
    PREC_MUL_OP  // * /
} IfjPrec;

// Forward deklarace parsovacích funkcí
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

// === Utilitní funkce ===========================================

static void next(void)
{
    // Uvolní lexém před načtením dalšího tokenu
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
        DIE_SYN("l%d: expected %s ('%s'), got %s",
                ifj_get_line(), convert(t), ctx ? ctx : "", convert(ifj_cur.type));
    }
    next();
    return true;
}

static void skip_eol_star(void)
{
    while (ifj_cur.type == EOL)
        next();
}

static bool consume_eol(enum EOL_POLICY p)
{
    if (p == EOL_ZERO_OR_MORE)
    {
        skip_eol_star();
        return true;
    }
    if (ifj_cur.type != EOL)
    {
        DIE_SYN("l%d: expected end-of-line, got %s",
                ifj_get_line(), convert(ifj_cur.type));
    }
    do
    {
        next();
    } while (ifj_cur.type == EOL);
    return true;
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

static bool accept(TokenType t)
{
    if (ifj_cur.type == t)
    {
        next();
        return true;
    }
    return false;
}

// === Vstupní bod parseru =======================================

AstNode *ifj_parse_program(void)
{
    ifj_seen_main = false;
    next();

    if (!parse_prolog())
    {
        return NULL; // v praxi se neprovede, parse_prolog() končí DIE_SYN
    }

    AstNode *program_node = parse_class();
    if (program_node == NULL)
    {
        return NULL;
    }

    if (!ifj_seen_main)
    {
        // Semantická chyba dle zadání
        free_ast_node(program_node);
        DIE_SEM("missing static main() without params");
    }

    return program_node;
}

// === Prolog =====================================================

static bool parse_prolog(void)
{
    skip_eol_star();

    if (ifj_cur.type != KEYWORD_import)
        DIE_SYN("missing prolog 'import'");
    next();

    if (ifj_cur.type != STRING || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "ifj25") != 0)
        DIE_SYN("expected string literal \"ifj25\" after import");
    next();

    expect(KEYWORD_for, "for");

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
        DIE_SYN("expected 'Ifj' after 'for'");
    }

    return consume_eol(EOL_ONE_EXACT);
}

// === Funkce/Program =============================================

static AstNode *parse_class(void)
{
    expect(KEYWORD_class, "class");

    if (ifj_cur.type != IDENTIFIER_LOCAL || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "Program") != 0)
        DIE_SYN("expected class name 'Program'");
    next();

    expect(L_CURLY, "{");
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
    expect(KEYWORD_static, "static");

    if (ifj_cur.type != IDENTIFIER_LOCAL)
        DIE_SYN("expected function identifier after 'static'");

    Token fname_id = ifj_cur; // přenecháme lexém do AST
    ifj_cur.lexeme = NULL;
    next();

    skip_eol_star();

    // GETTER
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

    // FUNKCE
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
                    free_ast_node((AstNode *)params);
                    free_token_lexeme(&fname_id);
                    DIE_SYN("expected parameter name");
                }
                add_to_list(params, create_variable(ifj_cur));
                ifj_cur.lexeme = NULL;
                arity++;
                next();
            } while (accept(COMMA));

            expect(R_ROUND, ")");
        }
        else
        {
            next(); // sežer ')'
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

    // SETTER
    if (accept(ASSIGN))
    {
        expect(L_ROUND, "(");

        if (ifj_cur.type != IDENTIFIER_LOCAL)
        {
            free_token_lexeme(&fname_id);
            DIE_SYN("l%d: expected parameter name in setter definition", ifj_get_line());
        }

        Token param_id = ifj_cur;
        ifj_cur.lexeme = NULL;
        next();

        expect(R_ROUND, ")");

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

    // Nic z výše uvedeného
    {
        const char *fn = fname_id.lexeme ? fname_id.lexeme : "<unknown>";
        free_token_lexeme(&fname_id);
        DIE_SYN("l%d: expected '(', '{' or '=' after function name '%s'",
                ifj_get_line(), fn);
    }

    // pro -Wreturn-type (neproveditelné)
    return NULL;
}

// === Bloky a příkazy ============================================

static AstNodeBlock *parse_block(void)
{
    int line = ifj_get_line();
    (void)line; // suppress unused if not referenced later
    expect(L_CURLY, "{");

    AstNodeBlock *block = create_block(line);

    if (ifj_cur.type == R_CURLY)
    {
        next();
        return block;
    }

    // Povolíme libovolný počet prázdných řádků i žádný
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
        Token func_id = ifj_cur; // předáme lexém do AST
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

    default:
        DIE_SYN("l%d: expected statement (id/if/while/return/var), got %s",
                ifj_get_line(), convert(ifj_cur.type));
    }

    // pro -Wreturn-type (neproveditelné)
    return NULL;
}

static AstNode *parse_vardecl_stmt(void)
{
    int line = ifj_get_line();
    next(); // 'var'

    if (ifj_cur.type != IDENTIFIER_LOCAL)
        DIE_SYN("l%d: expected identifier after 'var', got %s", line, convert(ifj_cur.type));

    Token var_id = ifj_cur;
    ifj_cur.lexeme = NULL;
    next();

    return create_vardecl(var_id);
}

static AstNode *parse_assign_or_call_stmt(void)
{
    int line = ifj_get_line();

    if (!(ifj_cur.type == IDENTIFIER_LOCAL || ifj_cur.type == IDENTIFIER_GLOBAL))
        DIE_SYN("l%d: expected identifier at start of assignment", ifj_get_line());

    Token target_id = ifj_cur;
    ifj_cur.lexeme = NULL;
    next();

    expect(ASSIGN, "=");

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
    next(); // 'if'

    expect(L_ROUND, "(");
    AstNode *condition = parse_expr();
    if (!condition)
        return NULL;
    expect(R_ROUND, ")");

    if (ifj_cur.type == EOL)
        DIE_SYN("l%d: newline not allowed between ')' and '{' in if", line);

    AstNodeBlock *then_block = parse_block();
    if (!then_block)
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
        if (!else_block)
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
    next(); // 'while'

    expect(L_ROUND, "(");
    AstNode *condition = parse_expr();
    if (!condition)
        return NULL;
    expect(R_ROUND, ")");

    if (ifj_cur.type == EOL)
        DIE_SYN("l%d: newline not allowed between ')' and '{' in while", line);

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
    (void)line;
    next(); // 'return'

    AstNode *expr = NULL;
    if (can_start_expr(ifj_cur.type))
    {
        expr = parse_expr();
        if (!expr)
            return NULL;
    }
    return create_return_stmt(expr, line);
}

// === Výrazy =====================================================

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
        Token t = ifj_cur; // přeneseme hodnotu tokenu do AST
        ifj_cur.lexeme = NULL;
        if (t.type == STRING) // zabráníme double-free při next()
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
        DIE_SYN("l%d: expected term in expression, got %s",
                ifj_get_line(), convert(ifj_cur.type));
    }

    // pro -Wreturn-type (neproveditelné)
    return NULL;
}

static AstNode *parse_expr(void)
{
    return parse_expr_bp(PREC_LOWEST);
}

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
        IfjPrec rbp = (IfjPrec)(lbp + 1); // left-assoc

        if (op == KEYWORD_is)
        {
            if (!(ifj_cur.type == KEYWORD_Num ||
                  ifj_cur.type == KEYWORD_String ||
                  ifj_cur.type == KEYWORD_Null))
            {
                free_ast_node(left);
                DIE_SYN("l%d: right side of 'is' must be type keyword (Num/String/Null)", ifj_get_line());
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

// === Volání a argumenty =========================================

static AstNodeList *parse_call_args_terms(void)
{
    expect(L_ROUND, "(");

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
