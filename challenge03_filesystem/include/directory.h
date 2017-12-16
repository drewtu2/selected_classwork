#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "inode.h"
#include "constants.h"

/*******************************************************************************
 * Structs
*******************************************************************************/
/**
This represents an entry in a directory, either a file or a subdirectory.
*/
typedef struct direntry {
    char   name[NUFS_MAX_NAME_LENGTH];   // the name of this directory entry
    int    node_id;                      // the inode id
    int    isUsed;                       // FREE & OCCUPIED are accepted values
} direntry;

/*
Directories: Stores references to other files/directories, has meta-data:
  - Name        (dirent->name)
  - num entries (dirent->node->refs??)
  - created time, modified time, access time (stored in corresponding inode)
  - permissions (stored in corresponding inode)
  - corresponding inode (dirent->node)
  - Size Distribution (dirent->record_length?)
special cases:
  - contains file pointing to itself
  - cotains file pointing to its parent
*/
/**
This represents a directory.
*/
typedef struct directory {
    int numEntries;               // the number of entries in this directory
    size_t entries_offset;        // array of offsets in data page for direntry
} directory;

/**
* Initialize the given directory, setting its numEntries to 0 and
* entries_offset so that entries may be accessed via index notation.
* @param dir    The directory to initialize.
*/
void directory_init(directory* dir);

/**
* Make a new directory at the given path, using the given node.
* @param node        The node to use for the directory.
* @return            The directory after creation.
*/
directory* directory_make(inode* node);

/**
* Initialize a new directory entry.
* @param entry     The directory entry to initialize.
* @param name      The name of the entry to make.
* @param node      The inode of the entry.
*/
void direntry_init(direntry* entry, const char* name, inode* node);

/**
* Create and Initialize a new directory entry.
* Wrapper for direntry_next_free and direntry_init
* @param path      The path of the entry to make.
* @param node      The inode of the entry.
*/
direntry* direntry_make(const char* path, inode* node);

/**
* Return a pointer to the direntry struct for the next free entry in the given
* directory dir, and sets the entry as used.
* @param dir        The directory to search for a free entry.
* @return           A pointer to a free direntry, or NULL on error.
*/
direntry* direntry_next_free(directory* dir);

/**
* Get the direntry at the given entry index in a directory.
* @param dir            The directory that contains the direntry.
* @param entry_index    The entry index of the direntry.
* @return               The direntry.
*/
direntry* direntry_from_entry_index(directory* dir, int entry_index);

/**
* Get a direntry from a given path name.
* @param path   The path name of the direntry.
* @return       The direntry at the given path, or NULL if it doesn't exist.
*/
direntry* direntry_from_path(const char* path);

/**
* Gets a directory structure from a a give inode id.
* @param inode_id    The inode id that points to the data for the directory.
* @return            The directory at the given page number.
*/
directory* directory_from_node_id(int inode_id);

/**
* Get a directory given its path name.
* @param path       The path name of the directory.
* @return           The directory, or NULL if it doesn't exist.
*/
directory* directory_from_path(const char* path);

/**
* Returns the index of the entry with the given name in a given directory. If
* the entry doesn't exist, -ENOENT is returned.
* @param dir    The directory that contains the requested entry.
* @param name   The name of the entry to get.
* @return       The entry index, or -ENOENT if the entry doesn't exist.
*/
int directory_lookup_entry_index(directory* dir, const char* name);

/**
* Insert a direntry at the given path.
* @param path       The path name for the direntry.
* @param node       The node to use for the direntry.
* @return           The index of insertion in the directory, or -ENOSPC on error.
*/
int directory_insert_entry(const char* path, inode* node);

/**
* Delete an entry with the given path.
* @param path    The path of the entry to delete.
*/
int directory_delete_entry(const char* path);

/**
* Delete the given directory.
* @param dir        The directory to delete.
* @param pageIndex  The data page index of the directory
* @return           1 on success, or -ENOENT on error.
*/
int delete_directory(directory* dir, int pageIndex);

/**
* Print the given directory's contents.
* @param dir     The directory to print.
*/
void print_directory(directory* dir);

/**
 * Mutates the given array of strings to contain all the contains in dir.
 *
 * @param dir        The directory who's contents we want to list
 * @param buffer     The array to add the directorys contents to.
 */
void directory_list_of_contents(directory* dir, char** buffer);

#endif
