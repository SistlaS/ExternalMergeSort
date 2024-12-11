#!/bin/bash

# Check if Test.exe exists
if [ ! -f "./Test.exe" ]; then
    echo "Error: Test.exe not found. Compile it first using 'make'."
    exit 1
fi

# Create output directory for test results
OUTPUT_DIR="test_cases"
mkdir -p "$OUTPUT_DIR"

# Run Test.exe with different test sizes
TEST_SIZES=(-2 0 1 4 19 91 252 921 1256 8889 40000 235875 400000 500000 1000000)
for SIZE in "${TEST_SIZES[@]}"; do
    if [ "$SIZE" -eq 40000 ]; then
        OUTPUT_FILE="$OUTPUT_DIR/test_case_with_duplicate_records_n${SIZE}.txt"
    else
        OUTPUT_FILE="$OUTPUT_DIR/test_case_n${SIZE}.txt"
    fi
    echo "Generating test case with -n $SIZE..."
    ./Test.exe -n "$SIZE" > "$OUTPUT_FILE"
    echo "Test case saved to $OUTPUT_FILE"
done

echo "All test cases generated in $OUTPUT_DIR."

# README
# Test cases have been generated to cover various conditions, edge cases, and large input sizes.

# A. Edge case: numRecords =  -2 (Negative number) : Invalid input, so fails 
# B. Edge case: numRecords = 0 : No records, nothing to generate

# C. numRecords = 1 - 10,00,000 : Sort works for different input sizes

# D. Duplicate Handling -- Given that our maximum column value is 9, and column count is 4, the maximum number of unique records = 10^4 = 10000.
# Hence, any testcase with greater than 10000 records will contain duplicates. Hence this scenario is tested as well.

# E. Sorted(Asc/Desc) Input -- Scenario is tested and validated to work correctly. Implemented in Line 46 @ Scan.cpp (Limitation, handles upto 1,00,000 records only)