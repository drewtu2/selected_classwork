#ifndef CONSTANTS_H
#define CONSTANTS_H

/*******************************************************************************
 * Constants
 *******************************************************************************/
#define NUFS_SIZE 1024 * 1024    // equals 4096 * 256
#define MAX_DIRECT 25            // Maximum number of direct pages per inode
#define PAGE_SIZE 4096           // bytes
#define NUFS_MAX_NAME_LENGTH 48  // the max length of a file/directory name
#define MAX_DEPTH 239            // The max depth of a directory, limited by # inodes
#define MAX_NUM_DIRENTRY 70      // the max number of entries in a single directory

#define BITS_PER_BYTE 8

#define INODE_FILE 1           // indicates an inode is used for a file
#define INODE_NONE 0           // indicates an inode is free
#define INODE_DIRECTORY 2      // indicates an inode is used for a directory

#define FREE 0                 // indicates an inode/page is free in bitmap
#define OCCUPIED 1             // indicates an inode/page is occupied in bitmap

#define NUM_INODES 240      // the maximum possible number of inodes
#define PAGE_COUNT 240      // the maximum number of data pages
#define BITMAP_SIZE NUM_INODES / BITS_PER_BYTE

// page count : NUFS_SIZE - sizeof(disk) - 1/8*NUM_INODES - 1/8*PAGE_COUNT

// Permissions for FILES
#define MODE_DIRECTORY 040755
#define MODE_DIRECTORY_STEM 040000
#define MODE_FILE 0100644

#endif
