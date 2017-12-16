#ifndef BITMAPS_H
#define BITMAPS_H

/**
A single byte for use in the bitmaps.
*/
typedef unsigned char byte;

/**
* Initialize a bitmap, setting all bits to FREE.
* @param bitmap     The bitmap to initialize.
*/
void init_bitmap(byte* bitmap);

/**
* Get the number of bytes required to store a given number of elements. The
* number of elements must be evenly divisible by BITS_PER_BYTE.
* @param numEl      The number of elements to calculate bitmap size with.
* @return           The size of the bitmap with the given number of elements.
*/
int bitmap_get_size(int numEl);

/**
* Sets the value of a bit as used at a given index in the bitmap.
* @param map        The bitmap that contains the bit to set.
* @param index      The index of the bit to set.
*/
void bitmap_set_bit(byte* map, int index);

/**
* Clears the value of a bit at a given index in the bitmap
* @param map        The bitmap that contains the bit to clear.
* @param index      The index of the inode/data page to set in the bitmap as free.
*/
void bitmap_clear_bit(byte* map, int index);

/**
* Returns the value of a bit at a given index in the bitmap.
* @param map        The bitmap that contains the bit to return.
* @param index      The index of the bit to get.
* @return           The value of the bit at the given index.
*/
int bitmap_get_bit(byte* map, int index);

#endif
