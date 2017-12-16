#include <metaTimes.h>
#include <time.h>
#include <stdio.h>

/*
typedef struct metaTimes
{
  struct timespec created;
  struct timespec lastAccess;
  struct timespec lastModified;
} metaTimes;
*/

metaTimes metaTimes_make()
{
    metaTimes myTime;
    struct timespec rawTime;
    clock_gettime(CLOCK_REALTIME, &rawTime);

    myTime.created = rawTime;
    myTime.lastAccess = rawTime;
    myTime.lastModified = rawTime;

    // Debug Statments...
    //printf("metaTimes_make: making new meta times\n");

    return myTime;
}

void update_last_access(metaTimes* oldTimes)
{
    printf("update_last_access:\n");
    //print_metaTime(oldTimes);
    struct timespec rawTime;
    clock_gettime(CLOCK_REALTIME, &rawTime);
    oldTimes->lastAccess = rawTime;
    //print_metaTime(oldTimes);
}

// and update last access, cause u have to access something
// in order to modify it
void update_last_modified(metaTimes* oldTimes)
{
    printf("update_last_modified:\n");
    //print_metaTime(oldTimes);
    struct timespec rawTime;
    clock_gettime(CLOCK_REALTIME, &rawTime);
    oldTimes->lastModified = rawTime;
    oldTimes->lastAccess = rawTime;
    update_last_access(oldTimes);
}

void print_metaTime(metaTimes* myTime)
{
    printf("created: %ld seconds and %ld nanoseconds.\n",
                    myTime->created.tv_sec, myTime->created.tv_nsec);
    printf("lastAccess: %ld seconds and %ld nanoseconds.\n",
                    myTime->lastAccess.tv_sec, myTime->lastAccess.tv_nsec);
    printf("lastModified: %ld seconds and %ld nanoseconds.\n",
                    myTime->lastModified.tv_sec, myTime->lastModified.tv_nsec);
}
