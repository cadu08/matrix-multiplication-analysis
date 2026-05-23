CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/*.c
OUT = matrix_project
SMOKE_CFLAGS = $(CFLAGS) -DNUM_SIZES=1 -DNUM_PAIRS=1
SMOKE_RESULTS = results/experiment_results_smoke.csv
FULL_RESULTS = results/experiment_results_sizes5_pairs10_seed42.csv
SMOKE_SUMMARY = results/experiment_summary_smoke.csv
FULL_SUMMARY = results/experiment_summary_sizes5_pairs10_seed42.csv
ANALYSIS_DIR = results/analysis

.PHONY: all debug run run-debug benchmark-smoke benchmark-smoke-resume benchmark-full benchmark-full-resume aggregate-smoke aggregate-full analyze-full clean

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

benchmark-smoke-resume:
	mkdir -p results
	$(CC) $(SMOKE_CFLAGS) $(SRC) -o $(OUT)
	./$(OUT) $(SMOKE_RESULTS) --resume

benchmark-full: all
	mkdir -p results
	./$(OUT) $(FULL_RESULTS)

benchmark-full-resume: all
	mkdir -p results
	./$(OUT) $(FULL_RESULTS) --resume

aggregate-smoke:
	python3 scripts/aggregate_results.py $(SMOKE_RESULTS) $(SMOKE_SUMMARY) --expected-samples 1

aggregate-full:
	python3 scripts/aggregate_results.py $(FULL_RESULTS) $(FULL_SUMMARY) --expected-samples 10

analyze-full:
	python3 scripts/plot_summary.py $(FULL_SUMMARY) $(ANALYSIS_DIR)

clean:
	rm -f $(OUT)
