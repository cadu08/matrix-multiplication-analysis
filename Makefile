CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/*.c
OUT = matrix_project

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)