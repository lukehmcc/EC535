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
- -static-libgcc: 1.97s
- -march=native: 2.25s
- -funroll-loops: 2.00s
- -ffast-math: 2.00s

So the full `-static -g -Ofast -march=native -pg -ffast-math`: 1.78s
Without -pg `-static -g -Ofast -march=native -ffast-math`: 1.56s

Compiler flags really aren't going to make this any faster. Like marginally maybe,
but not by much.

### 5 - My speed ups

#### Compiler Speed Up

Compiler flags. Going from the default (-O) command is about 1.61s, then with the
flags it brings it down to 1.56s.

#### Code Speed Up

There are actually multiple in code changes:

1. Distance is now stored in an int and the square root is not computed.
Computing the square root isn't really necessary because all it checks is > <
or ==, all things that don't rely on exact numbers, just magnitude. This
doesn't have a particularly large runtime effect due to the majority of
computation happening in the compare function, but it does exist.

2. Instead of a ternary operator (which does a jump), the compare function now
does just two comparisons and subtracts to return a ternary output `{-1,0,1]`.
This reduces jumps and speeds things up. The compiler actually works around
this normally so it only effects it when compiler optimizations aren't enabled.

3. Instead of printing out every line of output to `stdout`, buffer in 1MB
chunks and then dump them all at once. This has the biggest savings of what
I've worked on so far.

-------------------------------------------------------------------

Profiling Results

-------------------------------------------------------------------

qsort_large spends ~70% of its time executing the function compare() based
on the flat profile.

-------------------------------------------------------------------

Optimization

-------------------------------------------------------------------

+-----------------------+--------------------+--------------------+
|Optimizations          |Time on eng-grid    |Time on BeagleBone  |
+-----------------------+--------------------+--------------------+
|Default                | 0m3.523s           | 0m 0.71s           |
+-----------------------+--------------------+--------------------+
|myOptimization1        | 0m3.317s           | 0m 0.48s           |
+-----------------------+--------------------+--------------------+
|myOptimization2        | 0m3.469s           | 0m 0.67s           |
+-----------------------+--------------------+--------------------+
|Optional 1 & 2 combined| 0m3.047s           | 0m 0.46s           |
+-----------------------+--------------------+--------------------+

> Note: The eng-grid results were run on a 100x loop to reduce the dramatic
> inconsistencies that are present on those machines. It was run on a normal
> loop on the Bone.

myOptimization1: Compiler flags made the biggest difference in this exercise.
The codebase is not that complicated, so letting the compiler do the big
optimizations made the biggest difference. -Ofast made the biggest difference of
all of the flags (though it was only a tiny improvement over -O). Also it seems
that my code optimizations were significantly more potent when combined with
compiler optimizations.

myOptimization2: Instead of printing out every line of output to `stdout` (which
is rather slow), buffer in 1MB chunks and then dump them all at once. This has
the biggest savings of what I've worked on so far.

myOptimization3+: I made two other optimizations that I wanted to talk about
here. First, I removed all decimals and square roots. Since all we care about is
< = >, you don't actually have to square root the vectors. You can just store
the powers (x^2 + y^2 + z^2) and compare those directly. Second, I removed the
ternary return operator and just did a subtraction. This removes a branch from
the compare function which speeds things up. These both had smaller time
speedups over stock so I didn't rank them highe .

Difference between grid and embedded: The time to run was about 10x higher on the
BeagleBone. I didn't include results here, and while there was variation, each
unoptimized run on the grid took about .07s while the Bone took .7s. That's a
lot faster! On the other hand though, the Bone is much more consistent as there
isn't anything competing for resources.
