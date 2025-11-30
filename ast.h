/**
 * @file    ast.h
 * @brief   Definice struktur pro abstraktní syntaktický strom (AST) jazyka IFJ25
 */

#ifndef AST_H
#define AST_H

#include "lex_scanner.h" // Pro přístup k Token
#include "symtable.h"
#include <stdlib.h>

/**
 * @brief Typ uzlu v abstraktním syntaktickém stromu.
 */
typedef enum
{
    AST_PROGRAM,       ///< Kořen stromu, obsahuje seznam funkcí
    AST_FUNC_DEF,      ///< Definice funkce (static name(params) block)
    AST_FUNC_CALL,     ///< Volání funkce (name(args))
    AST_STMT_BLOCK,    ///< Blok kódu { ... }
    AST_STMT_VAR_DECL, ///< var id
    AST_STMT_ASSIGN,   ///< id = expr
    AST_STMT_IF,       ///< if (cond) then else
    AST_STMT_WHILE,    ///< while (cond) block
    AST_STMT_RETURN,   ///< return expr
    AST_EXPR_BINARY,   ///< expr OP expr
    AST_EXPR_LITERAL,  ///< 123, "abc", null
    AST_EXPR_VARIABLE, ///< id
    AST_NODE_LIST,     ///< Obecný seznam (pro argumenty, parametry, příkazy)
} AstNodeType;

/**
 * @brief Druh funkce (běžná funkce, getter, setter).
 */
typedef enum
{
    FUNC_IS_FUNC,
    FUNC_IS_GETTER,
    FUNC_IS_SETTER
} AstFuncKind;

/**
 * @brief Základní struktura každého uzlu AST.
 */
typedef struct AstNode
{
    AstNodeType type; ///< Typ uzlu.
    int line_number;  ///< Číslo řádku ve zdrojovém souboru.
} AstNode;

/**
 * @brief Obecný seznam AST uzlů.
 */
typedef struct
{
    AstNode base;
    struct AstNode **items; ///< Dynamické pole ukazatelů na uzly.
    int count;              ///< Aktuální počet položek.
    int capacity;           ///< Celková kapacita pole.
} AstNodeList;

// Uzly výrazů (Expressions)

typedef struct
{
    AstNode base;
    Token token;                // Uložíme si celý token s hodnotou (INT, FLOAT, STRING, null)
    symbol_data_type data_type; // pro semantiku
} AstNodeLiteral;

typedef struct
{
    AstNode base;
    Token token; // Uložíme si token s IDENTIFIER
    // semantika
    int stack_offset;
    symbol_data_type data_type;
} AstNodeVariable;

typedef struct
{
    AstNode base;
    TokenType op; // Např. PLUS, LESSER, KEYWORD_is
    struct AstNode *left;
    struct AstNode *right;
    symbol_data_type data_type; // semantika
} AstNodeBinaryExpr;

typedef struct
{
    AstNode base;
    Token func_id;     // IDENTIFIER nebo IFJ_WRITE, atd.
    AstNodeList *args; // Seznam argumentů (výrazů)
    symbol_data_type data_type;
} AstNodeFuncCall;

// Uzly příkazů (Statements) 

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
    // semantika
    int stack_offset;
    symbol_data_type data_type;
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

// Uzly nejvyšší úrovně 

typedef struct
{
    AstNode base;
    Token func_id;
    AstNodeList *params; // Seznam IDENTIFIERů
    AstNodeBlock *body;
    AstFuncKind kind; // pro semantiku
} AstNodeFuncDef;

typedef struct
{
    AstNode base;
    AstNodeList *functions; // Seznam AstNodeFuncDef
} AstNodeProgram;

// Prototypy konstruktorů (z ast.c)

/**
 * @brief Vytvoří AST uzel pro literál.
 */
AstNode *create_literal(Token token);

/**
 * @brief Vytvoří AST uzel pro proměnnou.
 */
AstNode *create_variable(Token token);

/**
 * @brief Vytvoří AST uzel pro binární výraz.
 */
AstNode *create_binary_expr(TokenType op, AstNode *left, AstNode *right, int line);

/**
 * @brief Vytvoří AST uzel pro volání funkce.
 */
AstNode *create_func_call(Token func_id, AstNodeList *args);

/**
 * @brief Vytvoří AST uzel pro příkaz if.
 */
AstNode *create_if_stmt(AstNode *cond, AstNodeBlock *then_b, AstNodeBlock *else_b, int line);

/**
 * @brief Vytvoří AST uzel pro příkaz while.
 */
AstNode *create_while_stmt(AstNode *cond, AstNodeBlock *body, int line);

/**
 * @brief Vytvoří AST uzel pro příkaz return.
 */
AstNode *create_return_stmt(AstNode *expr, int line);

/**
 * @brief Vytvoří AST uzel pro deklaraci proměnné.
 */
AstNode *create_vardecl(Token var_id);

/**
 * @brief Vytvoří AST uzel pro přiřazení.
 */
AstNode *create_assign_stmt(AstNodeVariable *lvalue, AstNode *rvalue, int line);

/**
 * @brief Vytvoří prázdný seznam AST uzlů.
 */
AstNodeList *create_list(void);

/**
 * @brief Přidá položku na konec seznamu AST uzlů.
 */
void add_to_list(AstNodeList *list, AstNode *item);

/**
 * @brief Vytvoří AST uzel pro blok příkazů.
 */
AstNodeBlock *create_block(int line);

/**
 * @brief Přidá příkaz do bloku.
 */
void add_stmt_to_block(AstNodeBlock *block, AstNode *stmt);

/**
 * @brief Vytvoří AST uzel pro definici funkce.
 */
AstNodeFuncDef *create_func_def(Token func_id, AstNodeList *params, AstNodeBlock *body, AstFuncKind kind);

/**
 * @brief Vytvoří kořenový uzel programu.
 */
AstNodeProgram *create_program(void);

/**
 * @brief Přidá definici funkce do kořenového uzlu programu.
 */
void add_func_to_program(AstNodeProgram *prog, AstNodeFuncDef *func);

/**
 * @brief Rekurzivně uvolní uzel AST a všechny jeho potomky.
 */
void free_ast_node(AstNode *node);

/**
 * @brief Uvolní dynamicky alokované části tokenu (lexeme, string_val).
 */
void free_token_lexeme(Token *t);

#endif // AST_H
