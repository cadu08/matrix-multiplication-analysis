# Experimental Protocol and Result Interpretation

This document records how the matrix multiplication experiments are organized,
which files are generated, and how future agents should interpret the exported
data.

## Purpose

The repository compares five square matrix multiplication implementations:

- `iterative`: classical three-loop baseline, theoretical time `O(n^3)`.
- `recursive`: pure divide-and-conquer, theoretical time `O(n^3)`.
- `hybrid_divide_conquer`: divide-and-conquer above a threshold and iterative
  multiplication below it.
- `strassen`: pure Strassen recursion, theoretical time `O(n^log2(7))`.
- `hybrid_strassen`: Strassen above a threshold and iterative multiplication
  below it.

The current experimental setup tests powers of two:

```text
64, 128, 256, 512, 1024
```

For each size, the benchmark generates ten deterministic pairs of matrices
using `FIXED_SEED = 42`. Matrix entries are pseudo-random `double` values in
the open interval `(0, 1)`.

## Benchmark Commands

Use smoke targets for quick pipeline validation:

```bash
make benchmark-smoke
make aggregate-smoke
```

Use full targets for the official experimental collection:

```bash
make benchmark-full
make aggregate-full
make analyze-full
```

If the full benchmark is interrupted, resume it instead of starting over:

```bash
make benchmark-full-resume
make aggregate-full
make analyze-full
```

Resume mode reads the existing raw CSV before running, marks only complete rows
with `is_correct=1` as reusable, and appends missing algorithm/size/pair runs.
This bookkeeping is intentionally outside the timed multiplication interval.

## Generated Files

The standard raw full benchmark writes:

```text
results/experiment_results_sizes5_pairs10_seed42.csv
```

The standard aggregated full benchmark writes:

```text
results/experiment_summary_sizes5_pairs10_seed42.csv
```

The first-pass analysis target writes:

```text
results/analysis/time_seconds_mean.svg
results/analysis/heap_peak_bytes_mean.svg
results/analysis/experimental_summary.md
```

CSV and analysis outputs under `results/` are generated artifacts and are
ignored by Git.

## Raw CSV Schema

Each raw row corresponds to one algorithm execution for one matrix size and one
matrix pair.

Important columns:

- `algorithm`: algorithm identifier.
- `matrix_size`: `n` for an `n x n` multiplication.
- `pair_id`: deterministic input-pair index, from `1` to `NUM_PAIRS`.
- `time_seconds`: wall-clock time for the multiplication call only.
- `memory_before_kb`, `memory_after_kb`, `memory_difference_kb`: process-level
  memory readings based on `getrusage`.
- `heap_current_bytes`: tracked heap bytes still active after the algorithm
  returns.
- `heap_peak_bytes`: maximum simultaneously active tracked heap bytes during
  the algorithm call.
- `heap_allocations`: number of successful `tracked_malloc` calls during the
  algorithm call.
- `is_correct`: `1` if the result matches the iterative reference within the
  configured floating-point tolerance.
- `max_abs_error`: maximum absolute difference from the iterative reference.
- `max_rel_error`: maximum relative difference from the iterative reference.

## Timing Interpretation

`time_seconds` is the primary execution-time metric.

It includes only:

- the selected multiplication algorithm call;
- allocations performed inside that algorithm;
- recursive work inside that algorithm.

It does not include:

- input matrix generation;
- reference result generation;
- correctness validation;
- CSV parsing for resume mode;
- CSV aggregation;
- SVG or Markdown generation.

This separation is important: resume mode and output flushing do not make
`time_seconds` a resume-aware or I/O-contaminated metric.

## Correctness Interpretation

The iterative algorithm is used as the numerical reference for each generated
matrix pair.

The benchmark does not require bitwise equality because all algorithms operate
on `double`, and recursive algorithms change the order of additions. Strassen in
particular performs extra additions and subtractions, so small floating-point
differences are expected even when the implementation is correct.

Treat a run as experimentally valid only when:

```text
is_correct = 1
```

The aggregation targets enforce sample completeness. For the full benchmark,
every `(algorithm, matrix_size)` group must contain ten samples.

## Memory Interpretation

Prefer `heap_peak_bytes` for algorithm-level memory analysis.

`heap_peak_bytes` is measured through `tracked_malloc` and `tracked_free`, which
are used by the recursive and hybrid implementations for temporary matrices.
It measures the active payload bytes requested by the algorithm, independent of
allocator metadata.

Interpretation notes:

- `iterative` usually reports `heap_peak_bytes = 0` because it does not allocate
  temporary matrices through `tracked_malloc`.
- Hybrid algorithms report `0` at sizes where the threshold sends execution
  directly to the iterative kernel.
- `strassen` and `hybrid_strassen` should use more tracked heap than
  divide-and-conquer variants because they allocate more temporary matrices.
- `heap_current_bytes` should be `0` after a correct run. Nonzero values would
  suggest a leak in tracked allocations.

Use `memory_difference_kb` cautiously. It is derived from process-level RSS
high-water behavior and can reflect allocator or operating-system effects rather
than the algorithm call alone.

## Aggregated CSV Schema

The aggregation script groups raw rows by:

```text
algorithm, matrix_size
```

For each metric it exports:

- `<metric>_mean`
- `<metric>_stddev`
- `<metric>_min`
- `<metric>_max`

It also exports:

- `sample_count`
- `correct_sample_count`
- `all_correct`

For scientific reporting, prefer the aggregated file for tables and plots, but
return to the raw CSV when investigating outliers.

## Current Plausibility Notes

The current full results are plausible:

- `recursive` grows close to the expected `O(n^3)` trend but is much slower than
  `iterative` because of recursive allocation and copying overhead.
- Pure `strassen` is faster than pure divide-and-conquer at larger sizes but
  still expensive because it recurses to `n == 1`.
- Hybrid algorithms are the practical winners.
- `hybrid_strassen` is fastest for the larger tested sizes.
- Strassen variants show larger floating-point errors than the iterative
  baseline, but current errors remain small relative to the result magnitudes.
- Tracked heap usage is higher for Strassen variants than for divide-and-conquer
  variants, as expected from the extra temporary matrices.

Avoid drawing strong conclusions from `64` and `128` alone. These runs are very
short and therefore more sensitive to measurement noise.

## Recommended Future Workflow

When changing algorithm code:

1. Run `make benchmark-smoke`.
2. Run `make aggregate-smoke`.
3. Check that all smoke rows have `is_correct=1`.
4. Run `make benchmark-full-resume` only when ready to extend or complete the
   official full collection.
5. Run `make aggregate-full`.
6. Run `make analyze-full`.

When changing only analysis scripts:

1. Reuse the existing aggregated CSV if the raw experiment has not changed.
2. Run `make analyze-full`.
3. Do not rerun the expensive full benchmark unless the algorithm or benchmark
   instrumentation changed.

## Reproducibility Controls

Key compile-time constants are defined in `src/main.c` and can be overridden
through `CFLAGS`:

- `NUM_SIZES`
- `NUM_PAIRS`
- `FIXED_SEED`
- `HYBRID_DIVIDE_CONQUER_THRESHOLD`
- `HYBRID_STRASSEN_THRESHOLD`
- `VALIDATION_ABS_TOLERANCE`
- `VALIDATION_REL_TOLERANCE`

The Makefile currently standardizes:

- smoke run: `NUM_SIZES=1`, `NUM_PAIRS=1`;
- full run: default constants, currently five sizes and ten pairs.
