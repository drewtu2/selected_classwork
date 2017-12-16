#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

// to run this test, go to https://www.onlinegdb.com/online_c_compiler
// copy and paste all of the code, and click run

int main()
{
    typedef struct disk {
        int inode_bitmap;             // offset for inode bitmap location
        int data_pages_bitmap;        // offset for data pages bitmap location
        int inode_list;               // offset for inode list location
        int data;                     // offset for data page location
    } disk;

    typedef struct inode {
        int node_id;                    // node id
        int blocks_used;                // the number of blocks used, calculated based on size
        size_t size;                    // size of the data pointed to by the inode in bytes
        int refs;                       // number of references to this inode
        int type;                       // what type of inode (type) 0 is none, 1 is file, 2 is directory
        mode_t permissions;             // the file permissions
        int direct_id[10];      // array of 10 page indexes
        int indirect_id;        // index of a page that is this inode's indirect link
        int double_indirect_id; // index of a page that is this inodes double indirect link
    } inode;


    disk* tempDisk = (disk*) malloc(sizeof(disk));

    printf("disk address is: %p\n", tempDisk);

    // get the offset for the inode_bitmap
    int offset = sizeof(disk);
    tempDisk->inode_bitmap = offset;
    printf("inode_bitmap offset is: %d\n", offset);

    // get the offset for the data_pages_bitmap
    offset += 1 + (128 / 8);
    tempDisk->data_pages_bitmap = offset;
    printf("data_pages_bitmap offset is: %d\n", offset);

    // get the offset for the inode_list
    offset += 1 + (128 / 8);
    tempDisk->inode_list = offset;
    inode* list = (inode*) (tempDisk + tempDisk->inode_list);
    printf("inode_list offset is: %d\n", offset);
    printf("inode_list address is: %p\n", list);
    inode* test_node = &list[1];
    printf("inode size is: %lu\n", sizeof(inode));
    printf("test_node address is: %p\n", test_node);
    // get the offset for the data pages
    offset += 128 * sizeof(inode);
    offset = offset + (offset % 4096);

    tempDisk->data = offset;
    printf("data offset is: %d\n", offset);
    printf("Hello World");

    return 0;
}
