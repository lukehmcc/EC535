# EC535 
# Makefile for ktimer (userland program)

# Makefile adapted from http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC = arm-linux-gnueabihf-gcc
CFLAGS = -std=c99 -Wall -I. -static -O0
DEPS = 
OBJ = ktimer.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ktimer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	
clean:
	rm *.o fasync_tester
