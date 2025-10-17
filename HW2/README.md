# How I optimized

## Pre-optimization

```
➜  HW2 git:(main) ✗ make profile
mkdir -p ./build
gcc -std=c11 -pg -Wall -Wextra ./src/main.c -o ./build/myISS.profile
././build/myISS.profile ./test.assembly
Total number of executed instructions: 31069037
Total number of clock cycles: 50413727
Number of hits to local memory: 18008754
Total number of executed LD/ST instructions: 18036018
gprof -p ./build/myISS.profile gmon.out
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ps/call  ps/call  name
 39.71      0.14     0.14 149311157   904.15   904.15  compareInstructions
 38.24      0.27     0.13                             main
 16.18      0.32     0.06 59131072   930.14   930.14  toInt
  5.88      0.34     0.02                             _fini
  0.00      0.34     0.00        1     0.00     0.00  getInstructionsFromFile

```

## Post-optimization

Hello

## How I optimized

The main thing that takes a while was string comparison and conversion. So lets store them first
