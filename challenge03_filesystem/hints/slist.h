#ifndef SLIST_H
#define SLIST_H

/**
This struct represents a singly linked list. Data field be used to map filenames
to inode id numbers.
Examples: "hello.txt/5", "Thnx4TheFish.txt/2"
*/
typedef struct slist {
    char* data;            // a mapping of file name to inode id number
    int   refs;            // the number of references to this inode id.
    struct slist* next;    // the rest of the list.
} slist;

/**
* Add an element to an slist.
* @param text   The data to add to the list.
* @param rest   The original list to add to.
* @return       The new list.
*/
slist* s_cons(const char* text, slist* rest);

/**
* Decrement the count of the references to the head of the given list. If it
* has no references, delete it as the file has been deleted (hence no references,
* though no 10000% sure on this).
* @param xs     The given list.
*/
void   s_free(slist* xs);

/**
* Get the slist around the given delimiter - not totally sure.
* @param text   The text to check for delim.
* @param delim  The delimiter.
* @return       The slist that doesn't contain the delimiter.
*/
slist* s_split(const char* text, char delim);

#endif
