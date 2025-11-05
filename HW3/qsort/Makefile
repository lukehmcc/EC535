FILE1 = qsort_small.c
FILE2 = qsort_large.c

# Detect cross compiler once
ifeq ($(shell which arm-linux-gnueabihf-gcc >/dev/null 2>&1; echo $$?),0)
    CROSS := arm-linux-gnueabihf-
else ifeq ($(shell which arm-linux-gnu-gcc >/dev/null 2>&1; echo $$?),0)
    CROSS := arm-linux-gnu-
else
    $(error No compatible ARM cross-compiler found)
endif

all: qsort_small qsort_large
arm: qsort_arm

qsort_small: qsort_small.c
	gcc -static-libgcc -g -O -pg $< -o $@ -lm

qsort_large: qsort_large.c
	gcc -static-libgcc $< -o $@ -lm

qsort_arm: qsort_large.c
	$(CROSS)gcc -static-libgcc -march=armv7 -Ofast -ffast-math $< -o qsort_large -lm
	# $(CROSS)gcc -static-libgcc $< -o qsort_large -lm

clean:
	rm -rf qsort_small qsort_large output*

