###External-Merge-Sort#####

Objective: 

To simulate a three-tier memory model(Cache, RAM, and Disk), and to implement and demonstrate the working of an optimized external merge sort algorithm on a database.

Implemented as part of the CS764 course - Topics in Database Systems, in the Fall 2024 semester at UW-Madison.

How to Run:

On a Linux terminal:


0) `cd` into the project folder. Run the command `make clean` to remove any intermediate/generated files

1) Build the project using the command `make`

2) Run the program using the command `./Test.exe`


Optimizations Implemented:

1) Tournament Trees:

We use the Tree of Losers to minimize the number of root-to-leaf passes required. Implemented in the file Tree.cpp, Line XYZ

2) Offset-Value Coding

We minimize the number of comparisons between keys in a record by computing each record's OVC, and comparing the OVCs. Ensures minimum number of row comparisons and minimum number of column comparisons.

 Implemented in the file Tree.cpp, Line XYZ

3) Graceful Degradation

To ensure that minimum number of records are spilled to the next memory level. Hence, we get a smooth(linearly increasing) cost function, when number of records to be sorted is greater than the size of memory. Graceful degradation has been implemented in two scenarios:

3.1  Internal -> External : 

spillBufferToDisk explain

Implemented in Sort.cpp, Line XYZ

3.2 1-Way -> N-Way Merge :

Use the formula [f1 = ((W-2)%(F-1)+2)] to determine the optimal number of runs to merge in a single step, to minimize the number of times we bring a record to memory from disk. 

Implemented in Sort.cpp, Line XYZ

4) Cache-Size Runs

Optimize cache usage by merging cache-size miniruns, using cache-level tournament trees


Members:

1) Amith Bhat Nekkare
   Campus ID: 9087340890

2) Soumya Sistla
   Campus ID: 

