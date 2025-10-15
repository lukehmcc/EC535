#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRING_LENGTH 256

char **getInstructionsFromFile(FILE *file, int *lineCount);

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;

  FILE *file = fopen(argv[1], "r");
  int i = 0;
  int lineCount = 0;
  if (!file)
    return 1;

  // get those instructions
  char **instructions = getInstructionsFromFile(file, &lineCount);

  // set up registers
  int reg[7];
  for (i = 1; i < 7; i++) {
    reg[i] = 0;
  }

  // set up flags
  int equal = 0;

  // set up counters
  int executedInstruction = 0;
  int clockCycles = 0;
  int totalMemoryHits = 0;
  int localMemoryHits = 0;

  // set up memory
  uint8_t memory[256];
  for (i = 0; i < 256; i++) {
    memory[i] = 0;
  }
  uint8_t touched[256];
  for (i = 0; i < 256; i++) {
    touched[i] = 0;
  }

  // Now move through instructions
  for (i = 0; i < lineCount; i++) {
    printf("Running Instruction: %d: %s\n", i, instructions[i]);
    executedInstruction++;
    // beep boop
    char inst[STRING_LENGTH], arg1[STRING_LENGTH], arg2[STRING_LENGTH];
    // for some commands [] are inserted around the register. Get rid of these
    // so we can use a single scan funciton
    char *instruction = instructions[i];
    while ((*instruction = *instruction) == '[' || *instruction == ']') {
      *instruction = ' '; /* turn brackets into spaces */
    }
    sscanf(instruction, "%s %[^,], %s", inst, arg1, arg2);
    // Now do heuristics
    if (strcmp(inst, "MOV") == 0) {
      int r1 = atoi(&arg1[1]);
      reg[r1] = atoi(arg2);
      clockCycles++;
    } else if (strcmp(inst, "ADD") == 0) {
      // There are two types of add, check which one is present
      if (arg2[0] == 'R') {
        // This one is a between register add
        int r1 = atoi(&arg1[1]);
        int r2 = atoi(&arg2[1]);
        reg[r1] = reg[r1] + reg[r2];
      } else {
        // This one is a add to register
        int r1 = atoi(&arg1[1]);
        reg[r1] += atoi(arg2);
      }
      clockCycles++;
    } else if (strcmp(inst, "CMP") == 0) {
      int r1 = atoi(&arg1[1]);
      int r2 = atoi(&arg2[1]);
      if (reg[r1] == reg[r2]) {
        equal = 1;
      } else {
        equal = 0;
      }
      clockCycles++;
    } else if (strcmp(inst, "JMP") == 0) {
      // jump to that spot
      int toJump = atoi(arg1);
      // you gotta subtract one because it'll iterate on the next go
      i = toJump - 1;
      clockCycles++;
    } else if (strcmp(inst, "JE") == 0 && equal) {
      // jump to that spot
      int toJump = atoi(arg1);
      // you gotta subtract one because it'll iterate on the next go
      i = toJump - 1;
      clockCycles++;
    } else if (strcmp(inst, "LD") == 0) {
      // loads from the address stored in Rm into Rn
      int rn = atoi(&arg1[1]);
      int rm = atoi(&arg2[1]);
      reg[rn] = memory[rm];
      clockCycles += 2;
      totalMemoryHits++;
    } else if (strcmp(inst, "ST") == 0) {
      // stores the contents of Rn into the memory address that is in Rm
      int rm = atoi(&arg1[1]);
      int rn = atoi(&arg2[1]);
      memory[rm] = reg[rn];
      // check if it has been touched before
      if (touched[rm] == 0) {
        touched[rm] = 1;
        localMemoryHits++;
        clockCycles += 50;
      } else {
        clockCycles += 2;
      }
      totalMemoryHits++;
    } else {
      printf("Unknown Instruction: %s\n", inst);
    }
  }

  // print regs:
  for (i = 1; i < 7; i++) {
    printf("R%d: %d, ", i, reg[i]);
  }
  printf("\n");

  printf("Total number of executed instructions: %d\nTotal number of clock "
         "cycles: %d\nNumber of hits to local memory: %d\nTotal number of "
         "executed LD/ST instructions: %d\n",
         executedInstruction, clockCycles, localMemoryHits, totalMemoryHits);

  // clean up
  fclose(file);
  for (int i = 0; i < lineCount; i++) {
    free(instructions[i]);
  }
  free(instructions);
  return 0;
}

char **getInstructionsFromFile(FILE *file, int *lineCount) {
  int i = 0;
  // read how many lines there are
  char buffer[STRING_LENGTH];
  while (fgets(buffer, sizeof(buffer), file)) {
    (*lineCount)++;
  }
  rewind(file);

  // then create an array and populate it based on that file
  char **instructions = malloc(*lineCount * sizeof(char *));
  for (i = 0; i < *lineCount; i++) {
    instructions[i] = malloc(STRING_LENGTH);
    if (!fgets(instructions[i], STRING_LENGTH, file)) {
      free(instructions[i]);
      break;
    }
    instructions[i][strcspn(instructions[i], "\n")] = '\0';
  }
  return instructions;
}
