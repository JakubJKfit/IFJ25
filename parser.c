#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lex_scanner.h"
#include "parser.h"

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
static bool ifj_seen_main = false; // to verify existence of static main()

static void next(void)
{
    ifj_cur = ifj_get_token();
    TRACEF("[TOK l%03d] %-12s %s\n",
           ifj_get_line(),
           tname(ifj_cur.type),
           (ifj_cur.lexeme ? ifj_cur.lexeme : ""));
}

static const char *tname(TokenType t)
{
    switch (t)
    {
    case T_EOF:
        return "EOF";
    case EOL:
        return "EOL";
    case ERROR:
        return "LEXERR";
    case KEYWORD_Ifj:
        return "Ifj";
    case KEYWORD_class:
        return "class";
    case KEYWORD_static:
        return "static";
    case KEYWORD_import:
        return "import";
    case KEYWORD_for:
        return "for";
    case KEYWORD_if:
        return "if";
    case KEYWORD_else:
        return "else";
    case KEYWORD_while:
        return "while";
    case KEYWORD_return:
        return "return";
    case KEYWORD_is:
        return "is";
    case KEYWORD_null:
        return "null";
    case KEYWORD_Num:
        return "Num";
    case KEYWORD_String:
        return "String";
    case KEYWORD_Null:
        return "Null";
    case KEYWORD_var:
        return "var";
    case IDENTIFIER_LOCAL:
        return "identifier";
    case IDENTIFIER_GLOBAL:
        return "__identifier";
    case INT:
        return "int";
    case FLOAT:
        return "float";
    case STRING:
        return "string";

    case L_ROUND:
        return "(";
    case R_ROUND:
        return ")";
    case L_CURLY:
        return "{";
    case R_CURLY:
        return "}";
    case COMMA:
        return ",";
    case ASSIGN:
        return "=";
    case PLUS:
        return "+";
    case MINUS:
        return "-";
    case TIMES:
        return "*";
    case DIVIDE:
        return "/";
    case LESSER:
        return "<";
    case GREATER:
        return ">";
    case LESSER_EQUAL:
        return "<=";
    case GREATER_EQUAL:
        return ">=";
    case EQUAL:
        return "==";
    case NOT_EQUAL:
        return "!=";
    case IFJ_READ_STR:
        return "Ifj.read_str";
    case IFJ_READ_NUM:
        return "Ifj.read_num";
    case IFJ_WRITE:
        return "Ifj.write";
    case IFJ_FLOOR:
        return "Ifj.floor";
    case IFJ_STR:
        return "Ifj.str";
    case IFJ_LENGTH:
        return "Ifj.length";
    case IFJ_SUBSTRING:
        return "Ifj.substring";
    case IFJ_STRCMP:
        return "Ifj.strcmp";
    case IFJ_ORD:
        return "Ifj.ord";
    case IFJ_CHR:
        return "Ifj.chr";
    default:
        return "token";
    }
}

static bool expect(TokenType t, const char *ctx)
{
    if (ifj_cur.type != t)
    {
        fprintf(stderr, "[SYNTAX] l%d: expected %s ('%s'), got %s\n", ifj_get_line(), tname(t), ctx ? ctx : "", tname(ifj_cur.type));

        return false; // error 2
    }
    next();
    return true;
}

static void skip_eol_star(void)
{
    while (ifj_cur.type == EOL)
        next();
}

// Exactly one statement per line enforcement
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
                ifj_get_line(), tname(ifj_cur.type));
        return false;
    }
    do
    {
        next();
    } while (ifj_cur.type == EOL);
    return true;
}

// Expressions
typedef enum
{
    PREC_LOWEST = 0,
    PREC_EQ_OP,  // == !=
    PREC_IS_OP,  // is
    PREC_REL_OP, // < > <= >=
    PREC_ADD_OP, // + -
    PREC_MUL_OP  // * /
} IfjPrec;

// Forward declarations
static bool parse_prolog(void);
static bool parse_class(void);
static bool parse_func_list(void);
static bool parse_funcdef(void);
static bool parse_block(void);
static bool parse_stmt_list(void);
static bool parse_stmt(void);
static bool parse_vardecl_stmt(void);
static bool parse_assign_or_call_stmt(void);
static bool parse_if_stmt(void);
static bool parse_while_stmt(void);
static bool parse_return_stmt(void);
static bool parse_ifj_call_stmt(void);
static bool is_ifj_builtin(TokenType t);
static bool parse_call_args_terms(void);
static bool parse_expr_bp_after_primary(IfjPrec minbp);
static bool parse_expr(void);
static bool parse_expr_bp(IfjPrec minbp);
static bool parse_primary(void);
static int lbp_of(TokenType t);

// Public entry
bool ifj_parse_program(void)
{
    ifj_seen_main = false;
    next();
    if (!parse_prolog())
        return false;
    if (!parse_class())
        return false;
    if (!ifj_seen_main)
    {
        fprintf(stderr, "[SEMANTIC] missing static main() without params (error 3)\n");
        ifj_error_code = 3;
        return false;
    }
    return true;
}

// Grammar
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
    // import "ifj25" for Ifj
    skip_eol_star();

    if (ifj_cur.type != KEYWORD_import)
    {
        fprintf(stderr, "[SYNTAX] missing prolog 'import' (error 2)\n");
        ifj_error_code = 2;
        return false;
    }
    next();

    // presne ifj25
    if (ifj_cur.type != STRING || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "ifj25") != 0)
    {
        fprintf(stderr, "[SYNTAX] expected string literal \"ifj25\" after import (error 2)\n");
        ifj_error_code = 2;
        return false;
    }
    next();

    if (!expect(KEYWORD_for, "for"))
    {
        ifj_error_code = 2;
        return false;
    }

    // Po 'for' očekáváme Ifj
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

static bool is_ifj_builtin(TokenType t)
{
    switch (t)
    {
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

static bool parse_call_args_terms(void)
{
    if (!expect(L_ROUND, "("))
        return false;
    if (ifj_cur.type != R_ROUND)
    {
        if (!parse_expr())
            return false;
        while (accept(COMMA))
        {
            if (!parse_expr())
                return false;
        }
        if (!expect(R_ROUND, ")"))
            return false;
    }
    else
    {
        next();
    }
    return true;
}

static bool parse_expr_bp_after_primary(IfjPrec minbp)
{
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
                fprintf(stderr, "[SYNTAX] right side of 'is' must be type keyword (Num/String/Null)\n");
                return false;
            }
            next();
            continue;
        }
        if (!parse_expr_bp(rbp))
            return false;
    }
    return true;
}

static bool parse_class(void)
{
    if (!expect(KEYWORD_class, "class"))
        return false;

    if (ifj_cur.type != IDENTIFIER_LOCAL || !ifj_cur.lexeme || strcmp(ifj_cur.lexeme, "Program") != 0)
    {
        fprintf(stderr, "[SYNTAX] expected class name 'Program' (error 2)\n");
        return false;
    }
    next();

    if (!expect(L_CURLY, "{"))
        return false;
    consume_eol(EOL_ZERO_OR_MORE);

    if (!parse_func_list())
        return false;

    if (!expect(R_CURLY, "}"))
        return false;
    consume_eol(EOL_ZERO_OR_MORE);
    return true;
}

static bool parse_func_list(void)
{
    while (ifj_cur.type == KEYWORD_static)
    {
        if (!parse_funcdef())
            return false;

        // dřív tu bylo: if (... R_CURLY/EOF) continue; else consume_eol(EOL_ONE_EXACT)
        // Povolit libovolný počet prázdných řádků i žádný.
        consume_eol(EOL_ZERO_OR_MORE);
    }
    return true;
}

static bool parse_funcdef(void)
{
    if (!expect(KEYWORD_static, "static"))
        return false;

    if (ifj_cur.type != IDENTIFIER_LOCAL)
    {
        fprintf(stderr, "[SYNTAX] expected function identifier after 'static'\n");
        return false;
    }
    const char *fname = ifj_cur.lexeme;
    next();

    // dovol volitelné EOL/whitespace mezi jménem a dalším znakem
    skip_eol_star();

    // 1) GETTER
    if (ifj_cur.type == L_CURLY)
    {
        return parse_block();
    }

    // 2) FUNKCE
    if (accept(L_ROUND))
    {
        int arity = 0;
        if (ifj_cur.type != R_ROUND)
        {
            do
            {
                if (ifj_cur.type != IDENTIFIER_LOCAL)
                {
                    fprintf(stderr, "[SYNTAX] expected parameter name\n");
                    return false;
                }
                arity++;
                next();
            } while (accept(COMMA));
            if (!expect(R_ROUND, ")"))
                return false;
        }
        else
        {
            next();
        }
        if (fname && strcmp(fname, "main") == 0 && arity == 0)
            ifj_seen_main = true;

        return parse_block();
    }

    // 3) SETTER
    if (accept(ASSIGN))
    {
        if (!expect(L_ROUND, "("))
            return false;
        if (!expect(IDENTIFIER_LOCAL, "setter param id"))
            return false;
        if (!expect(R_ROUND, ")"))
            return false;
        return parse_block();
    }
    return parse_block();
}

static bool parse_block(void)
{
    if (!expect(L_CURLY, "{"))
        return false;

    if (ifj_cur.type == R_CURLY) {
        next();
        return true;
    }

    consume_eol(EOL_ZERO_OR_MORE);

    if (!parse_stmt_list())
        return false;

    if (!expect(R_CURLY, "}"))
        return false;

    return true;
}


static bool parse_stmt_list(void)
{
    while (1)
    {
        // přeskoč prázdné řádky / komentářové EOL
        skip_eol_star();

        if (ifj_cur.type == R_CURLY || ifj_cur.type == T_EOF)
            return true;

        if (!parse_stmt())
            return false;

        // po příkazu toleruj libovolný počet EOL
        skip_eol_star();
    }
}

static bool parse_stmt(void)
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
        return parse_block();
    case IFJ_WRITE:
    case IFJ_READ_STR:
    case IFJ_READ_NUM:
    case IFJ_FLOOR:
    case IFJ_STR:
    case IFJ_LENGTH:
    case IFJ_SUBSTRING:
    case IFJ_STRCMP:
    case IFJ_ORD:
    case IFJ_CHR:
        return parse_ifj_call_stmt();
    default:
        return parse_assign_or_call_stmt();
    }
}

static bool parse_vardecl_stmt(void)
{
    next();
    if (!expect(IDENTIFIER_LOCAL, "identifier after 'var'"))
        return false;
    return true;
}

static bool parse_ifj_call_stmt(void)
{
    // na vstupu je IFJ_*
    next();
    return parse_call_args_terms(); // vyžaduje '(' ... ')'
}

static bool parse_assign_or_call_stmt(void)
{
    if (!(ifj_cur.type == IDENTIFIER_LOCAL || ifj_cur.type == IDENTIFIER_GLOBAL))
    {
        fprintf(stderr, "[SYNTAX] expected statement (id/if/while/return/var)\n");
        return false;
    }
    next();

    if (!expect(ASSIGN, "="))
        return false;

    // 1) volání uživatelské funkce
    if (ifj_cur.type == IDENTIFIER_LOCAL)
    {
        // sežer primární výraz (identifikátor)
        next();

        if (accept(L_ROUND))
        {
            if (ifj_cur.type != R_ROUND)
            {
                if (!parse_expr())
                    return false;
                while (accept(COMMA))
                {
                    if (!parse_expr())
                        return false;
                }
                if (!expect(R_ROUND, ")"))
                    return false;
            }
            else
            {
                next();
            }
            return true;
        }

        return parse_expr_bp_after_primary(PREC_LOWEST);
    }

    if (is_ifj_builtin(ifj_cur.type))
    {
        next();
        if (!parse_call_args_terms())
            return false;
        return parse_expr_bp_after_primary(PREC_LOWEST);
    }

    return parse_expr();
}

static bool parse_if_stmt(void)
{
    next();
    if (!expect(L_ROUND, "("))
        return false;
    if (!parse_expr())
        return false;
    if (!expect(R_ROUND, ")"))
        return false;
    if (ifj_cur.type == EOL)
    {
        fprintf(stderr, "[SYNTAX] newline not allowed between ')' and '{' in if\n");
        return false;
    }
    if (!parse_block())
        return false;
    // EOL před else povolíme
    consume_eol(EOL_ZERO_OR_MORE);
    if (ifj_cur.type == KEYWORD_else)
    {
        next();
        if (!parse_block())
            return false;
    }
    return true;
}

static bool parse_while_stmt(void)
{
    next();
    if (!expect(L_ROUND, "("))
        return false;
    if (!parse_expr())
        return false;
    if (!expect(R_ROUND, ")"))
        return false;
    if (ifj_cur.type == EOL)
    {
        fprintf(stderr, "[SYNTAX] newline not allowed between ')' and '{' in while\n");
        return false;
    }
    if (!parse_block())
        return false;
    return true;
}

static bool parse_return_stmt(void)
{
    next(); // sežer 'return'
    if (can_start_expr(ifj_cur.type))
    {
        if (!parse_expr())
            return false;
    }
    return true;
}
// Expressions
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

static bool parse_primary(void)
{
    switch (ifj_cur.type)
    {
    case INT:
    case FLOAT:
    case STRING:
    case KEYWORD_null:
        next();
        return true;
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
        next();
        if (!parse_call_args_terms())
            return false;
        return true;

    case IDENTIFIER_LOCAL:
    case IDENTIFIER_GLOBAL:
        next();
        return true;

    case L_ROUND:
        next();
        if (!parse_expr())
            return false;
        if (!expect(R_ROUND, ")"))
            return false;
        return true;

    default:
        fprintf(stderr, "[SYNTAX] expected term in expression, got %s\n", tname(ifj_cur.type));
        return false;
    }
}

static bool parse_expr(void) { return parse_expr_bp(PREC_LOWEST); }

static bool parse_expr_bp(IfjPrec minbp)
{
    if (!parse_primary())
        return false;

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
            if (!(ifj_cur.type == KEYWORD_Num || ifj_cur.type == KEYWORD_String || ifj_cur.type == KEYWORD_Null))
            {
                fprintf(stderr, "[SYNTAX] right side of 'is' must be type keyword (Num/String/Null)\n");
                return false;
            }
            next();
            continue;
        }

        if (!parse_expr_bp(rbp))
            return false;
    }
    return true;
}
