#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

//TODO: doplnit vsechna potrebna data k ulozeni
typedef struct{
    char *identifier;
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