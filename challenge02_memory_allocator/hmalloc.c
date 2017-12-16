#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "hmalloc.h"

/*
typedef struct hm_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} hm_stats;
*/

static hm_stats stats; // This initializes the stats to 0.
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

hm_stats*
hgetstats()
{
    stats.free_length = free_list_length(myList);
    return &stats;
}

void
hprintstats()
{
    stats.free_length = free_list_length(myList);
    fprintf(stderr, "\n== husky malloc stats ==\n");
    fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
    fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
    fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
    fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
    fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}

static
size_t
div_up(size_t xx, size_t yy)
{
    // This is useful to calculate # of pages
    // for large allocations.
    size_t zz = xx / yy;

    if (zz * yy == xx) {
        return zz;
    }
    else {
        return zz + 1;
    }
}

void*
hmalloc(size_t size)
{

    pthread_mutex_lock(&mutex);

    stats.chunks_allocated += 1;
    size += sizeof(header);
    void* myBlock;

    if(size >= PAGE_SIZE)
    {
        myBlock = allocate_pages(size);
    } else
    {
        // See if there's a big enough block on the free list. If so, select the
        // first one and remove it from the list
        long index_block = free_list_find(myList, size);

        if(index_block >= 0)
        {
            myBlock = free_list_get(&myList, index_block);
            check_valid(myBlock);
        } else
        {
            // If you don't have a block, mmap a new block (1 page = 4096)
            myBlock = allocate_pages(size); // Guarenteed to return a full page
        }

        // If the block is bigger than the request, and the leftover is big enough 
        // to store a free list cell, return the extra to the free list.
        if(block_size(myBlock, HEAD) - size >= sizeof(node))
        {
            void* split = myBlock + size;
            size_t split_size = block_size(myBlock, HEAD) - size;
            
            // myNode points to split which has been casted to a node value
            node* myNode = node_make(split_size, split);

            free_list_add(&myList, myNode);        
        } else
        {
            //printf("Node of size %lu too small; not adding to free list...\n", 
            //                block_size(myBlock, HEAD) - size);
        }
    }
       
    header myheader = header_make_nothread(size); 

    // Use the start of the block to store its size
    *((header*) myBlock) = myheader;

    pthread_mutex_unlock(&mutex);

    // Return a pointer to the block *after* the size field
    return (void*) (myBlock + sizeof(header));
}

void*
allocate_pages(size_t size)
{
    // Calculate the number of pages needed for this block
    int numPages = 1 + (size/PAGE_SIZE);

    // Allocate that many pages with mmap
    // Fill in the size of the block as (# of pages * 4096)
    void* block = mmap(0, numPages * PAGE_SIZE, 
                    PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    stats.pages_mapped += numPages;

    header myHeader = header_make_nothread(numPages*PAGE_SIZE);
    *((header*) block) = myHeader;
    return block;
}

void
hfree(void* item)
{

    pthread_mutex_lock(&mutex);

    stats.chunks_freed += 1;

    // Calculate the size of the given item
    size_t my_block_size = block_size(item, PREV);
   
    // If block size is a page or more, munmap it.  
    if(my_block_size >= PAGE_SIZE)
    {
        munmap(item, my_block_size);
        stats.pages_unmapped += (1 + (my_block_size/PAGE_SIZE));
    } else if(my_block_size > sizeof(node)) 
    { // Add it to the free blocks
        void* temp = (void*)item - sizeof(header);
        node* myNode = node_make(my_block_size, temp);
        
        free_list_add(&myList, myNode);        
    } else
    {
        printf("Memory block too small.... skipping... \n");
    }

    pthread_mutex_unlock(&mutex);
}

void* hrealloc(void* item, size_t size)
{
    // Get new chunk from hmalloc
    void* newChunk = hmalloc(size);

    // Initialize the new chunk to 0.
    newChunk = memset(newChunk, 0, size);

    // Copy the data from item into the new chunk.
    size_t oldSize = block_size(item, PREV);
    newChunk = memcpy(newChunk, item, oldSize);
    
    // Free item
    hfree(item);

    return newChunk;
}
