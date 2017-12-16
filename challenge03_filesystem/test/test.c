#ifndef MYTEST_H
#define MYTEST_H

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define NUFS_SIZE 4096

/*
 * This test code is a study to better understand how memory mapped IO works.
 * We wanted to better understand how data persisted through code runs, i.e. to
 * make sure that when data is written to the memory, the correct values are
 * stored.
 *
 * This test code shows that we can write the supernode directly to the memory
 * mapped io region... Writing the supernode (containing pointers) directly to
 * the memory will store the proper addreses of the disk to the beginning of the
 * memory mapped region.
 */



typedef struct testMapping
{
    void* test1;
    void* test2;
} testMapping;

int main()
{
    // Set up the mmap
    char* path = "test.data";
    void* pages_base;
    int pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);
    assert(pages_base != MAP_FAILED);

    // Create the Test object
    testMapping aTest;
    aTest.test1 = pages_base;
    aTest.test2 = pages_base + 20;

    // Print out the saved addreses
    printf("Newly created object...\n");
    printf("test1 loc %lu\n", aTest.test1);
    printf("test2 loc %lu\n", aTest.test2);

    // Save value into the page...
    *((testMapping*) pages_base) = aTest;

    // Try to read and access value...
    void* base2;
    base2 = mmap(0, NUFS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, pages_fd, 0);

    testMapping aTest2 = *((testMapping*) base2);
    // Print out the saved addreses
    printf("New Objects...\n");
    printf("test1 loc %lu\n", aTest2.test1);
    printf("test2 loc %lu\n", aTest2.test2);

    printf("Real placements...\n");
    printf("pages_base %lu\n", pages_base);
    printf("pages_base + 20 %lu\n", pages_base + 20);


//    testMapping* myTest;
//    myTest->test1 = 1;
//    myTest->test2 = 2;

    char test[] = "hello/world/test.txt";

    char *test2 = strtok(test, "/");    // hello
    printf("0: %s\n", test2);

    for(int ii = 1; ii < 4; ++ii)
    {
        test2 = strtok(NULL, "/");
        printf("%d: %s\n", ii, test2);
    }


    // testing dirname/basename
    char testDirname[] = "/mydir/folder/test.txt";
    char testDirname2[] = "/mydir";
    

    return 0;
}

#endif
