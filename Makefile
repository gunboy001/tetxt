CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99
DEPS=term.h
OBJ=term.o editor.o draw.o

editor: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm $(OBJ) editor