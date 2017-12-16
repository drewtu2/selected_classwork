#ifndef META_TIMES_H
#define META_TIMES_H

#include <time.h>

typedef struct metaTimes
{
  struct timespec created;
  struct timespec lastAccess;
  struct timespec lastModified;
} metaTimes;

metaTimes metaTimes_make();

/**
* Update the last access time field of the given old meta times struct.
* @param oldTimes       The metaTimes struct to update.
*/
void update_last_access(metaTimes* oldTimes);

/**
* Update the last access and modified time field of the given old meta times struct.
* @param oldTimes       The metaTimes struct to update.
*/
void update_last_modified(metaTimes* oldTimes);


/**
* Print the given metaTimes struct.
* @param oldTimes       The metaTimes struct to print.
*/
void print_metaTime(metaTimes* myTime);


#endif
