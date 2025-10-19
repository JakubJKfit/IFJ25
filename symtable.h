#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stdbool.h>

// uzel stromu
typdef struct tree_node{
    struct tree_node *left;
    struct tree_node *right;
    int height;
}tree_node;

#endif // SYMTABLE_H