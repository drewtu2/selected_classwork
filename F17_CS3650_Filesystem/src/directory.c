#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>

#include "directory.h"
#include "inode.h"
#include "util.h"
#include "constants.h"

void directory_init(directory* dir)
{
    dir->numEntries = 0;
    dir->entries_offset = sizeof(directory);  // offset for list of entries should be right
                                              // after the directory struct
}

directory* directory_make(inode* node)
{
    directory* dir = directory_from_node_id(node->node_id);
    directory_init(dir);
    node->size = sizeof(directory);
    node->blocks_used = calc_blocks_used(node->size);
    node->meta_time = metaTimes_make();
    return dir;
}

void direntry_init(direntry* entry, const char* name, inode* node)
{
    assert( sizeof(name) <= NUFS_MAX_NAME_LENGTH );
    strcpy(entry->name, name);
    entry->node_id = node->node_id;
    entry->isUsed = OCCUPIED;
}

direntry* direntry_make(const char* path, inode* node)
{
    char dirName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    char entryName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    strcpy(dirName, path);
    strcpy(entryName, path);

    directory* myDir = directory_from_path(dirname(dirName));
    direntry* myDirentry = direntry_next_free(myDir);

    // return error, there wasn't enough space to make a direntry
    if (!myDirentry)
    {
        return NULL;
    }

    direntry_init(myDirentry, basename(entryName), node);
    node->meta_time = metaTimes_make();
    return myDirentry;
}

direntry* direntry_next_free(directory* dir)
{
    direntry* currentEntry = NULL;
    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        currentEntry = direntry_from_entry_index(dir, ii);
        if (currentEntry->isUsed == FREE)
        {
            currentEntry->isUsed = OCCUPIED;
            return currentEntry;
        }
    }

    return NULL;
}

direntry* direntry_from_entry_index(directory* dir, int entry_index)
{
    direntry* entries = (direntry*) ((void*)dir + dir->entries_offset);
    direntry* desired_entry = &entries[entry_index];
    return desired_entry;
}

direntry* direntry_from_path(const char* path)
{
    // make copies of path name for calling dirname and basename
    char dirName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    char entryName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    strcpy(dirName, path);
    strcpy(entryName, path);

    // Get the directory that houses the entry to return
    directory* dir = directory_from_path(dirname(dirName));
    printf("direntry_from_path: have parent dir %s\n", dirName);
    int entryIndex = directory_lookup_entry_index(dir, basename(entryName));
    printf("direntry_from_path: have direntry %s\n", entryName);
    if (entryIndex < 0)
    {
        return NULL;
    }

    return direntry_from_entry_index(dir, entryIndex);
}

directory* directory_from_node_id(int inode_id)
{
    assert( inode_id <= NUM_INODES );
    inode* node = inode_get(inode_id);
    assert( node->type == INODE_DIRECTORY );
    int pnum = node->direct_id[0]; // directories never take more than a page or this breaks (based on # of inodes)
    assert( pnum <= PAGE_COUNT);
    return (directory*) data_page_get(pnum);
}

directory* directory_from_path(const char* path)
{
    inode* node = inode_from_path(path);
    if(!node)
    {
        return NULL;
    }
    directory* directory = directory_from_node_id(node->node_id);
    return directory;
}

int directory_lookup_entry_index(directory* dir, const char* name)
{
    direntry* entry;

    for (int ii = 0; ii < dir->numEntries; ++ii)
    {
        entry = direntry_from_entry_index(dir, ii);
        if(streq(entry->name, name))
        {
            return ii;
        }
    }
    // Couldn't find the requested entry
    return -ENOENT;
}

int directory_insert_entry(const char* path, inode* node)
{
    // create copies of the path in order to call dirname() and basename()
    char dirName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    char entryName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    strcpy(dirName, path);
    strcpy(entryName, path);

    // Get the directory that ths direntry will be housed in
    directory* parentDir = directory_from_path(dirname(dirName));
    // get the next free direntry in the directory that will house the entry
    direntry* entry = direntry_make(path, node);
    printf("directory_insert_entry: want to add entry %s\n", entry->name);
    printf("to parent directory %s\n", dirName);

    if (parentDir->numEntries >= MAX_NUM_DIRENTRY)
    {
        printf("directory_insert_ent: too many entries (%d) in dir\n", parentDir->numEntries);
        return -ENOSPC;
    }

    direntry* currentEntry = NULL;

    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        currentEntry = direntry_from_entry_index(parentDir, ii);
        if (currentEntry->isUsed == FREE)
        {
            direntry* dir_entries_start = (direntry*)((void*)parentDir + parentDir->entries_offset);
            dir_entries_start[ii] = *entry;
            entry->isUsed = OCCUPIED;
            //printf("directory number of entries before inserting: %d\n", parentDir->numEntries);
            parentDir->numEntries++;
            //printf("currentEntry isUsed field address: %p\n", &currentEntry->isUsed);
            //printf("currentEntry isUsed field value: %d\n", currentEntry->isUsed);
            //printf("entry isUsed field address: %p\n", &entry->isUsed);
            //printf("entry isUsed field value: %d\n", entry->isUsed);
            //printf("directory number of entries after inserting: %d\n", parentDir->numEntries);
            inode* directoryInode = inode_from_path(dirName);
            //printf("directory node size before inserting: %lu\n", directoryInode->size);
            directoryInode->size += sizeof(direntry);
            directoryInode->blocks_used = calc_blocks_used(directoryInode->size);
            update_last_modified(&directoryInode->meta_time);
            //printf("directory node size after inserting: %lu\n", directoryInode->size);
            return ii;
        }
    }

    printf("directory_insert_entry no free entries in dir\n");
    return -ENOSPC;
}

int directory_delete_entry(const char* path)
{
    printf("directory_delete_entry: want to delete %s\n", path);
    // make copies of path name for calling dirname
    char dirName[NUFS_MAX_NAME_LENGTH * MAX_DEPTH];
    strcpy(dirName, path);

    // Get the directory that houses the entry to delete.
    directory* dir = directory_from_path(dirname(dirName));
    inode* dirInode = inode_from_path(dirName);
    printf("directory_delete_entry: have parent dir %s\n", dirName);

    direntry* entry = direntry_from_path(path);
    if (!entry)
    {
        return -ENOENT;
    }

    dirInode->size -= sizeof(direntry);
    dirInode->blocks_used = calc_blocks_used(dirInode->size); // should always be 1, if not 1, things break
    dirInode->refs--;
    update_last_modified(&dirInode->meta_time);
    dir->numEntries--;
    entry->isUsed = FREE;
    strcpy(entry->name, "hi");
    return 1; // TODO: check what we should return here
}

// TODO: rewrite
int delete_directory(directory* dir, int pageIndex)
{
    if(!dir)
    {
        return -ENOENT;
    }
    else
    {
        bitmap_clear_bit(get_data_bitmap(), pageIndex);
        memset((void*) dir, 0, PAGE_SIZE);
        return 1;
    }
}

void print_directory(directory* dir)
{
    printf("Printing %d entries of the given entry", dir->numEntries);
    direntry* entry = NULL;
    for (int ii = 0; ii < dir->numEntries; ++ii)
    {
        entry = direntry_from_entry_index(dir, ii);
        printf("Entry at %i: %s\n", ii, entry->name);
        print_node(inode_get(entry->node_id));
        printf("\n");
    }
}


void directory_list_of_contents(directory* dir, char** buffer)
{
    direntry* entry = NULL;
    int numContents = 0;
    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        entry = direntry_from_entry_index(dir, ii);
        if(entry->isUsed == OCCUPIED)
        {
            buffer[numContents] = entry->name;
            ++numContents;
        }
    }
/*
    printf("directory_list_of_contents: in directory:\n");
    for(int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        entry = direntry_from_entry_index(dir, ii);
        if(entry->isUsed == OCCUPIED)
        {
            printf("entry has name %s\n", entry->name);
        }
    }

    printf("directory_list_of_contents: in buffer:\n");
    printf("directory_list_of_contents: dir->numEntries: %d:\n", dir->numEntries);
    for(int ii = 0; ii < dir->numEntries; ++ii)
    {
        printf("buffer has entry %s\n", buffer[ii]);
    }
*/
}
