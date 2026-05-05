CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/*.c
OUT = matrix_project

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

debug:
	$(CC) $(CFLAGS) -DDEBUG_OUTPUT=1 $(SRC) -o $(OUT)

run: all
	./$(OUT)

run-debug: debug
	./$(OUT)

clean:
	rm -f $(OUT)