#!/bin/bash

# Directory containing test executables (change if needed)
TEST_DIR="./cmake-build-debug/bin"

# Check if the directory exists
if [[ ! -d "$TEST_DIR" ]]; then
  echo "Error: Directory '$TEST_DIR' does not exist."
  exit 1
fi

# Find all executable files matching pattern *_test* and run them
find "$TEST_DIR" -type f -executable -name "*_test*" | while read -r test_file; do
  echo "Running: $test_file"
  "$test_file"
  echo "Exit code: $?"
  echo "----------------------------------------"
done
