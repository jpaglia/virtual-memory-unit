#!/bin/bash -e

# Virtual Memory Unit Compilation and Execution Script
# EECS 3221 Operating System Concepts
# Mini Project 3
# Author: Julia Paglia

# I verify that this work is my own. However, I will disclose that some conversations were had with students
# Amer Alshoghri and Daniel Santaguida about general troubleshooting and debugging, and some source code for
# the linked list LRU implementation was adapted from my Mini Project 2.

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
