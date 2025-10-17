# the vars
BUILD_DIR := ./build
SRC_DIR := ./src

# The generated vars
SRCS := $(wildcard $(SRC_DIR)/*.c)
TARGET := $(BUILD_DIR)/myISS

# Add the phony to keep overlapping files from breaking build
.PHONY: build run clean

# The targets (build is default)
build:  $(SRCS) 
	mkdir -p $(BUILD_DIR)
	gcc -std=c11 -Wall -Wextra -O2 $(SRCS) -o $(TARGET)

profile: $(SRCS)
	mkdir -p $(BUILD_DIR)
	gcc -std=c11 -pg -Wall -Wextra $(SRCS) -o $(TARGET).profile
	./$(TARGET).profile ./test.assembly
	gprof -p $(TARGET).profile gmon.out

clean:
	rm -rf ${BUILD_DIR}
