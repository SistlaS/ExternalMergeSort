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
TEST_SIZES=(-2 0 1 4 19 91 252 921 1256 8889 40000 235875 500000 1000000)
for SIZE in "${TEST_SIZES[@]}"; do
    OUTPUT_FILE="$OUTPUT_DIR/test_case_n${SIZE}.txt"
    echo "Generating test case with -n $SIZE..."
    ./Test.exe -n "$SIZE" > "$OUTPUT_FILE"
    echo "Test case saved to $OUTPUT_FILE"
done

echo "All test cases generated in $OUTPUT_DIR."
