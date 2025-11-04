FILE1 = qsort_small.c
FILE2 = qsort_large.c

all: qsort_small qsort_large

qsort_small: qsort_small.c Makefile
	gcc -static-libgcc -g -O -pg qsort_small.c -o qsort_small -lm
qsort_large: qsort_large.c Makefile
	# gcc -static-libgcc -g -Ofast -march=native -ffast-math qsort_large.c -o qsort_large -lm
	gcc -static-libgcc qsort_large.c -o qsort_large -lm
qsort_large_arm: qsort_large.c Makefile
	# arm-linux-gnueabihf-gcc -static-libgcc -march=armv7 -g -Ofast  -ffast-math qsort_large.c -o qsort_large -lm
	arm-linux-gnu-gcc -static-libgcc -march=armv7 qsort_large.c -o qsort_large -lm

clean:
	rm -rf qsort_small qsort_large output*
