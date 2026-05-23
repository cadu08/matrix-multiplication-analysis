# Matrix Multiplication Algorithms Analysis

This project presents a comparative study of different matrix multiplication algorithms, focusing on both theoretical complexity and practical performance.

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

### 3. Hybrid Approach
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

The tracked heap metrics are reset for each algorithm run and measure the recursive algorithm allocations directly, instead of relying only on process-level RSS.

---

## 📊 Analysis

The project compares:

- Theoretical complexity vs observed performance
- Impact of recursion overhead
- Effect of threshold selection in the hybrid approach
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
