#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "constants.h"


void print_stat(struct stat* st, const char* path)
{
//    st->st_mode = MODE_DIRECTORY;
//    st->st_size = 0;
    printf("path: %s, stat {nodeid: %lu, size: %lu, mode_t: %o}\n",
                    path, st->st_ino, st->st_size, st->st_mode);
}

int calc_blocks_used(size_t size)
{
    return (int) ceil(size / PAGE_SIZE);
}

void* nextPageBoundary(void* input)
{
    return (void*)(((long)input & (long)(~(PAGE_SIZE - 1))) + PAGE_SIZE);
}

inode* nextNodeBoundary(void* input)
{
    return (inode*)(((long)input & (long)(~(sizeof(inode) - 1))) + sizeof(inode));
}

int offset_next_page(int input_offset)
{
    if ((input_offset % PAGE_SIZE) == 0)
    {
        return 0;
    }
    int delta = (PAGE_SIZE - (input_offset % PAGE_SIZE));
    return input_offset + delta;
}

int
streq(const char* aa, const char* bb)
{
    return strcmp(aa, bb) == 0;
}

int
min(int x, int y)
{
    return (x < y) ? x : y;
}

int
max(int x, int y)
{
    return (x > y) ? x : y;
}
