#include <assert.h>
#include <errno.h>

#include "pages.h"
#include "storage.h"
#include "bitmaps.h"

page*
data_page_get(int page_index)
{
    assert( page_index <= PAGE_COUNT );
    page* data = get_data_pages();
    page* desired_page = &data[page_index];
    return desired_page;
}

int
data_page_next_free()
{
    int bitValue;
    int bitmap_size = bitmap_get_size(PAGE_COUNT);
    byte* data_bitmap = get_data_bitmap();

    for(int ii = 0; ii < PAGE_COUNT; ++ii) {
        bitValue = bitmap_get_bit(data_bitmap, ii);
        if (bitValue == FREE) {
            bitmap_set_bit(data_bitmap, ii);
            return ii;
        }
    }

    printf("data_page_next_free: couldn't find a free bit\n");
    return -ENOMEM; // TODO: or -ENOSPC??;
}
