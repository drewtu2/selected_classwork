# Northeastern University File System (NUFS) Report
Authors: Andrew Tu and Thomas Harmon
CS3650
Professor Nat Tuck

## Prompt
- Write a report.txt. Include at least the following:
- What advantages and drawbacks does your filesystem have? How would you improve
it if you had more time?
- What features did you complete? What is still missing? Did you implement any
additional functionality that wasn't required?

## NUFS
## Features
Our filesystem supports the following.
1. Nested directories (depth only limited by total storage disk size)
2. Hard links
3. Reading and writing very large files
4. Meta-data for files and directories (i.e. permissions and timestamps)

We managed to implement everything that was described in the assignment.
We also managed to implement bitmaps even though it wasn't required, because it
made managing our free space and free inodes easier and faster.

NOTE: In order for the timestamps to properly show the correct local time,
the user's system must have properly configured their settings for the local
time zone. In Ubuntu 16.04, one way to do set this to EST is the following:

`apt-get update`
`apt-get upgrade`
`apt-get install tzdata`
`echo America/New_York >/etc/timezone`
`ln -sf /usr/share/zoneinfo/America/New_York /etc/localtime`


## Advantages and Drawbacks
- As a programmer, one advantage of our filesystem is that we have very
straight-forward code that makes using our design and implementing features on
it very easy. Our code is also very well documented with Doxygen comments making
it very easy to use and understand. As a user, one advantage to our filesystem
is that it implements bitmaps, which makes the filesystem faster when trying to
search for free space rather than using a list of free data chunks. This is
a faster approach because there is more locality to the meta-data of the storage
disk, ie where it has free files and where files in use store their data,
meaning there is less random seeking and traversing of lists.
- As a user, another advantage to our filesystem is that we optimized the number
of files that you can use, given the constraint of the total storage disk being
only 1MB. We did this by using the maximum number of inodes we could while
keeping the page count >= to the number of inodes. This is a big advantage
because studies have shown that the average file size is < 2kB, so the ability
to efficiently store many small files is more important is many use cases.
- Another advantage to our filesystem is that the depth of nested directories is
only limited by the free space on the disk.
- Another advantage is that there is not limit to the maximum depth of
directories that can be made.

- One drawback of our filesystem is that the maximum number of entries in a single
directory is limited to 70, this provides a lot of simplification because
it means that a directory will never take more than a page of data, avoiding
fragmentation.
- Another drawback to our filesystem is that file and directory names are
limited to a maximum length of 48 characters, though this is not too bad as
even many recently modern filesystems have name length limits, and our limit
is much longer than say, the original implementation of the EXT filesystem.

## Improvements
If we had more time, we would implement (double) indirect blocks, balanced trees,
and soft links.

Our implementation doesn't implement indirect blocks and only uses direct blocks.
This means for all inodes, space for 25 integers must be stored for all of the
possible direct blocks an inode can have. This increases the size of storing an
inode, increasing the overhead of our file system.

As shown in EXT4, balanced trees are more optimized for storing a large
amount of files in a directory. This is helpful because it takes O(n) time to
search for an entry in our filesystem, but with the implementation of B or H
trees, the lookup time can be improved to O(log n).

Soft links are special types of files that contain the path to another file,
making them shortcuts to the other file. These are known as symbolic links.
This functionality is commonly used on Windows to create shortcuts to
applications.

Another improvement we would make is not limiting the breadth of entries in a
directory artificially, and only limiting it by space. This would take a lot
more work to deal with fragmentation and a decent rework of our directory work,
but would be a nice addition. Fortunately, this wouldn't be too big of an
issue for most people as 70 files is a lot to have in a single folder.
