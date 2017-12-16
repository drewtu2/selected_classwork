#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "pages.h"
#include "constants.h"
#include "bitmaps.h"
#include "inode.h"
#include "util.h"
#include "directory.h"

static int   disk_fd   = -1;
static void* disk_base =  0;
disk*        myDisk    =  NULL;

/*******************************************************************************
* High Level Operations
*******************************************************************************/
// Creates the stack reperesentation of the disk by calling disk_make, and then
// writes the superblock to the disk...
void
storage_init(const char* path)
{
    myDisk = disk_make(path);
    inode* root = get_root();
    if(root->size == 0)
    {
        printf("first time initializing the disk\n");
        // init inode bitmap
        init_bitmap(get_inode_bitmap());
        // init data bitmap
        init_bitmap(get_data_bitmap());
        // init root inode
        inode* root = inode_make(MODE_DIRECTORY, INODE_DIRECTORY);
        // init root directory
        directory* root_dir = directory_make(root);
    }
    *((disk*)disk_base) = *myDisk;

    printf("storage_init: id of root inode is %d in data page %d\n",
                    root->node_id, root->direct_id[0]);
}

void
storage_free()
{
    int rv = munmap(disk_base, NUFS_SIZE);
    assert(rv == 0);
}

/*******************************************************************************
* Disk Operations
*******************************************************************************/
// Creates the representation of the disk on the stack
disk*
disk_make(const char* path)
{
    disk_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(disk_fd != -1);

    int rv = ftruncate(disk_fd, NUFS_SIZE);
    assert(rv == 0);

    disk_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, disk_fd, 0);
    assert(disk_base != MAP_FAILED);

    disk* tempDisk = (disk*) disk_base;
    printf("disk_base address is: %p\n", tempDisk);

    // get the offset for the inode_bitmap
    int offset = sizeof(disk);
    tempDisk->inode_bitmap = offset;
    printf("inode_bitmap size: %d\n", bitmap_get_size(NUM_INODES));
    printf("inode_bitmap offset is: %d\n", offset);

    // get the offset for the data_pages_bitmap
    offset += bitmap_get_size(NUM_INODES);
    tempDisk->data_pages_bitmap = offset;
    printf("data_pages_bitmap size: %d\n", bitmap_get_size(PAGE_COUNT));
    printf("data_pages_bitmap offset is: %d\n", offset);

    // get the offset for the inode_list
    offset += bitmap_get_size(PAGE_COUNT);
    tempDisk->inode_list = offset;
    printf("inode_list offset is: %d\n", offset);

    // get the offset for the data pages
    offset += NUM_INODES * sizeof(inode);
    printf("inode size: %lu\n", sizeof(inode));
    printf("offset value: %d\n", offset);
    offset = offset_next_page(offset);
    assert( offset % PAGE_SIZE == 0 );
    tempDisk->data = offset;
    printf("data offset is: %d\n", offset);

    return tempDisk;
}

int get_stat(const char* path, struct stat* st)
{
    inode* tempNode = inode_from_path(path);

    if(!tempNode) {
        printf("get_stat: %s does not exist\n", path);
        return -ENOENT;
    }
    else {
        //printf("Path: %s, Inode: %d\n", path, tempNode->node_id);
        st->st_ino = tempNode->node_id;    /* inode's number */
        st->st_mode = tempNode->permissions;   /* inode protection mode */
        st->st_nlink = tempNode->refs;  /* number of hard links to the file */
        st->st_atim = tempNode->meta_time.lastAccess;  /* time of last access */
        st->st_mtim = tempNode->meta_time.lastModified;  /* time of last data modification */
        st->st_ctim = tempNode->meta_time.created;  /* time of last file status change */
        st->st_size = tempNode->size;   /* file size, in bytes */
        st->st_blocks = calc_blocks_used(tempNode->size); /* blocks allocated for file */
        st->st_blksize = PAGE_SIZE;/* optimal file sys I/O ops blocksize */
        st->st_uid = getuid();
        st->st_gid = getgid();
        return 0;
    }
}

char* get_data(const char* path)
{
    // get the inode from the given path
    inode* node = inode_from_path(path);
    if (!node)
    {
        return NULL;
    }

    // test to see if nufs_read and thus get_data is ever called on directories by FUSE
    assert( node->type != INODE_DIRECTORY );

    void* buf = malloc(node->blocks_used * PAGE_SIZE);
    for(int ii = 0; ii < node->blocks_used; ++ii)
    {
        void* data = (void*) (data_page_get(inode_get_page_index(node, ii)));
        memcpy(buf + (ii * PAGE_SIZE), data, PAGE_SIZE);
    }

    // TODO: maybe follow indirect links, or increase MAX_DIRECT

    update_last_access(&node->meta_time);
    return buf;
}

disk* get_disk()
{
    return myDisk;
}

byte* get_inode_bitmap()
{
    return (byte*) (get_disk() + get_disk()->inode_bitmap);
}

byte* get_data_bitmap()
{
    return (byte*) (get_disk() + get_disk()->data_pages_bitmap);
}

inode* get_inode_list()
{
    return (inode*) (get_disk() + get_disk()->inode_list);
}

page* get_data_pages()
{
    return (page*) (get_disk() + get_disk()->data);
}

inode* get_root()
{
    return get_inode_list();
}
