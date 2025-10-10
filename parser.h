#ifndef IFJ_PARSER_H
#define IFJ_PARSER_H
#include <stdbool.h>

// Entry point for the syntactic analysis (LL + precedence for expressions).
// Returns true on success. On syntax error, prints to stderr and returns false.
bool ifj_parse_program(void);

// globální kód chyby
extern int ifj_error_code;

#endif // IFJ_PARSER_H