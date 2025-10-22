# The vars
SRCS := main.c
TARGET := myISS

# Add the phony to keep overlapping files from breaking build
.PHONY: build run clean

# The targets (build is default)
build: $(SRCS)
	gcc -std=c11 -Wall -Wextra -O2 $(SRCS) -o $(TARGET)

run: build
	./$(TARGET) ./test.assembly

profile: $(SRCS)
	gcc -std=c11 -pg -Wall -Wextra $(SRCS) -o $(TARGET).profile
	./$(TARGET).profile ./test.assembly
	gprof -p $(TARGET).profile gmon.out

clean:
	rm -f $(TARGET) $(TARGET).profile
