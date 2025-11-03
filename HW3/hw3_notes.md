# HW3 Notes

## Performance Optimization

> Note: I will be putting the `qsort` algorithms in a 100x loop to make it slower.
> It's too fast right now.

### 1 - Using time

Before `-gO`:

```bash
➜  qsort git:(main) ✗ make clean && make
rm -rf qsort_small qsort_large output*
gcc -static qsort_small.c -o qsort_small -lm
gcc -static qsort_large.c -o qsort_large -lm
➜  qsort git:(main) ✗ time ./runme_small.sh
./runme_small.sh  0.17s user 0.01s system 99% cpu 0.183 total
➜  qsort git:(main) ✗ time ./runme_large.sh
./runme_large.sh  1.97s user 0.19s system 99% cpu 2.163 total
➜  qsort git:(main) ✗
```

After `-gO`:

```bash
➜  qsort git:(main) ✗ make clean && make
rm -rf qsort_small qsort_large output*
gcc -static -g -O qsort_small.c -o qsort_small -lm
gcc -static -g -O qsort_large.c -o qsort_large -lm
➜  qsort git:(main) ✗ time ./runme_small.sh
./runme_small.sh  0.15s user 0.01s system 99% cpu 0.158 total
➜  qsort git:(main) ✗ time ./runme_large.sh
./runme_large.sh  1.61s user 0.19s system 99% cpu 1.801 total
➜  qsort git:(main) ✗
```

Wow that is slightly faster! I tried other modes like -O2, -O3, and -Ofast which
got no performance gains.

### 2 - Now time for `gprof`

> Note: static linking makes the `gprof` output all weird. So I'll be disabling
static for this.

Large:

```bash
➜  qsort git:(main) ✗ gprof qsort_large
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  Ts/call  Ts/call  name
 70.83      0.17     0.17                             compare
 27.08      0.23     0.07                             main
  2.08      0.24     0.01                             frame_dummy
...
```

Small:

```bash
➜  qsort git:(main) ✗ gprof qsort_small
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  Ts/call  Ts/call  name
 79.17      0.19     0.19                             __do_global_dtors_aux
 16.67      0.23     0.04                             _fini
  4.17      0.24     0.01                             register_tm_clones
```

I'm just going to base this on the large because it's more helpful.

### 3 - Highest %

The vast majority of the computation is in compare. Lets optimize that.

### 4 - Different compiler flags

-static -march=native -g -O -pg
Let's make a list of the compiler flags

- None: 1.97s
- -pg: 2.19s
- -O: 1.61s
- -O2: 1.59s
- -O3: 1.59s
- -Ofast: 1.62s
- -static: 2.10s
- -march=native: 2.25s
- -funroll-loops: 2.00s
- -ffast-math: 2.00s

So the full `-static -g -Ofast -march=native -pg -ffast-math`: 1.78s
Without -pg `-static -g -Ofast -march=native -ffast-math`: 1.56s

Compiler flags really aren't going to make this any faster. Like marginally maybe,
but not by much.

### 5 - My two speed ups

#### Speed up 1

Compiler flags. Going from the default command is about 1.61s, then with the flags
it brings it down to 1.56s.

#### Speed up 2

Gotta fix the code or some shit.
