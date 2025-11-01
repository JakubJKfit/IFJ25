#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>


typedef enum{
    VARIABLE_ID,
    FUNC_ID,
    CLASS_ID
}symbol_id_type;

typedef enum{
    Undefined,
    Null,
    String,
    Num
}symbol_data_type;


//TODO: doplnit vsechna potrebna data k ulozeni
typedef struct{
    symbol_id_type id_type;
    symbol_data_type data_type;
    char *identifier;
    int arity;
    int offset; // pro lokalni promenne
    bool is_init;
} symbol_data;


// uzel stromu
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