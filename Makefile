CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iinclude
SRC = src/*.c
OUT = matrix_project
SMOKE_CFLAGS = $(CFLAGS) -DNUM_SIZES=1 -DNUM_PAIRS=1
THRESHOLD_SMOKE_CFLAGS = $(CFLAGS) -DTHRESHOLD_SWEEP=1 -DNUM_SIZES=1 -DNUM_PAIRS=1
THRESHOLD_FULL_CFLAGS = $(CFLAGS) -DTHRESHOLD_SWEEP=1 -DNUM_SIZES=3 -DNUM_PAIRS=5
SMOKE_RESULTS = results/experiment_results_smoke.csv
FULL_RESULTS = results/experiment_results_sizes9_pairs30_seed42.csv
THRESHOLD_SMOKE_RESULTS = results/threshold_results_smoke.csv
THRESHOLD_FULL_RESULTS = results/threshold_results_sizes3_pairs5_thresholds5_seed42.csv
SMOKE_SUMMARY = results/experiment_summary_smoke.csv
FULL_SUMMARY = results/experiment_summary_sizes9_pairs30_seed42.csv
THRESHOLD_SMOKE_SUMMARY = results/threshold_summary_smoke.csv
THRESHOLD_FULL_SUMMARY = results/threshold_summary_sizes3_pairs5_thresholds5_seed42.csv
ANALYSIS_DIR = results/analysis

.PHONY: all debug run run-debug benchmark-smoke benchmark-smoke-resume benchmark-full benchmark-full-resume threshold-smoke threshold-full aggregate-smoke aggregate-full aggregate-threshold-smoke aggregate-threshold-full analyze-full notebook-analysis clean

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

threshold-smoke:
	mkdir -p results
	$(CC) $(THRESHOLD_SMOKE_CFLAGS) $(SRC) -o $(OUT)
	./$(OUT) $(THRESHOLD_SMOKE_RESULTS)

threshold-full:
	mkdir -p results
	$(CC) $(THRESHOLD_FULL_CFLAGS) $(SRC) -o $(OUT)
	./$(OUT) $(THRESHOLD_FULL_RESULTS)

aggregate-smoke:
	python3 scripts/aggregate_results.py $(SMOKE_RESULTS) $(SMOKE_SUMMARY) --expected-samples 1

aggregate-full:
	python3 scripts/aggregate_results.py $(FULL_RESULTS) $(FULL_SUMMARY) --expected-samples 30

aggregate-threshold-smoke:
	python3 scripts/aggregate_results.py $(THRESHOLD_SMOKE_RESULTS) $(THRESHOLD_SMOKE_SUMMARY) --expected-samples 1

aggregate-threshold-full:
	python3 scripts/aggregate_results.py $(THRESHOLD_FULL_RESULTS) $(THRESHOLD_FULL_SUMMARY) --expected-samples 5

analyze-full:
	python3 scripts/plot_summary.py $(FULL_SUMMARY) $(ANALYSIS_DIR)

notebook-analysis:
	.venv/bin/jupyter nbconvert --to notebook --execute notebooks/analyze_results.ipynb --output analyze_results.ipynb --output-dir notebooks --ExecutePreprocessor.timeout=120

clean:
	rm -f $(OUT)
