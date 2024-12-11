###External-Merge-Sort#####

Objective: 

To simulate a three-tier memory model(Cache, RAM, and Disk), and to implement and demonstrate the working of an optimized external merge sort algorithm on a database of records.

Implemented as part of the CS764 course - Topics in Database Systems, in the Fall 2024 semester at UW-Madison.


Steps to Run:

A: Project:
On a Linux terminal:

0) `cd` into the project folder. Run the command `make clean` to remove any intermediate/generated files

1) Build the project using the command `make`

2) Run the program using the command `./Test.exe -n XYZ`, where XYZ indicates the number of records to be generated & sorted. Output can be seen in the terminal and the files `Disk.txt` and `Output.txt`

B: Tests:
On a Linux terminal:

0) `cd` into the project folder. Run the command `make clean` to remove any intermediate/generated files

1) Run the Bash script `test-suite.sh` by running the command `bash test-suite.sh`. Logs are stored in the `test_cases` sub-folder.


Final output is stored in Output.txt (after removing the OVCs).


********NOTE********
1. Our record is of length 4 with each column value in the range of [0,9] -- |a,b,c,d|. These fields are configurable in the config(present in Config class in defs.h).
2. The program works efficiently for input size  <= cache size and <= ram size by doing internal sort only and gracefully degrading to external sort once the input size is greater than ram size. 
3. Duplicates are handled correctly as shown in one of the test cases(Eg: for n = 40000, the data generating algo is bound to generate duplicates as |a,b,c,d| with a/b/c/d in [0,9] has only 10,000 unique permutations).
4. Sorting on 5,00,000 records takes approximately 7917ms on a Mac machine with M3 chip.
5. Tests are described in detail in the Bash script `test-suite.sh`
6. All configurations are present in the Config class in defs.h
*********************



Features:



Scan: (in Scan.cpp)
1) Randomly generates records on demand on the basis of the configured column count and maximum column value. Line 45.


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

If the number of records exceeds RAM capacity, we store the excess records in the RAM buffer. If the buffer is full as well, we spill the smallest run to disk for optimal results. This is implemented using a min-heap priority queue (since the buffer is relatively small, its okay)

Implemented in Sort.cpp, Line 309,328

3.2 1-Way -> N-Way Merge :

Use the formula [f1 = ((W-2)%(F-1)+2)] to determine the optimal number of runs to merge in a single step, to minimize the number of times we bring a record to memory from disk. Used to merge runs at both the RAM level and the Disk level.

Implemented in Sort.cpp, Line 449

4) Cache-Size Runs

Optimize cache usage by merging cache-size miniruns, using cache-level tournament trees. Implemented in Sort.cpp, Line 258

5) Recycling the priority queue

We reuse the tournament tree for cache, RAM, and disk level sorting. Impemented in Sort.cpp, Line 283, 495

6) Internal & External Sort

Using the optimizations mentioned in points 1-5, we implement the internal and external merge sort algorithms and successfully sort records. 

Implemented in Sort.cpp, Line 168

7) Merge planning & Optimization:

Features such as graceful degradation(both int->ext, and 1-way to K-way) and offset-value coding have been implemented to optimize the merge steps and minimize the data loaded onto disk.

Members:

1) Campus ID: 9087340890

2) Campus ID: 9087207966

