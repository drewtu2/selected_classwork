#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <math.h>
#include <stdbool.h>
#include <sys/mman.h>

static pthread_mutex_t mutexx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_thread_counter = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_mailbox = PTHREAD_MUTEX_INITIALIZER;

node* global_ar = NULL;

__thread int my_thread_id;

void set_thread_id()
{
    pthread_mutex_lock(&mutex_thread_counter);
    my_thread_id = thread_counter; 
    thread_counter++;
    pthread_mutex_unlock(&mutex_thread_counter);
}

/******************************************************************************
 * Manipulations to Mailbox
******************************************************************************/
// The global maibox_head
mailbox* mailbox_head = NULL;

mailbox mailbox_make()
{
    mailbox myBox;
    myBox.data = NULL;
    myBox.mutex_box = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    myBox.mutex_notif = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    myBox.data_available = false;
    myBox.next = NULL;

    return myBox;
}

void mailbox_add(mailbox* newBox)
{
    pthread_mutex_lock(&mutex_mailbox);
    if(mailbox_head == NULL)
    {
        mailbox_head = newBox;
    } 
    else 
    {
        mailbox* temp = mailbox_head;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }

        temp->next = newBox;
    }
    pthread_mutex_unlock(&mutex_mailbox);
}

void mailbox_post(int boxNum, node* mail)
{
    check_valid_size(mail);

    // Get the desired box 
    mailbox* dest_box = mailbox_get_index(boxNum);
   
    // Lock the box's mutex
    pthread_mutex_lock(&(dest_box->mutex_box));
  

    // Insert mail into the front of the list 
    mail->next = dest_box->data; 

    // Move the pointer to mail
    dest_box->data = mail;
    
    // Signal Data has been made available  
    pthread_mutex_lock(&(dest_box->mutex_notif));
    dest_box->data_available = true;
    pthread_mutex_unlock(&(dest_box->mutex_notif));

    // Unlock mutex
    pthread_mutex_unlock(&(dest_box->mutex_box));
}

void  mailbox_receive(mailbox* dest_box, arena* myArena)
{
    // Get the box corresponding to this thread
    assert(dest_box != NULL);
    
    pthread_mutex_lock(&(dest_box->mutex_box));

    node* key = dest_box->data;
    node* temp;

    while(key != NULL)
    {
        temp = key;
        key = key->next;

        // Detach temp from the list. 
        temp->next = NULL;
        arena_add(myArena, temp); 
    }

    dest_box->data = NULL;
    
    // Reset the data available flag 
    pthread_mutex_lock(&(dest_box->mutex_notif));
    dest_box->data_available = false;
    pthread_mutex_unlock(&(dest_box->mutex_notif));
    
    pthread_mutex_unlock(&(dest_box->mutex_box));
}

mailbox* mailbox_get_index(int index)
{
    int counter = 0;
    mailbox* temp = mailbox_head;
    while(temp != NULL)
    {
        if(index == counter)
        {
            return temp; 
        }
        temp = temp->next;
        counter++;
    }

    return NULL;

}

/******************************************************************************
 * Manipulations to Arena Make
******************************************************************************/

// O(1)
void arena_set(arena* myArena)
{
    void* initialBlock = mmap(0, INITIAL_PAGES * PAGE_SIZE, 
                    PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    node* temp; 

    for(int ii = 0; ii < INITIAL_PAGES - 1; ++ii)
    {
        temp = node_make(PAGE_SIZE, initialBlock + (ii * PAGE_SIZE)); 
        temp->size = PAGE_SIZE;
        temp->next = (node*)((void*)temp + PAGE_SIZE);

        //if(ii == 0)
        //{
        //    arena_add(myArena, temp);
        //}
    }
    temp->next->size = PAGE_SIZE;
    
    myArena->bins[8] = (node*)initialBlock;
    
}

// O(nLogN)
node* arena_get(arena* myArena, size_t size)
{
    int index = arena_find(myArena, size);
    node* temp = arena_get_index(myArena, index);

    // Break out and return NULL if a suitable node could not be found
    if(temp == NULL)
    {
        return NULL;
    }

    node_truncate(temp, size, myArena);
    return temp;
}

//O(nLogN)
void node_truncate(node* myNode, size_t size, arena* myArena)
{
    //TODO: HERE for testing pruposes...
    int temp_size = myNode->size;
    node* second_half;

    // If the node returned is greater than 2X the size requested, we need to 
    // split until our size is less than 2x the requested size.
    while(temp_size >= size*2)
    {
        temp_size = myNode->size;

        second_half = node_split(myNode);
        arena_add(myArena, second_half);

        // Making sure we updated the size in temp... 
        assert(myNode->size != temp_size);
        temp_size = myNode->size; 
    }
}

// O(c)
int arena_find(arena* myArena, size_t size)
{
    // Search the array...
    for(int ii = 0; ii < NUM_BINS; ++ii)
    {
        bool largeEnough = pow(2, ii + 4) >= size;
        bool notNull = myArena->bins[ii] != NULL;

        if(largeEnough && notNull)
        {
            return ii;
        }
    }    

    // Couldn't find a suitable bin size
    return -1;
}

// O(c)
node* arena_get_index(arena* myArena, int index)
{
    // Get the HEAD node from the list
    node* tempList = myArena->bins[index]; 
   
    if(tempList != NULL) 
    {
        // Move the HEAD node of the arena to the next node. 
        myArena->bins[index] = tempList->next; 
    }

    return tempList;
}

// O(n)
void arena_add(arena* myArena, node* myNode)
{
    // Get the HEAD node from the list
    node** tempList;
    size_t size;
    bool added = false;
    
    check_valid((void *) myNode); 
    check_valid_size(myNode);

    size = myNode->size;

    if(size <= PAGE_SIZE)
    {
        // Search the array...
        for(int ii = 0; ii < NUM_BINS; ++ii)
        {
            tempList = &myArena->bins[ii]; 

            bool correctSize = pow(2, ii + 4) == size;
            
            if(correctSize)
            {
                free_list_add_opt(tempList, myNode, myArena);         
                added = true;
                break;
            }
        }    
    }
    else if(size % PAGE_SIZE == 0)
    {
        myNode->next = NULL;
        void* initialBlock = (void*)myNode;
        node* temp;
        for(int ii = 0; ii < size/PAGE_SIZE; ++ii)
        {
            temp = node_make(PAGE_SIZE, initialBlock + (ii * PAGE_SIZE)); 
            temp->size = PAGE_SIZE;

            arena_add(myArena, temp);
        }
        added = true;
    }
    if(!added)
    {
        printf("Error in Arena Add: item of size %lu never added to the arena\n"
                        , size); 
    }
    
}

// O(n)
void arena_2global(arena* ar)
{
    int pageIndex = NUM_BINS - 1;
    int sizePageBin = free_list_length(ar->bins[pageIndex]);
    
    while(sizePageBin >= MAX_LOCAL_LENGTH)
    {
        node* temp = arena_get_index(ar, pageIndex);
        temp->next = NULL;

        global_add(temp); 

        sizePageBin--; 
    }
}

/******************************************************************************
 * Manipulations to Free List
******************************************************************************/
// O(c)
node* node_make(size_t size, void* me)
{
    node* myNode = (node*) me;
    myNode->size = size;
    myNode->next = NULL;

    return myNode;
}
// N/A
void check_free_list(node* head)
{
    node* temp = head;
    long length = 0; 

    if (temp != NULL)
    {
    while(1)
    {
        if(temp->next != NULL && temp > temp->next)
        {
            printf("Error: out of order at index %lu\n", length);
        }
        if((void*)temp + temp->size == temp->next)
        {
            printf("Error: index %lu can be combined with index %lu \n", length, length + 1);
            printf("temp: %lu, length: %lu\n", *(long*)temp, temp->size);
            printf("temp-next: %lu, length: %lu\n", *(long*)temp->next, temp->next->size);
        }
        if(temp->next == NULL)
        {
            break;    
        } else {
            temp = temp->next;     
            length++;
        }
    }
    }
}


// O(n)
long free_list_min(node* head)
{
    node* temp = head;
    long length = 1; 
    long min = PAGE_SIZE;
    
    if(temp == NULL)
    {
        return 0;
    }

    while(1)
    {
        if(temp->next == NULL)
        {
            break;    
        } else {
            if(temp->size < min)
            {
                min = temp->size;
            }
            temp = temp->next;     
            length++;
        }
    }

    return min;
}

// O(n)
long free_list_max(node* head)
{
    node* temp = head;
    long length = 1; 
    long max = 0;
    
    if(temp == NULL)
    {
        return 0;
    }

    while(1)
    {
        if(temp->next == NULL)
        {
            break;    
        } else {
            if(temp->size > max)
            {
                max = temp->size;
            }
            temp = temp->next;     
            length++;
        }
    }

    return max;
}

// O(n)
long free_list_length(node* head)
{
    node* temp = head;
    long length = 1; 
    
    if(temp == NULL)
    {
        return 0;
    }

    while(1)
    {
        if(temp->next == NULL)
        {
            break;    
        } else {
            temp = temp->next;     
            length++;
        }
    }

    return length;
}

//O(n^2)
void free_list_add_opt(node** head, node* item, arena* l_arena)
{
    check_valid((void*) item);
    check_valid_size(item);
    free_list_add_helper(head, item, l_arena);
}

void free_list_coalesce_opt2(node** head, node* prev_prev, node* prev, node* key,  arena* l_arena)
{
    check_valid((void*)*head);
    if(key->size != PAGE_SIZE)
    {
        node* next = key->next; 

        // Item was inserted AFTER Key
        if(prev != NULL && (void*)prev + prev->size == key)
        {
            prev->size += key->size;
            prev->next = NULL;
            arena_add(l_arena, prev); 

            if(prev_prev == NULL)
            {
                check_valid((void*)next);
                *head = next;
            } else
            {
                 prev_prev->next = next;
            }

        }
        // Item was inserted BEFORE key
        else if((void*)key + key->size == next)
        {
            // remove key and next 
            key->size += next->size;
            key->next = NULL;
           
            // Key & Next are now detached... 
            
            // Stick key back into the arena
            arena_add(l_arena, key);
            
            if(prev == NULL) { 
            // If the first item coalesces, then prev will not have been set. 
            // Move the HEAD to be next->next
                check_valid((void*)(next->next));
                *head = next->next; 
            }
            else {
                // Guarenteed that next exists... Safe to assign key to next->next
                key = next->next;
                prev->next = key;
            }

        } 
    }
    check_valid((void*)(*head));
    check_valid((void*)(prev_prev));
    check_valid((void*)(prev));
    check_valid((void*)(key));
}

// NO LONGER BEING USED....
void free_list_coalesce_opt(node** head, arena* l_arena)
{
    // *************************************************************
    // Coalesce Opt  
    // First try to coalesce with key. 
    // If we can't, try to coalesce with next.
    // If we sucessfully coalesce, remove it from the bin and add it 
    // read it back to the arena
    // *************************************************************
    assert(false); 
    node* key = *head; 
    node* next = key->next;
    node* prev = NULL;

    while(next != NULL)
    { 
        //assert(key->next > 0x700000000000 || key->next == 0x0);
        
        // Check if we can coalesce with next 
        if((void*)key + key->size == next)
        {
            // remove key and next 
            key->size += next->size;
            key->next = NULL;
           
            // Key & Next are now detached... 
            
            // Stick key back into the arena
            arena_add(l_arena, key);
            
            if(prev == NULL) { 
            // If the first item coalesces, then prev will not have been set. 
            // Move the HEAD to be next->next
                *head = next->next; 
                key = next->next;

                // Handle case where we only had two elements
                if(key == NULL)
                {
                    break;
                }
                next = key->next;
            }
            else {
                // Guarenteed that next exists... Safe to assign key to next->next
                key = next->next;
                prev->next = key;

                // Key is the last node in the list, break...
                // if key == null, coalesced last two items
                // if key->next == null, key now points to the last item in the 
                // list, nothing to coalesce with
                // otherwise, we can continue...
                if(key == NULL || key->next == NULL)
                {
                    break;
                } else
                {
                    next = key->next;
                }
            }

        } else // Didn't coalesce
        {
            prev = key;
            key = next;
            next = key->next;
        }
    }
}

//O(n)
void free_list_add_helper(node** head, node* item, arena* l_arena)
{
    node* key = *head;
    node* prev = NULL;
    
    //check_valid((void*)key);

    if(key == NULL)
    {
        *head = item;
    } else // There is at least one item in the list...
    { 
        node* next;
        while(1)
        {
            next = key->next;
            if(next == NULL)
            {
                // the key is the last item in the list.
                break;    
            } 
            else if(next > item)
            {
            // When the next element in the list is greater than our current element,
            // break... Otherwise, increment up to that point. 
                break;
            } else
            {
                prev = key;
                key = next; 
            }
        }    

        // Put the item before the key. 
        if(key > item)
        {        
            if(key != *head)
            {
                printf("Key doesn't equal head\n");
            }
            
            *head = item;
            item->next = key;

            free_list_coalesce_opt2(head, NULL, prev, item, l_arena);
        }
        else // Put the item after the key
        {
            next = key->next;
            key->next = item;
            item->next = next;
            free_list_coalesce_opt2(head, prev, key, item, l_arena);
        }
    }
}

void free_list_add(node** head, node* item)
{
    node* key = *head;

    if(key == NULL)
    {
        *head = item;
    } else // There is at least one item in the list...
    { 
        node* next;
        while(1)
        {
            next = key->next;
            if(next == NULL)
            {
                // the key is the last item in the list.
                break;    
            } 
            else if(next > item)
            {
            // When the next element in the list is greater than our current element,
            // break... Otherwise, increment up to that point. 
                break;
            } else
            {
               key = next; 
            }
        }    


        // Put the item before the key. Should only occur when the key is the first
        // and only element
        if(key > item)
        {
            *head = item;
            item->next = key;  
            //key->next = NULL;
            //printf("insert before key\n");
        }
        else // Put the item after the key
        {
            //long diff = (void*)item - (void*)key;
            //if(diff != 24)
            //{
            //    printf("Added item %lu with after item %lu with difference of %lu\n", 
            //    item, key, diff);
            //}
            
            next = key->next;
            key->next = item;
            item->next = next;
            //printf("insert after key\n");
             
        }

        // Check if we can coalesce with next 
        // If we can, item becomes item + next.
        // key->next = item
        if((void*)item + item->size == next)
        {
            //long diff = (void*)next - (void*)item;
            //if(diff != 24)
            //printf("Combined item %lu with next %lu with difference of %lu\n", item, next, diff);
            //printf("front combined\n");

            // Replace next 
            item->size += next->size;
            item->next = next->next;
        }

        // Check if we can coalesce with key
        //printf("key: %lld, key+key+size: %ld, item: %ld\n", key, (void*) key + key->size, item);
        if((void*)key + key->size == item)
        {
            //long diff = (void*)item- (void*)key;
            //if(diff != 24)
            //printf("Combined item %lu with key %lu with difference of %lu\n", item, key, diff);

            //printf("back combined\n");
            key->size += item->size;
            key->next = item->next;
        } 
    }
}

long free_list_find(node* head, size_t size)
{
    node* temp = head;
    long index = 0; 

    if(temp == NULL)
    {
        return -1;
    }

    while(1)
    {
        // If this block is large enough, return it
        if(temp->size > size)
        {
            return index;
        }

        if(temp->next == NULL)
        {
            break;
        } else
        {
            temp = temp->next;     
            index++;
        }
    }

    return -1;

}

void* free_list_get(node** head, long index)
{
    node* temp = *head;
    long current_index = 0; 
    
    if(index == 0)
    {
        *head = (*head)->next;
        *((size_t*) temp) = temp->size;
        return temp; 
    }

    while(1)
    {
        // The next block is the block we want to remove. 
        if(current_index + 1 == index)
        {
            node* return_block = temp->next;
           
            // Now temp points to the position AFTER we're planning on returning 
            temp->next = return_block->next;

            *((size_t*) return_block) = return_block->size;
            return (void*) return_block; 
        }
        if(temp->next == NULL)
        {
            fprintf(stderr, "Couldn't find requested index");
            return NULL;
        } else
        {
            temp = temp->next;     
            current_index++;
        }
    }
}

size_t block_size(void* block, int head)
{
    header temp_header; 

    if(head)
    {
        temp_header = *(header*)(block);
    } else
    {
        temp_header = *(header*)(block - sizeof(header));
    }
    return temp_header.size;
}

header block_header(void* block, int head)
{
    header temp_header; 

    if(head)
    {
        temp_header = *(header*)(block);
    } else
    {
        temp_header = *(header*)(block - sizeof(header));
    }
    return temp_header;
}

void check_valid(void* block)
{
    assert(block > (void*)0x700000000000 || block == (void*)0x0);
}

void check_valid_size(node* myNode)
{
    assert(myNode->size % 2 == 0);
    assert(myNode->size != 0);
}

void global_add(node* myNode)
{
    pthread_mutex_lock(&mutexx);

    if (global_ar == NULL) {
        global_ar = myNode;
        global_ar->next = NULL;
        check_valid((void*) myNode);
    }
    else {
        myNode->next = global_ar;
        global_ar = myNode;
        check_valid((void*) global_ar);
    }

    pthread_mutex_unlock(&mutexx);
}

node* global_get()
{
    pthread_mutex_lock(&mutexx);

    node* toBeReturned;

    if (global_ar == NULL) {
        toBeReturned = NULL;
    }
    else {
        check_valid((void*) global_ar);
        toBeReturned = global_ar;
        toBeReturned->next = NULL;
        global_ar = global_ar->next;
        check_valid((void*) global_ar);
    }

    pthread_mutex_unlock(&mutexx);

    return toBeReturned;
}

node* node_split(node* myNode)
{
    size_t newSize = myNode->size / 2;
    myNode->size = newSize;
    node* secondNode = node_make(newSize, (void*)myNode + newSize);
    return secondNode;
}

header header_make(size_t size)
{
    header myHeader;
    myHeader.size = size;
    myHeader.parent = my_thread_id; 

    return myHeader;
}

header header_make_nothread(size_t size)
{
    header myHeader;
    myHeader.size = size;
    myHeader.parent = 0; 

    return myHeader;
}
