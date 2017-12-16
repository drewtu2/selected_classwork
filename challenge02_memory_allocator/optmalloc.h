#ifndef OPTMALLOC_H
#define OPTMALLOC_H

#include "utils.h"

typedef struct opt_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} opt_stats;

opt_stats* optgetstats();
void optprintstats();

// O(arena_find) + O(global_get) + O(arena_get_index) + O(node_truncate)
// O(c) + O(c) + O(c) + O(logn)
// O(logn)
void* opt_malloc(size_t size);

// O(arena_add) + O(arena_2global)
void opt_free(void* item);

// O(opt_malloc) + O(opt_free)
void* opt_realloc(void* item, size_t size);

void* allocate_pages(size_t size);

node* myList;

#endif
