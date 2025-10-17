# How I optimized

## Pre-optimization

```bash
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

```bash
➜  HW2 git:(main) ✗ time make run
mkdir -p ./build
gcc -std=c11 -Wall -Wextra -O2 ./src/main.c -o ./build/myISS
././build/myISS ./test.assembly
Total number of executed instructions: 31069037
Total number of clock cycles: 49117535
Number of hits to local memory: 18035758
Total number of executed LD/ST instructions: 18036018
make run  3.71s user 0.04s system 99% cpu 3.770 total
```

## Post-optimization

```bash
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
 time   seconds   seconds    calls  Ts/call  Ts/call  name
100.00      0.19     0.19                             main
  0.00      0.19     0.00      169     0.00     0.00  compareInstructions
  0.00      0.19     0.00       78     0.00     0.00  toInt
  0.00      0.19     0.00        1     0.00     0.00  getInstructionsFromFile
```

```bash
➜  HW2 git:(main) ✗ time make run
mkdir -p ./build
gcc -std=c11 -Wall -Wextra -O2 ./src/main.c -o ./build/myISS
././build/myISS ./test.assembly
Total number of executed instructions: 31069037
Total number of clock cycles: 49117631
Number of hits to local memory: 18035756
Total number of executed LD/ST instructions: 18036018
make run  0.18s user 0.04s system 99% cpu 0.225 total
```

## Methodology

The main thing that takes a while was string comparison and conversion. So I
pre-baked them in a custom struct so only int comparisons had to be done. This
lead to a 17x speed up when running on my custom ~30 million instruction loop.
