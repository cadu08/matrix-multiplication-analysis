CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/*.c
OUT = matrix_project
SMOKE_CFLAGS = $(CFLAGS) -DNUM_SIZES=1 -DNUM_PAIRS=1
SMOKE_RESULTS = results/experiment_results_smoke.csv
FULL_RESULTS = results/experiment_results_sizes5_pairs10_seed42.csv

.PHONY: all debug run run-debug benchmark-smoke benchmark-full clean

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

debug:
	$(CC) $(CFLAGS) -DDEBUG_OUTPUT=1 $(SRC) -o $(OUT)

run: all
	./$(OUT)

run-debug: debug
	./$(OUT)

benchmark-smoke:
	mkdir -p results
	$(CC) $(SMOKE_CFLAGS) $(SRC) -o $(OUT)
	./$(OUT) $(SMOKE_RESULTS)

benchmark-full: all
	mkdir -p results
	./$(OUT) $(FULL_RESULTS)

clean:
	rm -f $(OUT)
