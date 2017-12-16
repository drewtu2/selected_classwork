#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "constants.h"
#include "inode.h"
#include "storage.h"
#include "bitmaps.h"
#include "metaTimes.h"
#include "util.h"

/*
This is the struct that points to a specific location in memory that the data
for our file is stored. It has its own id (index)
typedef struct inode {
    int node_id;                    // node id
    int blocks_used;                // the number of blocks used, calculated based on size
    size_t size;                    // size of the data pointed to by the inode in bytes
    int refs;                       // number of references to this inode
    int type;                       // what type of inode (type) 0 is none, 1 is file, 2 is directory
    mode_t permissions;             // the file permissions
    metaTimes meta_time;            // creation/access/modification times
    int direct_id[MAX_DIRECT];      // array of MAX_DIRECT page indexes
    int indirect_id;        // index of a page that is this inode's indirect link
} inode;
*/

inode* inode_make(mode_t permissions, int type)
{
    int id = inode_next_free();
    if(id < 0) // there was no free Inodes
    {
        return NULL;
    }

    bitmap_set_bit(get_inode_bitmap(), id);

    inode* myInode = inode_get(id);
    int page_id = data_page_next_free();

    if(page_id < 0) // there was no free data pages
    {
        return NULL;
    }

    printf("inode_make: creating inode %d linked to data page %d\n", id, page_id);

    myInode->node_id = id;    // node id
    myInode->blocks_used = 1;
    myInode->size = 0;
    myInode->refs = 1;     // number of references to this inode TODO: make sure # refs is right
    myInode->type = type;
    myInode->permissions = permissions; // Permissions..
    myInode->meta_time = metaTimes_make();
    myInode->direct_id[0] = page_id;
    myInode->indirect_id = -1; //
    return myInode;
}

int inode_next_free()
{
    int isFREE;
    byte* inode_bitmap = get_inode_bitmap();
    for(int ii = 0; ii < NUM_INODES; ++ii)
    {
        isFREE = bitmap_get_bit(inode_bitmap, ii);
        if(isFREE == FREE)
        {
            bitmap_set_bit(inode_bitmap, ii);
            return ii;
        }
    }

    return -ENOSPC;
}

inode* inode_get(int index)
{
    assert(index <= NUM_INODES);
    inode* inode_list = get_inode_list();
    inode* node = &inode_list[index];
    return node;
}

// mustHave 1101
// given    1001    Not Valid
// given    1111    Valid
int comparePermissions(mode_t mustHave, mode_t given)
{
    int mustHaveBit;
    int givenBit;

    for(int bitIndex = 0; bitIndex < sizeof(mode_t) * BITS_PER_BYTE; ++bitIndex)
    {
        // Get access the value of the specific bit....
        mustHaveBit = (mustHave >> bitIndex) & 0x01;
        givenBit    = (given >> bitIndex) & 0x01;

        // If the mustHaveBit is high and the givenBit is low, the permissions
        // don't match. Return failed.
        if(mustHaveBit > givenBit)
        {
            return 0;
        }
    }

    return 1;
}

void inode_delete(inode* myInode)
{
    printf("inode_delete: deleteing inode %d, size %lu, %d blocks\n", myInode->node_id, myInode->size, myInode->blocks_used);
    byte* inode_bitmap = get_inode_bitmap();
    byte* data_bitmap = get_data_bitmap();
    bitmap_clear_bit(inode_bitmap, myInode->node_id);
    for(int ii = 0; ii < myInode->blocks_used; ++ii)
    {
        bitmap_clear_bit(data_bitmap, inode_get_page_index(myInode, ii));
    }
    memset((void*) myInode, 0, sizeof(inode));
}

int inode_id_from_path(const char* path)
{
    inode* node = inode_from_path(path);
    if (!node)
    {
        return -ENOENT;
    }
    return node->node_id;
}

inode* inode_from_path(const char* path)
{
    inode* currentInode = get_root();

    if(streq(path, "/"))
    {
        //printf("inode_from_path: just asking for root, return root\n");
        return currentInode;
    }

    char path_copy[MAX_DEPTH * NUFS_MAX_NAME_LENGTH];
    strcpy(path_copy, path);
    const char delimiter[2] = "/";

    char* token;
    int page_index = currentInode->direct_id[0];
    directory* currentDir = directory_from_node_id(currentInode->node_id);
    direntry* currentDirentry = NULL;

    // get the first token (name of top-level directory of path)

    // /top/middle/file.txt
    token = strtok(path_copy, delimiter);
    // top
    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        currentDirentry = direntry_from_entry_index(currentDir, ii);
        // If currentDirentry is equal to the directory we're looking for
        if (streq(token, currentDirentry->name)
            && currentDirentry->isUsed == OCCUPIED) {
            // We know we want to get the inode of the currentDirentry
            currentInode = inode_get(currentDirentry->node_id);

            // Set token equal to the next part of the path. If the next token
            // is equal to null, this is the inode we want to return
            token = strtok(NULL, delimiter);
            // This was the final part of the path
            if(!token)
            {
                return currentInode;
            }
            //printf("the current part of the path is %s\n", token);

            // Otherwise, this was an intermediary directory, we want to go one
            // level down
            currentDir = directory_from_node_id(currentInode->node_id);

            // Reset counter to -1 because we want ii to equal 0 at the start of
            // the next iteration
            ii = -1;
        }
    }
    printf("inode_from_path: couldn't find the requested inode at %s. Returning NULL...\n", path);
    return NULL;

}

int inode_get_page_index(inode* myInode, int idIndex)
{
    assert( idIndex < myInode->blocks_used );

    if(idIndex < MAX_DIRECT)
    {
        return myInode->direct_id[idIndex];
    }
    else
    {
        // Taking out all indirect ids... therfore we should always request
        // something less than the MAX_DIRECT
        assert( 0 );
        return -1;
    }
}

void
print_node(inode* node)
{
    if (node) {
        printf("node{id: %d, blocks_used: %d ", node->node_id, node->blocks_used);
        printf("size: %lu, refs: %d, type: %d", node->size, node->refs, node->type);
        printf("\npermissions: %u, created: time", node->permissions); // TODO: add times for debug
        printf(" indirect: %d ", node->indirect_id);
        printf("direct links:\n");
        for(int ii = 0; ii < node->blocks_used; ++ii)
        {
            printf("%d\n", node->direct_id[ii]);
        }
    }
    else {
        printf("node{null}\n");
    }
}
