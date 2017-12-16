#include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#include "optmalloc.h"

/*
typedef struct opt_stats {
    long pages_mapped;
    long pages_unmapped;
    long chunks_allocated;
    long chunks_freed;
    long free_length;
} opt_stats;
*/

static opt_stats stats; // This initializes the stats to 0.

__thread arena ar;
__thread bool firstRun = true;
__thread mailbox myMailbox;

void*
opt_malloc(size_t size)
{
    stats.chunks_allocated += 1;
    size += sizeof(header);
    void* myBlock;
    node* myNode;

    if(firstRun)
    {
        //arena_set(&ar);  
        firstRun = false;
        set_thread_id();

        // Set up mailbox to receive data
        myMailbox = mailbox_make();
        mailbox_add(&myMailbox);

        printf("Thread %d first malloc\n", my_thread_id);
    }
  
    // Check if data is available in our mailbox
    pthread_mutex_lock(&(myMailbox.mutex_notif));
    bool data_available = myMailbox.data_available;
    pthread_mutex_unlock(&(myMailbox.mutex_notif));
   
    // If data is available, move it to our local arena 
    if(data_available)
    {
        mailbox_receive(&myMailbox, &ar);
    }

    // The requested size is over a page; mmap it directly.
    if(size >= PAGE_SIZE) {
        //printf("opt_malloc; large request...\n");
        myBlock = allocate_pages(size);
    }
    // The requested size is under a page; go through the arenas. 
    else {
        // Find the bin that contains the smallest node greater than size.
        int arena_index = arena_find(&ar, size);
        
        // There is nothing in the local arena that can be used.
        if(arena_index == -1) {
            myNode = global_get();

            // There is nothing in the global arena that can be used.            
            if (myNode == NULL) {
                myNode = mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
                myNode->size = PAGE_SIZE;
                myNode->next = NULL;
                stats.pages_mapped += 1;
                //printf("opt_malloc: mmapped new page for size %lu\n", size);
            } else
            {
                //printf("opt_malloc: used global arena for size %lu\n", size);
            }
        }
        // The local arena has a node available.
        else {
            // Get the first available node greater than size.
            myNode = arena_get_index(&ar, arena_index);
            //printf("opt_malloc: used local arena for size %lu\n", size);
        }
    
        check_valid_size(myNode);

        // Truncate the node as needed
        node_truncate(myNode, size, &ar);

        myBlock = (void*) myNode;

        check_valid_size(myNode);

        // Create a temp header...
        header myHeader = header_make(myNode->size);

        // Make sure the size is set correctly.
        *((header*) myBlock) = myHeader; 
    }

    // Return the chunk AFTER the size field.
    return myBlock + sizeof(header);
}

void
opt_free(void* item)
{
    stats.chunks_freed += 1;

    header my_block_header = block_header(item, PREV);
    // Calculate the size of the given item
    size_t my_block_size = my_block_header.size;
    
    if(my_block_header.parent == my_thread_id)
    { 
       
        // If block size is a page or more, munmap it.  
        if(my_block_size > PAGE_SIZE)
        {
            munmap(item, my_block_size);
            stats.pages_unmapped += (1 + (my_block_size/PAGE_SIZE));
        } else if(my_block_size > sizeof(node)) 
        { // Add it to the free blocks
            void* temp = (void*)item - sizeof(header);
            node* myNode = node_make(my_block_size, temp);

            arena_add(&ar, myNode);        
        } else
        {
            //printf("Memory block too small.... skipping... \n");
        }
        
        // Return blocks to the global scope if necessary
        arena_2global(&ar);
        //optprintstats();
    } 
    else // We don't own the block, post it to the appropriate mailbox
    {
        void* temp = (void*)item - sizeof(header);
        node* myNode = node_make(my_block_size, temp);
        mailbox_post(my_block_header.parent, (node*)myNode);
    }
}

void* opt_realloc(void* item, size_t size)
{
    // Get the new chunk from opt_realloc
    void* newChunk = opt_malloc(size);
  
    size_t actual_size = block_size(newChunk, PREV); 
    // Initialzie the new chunk to 0 
    newChunk = memset(newChunk, 0, actual_size - sizeof(header));

    // Copy the data from item into the new chunk
    size_t oldSize = block_size(item, PREV);
    newChunk = memcpy(newChunk, item, oldSize - sizeof(header));

    // Free the old item
    opt_free(item);
   
    // Return the new item 
    return (void*) newChunk;
}

opt_stats*
optgetstats()
{
    stats.free_length = free_list_length(myList);
    return &stats;
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
    
    header myHeader = header_make(numPages * PAGE_SIZE);

    *((header*) block) = myHeader;
    return block;
}

void
optprintstats()
{
    //stats.free_length = free_list_length(myList);
    fprintf(stderr, "\n== husky malloc stats ==\n");
    fprintf(stderr, "Mapped:   %ld\n", stats.pages_mapped);
    fprintf(stderr, "Unmapped: %ld\n", stats.pages_unmapped);
    fprintf(stderr, "Allocs:   %ld\n", stats.chunks_allocated);
    fprintf(stderr, "Frees:    %ld\n", stats.chunks_freed);
    fprintf(stderr, "Freelen:  %ld\n", stats.free_length);
}
