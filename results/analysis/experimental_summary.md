# Experimental Summary

This report is generated from the aggregated benchmark CSV.

## Fastest Algorithm By Size

| Matrix size | Algorithm | Mean time |
| --- | --- | ---: |
| 64 | Hybrid Strassen | 233.9 us |
| 128 | Hybrid D&C | 2.11 ms |
| 256 | Hybrid Strassen | 12.69 ms |
| 512 | Hybrid Strassen | 79.79 ms |
| 1024 | Hybrid Strassen | 560.21 ms |

## Lowest Tracked Heap Peak By Size

| Matrix size | Algorithm | Mean tracked heap peak |
| --- | --- | ---: |
| 64 | Hybrid D&C | 0 B |
| 128 | Iterative | 0 B |
| 256 | Iterative | 0 B |
| 512 | Iterative | 0 B |
| 1024 | Iterative | 0 B |

## Lowest Positive Tracked Heap Peak By Size

| Matrix size | Algorithm | Mean tracked heap peak |
| --- | --- | ---: |
| 64 | Divide and Conquer | 149.3 KiB |
| 128 | Hybrid D&C | 448.0 KiB |
| 256 | Hybrid D&C | 2.2 MiB |
| 512 | Hybrid D&C | 9.2 MiB |
| 1024 | Hybrid D&C | 37.2 MiB |

## Notes

- Time statistics are based on the strict multiplication interval recorded by the benchmark.
- Heap statistics include only allocations routed through `tracked_malloc`/`tracked_free`.
- Zero tracked heap values mean the algorithm did not allocate through the tracker for that configuration.
