### Old small.c

➜  class11 git:(main) ✗ gcc -g -p small.c
➜  class11 git:(main) ✗ ./a.out
The result is 948100
➜  class11 git:(main) ✗ gprof a.out
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  us/call  us/call  name
100.00      0.26     0.26   499500     0.52     0.52  two
  0.00      0.26     0.00     1000     0.00   260.00  one

 %         the percentage of the total running time of the
time       program used by this function.

cumulative a running sum of the number of seconds accounted
 seconds   for by this function and those listed above it.

 self      the number of seconds accounted for by this
seconds    function alone.  This is the major sort for this
           listing.

calls      the number of times this function was invoked, if
           this function is profiled, else blank.

 self      the average number of milliseconds spent in this
ms/call    function per call, if this function is profiled,
           else blank.

 total     the average number of milliseconds spent in this
ms/call    function and its descendents per call, if this
           function is profiled, else blank.

name       the name of the function.  This is the minor sort
           for this listing. The index shows the location of
           the function in the gprof listing. If the index is
           in parenthesis it shows where it would appear in
           the gprof listing if it were to be printed.

Copyright (C) 2012-2025 Free Software Foundation, Inc.

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.

                     Call graph (explanation follows)

granularity: each sample hit covers 4 byte(s) for 3.85% of 0.26 seconds

index % time    self  children    called     name
                0.26    0.00  499500/499500      one [2]
[1]    100.0    0.26    0.00  499500         two [1]
-----------------------------------------------

                0.00    0.26    1000/1000        main [3]

[2]    100.0    0.00    0.26    1000         one [2]
                0.26    0.00  499500/499500      two [1]
-----------------------------------------------

                                                 <spontaneous>

[3]    100.0    0.00    0.26                 main [3]
                0.00    0.26    1000/1000        one [2]
-----------------------------------------------

### New small.c

➜  class11 git:(main) ✗ gcc -g -p small_new.c
➜  class11 git:(main) ✗ ./a.out
The result is 948100
➜  class11 git:(main) ✗ gprof a.out
Flat profile:

Each sample counts as 0.01 seconds.
 no time accumulated

  %   cumulative   self              self     total
 time   seconds   seconds    calls  Ts/call  Ts/call  name
  0.00      0.00     0.00    48775     0.00     0.00  two
  0.00      0.00     0.00     1000     0.00     0.00  one

 %         the percentage of the total running time of the
time       program used by this function.

cumulative a running sum of the number of seconds accounted
 seconds   for by this function and those listed above it.

 self      the number of seconds accounted for by this
seconds    function alone.  This is the major sort for this
           listing.

calls      the number of times this function was invoked, if
           this function is profiled, else blank.

 self      the average number of milliseconds spent in this
ms/call    function per call, if this function is profiled,
           else blank.

 total     the average number of milliseconds spent in this
ms/call    function and its descendents per call, if this
           function is profiled, else blank.

name       the name of the function.  This is the minor sort
           for this listing. The index shows the location of
           the function in the gprof listing. If the index is
           in parenthesis it shows where it would appear in
           the gprof listing if it were to be printed.

Copyright (C) 2012-2025 Free Software Foundation, Inc.

Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.

                     Call graph (explanation follows)

granularity: each sample hit covers 4 byte(s) no time propagated

index % time    self  children    called     name
                0.00    0.00   48775/48775       one [2]
[1]      0.0    0.00    0.00   48775         two [1]
-----------------------------------------------

                0.00    0.00    1000/1000        main [8]

[2]      0.0    0.00    0.00    1000         one [2]
                0.00    0.00   48775/48775       two [1]
-----------------------------------------------
