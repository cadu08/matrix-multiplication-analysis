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

### 4. Strassen Algorithm (Optional)
- Reduces the number of multiplications
- Time complexity: approximately O(n^2.81)
- Higher overhead, beneficial for large matrices only

---

## 🧪 Experimental Methodology

The algorithms are evaluated using:

- Different matrix sizes (e.g., 64 to 1024)
- Execution time measurements
- Multiple runs to compute average performance

All implementations are written in C to minimize external performance influences.

---

## 📊 Analysis

The project compares:

- Theoretical complexity vs observed performance
- Impact of recursion overhead
- Effect of threshold selection in the hybrid approach
- When advanced algorithms outperform the classical method

---

## ⚙️ How to Compile and Run

### Compile:
```bash
make
