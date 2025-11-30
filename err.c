/**
 * @file err.c
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Soubor obsahující jednotnou fuknci pro správné vypnutí programu se správnou návratovou hodnotou
 */

#include <stdio.h>
#include <stdlib.h>
#include "err.h"

/**
 * @brief Funkce vypne program
 * @param err_code návratová hodnota
 * @returns void
 */
void ifjexit(int err_code){
    fprintf(stderr, "Error code: %d\n", err_code);
    exit(err_code);
}