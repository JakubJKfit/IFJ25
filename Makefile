CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Werror -pedantic
TARGET  = ifj

SRC		= lex_scanner.c parser.c main.c ast.c ast_printer.c err.c symtable.c symstack.c codegen.c codegenhelp.c
OBJ     = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	python3 test.py

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean test
