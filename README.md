# multithreading

In this project, you will engage (at a low level) with a range of synchronization problems. Part A of the project (this part!) deals with conflicting read-modify-write operations on single variables and complex data structures (an ordered linked list). It is broken up into multiple steps:

Part 1 - updates to a shared variable:
Write a multithreaded application (using pthreads) that performs parallel updates to a shared variable.
Demonstrate the race condition in the provided add routine, and address it with different synchronization mechanisms.
Do performance instrumentation and measurement.
Analyze and explain the observed performance.
Part 2 - updates to a shared complex data structure:
Implement the four routines described in SortedList.h: SortedList_insert, SortedList_delete, SortedList_lookup, SortedList_length.
Write a multi-threaded application, using pthread that performs, parallel updates to a sorted doubly linked list data structure (using methods from the above step).
Recognize and demonstrate the race conditions when performing linked list operations, and address them with different synchronization mechanisms.
Do performance instrumentation and measurement.
Analyze and explain the observed performance.
Viewed from a skills (rather than problem domain) perspective, this project focuses less on programming, and more on performance measurement and analysis.
