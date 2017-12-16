#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bitmaps.h"
#include "constants.h"

void init_bitmap(byte* bitmap)
{
    assert( NUM_INODES == PAGE_COUNT );
    memset(bitmap, FREE, BITMAP_SIZE);
}

int bitmap_get_size(int numEl)
{
    assert( (numEl % BITS_PER_BYTE) == 0 );
    return (numEl / BITS_PER_BYTE);
}

void bitmap_set_bit(byte* map, int index)
{
    int bitmap_index = index / BITS_PER_BYTE;     // i = array index (use: A[i])
    int bitmap_pos = index % BITS_PER_BYTE;       // pos = bit position in A[i]
    byte flag = 1;
    flag = flag << bitmap_pos;

    map[bitmap_index] = map[bitmap_index] | flag;

}

void bitmap_clear_bit(byte* map, int index)
{
    int bitmap_index = index / BITS_PER_BYTE;     // i = array index (use: A[i])
    int bitmap_pos = index % BITS_PER_BYTE;       // pos = bit position in A[i]
    byte flag = 1;
    flag = flag << bitmap_pos;
    flag = ~flag;

    map[bitmap_index] = map[bitmap_index] & flag;
}

int bitmap_get_bit(byte* map, int index)
{

    int bitmap_index = index / BITS_PER_BYTE;     // i = array index (use: A[i])
    int bitmap_pos = index % BITS_PER_BYTE;       // pos = bit position in A[i]
    byte flag = 1;
    flag = flag << bitmap_pos;

    return (map[bitmap_index] & flag);

}
