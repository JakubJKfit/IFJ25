/**
 * @file    ast.h
 * @brief   Definice struktur pro Abstraktní Syntaktický Strom (AST) jazyka IFJ25
 */

#ifndef AST_H
#define AST_H

#include "lex_scanner.h" // Pro přístup k Token
#include <stdlib.h>

// --- Typy uzlů ---
typedef enum
{
    AST_PROGRAM,       // Kořen stromu, obsahuje seznam funkcí
    AST_FUNC_DEF,      // Definice funkce (static name(params) block)
    AST_FUNC_CALL,     // Volání funkce (name(args))
    AST_STMT_BLOCK,    // Blok kódu { ... }
    AST_STMT_VAR_DECL, // var id
    AST_STMT_ASSIGN,   // id = expr
    AST_STMT_IF,       // if (cond) then else
    AST_STMT_WHILE,    // while (cond) block
    AST_STMT_RETURN,   // return expr
    AST_EXPR_BINARY,   // expr OP expr
    AST_EXPR_LITERAL,  // 123, "abc", null
    AST_EXPR_VARIABLE, // id
    AST_NODE_LIST,     // Obecný seznam (pro argumenty, parametry, příkazy)
} AstNodeType;

// --- Základní struktura ---
typedef struct AstNode
{
    AstNodeType type;
    int line_number;
} AstNode;

// --- Helper pro seznamy ---
typedef struct
{
    AstNode base;
    struct AstNode **items; // Dynamické pole ukazatelů
    int count;
    int capacity;
} AstNodeList;

// --- Uzly výrazů (Expressions) ---

typedef struct
{
    AstNode base;
    Token token; // Uložíme si celý token s hodnotou (INT, FLOAT, STRING, null)
} AstNodeLiteral;

typedef struct
{
    AstNode base;
    Token token; // Uložíme si token s IDENTIFIER
} AstNodeVariable;

typedef struct
{
    AstNode base;
    TokenType op; // Např. PLUS, LESSER, KEYWORD_is
    struct AstNode *left;
    struct AstNode *right;
} AstNodeBinaryExpr;

typedef struct
{
    AstNode base;
    Token func_id;     // IDENTIFIER nebo IFJ_WRITE, atd.
    AstNodeList *args; // Seznam argumentů (výrazů)
} AstNodeFuncCall;

// --- Uzly příkazů (Statements) ---

typedef struct
{
    AstNode base;
    AstNodeList *statements; // Seznam příkazů v bloku
} AstNodeBlock;

typedef struct
{
    AstNode base;
    struct AstNode *condition;
    AstNodeBlock *then_block;
    AstNodeBlock *else_block; // Může být NULL
} AstNodeIfStmt;

typedef struct
{
    AstNode base;
    struct AstNode *condition;
    AstNodeBlock *body_block;
} AstNodeWhileStmt;

typedef struct
{
    AstNode base;
    Token var_id; // Token s IDENTIFIER
} AstNodeVarDecl;

typedef struct
{
    AstNode base;
    AstNodeVariable *lvalue; // Levá strana (proměnná)
    struct AstNode *rvalue;  // Pravá strana (výraz)
} AstNodeAssignStmt;

typedef struct
{
    AstNode base;
    struct AstNode *expr; // Výraz, který se vrací (může být NULL)
} AstNodeReturnStmt;

// --- Uzly nejvyšší úrovně ---

typedef struct
{
    AstNode base;
    Token func_id;
    AstNodeList *params; // Seznam IDENTIFIERů
    AstNodeBlock *body;
    // POZNÁMKA: Gettery (0 params) a Settery (1 param, stejné jméno jako getter)
    // jsou zde reprezentovány stejně jako běžné funkce.
    // Sémantická analýza je musí rozlišit.
} AstNodeFuncDef;

typedef struct
{
    AstNode base;
    AstNodeList *functions; // Seznam AstNodeFuncDef
} AstNodeProgram;

// --- Prototypy konstruktorů (z ast.c) ---

AstNode *create_literal(Token token);
AstNode *create_variable(Token token);
AstNode *create_binary_expr(TokenType op, AstNode *left, AstNode *right, int line);
AstNode *create_func_call(Token func_id, AstNodeList *args);

AstNode *create_if_stmt(AstNode *cond, AstNodeBlock *then_b, AstNodeBlock *else_b, int line);
AstNode *create_while_stmt(AstNode *cond, AstNodeBlock *body, int line);
AstNode *create_return_stmt(AstNode *expr, int line);
AstNode *create_vardecl(Token var_id);
AstNode *create_assign_stmt(AstNodeVariable *lvalue, AstNode *rvalue, int line);

AstNodeList *create_list(void);
void add_to_list(AstNodeList *list, AstNode *item);

AstNodeBlock *create_block(int line);
void add_stmt_to_block(AstNodeBlock *block, AstNode *stmt);

AstNodeFuncDef *create_func_def(Token func_id, AstNodeList *params, AstNodeBlock *body);
AstNodeProgram *create_program(void);
void add_func_to_program(AstNodeProgram *prog, AstNodeFuncDef *func);

// Uvolnění paměti
void free_ast_node(AstNode *node);
void free_token_lexeme(Token *t);

#endif // AST_H