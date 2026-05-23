# Matrix Multiplication Algorithms Analysis

This project presents a comparative study of different matrix multiplication algorithms, focusing on both theoretical complexity and practical performance.

For the detailed experimental protocol, output schema and interpretation notes,
see [`docs/experimental_protocol.md`](docs/experimental_protocol.md).

## 📌 Objective

The goal of this project is to analyze and compare multiple approaches to matrix multiplication, evaluating:

- Execution time
- Memory usage (qualitative)
- Trade-offs between algorithms
- Differences between theoretical and empirical performance

---

## 🧠 Algorithms Implemented

The following algorithms are implemented in C:

### 1. Iterative (Classical Algorithm)
- Uses three nested loops
- Time complexity: O(n³)
- Serves as a baseline for comparison

---

### 2. Divide and Conquer
- Recursively splits matrices into submatrices
- Same asymptotic complexity as the iterative approach: O(n³)
- Different memory and cache behavior

---

### 3. Hybrid Divide-and-Conquer Approach
- Combines recursion and iteration
- Uses divide and conquer for large matrices
- Switches to iterative multiplication below a threshold
- Reduces recursion overhead in practice

---

### 4. Strassen Algorithm
- Reduces the number of multiplications
- Time complexity: approximately O(n^2.81)
- Implemented as a pure recursive algorithm, without an iterative threshold
- Higher overhead from temporary matrices and extra additions/subtractions
- Expected to become advantageous only when the reduction from eight to seven
  recursive multiplications offsets the larger memory and recursion overhead

---

### 5. Hybrid Strassen Approach
- Combines Strassen recursion and iterative multiplication
- Uses Strassen's seven recursive products for large matrices
- Switches to iterative multiplication below a threshold
- Expected to reduce Strassen's allocation and recursion overhead on small
  subproblems while preserving its asymptotic advantage at upper levels

---

## 🧪 Experimental Methodology

The experiments use square matrices with the following sizes:

- 64 × 64
- 128 × 128
- 256 × 256
- 512 × 512
- 1024 × 1024

For each size, 10 pairs of matrices `A` and `B` are generated.

The matrices are filled with pseudo-random floating-point values in the open interval `(0, 1)`, excluding both zero and one. A fixed random seed is used to ensure reproducibility, meaning that the same input matrices can be generated again in future executions unless the seed is changed.

The CSV output also includes heap instrumentation for the algorithm execution:

- `heap_current_bytes`: memory still allocated by tracked algorithm allocations after the call finishes
- `heap_peak_bytes`: maximum tracked heap memory in use at the same time
- `heap_allocations`: total number of successful tracked dynamic allocations
- `is_correct`: whether the measured result matches the iterative reference within the configured numerical tolerance
- `max_abs_error`: largest absolute difference from the iterative reference matrix
- `max_rel_error`: largest relative difference from the iterative reference matrix

The tracked heap metrics are reset for each algorithm run and measure the recursive algorithm allocations directly, instead of relying only on process-level RSS.

Before each measured run, the iterative algorithm computes a reference result for the generated matrix pair. Each algorithm result is validated against that reference after timing ends, so correctness checks do not inflate the recorded multiplication time. Because the matrices use floating-point values, validation uses absolute and relative error tolerances instead of requiring bitwise equality; this avoids rejecting numerically equivalent results caused only by different addition orders.

---

## 📊 Analysis

The project compares:

- Theoretical complexity vs observed performance
- Impact of recursion overhead
- Effect of threshold selection in the hybrid divide-and-conquer and hybrid
  Strassen approaches
- Whether the asymptotic reduction of Strassen compensates for its additional
  memory traffic and arithmetic overhead at the tested sizes
- When advanced algorithms outperform the classical method

---

## ⚙️ How to Compile and Run

### Compile:

To compile the project, run:

```bash
make
```

This generates the executable:

```bash
./matrix_project
```

### Run

To execute the program:

```bash
./matrix_project
```

Or use:

```bash
make run
```

By default, the executable writes to:

```bash
results/experiment_results.csv
```

An alternative CSV path can be supplied as the first argument:

```bash
./matrix_project results/custom_experiment.csv
```

### Benchmark Targets

For quick validation during development, run:

```bash
make benchmark-smoke
```

This compiles a short benchmark configuration with one matrix size and one
matrix pair, then writes:

```bash
results/experiment_results_smoke.csv
```

To resume this smoke benchmark without duplicating already completed rows, run:

```bash
make benchmark-smoke-resume
```

For the full experimental collection described above, run:

```bash
make benchmark-full
```

This writes:

```bash
results/experiment_results_sizes5_pairs10_seed42.csv
```

If a long benchmark is interrupted, continue from the already completed
algorithm/size/pair rows with:

```bash
make benchmark-full-resume
```

The smoke target is intended only to verify compilation, CSV generation,
correctness validation and heap instrumentation. The full target is the one
intended for later statistical aggregation.

Resume mode reads the existing CSV before the experiment starts, marks only
complete rows with `is_correct=1` as reusable, and appends missing runs to the
same file. This bookkeeping, CSV flushing and progress output happen outside
the timed multiplication interval, so the recorded `time_seconds` metric remains
restricted to the algorithm call itself.

### Result Aggregation

After generating a raw benchmark CSV, aggregate the samples by algorithm and
matrix size with:

```bash
python3 scripts/aggregate_results.py results/experiment_results_sizes5_pairs10_seed42.csv results/experiment_summary_sizes5_pairs10_seed42.csv
```

Equivalent Make targets are available for the standard benchmark outputs:

```bash
make aggregate-smoke
make aggregate-full
```

The aggregation output includes `sample_count`, `correct_sample_count`,
`all_correct`, and mean, sample standard deviation, minimum and maximum for
each recorded metric. These grouped values are intended for comparing observed
growth trends against the theoretical complexity of each algorithm.

The standard Make targets also validate the expected number of samples per
group: `aggregate-smoke` requires one sample and `aggregate-full` requires ten
samples for every algorithm/size combination. This prevents an interrupted
benchmark run from being summarized as if it were complete.

### First-Pass Analysis Artifacts

After `make aggregate-full` succeeds, generate initial comparison artifacts with:

```bash
make analyze-full
```

This creates SVG plots and a Markdown summary under:

```bash
results/analysis/
```

The generated plots include mean execution time and mean tracked heap peak by
algorithm and matrix size. The charts use logarithmic scales to make growth
patterns visible across the full range of tested inputs.

### Debug Mode

The project also includes an optional debug mode.

Debug mode enables additional validation output, such as saving sample generated matrices to text files. This is useful for checking whether the matrix generation process is working correctly without printing large matrices directly in the terminal.

To compile in debug mode:

```bash
make debug
```

To compile and run directly in debug mode:

```bash
make run-debug
```

When debug mode is enabled, the program uses the `DEBUG_OUTPUT` flag. This allows debug-specific behavior to be activated without changing the main experiment logic.

Debug mode should be used only for validation and development. It should not be used for final performance measurements, because writing matrices to files adds extra I/O overhead that can affect execution time.

### Clean

To remove the compiled executable:

```bash
make clean
```

---

## Notes

- The project is implemented in C.
- Matrix sizes follow a power-of-two progression to simplify recursive algorithms.
- A fixed random seed is used for reproducibility.
- Debug output is optional and controlled through the Makefile.
- Performance measurements should be executed without debug mode enabled.
