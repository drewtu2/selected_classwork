#ifndef PAGES_H
#define PAGES_H

#include "constants.h"

/**
This struct represents a contiguous block of data, of size equal to 1 page (4096).
*/
typedef struct page {
    char data[PAGE_SIZE];
} page;

/**
* Get the page of data at the given page index.
* @param page_index     The data page index to get.
* @return               The data page.
*/
page* data_page_get(int page_index);

/**
* Get the index of the first page that's empy using the bitmap and set it as
* no longer free in the bitmap.
* @return               The index of the first empty page.
*/
int data_page_next_free();

#endif
