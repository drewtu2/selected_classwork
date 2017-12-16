# Challenge 2 - Thread-Safe High Performance Allocator

## Data
Pre-Mailbox Method:

|               |       Sys         |       HW7         |           Par        |
|---------------|-------------------|-------------------|----------------------|
| list-2k input |       0.03        |       4.8         |         1.52         |
|ivec-10k input |       0.14        |      8.528        |         0.23         |

Post-Mailbox Method:

|               |       Sys         |       HW7         |           Par        |
|---------------|-------------------|-------------------|----------------------|
| list-2k input |        0.04       |       6.02        |         0.22         |
|ivec-10k input |        0.13       |       9.67        |         0.2          |

![alt text](graph.png "Graph 1: Allocator comparision")

## System
OS: Ubuntu 16.04
Number of Cores: 2

## Strategy

In the homework 7 allocator, one of the main bottlenecks of the program was 
repeated lock contention when attempting to access the free_list. As the number 
of simulataneous attempts to access the free_list increased, the longer each 
thread had to wait at the mutex before being able to access the memory. Therefore
our first optimization strategy focused on reducing lock contention in order achieve
better performance than our Homework 7 allocator. 

### Thread Private Arenas

We implemented a thread private arena structure for each thread in order to reduce 
the number of accesses to shared memory. Whenever a thread needed to allocate 
memory, it first checked its own local arena to see if a suitable block existed. 
If a suitable block existed, it was truncated to the proper size (see section on 
binning) and allocated. This was the optimal event because it didn't require 
accessing the global arena or making a system call to mmap. If a suitable node 
was NOT found in the local arena, the global arena was checked. Finally, if a 
suitable node could not be found in either location, a new page of memory was 
allocated via mmap with any excess being broken up into the local arena. 

### Binning Approach and Arenas

In our allocations, we attempted to improve allocation speeds by using a binning 
approach in the local arenas. For each local arena, we created 9 bins, each 
intended to hold blocks of size 2^(n+4), i.e. bins ranging from 16 bytes to 4096
bytes. The arena structure was composed of an array of sorted singly-linked lists.
Unlike the local arena which contained multiple block sizes, the global arena 
only contained a single bin of size 4096. Originally we believed this was all
would be suitable for our program but later realized a more robust global arena 
was needed. This is further disussed in the "Challenges and Future Work" section. 

When an allocation of size T was requeted, an additional 8 bytes of memory were 
added to the original request to make space for an allocation header. Then a 
block of the first size greater than the total size was found, i.e. if a block 
of 64 bytes were requested, 8 bytes were added for a header and the first available
block over 72 bytes was used (in this example, 128 byte block). If 128 byte 
block did not exist, the bin in the next level up was checked for available blocks. 
If a block was found, it was split, until the appropraite sized block was found 
and used. The remaining blocks were added back to the local arena. As mentioned 
in the previous section, if a block of suitable size could not be found in the 
local arena, the global arena was searched. 

Using power of 2 sized bins meant that splitting and coalescing bins was 
extremely easy. If two adjacent nodes in a given list could be coalesced, they 
were combined, removed, from the list, and added into the next appropriate level.
We did NOT coalesce bins of size 4096, but instead, let the list in the local
arena grow. When the length of the local_arena exceeded a given set value, nodes
were removed and added to the gloal arena. 

In our implementation, we coalesced bins unconditionally: in retrospect another
design decision that negatively impacted performance. The consequences are
explained in the "Challenges and Future Work" section. 

## Results

As can be seen in the table above and in our graph, our optimized memory allocator
ultimately did not beat the system allocator, although it did blow the allocator
from homework 7 out of the water. Running the list program on each allocator seems
to take significantly longer than the ivec program, so in our tests we decided to
run the ivec programs on an input of 10 thousand and the list programs on an input
of only 2 thousand. This is okay because we are not testing to see how the list and
ivec programs do against each other, we are testing to see how the memory allocators
do against each other, and this test stays intact as long as the input within each
program for each allocator is the same. For the ivec program, our optimized allocator
and the system allocator had relatively close times (even though the system allocator
won) while the hw7 allocator did pretty terribly. For the list program, there is a
clear trend. The system is unambiguously faster than our optimized allocator, which
is unambiguously faster than the hw7 allocator.

### Challenges and Future Work

This was an intensive and fairly complex challenge assignment given under very tight 
time conditions. As a result, we made a number of design decisions that we initially
believed to be okay but later discovered to have a detrimental impact on our results.
Unfortuantely the design flaws were discovered during the testing phase after
over 40 hours of code and would require a signifcant overhaul to reimplement
better features. The following describes what we would have done differently 
to improve performance given enough time.

1. Global arena - Data Cube
When we first started this assignment, we thought that blocks allocated by a given
thread would always stay in the same thread and would tus be freed by the same 
thread, therefore data would always coalesce back to one page. Therefore, we implemented
a global arena system that held entire pages of mapped memory that any thread. 

Unfortunately, this is not the case. This caused excessive memory fragmentation 
since adjacent blocks would end up trapped in different private arenas, forcing 
high lock contention and more mmap'ed memory allocations. 

Knowing this, we would redesign our global arena as the following data "cube". 
The first dimension of our data cube would be an array equal to the number of 
threads running. At the index of each thread is an arena, parallel in structure to the 
local arenas used in our implementation (each arena was an array of singly linked 
lists). In this new model, we would also add the allocating thread's id to the 
header of every block. 

The proposed system would primarly affect how data is freed on each thread. 
Whenever a block of data is freed, the freeing thread checks whether or not they
were the original allocator of the the block. If they were, they free it normally,
adding it to their local arena. If they were not the original allocator, they would
place the block int the appropriate bin of the appropriate arena of the appropriate 
thread in the global_arena. When data is added to a given free bin, an thread
conditional variable is tripped to inform the original thread new data has 
"been delivered" at which point they can move the data back to their local free
list. 

This new design approach addresses two problems. 

The first problem this addresses is memory fragmention. In our current
implementation, memory fragmention is a severe issue because memory blocks 
can end up uncoalessable in any thread's local arena. The proposed system ensures 
that all memory chunks are freed back to the allocators local list and thus can 
be coalesced. 

The second problem this addresses is lock contention. In the proposed model,
a given thread can be post fewer blocks to areas guarded by a number of a 
different locks as opposed to to high numbers of blocks to the an area guarded
by a single lock. 


2. Coalescing w/ Buddy System
Another optimization we should have made was coalescing in a buddy system. In
our current implementaiton, we coalesce unconditionally with either the node
before or after our newly inserted node. This means it is possible to coalesce 
the two middle nodes of a four node stack, leaving the end nodes in a stranded 
fragmentation. If we added another component to our block header dictating whether 
the a given node was the first or second half of a split, it could force chunks
to always coalesce such that no fragmentation occurs. 

## **EXTENSION UPDATE - Thursday, Nov 09, 2017**
The 2 day extension allowed us to continue developing some of the items we had
slated as future work in our original report. In particular, we were able to 
implement a version of the datacube strucutre called a "mailbox". We used this 
structure in parallel with our current version of the global arena. Our changes 
theoretically helped with memory fragmention between threads; this was seen 
in a difference with the timing of the list implemenations. The pre-mailbox 
execution time for the parallel list implmenation was 1.52 seconds while the 
post mailbox implementation ran in .22 seconds, a speedup of 6.9 (see graph 2 in
graph.png)! Unfortuantely our ivec implementation did not receive a similar 
performance boost. This likely has to do with the memory allocation and 
deallocation patterns of each implementation.  

This lead us to consider what other latency bottlenecks we could be facing. 
One area we thought to check was lock contention on the global cache system. 
However, after commenting out any accesses to the global arena, we saw no difference
in performance, leading us to believe our bottleneck lay elsewhere. 

3. Algorithmic complexity
It is possible that our algorithmic complexity is acting as a significant bottleneck
in our end to end latency. Back-of-the-napkin calculations showed certain functions
executing with n^2 complexity (although we do not know if these calcualtions were
done correctly). Given more time (and knowledge), we would revist our algorithm 
and perform a full complexity analysis to determine where our bottleneck was located.
