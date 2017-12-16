#ifndef HMALLOC_H
#define HMALLOC_H

#include "utils.h"

// Husky Malloc Interface
// cs3650 Starter Code

typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;

hm_stats* hgetstats();
void hprintstats();

void* hmalloc(size_t size);
void hfree(void* item);
void* hrealloc(void* item, size_t size);

void* allocate_pages(size_t size);
node* myList;

#endif