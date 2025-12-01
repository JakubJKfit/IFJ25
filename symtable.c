/**
 * @name IFJ25
 * 
 * @file symtable.c
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Tabulka symbolů implementována pomocí výškově vyváženého bin. stromu
 */

#include "symtable.h"
#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Vrací větší z dvou čísel
 * 
 * @param a int
 * @param b int
 * @return int maximum
 */
static int maximum(int a, int b){
    return (a > b ? a : b);
}

/**
 * @brief Pomocná funkce pro načtení dat a přidělení paměti pro identifikátor načteného symbolu
 * 
 * @param target Proměnná, které data budou uložená do tabulky
 * @param source Data načteného symbolu
 * @return void
 */
static void load_data(symbol_data *target, symbol_data *source){
    // prideleni dat
    target->id_type = source->id_type;
    target->data_type = source->data_type;
    target->offset = source->offset;
    target->arity = source->arity;
    
    if(target->identifier != NULL) free(target->identifier);
    target->identifier = malloc(strlen(source->identifier) + 1); // prideleni pameti pro identifikator
    if(target->identifier == NULL){
        fprintf(stderr, "Malloc error while loading symbol data to node\n");
            ifjexit(ERR_INTERNAL);
        }
    strcpy(target->identifier, source->identifier);
}

/**
 * @brief Inicializace stromu
 * 
 * @param tree ukazatel na ukazatel na kořen stromu
 * @return void
 */
void tree_init(tree_node **tree){
    *tree = NULL;
}

/**
 * @brief Zjištění výšky uzlu
 * 
 * @param node ukazatel na uzel stromu
 * @return int výška uzlu
 */
int get_height(tree_node *node){
    if(node != NULL){
        return node->height;
    }
    return 0;
}

/**
 * @brief Zjištění a přiřazení výšky uzlu
 * 
 * @param node ukazatel na uzel stromu
 * @return void
 */
void put_height(tree_node *node){
    if(node != NULL){
        int left_h = get_height(node->left);
        int right_h = get_height(node->right);
        node->height = maximum(left_h, right_h) + 1;
    }
}


/**
 * @brief Získání hodnoty vyváženosti stromu
 * 
 * @param node ukazatel na uzel stromu
 * @return int hodnota vyváženosti
 */
int get_balance(tree_node *node){
    if(node != NULL){
        return get_height(node->left) - get_height(node->right);
    }
    return 0;
}

/**
 * @brief Rotace uzlů směrem doprava
 * 
 * @param tree ukazatel na ukazatel na kořen stromu
 * @returns void
 */
void rotate_right(tree_node **tree){
    if(*tree == NULL || (*tree)->left == NULL) return;
    
    // rotace uzlů
    tree_node *old_root = *tree;
    tree_node *new_root = old_root->left;
    tree_node *temp = new_root->right;

    new_root->right = old_root;
    old_root->left = temp;

    // změna výšky uzlů na nových místech
    put_height(old_root);
    put_height(new_root);

    *tree = new_root;
}

/**
 * @brief Rotace uzlů směrem doleva
 * 
 * @param tree ukazatel na ukazatel na kořen stromu
 * @return void
 */
void rotate_left(tree_node **tree){
    if(*tree == NULL || (*tree)->right == NULL) return;
    
    tree_node *old_root = *tree;
    tree_node *new_root = old_root->right;
    tree_node *temp = new_root->left;

    new_root->left = old_root;
    old_root->right = temp;

    put_height(old_root);
    put_height(new_root);

    *tree = new_root;
}

/**
 * @brief Vyvážení stromu
 * 
 * @param tree ukazatel na ukazatel na kořen
 * @return void
 */
void balance_tree(tree_node **tree){
    if(*tree == NULL) return;

    // získání výšky a hodnoty vyváženosti
    put_height(*tree);
    int balance = get_balance(*tree);

    // rotace uzlů, pokud je strom nevyvážený 
    if(balance > 1){
        if(get_balance((*tree)->left) < 0){
            rotate_left(&(*tree)->left);
        }
        rotate_right(tree);
    }
    else if(balance < -1){
        if(get_balance((*tree)->right) > 0){
            rotate_right(&(*tree)->right);
        }
        rotate_left(tree);
    }
}

/**
 * @brief Vyhledání daného symbolu v tabulce
 * 
 * @param node ukazatel na uzel
 * @param identifier identifikátor symbolu
 * @param found ukazatel na nalezený symbol
 * 
 * @returns bool true pokud byl symbol nalezen
 */
bool search_symbol(tree_node *node, char *identifier,symbol_data **found){
    if(node != NULL){
        if(strcmp(node->data.identifier, identifier) == 0){ // found
            *found = &node->data;
            return true;
        }

        int cmp = strcmp(node->data.identifier, identifier); // pruchod
        if (cmp > 0) {
            return search_symbol(node->left, identifier, found);
        }
        else
            return search_symbol(node->right, identifier, found);
    }

    return false;
}

/**
 * @brief Vložení symbolu do tabulky
 * 
 * @param tree ukazatel na ukazatel na kořen stromu
 * @param data ukazatel na data symbolu
 * 
 * @return void
 */
void insert_symbol(tree_node **tree, symbol_data *data){
    if(*tree == NULL){ // ulozeni na prazdny uzel tzn, vlozeni noveho uzlu
        *tree = (tree_node *)malloc(sizeof(tree_node));
        if(*tree == NULL){
            fprintf(stderr, "Malloc error for inserting symbol\n");
            ifjexit(ERR_INTERNAL);
        }

        (*tree)->data.identifier = NULL;
        load_data(&(*tree)->data, data); // nacteni vsech dat pro ulozeni
        (*tree)->left = NULL;
        (*tree)->right = NULL;
        (*tree)->height = 1;

        return;
    }

    int cmp = strcmp(data->identifier, (*tree)->data.identifier); // ulozeni do praveho nebo leveho podstromu
    if(cmp < 0){
        insert_symbol(&(*tree)->left, data);
    }else if(cmp > 0){
        insert_symbol(&(*tree)->right, data);
    }
    else // jiz existuje
        return;
    put_height(*tree);
    balance_tree(tree);
}

/**
 * @brief Nahrazení uzlu nejpravějším uzlem
 * 
 * @param target cílový uzel k nahrazení
 * @param tree ukazatel na ukazatel na kořen stromu
 * @return void
 */
void replace_by_rightmost(tree_node *target, tree_node **tree){
    if (*tree != NULL){
        if ((*tree)->right == NULL){
            load_data(&target->data, &(*tree)->data);
            tree_node *to_delete = *tree;
            *tree = (*tree)->left;
            free(to_delete->data.identifier);
            free(to_delete);
        }
        else
            replace_by_rightmost(target, &(*tree)->right);
    }
}

/**
 * @brief Smazání symbolu z tabulky
 * 
 * @param tree ukazatel na ukazatel na kořen stromu
 * @param identifier identifikátor
 * 
 * @return void
 */
void delete_symbol(tree_node **tree, char *identifier){
    if (*tree == NULL) return; // symbol nenalezen

    // průchod stromem
    int cmp = strcmp(identifier, (*tree)->data.identifier);
    if(cmp < 0){
        delete_symbol(&(*tree)->left, identifier);
    }else if(cmp > 0){
        delete_symbol(&(*tree)->right, identifier);
    }else{ // nalezení a smazání symbolu
        tree_node *to_delete = *tree;
        if ((*tree)->left == NULL){
            *tree = (*tree)->right;
            free(to_delete->data.identifier);
            free(to_delete);
        }else if((*tree)->right == NULL){
            *tree = (*tree)->left;
            free(to_delete->data.identifier);
            free(to_delete);
        }else{
            replace_by_rightmost(*tree, &(*tree)->left);
        }
    }
    if(*tree != NULL){
        put_height(*tree);
        balance_tree(tree);
    }
}

/**
 * @brief Smazání tabulky symbolů a uvolnění paměti
 * 
 * @param tree ukazatel na ukazatel na kořen
 * @return void
 */
void tree_dispose(tree_node **tree){
    if(*tree != NULL){
        tree_dispose(&(*tree)->left);
        tree_dispose(&(*tree)->right);
        free((*tree)->data.identifier);
        free(*tree);
        *tree = NULL;
    }
}