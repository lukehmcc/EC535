# the vars
BUILD_DIR := ./build
SRC_DIR := ./src

# The generated vars
SRCS := $(wildcard $(SRC_DIR)/*.c)
TARGET := $(BUILD_DIR)/MyBitApp

# Add the phony to keep overlapping files from breaking build
.PHONY: build run clean

# The targets (build is default)
build:  $(SRCS) 
	mkdir -p $(BUILD_DIR)
	$(CXX) -std=c++17 $(SRCS) -o $(TARGET)

clean:
	rm -rf ${BUILD_DIR}
