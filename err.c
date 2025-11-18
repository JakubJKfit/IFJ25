#include <stdio.h>
#include <stdlib.h>
#include "err.h"


void ifjexit(int err_code){
    fprintf(stderr, "Error code: %d\n", err_code);
    exit(err_code);
}