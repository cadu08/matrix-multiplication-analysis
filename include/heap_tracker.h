#ifndef HEAP_TRACKER_H
#define HEAP_TRACKER_H

#include <stddef.h>

typedef struct {
    size_t current_bytes;
    size_t peak_bytes;
    size_t allocation_count;
} heap_stats_t;

void heap_tracker_reset(void);
heap_stats_t heap_tracker_get_stats(void);
void *tracked_malloc(size_t size);
void tracked_free(void *ptr);

#endif
