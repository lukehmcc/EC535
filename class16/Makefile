# Compiler and flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11

# Header dependencies (add any additional headers here)
DEPS    := actors.h fifo.h

# Object files
OBJ     := actors.o fifo.o main.o

# Output executable name
TARGET  := class16

# --------------------------------------------------------------------
# Primary targets
# --------------------------------------------------------------------
.PHONY: all clean run
all: $(TARGET)          # default build

run: $(TARGET)          # build if necessary, then execute
	@echo "Running $(TARGET)…"
	./$(TARGET)

# --------------------------------------------------------------------
# Link step
# --------------------------------------------------------------------
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# --------------------------------------------------------------------
# Compile step (generic rule)
# --------------------------------------------------------------------
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# --------------------------------------------------------------------
# Clean up generated files
# --------------------------------------------------------------------
clean:
	rm -f $(OBJ) $(TARGET)
