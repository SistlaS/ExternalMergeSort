###External-Merge-Sort#####

Objective: 

To simulate a three-tier memory model(Cache, RAM, and Disk), and to implement and demonstrate the working of an optimized external merge sort algorithm on a database.

Implemented as part of the CS764 course - Topics in Database Systems, in the Fall 2024 semester at UW-Madison.


Steps to Run:

On a Linux terminal:

0) `cd` into the project folder. Run the command `make clean` to remove any intermediate/generated files

1) Build the project using the command `make`

2) Run the program using the command `./Test.exe -n XYZ`, where XYZ indicates the number of records to be generated & sorted.

Final output is stored in Output.txt (after removing the OVCs).


********NOTE********
1. Our record is of length 4 with each column value in the range of [0,9] -- |a,b,c,d|. These fields are configurable in the config.
2. The program works efficiently for input size  <= cache size and <= ram size by doing internal sort only and gracefully degrading to external sort once the input size is greater than ram size. 
3. Duplicates are preserved as shown in one of the test cases(Eg: for n = 40000, the data generating algo is bound to generate duplicates as |a,b,c,d| with a/b/c/d in [0,9] has only 10000 unique possible permutations).
*********************

Filter: (in Filter.cpp)
1) Implements a basic predicate of filtering out rows that have column value less than 2 : L45

Witness: (in Witness.cpp)
1) Checks for the number of rows : L101
2) Parity(XOR) of the data : L48
3) Number of inversions : L83

Optimizations Implemented:

1) Tournament Trees:

We use the Tree of Losers to minimize the number of root-to-leaf passes required. Implemented in the file Tree.cpp : L379, L290, L213

2) Offset-Value Coding

We minimize the number of comparisons between keys in a record by computing each record's OVC, and comparing the OVCs. Ensures minimum number of row comparisons and minimum number of column comparisons.

 Implemented in the file Tree.cpp:
 1. OVC comparisons : L112
 2. OVCs are written to the disk inorder to avoid recomputing the offsets in every merge step : L75

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

5) Cache-Size Runs

Optimize cache usage by merging cache-size miniruns, using cache-level tournament trees

Members:

1) Campus ID: 9087340890

2) Campus ID: 9087207966

