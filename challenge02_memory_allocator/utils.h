#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define HEAD 1
#define PREV 0
#define NUM_BINS 9
#define MAX_LOCAL_LENGTH 10
#define PAGE_SIZE 4096
#define INITIAL_PAGES 1000



/******************************************************************************
 * Important Data Structures
******************************************************************************/
// A node in a used to keep track of free memory. 
typedef struct node node;
struct node {
    size_t size;
    node* next;
};

// The header of an allocated block of memory given to the user.
// Contains information on who allocated the block and how large of a block
// was allocated. 
typedef struct header
{
  size_t size;      // The size of this node. 
  int parent; // The thread that allocated this node
} header;

// An arena is an array of free list with each list holding nodes of a given size.
// The arena holds a free list containing nodes of size 2^(n+4) up to 2^n
typedef struct arena{
    node* bins[NUM_BINS];
} arena;

// The mailbox is a list of arenas whos length is equal to the number of
// threads allocating memory. The index in the list corresponds to the thread
// number. The mailbox is a means to allow threads to return freed nodes to the
// allocating thread to reduce fragmentation
typedef struct mailbox mailbox;
struct mailbox{
    node* data; 
    pthread_mutex_t mutex_box;
    pthread_mutex_t mutex_notif;
    bool data_available;
    mailbox* next;
};

//node* global_ar = NULL;

// Used to count how many threads are allocating memory
static int thread_counter = 0;

// Thread Private variable keeping track of its own thread id
extern __thread int my_thread_id;

// Set the thread_id private variable of the calling thread 
void set_thread_id();

/******************************************************************************
 * Manipulations for a mailbox
******************************************************************************/

// THE head node into the mailboxes
extern mailbox* mailbox_head;

// Makes a mailbox 
mailbox mailbox_make();

// Adds a given mailbox to the end of the list
void mailbox_add(mailbox* newBox);

// Adds a given node to the appropriate mailbox. 
void mailbox_post(int boxNum, node* mail);

// Moves the mail from a given mailbox to a given arena
void mailbox_receive(mailbox* dest_box, arena* myArena);

// Returns a given mailbox for manipulaiton. Returns null if the index wasn't 
// found
mailbox* mailbox_get_index(int index);

/******************************************************************************
 * Manipulations for an Arena 
******************************************************************************/
// Initializes an arena to have a number of pages already allocated...
// O(c)
void arena_set();

// Returns a node of the requested size from the arena. If a suitable node couldn't
// be found, returns NULL. This function will break up larger nodes to get smaller
// nodes if necessary.
// O(arena_find) + O(arena_get_index) + O(node_truncate)
// O(c) + O(c) + O(logn)
// O(logN)
node* arena_get(arena* myArena, size_t size);

// Returns the bin that conains the smallest node greater than the requested size
// Returns -1 if the suitable bin was not found
// O(c)
int arena_find(arena* myArena, size_t size);

// Returns the first node at a given index in an arena. The node is removed from 
// free list and returned to the user. 
// O(c)
node* arena_get_index(arena* myArena, int index);

// Adds a given node into the appropriate location in an arena... 
// O(free_list_add_opt)
void arena_add(arena* myArena, node* myNode);

// O(arena_get_index) + O(global_add)
void arena_2global(arena* ar);

/******************************************************************************
 * Manipulations for Global
******************************************************************************/

// Adds a given node into the global free list...
// !! Thread Safe !!
// O(c)
void global_add(node* myNode);

// Gets a node from the global free list... The global free lists are all full
// pages... If no nodes were availale, NULL is returned. 
// !! Thread safe !!
// O(c)
node* global_get();


/******************************************************************************
 * Manipulations for a Single Free List
******************************************************************************/
// Given the head of a free list, add the given item into the free list in the
// correct spot. Nodes are sorted by address. Coalesce when able to. .
// O(n)
void free_list_add(node** head, node* item);

// Given the head of a free list, add the given item into the free list in the
// correct spot. Nodes are sorted by address. If possible, the node is coalesced 
// with the adjacent node (previous first, then next). If a node is coalesced, 
// it is added to the bin 
// O(free_list_add_helper) + O(free_list_coalesce_opt)
// O(n) + O(n* arena_add)
// O(n* arena_add)
void free_list_add_opt(node** head, node* item, arena* l_arena);

void free_list_coalesce_opt2(node** head, node* prev_prev, node* prev, node* key,  arena* l_arena);

// Performs in the insert aspect of the add
// O(n)
void free_list_add_helper(node** head, node* item, arena* l_arena);

// Performs all possible coalesces... If a coalesce occurs, the combined node
// is removed from the list and added into the proper bin by recursively calling
// arena_add.
// O(n* arena_add)
void free_list_coalesce_opt(node** head, arena* l_arena);

// Given the head of a free list, return the length of the free list
// O(n)
long free_list_length(node* head);
// O(n)
long free_list_max(node* head);
// O(n)
long free_list_min(node* head);
void check_free_list(node* head);

// Return a pointer to a given index of the free list. Pop the given index out
// The first sizeof(size_t) bytes will contain the size of the given block.
// O(n)
void* free_list_get(node** head, long index);

// Find the index of the first block out of our free list whose size is greater than a given
// size. If no index is found, return -1
// O(n)
long free_list_find(node* head, size_t size);

// Returns the size of a given void block. This assumes that the size of a block
// is stored in the space immediately before the pointer to the given block or
// as the first sizeof(size_t) bytes. If head = true, then it is assumed to be
// in the beginning of the block. Otherwise, before the beginning
// O(c)
size_t block_size(void* block, int head);

header block_header(void* block, int head);

// Ensures given pointer is not NULL
void check_valid(void* block);
void check_valid_size(node* myNode);

/*******************************************************************************
 * Manipulations for a Single Node
*******************************************************************************/
// Given an void pointer, cast it to a node pointer and set the size to the given
// size. The next pointer is initialzied to NULL.
node* node_make(size_t size, void* me);

// Given a node, truncate it to the appropriate sized node for a given size. 
// All excess sizes are added back to the local arena
// O(logn)
void node_truncate(node* myNode, size_t size, arena* myArena);

// Given a node, split it into two equally sized nodes.
// O(c)
node* node_split(node* myNode);

/*******************************************************************************
 * Header Manipulations
*******************************************************************************/
// Creates a header object given the size of the node. The thread_id is pulled 
// from the thread_private global variable. 
header header_make(size_t size);
header header_make_nothread(size_t size);

#endif
