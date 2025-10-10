// main.c
#include <stdio.h>
#include "parser.h"

int main(void)
{
    if (ifj_parse_program()) {
        return 0;
    }
    if (ifj_error_code) {
        return ifj_error_code;  // 3 pro chybějící main
    }
    return 2; // obecná syntaktická/lexikální chyba
}
