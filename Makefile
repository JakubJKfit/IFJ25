CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11

TARGET = compiler

all: $(TARGET)

$(TARGET): lex_scanner.o
	$(CC) $(CFLAGS) -o compiler lex_scanner.o

lex_scanner.o: lex_scanner.c lex_scanner.h
	$(CC) $(CFLAGS) -c lex_scanner.c -o lex_scanner.o

clean:
	rm -f lex_scanner.o compiler

.PHONY: all clean
