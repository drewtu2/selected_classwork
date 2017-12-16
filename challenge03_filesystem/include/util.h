#ifndef UTIL_H
#define UTIL_H
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>
#include <inode.h>

/**
* Print the contents of the given stat structure and its path.
* @param st           The stat structure to print.
* @param path         The path for the file that the stat structure represents.
*/
void print_stat(struct stat* st, const char* path);

/**
* Calculate the number of blocks used based on the given size.
*
* @param size     The size of the inode data.
* @return         The number of blocks used.
*/
int calc_blocks_used(size_t size);

/**
* Get the next page boundary.
* @param input    The address to calculate the next sequential page boundary.
* @return         The address of the next page boundary.
*/
void* nextPageBoundary(void* input);

/**
* Get the node node boundary.
* @param input    The address to calculate the next sequential inode boundary.
* @return         The address of the next inode boundary.
*/
inode* nextNodeBoundary(void* input);

/**
* Get the offset of the next page.
* @param input_offset   The input offset.
* @return               The offset for the next page boundary.
*/
int offset_next_page(int input_offset);

/**
* Check if the two given strings are equal.
* @param aa         One string to check.
* @param bb         The other string to check.
* @return           1 if the strings are 0, if not, then return 0.
*/
int
streq(const char* aa, const char* bb);

/**
* Return the minimum of two given numbers.
* @param x      One of the numbers.
* @param y      The other given number.
* @return       The smaller of the two numbers, x or y.
*/
int
min(int x, int y);

/**
* Return the maximum of two given numbers.
* @param x      One of the numbers.
* @param y      The other given number.
* @return       The larger of the two numbers, x or y.
*/
int
max(int x, int y);

#endif
