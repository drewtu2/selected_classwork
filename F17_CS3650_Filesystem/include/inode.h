#ifndef INODE_H
#define INODE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "metaTimes.h"
#include "storage.h"

typedef struct disk disk; // forward declaration to resolve circular dependency

/**
This is the struct that points to a specific location in memory where the data
for our file/directory  is stored.
*/
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

/**
* Make a new inode.
* @param permissions    The permissions for the file/directory of this inode.
* @param type           The type of the inode (file or directory or none).
* @return               The newly created inode, or NULL if there is either no
                        free inodes or no free data pages.
*/
inode* inode_make(mode_t permissions, int type);

/**
* Get the id of the next free inode, and set that inode as used in the inode
* bitmap.
* @return           The inode index of the next free inode, or -ENOSPC on error.
*/
int inode_next_free();

/**
* Get the inode with the given id.
* @param index      The id of the inode to get.
* @return           The inode with the given index.
*/
inode* inode_get(int index);

/**
* Returns 1 if the given has all of the permissions ask for by the mustHaves...
* (Done via bit comparision)
* @param mustHave   An int representing the bitmap of permissions required
* @param given      An int representing the bitmap of the permissions given
* @return           A bool stating if the given permissions have all the required
*                   permissions.
*/
int comparePermissions(mode_t mustHave, mode_t given);

/**
* Delete the given inode, clear it as free in the inode bitmap, and clear all
* of its data pages as free in the data bitmap.
* @param myInode      The inode to delete.
*/
void inode_delete(inode* myInode);

/**
 * Returns the inode id to the file indicated by a given path. Searches from the
 * root node stored in the super block.
 * @param path    The path of the requested file.
 * @return        The inode_id of the inode for the given path, or -ENOENT on error.
 */
int inode_id_from_path(const char* path);

/**
 * Returns the inode to the file indicated by a given path. Searches from the
 * root node stored in the super block.
 * @param path    The path of the requested file.
 * @return        The inode of the file requeted or NULL if the file cannot be
 *                found.
 */
inode* inode_from_path(const char* path);

/**
 * Returns the page index in the data block associeted to a given internal id.
 * This serves as a wrapper to make sure user is only accessing data blocks we
 * have allocated to this inode.
 *
 * @param myInode       The inode that we're searching through
 * @param id            The index of the id requested from internal page mapping
 *                      system. This number must be less than the total number
 *                      of blocks associated to this inode.
 * @return              The page index of the requested idIndex. Errors out for
 *                      invalid requests.
 */
int inode_get_page_index(inode* myInode, int idIndex);

/**
* Print information about this node.
*/
void print_node(inode* node);

#endif
