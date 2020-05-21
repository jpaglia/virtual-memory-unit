#!/bin/bash -e

# Virtual Memory Unit Compilation and Execution Script
# Author: Julia Paglia

echo "Compiling"
gcc mmu.c -o mmu
echo "Running, printing stats:"
echo "------------------------"
./mmu BACKING_STORE.bin addresses.txt
echo "------------------------"
#./mmu BACKING_STORE.bin addresses.txt > out.txt
#echo "Comparing"
#diff out.txt correct.txt
echo "Complete, check output.csv for output columns"
