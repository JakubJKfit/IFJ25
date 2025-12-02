/**
 * @name IFJ25
 * 
 * @file symtable.h
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Hlavičkový soubor pro tabulku symbolů
 */
#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

/**
 * @brief Uložení symbolu jako identifikátor funkce nebo proměnné
 */
typedef enum{
    VARIABLE_ID,
    FUNC_ID
}symbol_id_type;

/**
 * @brief Datový typ uloženého symbolu, návratové pro funkce
 */
typedef enum{
    Undefined,
    Null,
    String,
    Num
}symbol_data_type;

/**
 * @brief Data uloženého symbolu
 */
typedef struct{
    symbol_id_type id_type;
    symbol_data_type data_type;
    char *identifier;
    int arity;
    int offset; // posun pro codegen
} symbol_data;


/**
 * @brief Uzel stromu
 */
typedef struct tree_node{
    symbol_data data;
    struct tree_node *left;
    struct tree_node *right;
    int height;
}tree_node;

void tree_init(tree_node **tree);
bool search_symbol(tree_node *node, char *identifier,symbol_data **found);
void insert_symbol(tree_node **tree, symbol_data *data);
void delete_symbol(tree_node **tree, char *identifier);
void tree_dispose(tree_node **tree);


#endif // SYMTABLE_H