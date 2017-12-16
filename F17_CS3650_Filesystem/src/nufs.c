#define FUSE_USE_VERSION 26
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <bsd/string.h>
#include <assert.h>
#include <libgen.h>
#include <errno.h>
#include <stdlib.h>
#include <fuse.h>

#include "directory.h"
#include "storage.h"
#include "inode.h"
#include "util.h"

// implementation for: man 2 access
// Checks if a file/directory exists.
int
nufs_access(const char *path, int mode)
{
    printf("access(%s, %04o)\n", path, mode);
    inode* node = inode_from_path(path);

    // return error if no inode exists at the given path
    if (node == NULL)
    {
        printf("access: error no entry\n");
        return -ENOENT;
    }
    else if (!comparePermissions((mode_t)mode, node->permissions))
    {
        printf("access: permissions error but not throwing error as per nats instructions\n");
        //return -EACCES;
        return 0; 
    }
    else { // Valid node could be found
        //printf("access: success\n");
        update_last_access(&node->meta_time);
        return 0;
    }
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    printf("getattr(%s)\n", path);
    int rv = get_stat(path, st);
    print_stat(st, path);
    if (rv != 0) {
        printf("getattr: failed\n");
        return rv;
    }
    else {
        //printf("getattr: success\n");
        return 0;
    }
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    printf("readdir(%s)\n", path);
    struct stat st;

    int rv = get_stat("/", &st);
    if (rv == -1) {
        printf("readdir: entry doesn't exist\n");
        return -ENOENT;
    }
    print_stat(&st, path);

    directory* myDir = directory_from_path(path);
    inode* dirInode = inode_from_path(path);
    update_last_access(&dirInode->meta_time);
    // An array of size [MAX_NUM_DIRENTRY] containing pointers to file names
    struct stat st_file[MAX_NUM_DIRENTRY];
    char *fileNames[MAX_NUM_DIRENTRY];
    directory_list_of_contents(myDir, fileNames);
    char *tempName;
    char totalPath[MAX_DEPTH * NUFS_MAX_NAME_LENGTH];

    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    rv = filler(buf, ".", &st,0);
    if(rv != 0)
    {
        return 0;
    }
    rv = filler(buf, "..", &st, 0);
    if(rv != 0)
    {
        return 0;
    }

    printf("readdir: number of entries in %s: %d\n", path, myDir->numEntries);

    for(int ii = 0; ii < myDir->numEntries; ++ii)
    {
        // Wipe total path
        memset(totalPath, 0, NUFS_MAX_NAME_LENGTH * MAX_DEPTH);

        // Get the file name of the current iteration...
        tempName = fileNames[ii];
        printf("readdir: fileName: %s\n", tempName);

        // Concatenate path with file name
        snprintf(totalPath, sizeof(totalPath), "%s/%s", path, tempName);

        // Chck to make sure its real
        //printf("total path: %s\n", totalPath);
        rv = get_stat(totalPath, &st_file[ii]);
        if (rv == -1) {
            printf("getattr: failed\n");
            return -ENOENT;
        }
        // TODO: This probably needs to be made so that each has its own struct
        print_stat(&st_file[ii], totalPath);

        // Fill in buffer
        rv = filler(buf, tempName, &st_file[ii], 0);
        if(rv != 0)
        {
            return 0;
        }
    }

    printf("readdir: sucessfuly reached end of readdir\n");
    return 0;
}

// mknod makes a filesystem object like a file or directory
// returns the node id of the crated node.
// called for: man 2 open, man 2 link
int
nufs_mknode(const char *path, mode_t mode, dev_t rdev)
{
    printf("mknod(%s, %04o)\n", path, mode);

    inode* myNode = inode_from_path(path);

    if(myNode) // if a node at given path was found, it exists
    {
        printf("mknode: node exists already, shouldn't make new\n");
        print_node(myNode);
        return -EEXIST; // the path exists, shouldnt make new node TODO: but should link??
    }

    myNode = inode_make(mode, INODE_FILE); // mknode makes files, mkdir makes dirs

    if(!myNode) // if we couldn't make a new node, there isn't enough space
    {
        printf("mknode: couldn't make a new node (not enough data or nodes)\n");
        return -ENOSPC; // return error
    }

    printf("mknode: made a new node:\n");
    print_node(myNode);

    // Make an entry and insert the entry into the parent dir.
    int rv = directory_insert_entry(path, myNode);
    if(rv < 0 )
    {
        return rv;
    }
    // otherwise, TODO: hopefully return the node id of the direntry but might not be able to bc of
    // the return code supposed to be 0.... was working b4 tho
    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual

// Create a directory and return the node index of the directory.
// TODO: the mode is wrong and causes erros???
int
nufs_mkdir(const char *path, mode_t mode)
{
    mode_t permissions = MODE_DIRECTORY_STEM | mode;
    printf("mkdir(%s, %04o)\n", path, permissions);

    // Create a node for this directory to be represented by. Inode will be
    // created in the appropriate parent directory.
    inode* myNode = inode_from_path(path);

    if(myNode) // if a node at given path was found, it exists
    {
        printf("mkdir: node exists already, shouldn't make new\n");
        print_node(myNode);
        return -EEXIST; // the path exists, shouldnt make new node TODO: but should link??
    }
    myNode = inode_make(permissions, INODE_DIRECTORY); // mknode makes files, mkdir makes dirs

    if(!myNode) // if we couldn't make a new node, there isn't enough space
    {
        printf("mkdir: couldn't make a new node (not enough data? or nodes?)\n");
        return -ENOSPC; // return error
    }



    printf("mkdir: made a new dir:\n");
    directory_make(myNode);
    print_node(myNode);
    printf("mkdir: Creating dir at inode %d\n", myNode->node_id);
    // Put the entry into the parent dir.
    int rv = directory_insert_entry(path, myNode);
    if(rv < 0)
    {
        return -ENOSPC;
    }


    // otherwise, TODO: hopefully return the node id of the direntry;
    return 0;
}

// Creates a hard link between two files
int
nufs_link(const char* from, const char* to)
{
    printf("link(%s, %s)\n", from, to);
    int rv;

    inode* myInode = inode_from_path(from);

    // Make sure we were able to find the file
    if(!myInode)
    {
        return -ENOENT;
    }

    rv = directory_insert_entry(to, myInode);

    // Check to make sure that the insert succeded.
    if(rv < 0)
    {
        return rv;
    }

    myInode->refs++;
    update_last_modified(&myInode->meta_time);

    return 0;
}


// Deletes a name from the fs. If the name as the last link to a file and no procs
// have the file open, the file is deleted and the space it was using is made
// available for reuse.
// TODO: Check for errors
int
nufs_unlink(const char *path)
{
    printf("unlink(%s)\n", path);

    // Get the requested inode
    inode* myInode = inode_from_path(path);

    if(!myInode) // if the path doesn't exist, return an error
    {
        return -ENOENT;
    }

    if(myInode->type == INODE_DIRECTORY)
    {
        printf("unlink: trying to unlink a dir\n");
        print_node(myInode);
        return -EISDIR;
    }

    // Delete the entry in the directory
    directory_delete_entry(path);

    // Decrement the number of refs to the inode.
    myInode->refs--;
    update_last_modified(&myInode->meta_time);

    // If the number of refs == 0, no one points to this inode anymore, delete
    if(myInode->refs == 0) {
        // Delete the given node and free its associated pages in the data bitmap
        inode_delete(myInode);
    }

    return 0;
}

int
nufs_rmdir(const char *path)
{
    printf("rmdir(%s)\n", path);

    directory* myDir = directory_from_path(path);
    inode* dirInode = inode_from_path(path);

    if(!myDir) // check if the directory exists
    {
        return -ENOENT;
    }
    else if(myDir->numEntries != 0) // Ensure the directory is empty
    {
        return -ENOTEMPTY;
    }

    // Delete the entry at the given path
    directory_delete_entry(path);
    inode_delete(dirInode);

    return 0;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    printf("rename(%s => %s)\n", from, to);

/*    direntry* entry = direntry_from_path(from);

    if(!entry)
    {
        return -ENOENT; // the entry could not be found
    }

    printf("current name %s\n", entry->name);
    memset(entry->name, 0, NUFS_MAX_NAME_LENGTH);
    printf("current name %s\n", entry->name);
    strcpy(entry->name, to);
    printf("current name %s\n", entry->name);
*/
    inode* node = inode_from_path(from);
    nufs_link(from, to);
    nufs_unlink(from);

    update_last_modified(&node->meta_time);

    return 0;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    printf("chmod(%s, %04o)\n", path, mode);

    inode* node = inode_from_path(path);
    if(!node)
    {
        return -ENOENT;
    }
    else if (!comparePermissions(mode, node->permissions))
    {
        printf("chmod: permissions error\n");
        return -EACCES;
    }
    node->permissions = mode;
    update_last_modified(&node->meta_time);
    return 0;
}

int
nufs_truncate(const char *path, off_t size)
{
    printf("truncate(%s, %ld bytes)\n", path, size);

    // Which direntry index is this?
    inode* node = inode_from_path(path);

    // return error code if no inode is found
    if(!node)
    {
        return -ENOENT;
    }

    // clear indexes for truncated data pages
    int direct_id_index = size / PAGE_SIZE;
    byte* data_bitmap = get_data_bitmap();
    for(int ii = direct_id_index; ii < MAX_DIRECT; ++ii)
    {
        bitmap_clear_bit(data_bitmap, node->direct_id[ii]);
        node->direct_id[ii] = -1;
    }
    node->size = size;
    node->blocks_used = calc_blocks_used(size);
    update_last_modified(&node->meta_time);


    return 0;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);

    int inode_id = inode_id_from_path(path);
    if (inode_id < 0)
    {
        return inode_id;
    }

    return 0; // has to return zero
}

/**
* Read the given size amount of data, starting from the given offset, from the
* given path.
* @param path       The path of the data to read.
* @param buf        The buffer to read the data into.
* @param size       The amount of data to read.
* @param offset     How much to offset the starting address when reading data.
* @param fuse_file_info   A struct for the FUSE api.
* @return           The number of bytes read.
*/
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
    char* data = get_data(path);
    int bytes2read = size;
    int zeroes2write = 0;

    if(!data)
    {
        return -ENOENT;
    }

    // truncates data if needed
    int len = strlen(data) + 1;
    printf("nufs_read: data file size is %d bytes\n", len);

    // len = the amount of data we actually have to give
    // size = total number of bytes that need to be read to satisfy requirements
    // size + offset = the amount of data that needs to exist in order for us to
    // read it.
    //
    // If we need to read more data than we can from the buffer
    if (bytes2read > (len - offset))
    {
        zeroes2write = bytes2read - (len - offset);
        bytes2read = (len - offset);
    }

    // Trying to make sure we can't read passed the end of our actual data...
    //assert( len > (size + offset) );
    // Copying in the amount of data we actually have available
    strlcpy(buf, data + offset, bytes2read);
    // Setting the excess bytes requested to 0.
    // memset(buf + bytes2read, 0, zeroes2write);

    free((void*) data);
    printf("nufs_read: read %d bytes\n", bytes2read);
    return bytes2read-1;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
    // get the inode of the given path 
    inode* node = inode_from_path(path);
    
    if(!node)
    {
        printf("write: no entry. but should never see this because mknode will be called prior\n");
        return -ENOENT;
    }

    int direct_id_index = offset / PAGE_SIZE; // Which page we want to write to
    int page_offset = offset % PAGE_SIZE; // Where in that page we want to start writing
    int bytesWritten = 0;
    int bytes2write_this_iteration;
    int totalBytes2Write = size;
    int block2alloc = direct_id_index - (node->blocks_used - 1);
    int newPageIndex;

    if(node->blocks_used + block2alloc > MAX_DIRECT)
    {
        printf("Write Error: file this large is unsupported Requested write requires %d blocks but only %d blocks are supported.\n",
                        node->blocks_used + block2alloc, MAX_DIRECT);
        return 0;
    }
    
    // Claim more pagse for this file if necessary
    for(int ii = 0; ii < block2alloc; ++ii)
    {
        newPageIndex = data_page_next_free();
        if(newPageIndex == -ENOSPC)
        {
            return 0;
        }
        node->direct_id[node->blocks_used + ii] = newPageIndex;
    } 

    node->blocks_used += block2alloc;
    
    printf("write: writing to direct_id[%d], page_index: %d\n", 
                    direct_id_index, node->direct_id[direct_id_index]);

    page* workingPage = data_page_get(inode_get_page_index(node, direct_id_index));

    // Calculate the number of bytes to write to the first block.
    bytes2write_this_iteration = (PAGE_SIZE - page_offset);
    if (bytes2write_this_iteration > size)
    {
        bytes2write_this_iteration = size;
    }

    memcpy((void*) workingPage + page_offset, (void*) buf, bytes2write_this_iteration);
    bytesWritten += bytes2write_this_iteration;
    node->size += bytesWritten;

    // Write data to all the other data pages...
    int iterations = 1; // to be used as an offset...
    while(bytesWritten != size)
    {
        page* workingPage = data_page_get(node->direct_id[direct_id_index + iterations]);

        // block hadn't been allocated, allocate one.
        if (!workingPage)
        {
            if(node->blocks_used < MAX_DIRECT) // Still have free room!
            {
                int newPageIndex = data_page_next_free();
                if(newPageIndex < 0)
                {
                    return -ENOSPC;
                }
                // changed from *workingPage = &data_page_get(newPageIndex);
                workingPage = data_page_get(newPageIndex);

                node->blocks_used++;
            } else { // Already used the max direct blocks,
                return -ENOSPC;
            }
        }
        // At this point, workingPage is equal to useable page we can write to on disk

        if(totalBytes2Write > PAGE_SIZE)
        {
            bytes2write_this_iteration = PAGE_SIZE;
        } else
        {
            bytes2write_this_iteration = totalBytes2Write;
        }

        memcpy((void*) workingPage, (void*) buf + bytesWritten, bytes2write_this_iteration);
        // Update the number of bytes we've already written
        bytesWritten += bytes2write_this_iteration;
        iterations++;
        node->size += bytes2write_this_iteration;
    }
    update_last_modified(&node->meta_time);
    return bytesWritten;
}

//TODO: integrate this and make sure meta_time updating is consistent
// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = -1;

    inode* node = inode_from_path(path);
    if (node)
    {
        node->meta_time.lastAccess = ts[0];
        node->meta_time.lastModified = ts[1];
        rv = 0;
    }
    else
    {
        return -ENOENT;
    }
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknode;
    ops->mkdir    = nufs_mkdir;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->link     = nufs_link;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}
