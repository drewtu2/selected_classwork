#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "inode.h"
#include "pages.h"
#include "bitmaps.h"

typedef struct inode inode; // forward declaration
typedef struct page page; // forward declaration

/*******************************************************************************
 * Structs
*******************************************************************************/
/**
* This is the struct that represents our super block.
*/
typedef struct disk {
  int inode_bitmap;             // offset for inode bitmap location
  int data_pages_bitmap;        // offset for data pages bitmap location
  int inode_list;               // offset for inode list location
  int data;                     // offset for data page location
} disk;

extern disk* myDisk;

/*******************************************************************************
 * High level Operations
*******************************************************************************/
/**
* Load the disk if it's already been initialize it or initialize it if it's
* never been set up before.
* @param path   The path at which to set the root directory.
*/
void storage_init(const char* path);

/**
* Deallocate the file system.
*/
void storage_free();

/*******************************************************************************
 * Disk Operations
*******************************************************************************/
/**
* Load a disk for use.
* @param path   The path at which to load the disk.
*/
disk* disk_make(const char* path);

/*******************************************************************************
 * Data Operations
*******************************************************************************/
/**
* Fill in the stat structure for use with the FUSE API,
* @param path       The path of the file to use.
* @param st         The stat structure to fill in.
*/
int get_stat(const char* path, struct stat* st);

/**
* Get the raw data at the given path.
* @param path       The path name.
* @return           The raw data at the path.
*/
char* get_data(const char* path);

/**
* Return a pointer to the storage disk structure.
* @return           The storage disk structure.
*/
disk* get_disk();

/**
* Return the storage disk's inode bitmap.
* @return           The inode bitmap.
*/
byte* get_inode_bitmap();

/**
* Return the data_pages_bitmap.
* @return           The data pages bitmap.
*/
byte* get_data_bitmap();

/**
* Return the start of the inode_list.
* @return           The start of the inode_list.
*/
inode* get_inode_list();

/**
* Return the beginning of the data pages.
* @return           The start of the data pages.
*/
page* get_data_pages();

/**
* Return the address of the root inode.
* @return           The address of the root inode.
*/
inode* get_root();

#endif
