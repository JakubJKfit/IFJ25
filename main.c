#include <stdio.h>
#include "parser.h"

int main(void) {
    if (!ifj_parse_program()) {
        // Parser sám rozeznává chybějící main() a tiskne msg;
        // pro rozlišení 2/3 můžeš si uvnitř parseru nastavit globální errcode.
        // Základně vrať 2 (syntaktická chyba); pokud chceš 3, propaguj to proměnnou.
        return 2;
    }
    return 0;
}
