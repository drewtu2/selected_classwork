
from pages.h and pages.c
/**
* Get the first page of data for a given inode_id.
* @param inode_id       The inode id for the node that points to the page to get.
* @return               The data page.
*/
page* data_page_get_with_node_id(int inode_id);

page* data_page_get_with_node_id(int inode_id)
{
    assert( inode_id <= NUM_INODES);
    inode* node = inode_get(inode_id);
    int page_index = node->direct_id[0];
    page* page = data_page_get(page_index);
    return page;
}

// TODO: only searches for inodes in pages linked to inode through direct links
/**
* Get the inode that points at the given page_index.
* @param page_index     The page index.
* @return               The inode that points to the given page index.
*/
inode* pages_get_node(int page_index);

// TODO: only searches for inodes in pages linked to inode through direct links
inode*
pages_get_node(int page_index)
{
    // loop through inode bitmap and check each node that is not free
    // for each inode, check if any of the links point to the given page_index
    byte* inode_bitmap = get_inode_bitmap();
    inode* node = NULL;
    int bitmap_size = bitmap_get_size(NUM_INODES);
    for(int ii = 0; ii < bitmap_size; ++ii)
    {
        // the inode is in use & may point to the given page index
        if(inode_bitmap[ii] == 1)
        {
            node = inode_get(ii);
            for(int jj = 0; jj < MAX_DIRECT; ++jj)
            {
                if(page_index == node->direct_id[jj])
                {
                    return node;
                }
            }
        }
    }
    return node; // hopefully this never happens
}


from storage.h
/**
* Return the address of the storage disk.
* @return           The storage disk address.
*/
void* disk_addr();

/**
* Return the address of the storage disk's inode bitmap.
* @return           The starting address of the inode bitmap.
*/
void* inode_bitmap_addr();

/**
* Return the address of the data_pages_bitmap.
* @return           The starting address of the data pages bitmap.
*/
void* data_bitmap_addr();

/**
* Return the address of the start of the inode_list.
* @return           The address of the start of the inode_list.
*/
void* inode_list_addr();

/**
* Return the address of the beginning of the data pages.
* @return           The address of the start of the data pages.
*/
void* data_pages_addr();

void* data_pages_addr()
{
    return (void*) get_data_pages();
}

void* inode_list_addr()
{
    return (void*) get_inode_list();
}

void* data_bitmap_addr()
{
    return (void*) get_data_bitmap();
}

void* inode_bitmap_addr()
{
    return (void*) get_inode_bitmap();
}

void* disk_addr()
{
    return (void*) get_disk();
}


from util.c and util.h
/**
* Split the name at the top level of the given path.
* @param path    The given path name.
* @return        The name of the top level directory of the given path.
*/
char* split_first(char* path);

char* split_first(char* path)
{
    const char delim[2] = "/";
    path = strtok(path, delim);
    return path;
}


from directory.c and directory.h

/**
* Get the 1st data page that the direntry with the given name points to.
* @param dir        The directory containing the entry with the given name.
* @param name       The name of the direntry.
* @return           The 1st data page of the direntry with the given name.
*/
page* directory_lookup_page(directory* dir, const char* name);

page* directory_lookup_page(directory* dir, const char* name)
{
    return data_page_get(directory_lookup_pnum(dir, name));
}


/**
* Make a new direntry and insert it.
* @param dir            The directory to put the direntry into.
* @param name           The name of the direntry to make.
* @return               An error code on failure, or the index of insertion on
                        success.
*/
int directory_insert_entry(directory* dir, const char* name);

int directory_insert_entry(directory* dir, const char* name, int inode_id)
{
    if (dir->numEntries >= MAX_NUM_DIRENTRY)
    {
        printf("the given dir does not have enough room to insert an entry\n");
        return -ENOSPC;
    }

    assert( sizeof(*name) <= NUFS_MAX_NAME_LENGTH );

    direntry entry =

    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        dirnetry  currentEntry = direntry_from_entry_index(dir, ii);
        if (currentEntry.isUsed == FREE)
        {
            entry->isUsed = OCCUPIED;
            dir->entries_offset[ii] = entry;
            dir->numEntries++;
            printf("inserted entry %s at index %d\n", name, ii);
            return ii;
        }
    }

    printf("failed to insert entry\n");
    return -ENOSPC;
}

/**
* Put the entry (either file or directory) with the given name and inode id into
* the given directory.
* @param dd     The directory to place the entry into.
* @param entry  The entry to add to the given directory.
* @return       index of entry on success, otherwise error code.
*/
int directory_put_ent(directory* dir, direntry* entry);

// TODO: Someohow add the size back to the corresponding inode
int directory_put_ent(directory* dir, direntry* entry)
{
    printf("in directory_put_ent, want to add %s\n", entry->name);
    if (dir->numEntries >= MAX_NUM_DIRENTRY)
    {
        printf("directory_put_ent: too many entries (%d) in dir\n", dir->numEntries);
        return -ENOSPC;
    }

    //inode* directoryInode = inode_get(dir->node_id);
    direntry* currentEntry = NULL;

    for (int ii = 0; ii < MAX_NUM_DIRENTRY; ++ii)
    {
        currentEntry = direntry_from_entry_index(dir, ii);
        if (currentEntry->isUsed == FREE)
        {
            entry->isUsed = OCCUPIED;
            direntry* dir_entries_start = (direntry*)((void*)dir + dir->entries_offset);
            dir_entries_start[ii] = *entry;

            dir->numEntries++;
     //       directoryInode->size += sizeof(direntry);
            return 0;
        }
    }

    printf("directory_put_ent: no free entries\n");
    return -ENOSPC;
}


/**
* Creates a directory structure froma a give page num (from serialized data).
* @param pnum  The page number that contains the data for the directory to return.
* @return      The directory at the given page number.
*/
directory* directory_from_pnum(int pnum);

directory* directory_from_pnum(int pnum)
{
    assert( pnum <= PAGE_COUNT );
    return (directory*) data_page_get(pnum);
}


/**
* Returns the inode id of a requested entry in a given directory. If the entry
* doesn't exist, -ENOT is returned.
* @param dir    The directory that contains the requested entry.
* @param name   The name of the entry to get.
* @return       The inode id of the requested entry, or -ENONT if the entry
*               doesn't exist.
*/
int directory_lookup_node_id(directory* dir, const char* name);

int directory_lookup_node_id(directory* dir, const char* name)
{
    inode* node = directory_lookup_inode(dir, name);

    if (!node)
    {
        return -ENOENT;
    }
    else
    {
        return node->node_id;
    }
}

/**
* Returns the inode of a requested entry in a given directory. If the entry
* doesn't exist, -ENOT is returned.
* @param dir    The directory that contains the requested entry.
* @param name   The name of the entry to get.
* @return       The inode of the requested entry, or -ENONT if the entry
*               doesn't exist.
*/
inode* directory_lookup_inode(directory* dir, const char* name);

inode* directory_lookup_inode(directory* dir, const char* name)
{
    int entry_index = directory_lookup_entry_index(dir, name);

    if (entry_index == -ENOENT)
    {
        return NULL;
    }

    direntry* entry = direntry_from_entry_index(dir, entry_index);
    return inode_get(entry->node_id);
}

/**
* Get the page number of the entry with the given name in the given directory.
* @param dir        The directory.
* @param name       The name of the entry.
* @return           The page number of the entry.
*/
int directory_lookup_pnum(directory* dir, const char* name);

int directory_lookup_pnum(directory* dir, const char* name)
{
    inode* node = directory_lookup_inode(dir, name);

    if (!node)
    {
        return -ENOENT;
    }

    return node->direct_id[0];
}



directory* directory_from_pnum(int pnum)
{
    assert( pnum <= PAGE_COUNT );
    return (directory*) data_page_get(pnum);
}




from inode.c

inode_from_path alternative
/*
printf("getting inode_id from path: %s\n", path);
inode* current_node = get_root();
if(streq(path, "/"))
{
    return current_node;
}

slist* path_list = s_split(path, "/");
printf("first path: %s, next path: %s\n", path_list, path_list->next);
directory* current_dir = current_node;

int direntry_index = -ENOENT;
while (!path_list) {
    printf("current path_list->data: %s\n", path_list->data);
    direntry_index = directory_lookup_entry_index(current_dir, path_list->data);
    if(index < 0)
    {
        printf("the current dir didnt have a matching entry\n");
        return -ENOENT;
    }

    direntry entry = current_dir->entries[direntry_index];
    int node_index = entry.node_id;
    printf("the entry has a node_id of %d", node_index);

    if(!path_list->next)
    {
        return inode_get(node_index);
    }
    else
    {
        current_dir = directory_from_node_id(node_index);
        path_list = path_list->next;
    }
}
return -ENOENT;

*/
