#include <stddef.h>
#include <stdlib.h>

#include "heap_tracker.h"

typedef union {
    size_t size;
    max_align_t align;
} allocation_header_t;

static heap_stats_t stats = {0, 0, 0};

void heap_tracker_reset(void) {
    stats.current_bytes = 0;
    stats.peak_bytes = 0;
    stats.allocation_count = 0;
}

heap_stats_t heap_tracker_get_stats(void) {
    return stats;
}

void *tracked_malloc(size_t size) {
    allocation_header_t *header = malloc(sizeof(*header) + size);

    if (header == NULL) {
        return NULL;
    }

    /*
     * Store the requested payload size so recursive frees can decrement the
     * active footprint exactly, independent of allocator metadata overhead.
     */
    header->size = size;
    stats.current_bytes += size;
    stats.allocation_count++;

    if (stats.current_bytes > stats.peak_bytes) {
        stats.peak_bytes = stats.current_bytes;
    }

    return header + 1;
}

void tracked_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    allocation_header_t *header = ((allocation_header_t *) ptr) - 1;
    stats.current_bytes -= header->size;
    free(header);
}
