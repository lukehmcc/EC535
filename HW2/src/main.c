#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRING_LENGTH 256

char **getInstructionsFromFile(FILE *file, int *lineCount, int *firstLine);
size_t compareInstructions(char *str1, char *str2);
int toInt(char *str);

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 1;

  FILE *file = fopen(argv[1], "r");
  int i = 0;
  // int j = 0;
  int lineCount = 0;
  int firstLine = 0;
  if (!file)
    return 1;

  // get those instructions
  char **instructions = getInstructionsFromFile(file, &lineCount, &firstLine);

  // set up registers
  size_t reg[7];
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
  for (i = firstLine; i < lineCount + firstLine; i++) {
    executedInstruction++;
    // beep boop
    char inst[STRING_LENGTH], arg1[STRING_LENGTH], arg2[STRING_LENGTH];
    int line; // ignored line
    // printf("%s\n", instructions[i]);
    sscanf(instructions[i], "%d %s %[^,], %s", &line, inst, arg1, arg2);
    // Now do heuristics
    if (compareInstructions(inst, "MOV")) {
      int r1 = atoi(&arg1[1]);
      reg[r1] = atoi(arg2);
      clockCycles++;
    } else if (compareInstructions(inst, "ADD")) {
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
    } else if (compareInstructions(inst, "CMP")) {
      int r1 = atoi(&arg1[1]);
      int r2 = atoi(&arg2[1]);
      if (reg[r1] == reg[r2]) {
        equal = 1;
      } else {
        equal = 0;
      }
      clockCycles++;
    } else if (compareInstructions(inst, "JMP")) {
      // jump to that spot
      int toJump = atoi(arg1);
      // you gotta subtract one because it'll iterate on the next go
      i = toJump - 1;
      clockCycles++;
    } else if (compareInstructions(inst, "JE")) {
      if (equal) {
        // jump to that spot
        int toJump = atoi(arg1);
        // you gotta subtract one because it'll iterate on the next go
        i = toJump - 1;
      }
      clockCycles++;
    } else if (compareInstructions(inst, "LD")) {
      // loads from the address stored in Rm into Rn
      int rn = atoi(&arg1[1]);
      int rm = atoi(&arg2[2]);
      reg[rn] = memory[reg[rm]];
      clockCycles += 2;
      // the assumption is you HAVE to read from something that already exists
      // thus it is both a local memory & total memory hit
      localMemoryHits++;
      totalMemoryHits++;
    } else if (compareInstructions(inst, "ST")) {
      // stores the contents of Rn into the memory address that is in Rm
      int rm = atoi(&arg1[2]);
      int rn = atoi(&arg2[1]);
      // printf("STORING TO: %d\n", reg[rm]);
      memory[reg[rm]] = reg[rn];
      // check if it has been touched before
      if (touched[reg[rm]] == 0) {
        touched[reg[rm]] = 1;
        clockCycles += 50;
      } else {
        localMemoryHits++;
        clockCycles += 2;
      }
      totalMemoryHits++;
    } else {
      printf("Unknown Instruction: %s\n", inst);
    }
    // print regs:
    // for (j = 1; j < 7; j++) {
    //   printf("R%d: %d, ", j, reg[j]);
    // }
    // printf("\n");
  }

  // print regs:
  // for (i = 1; i < 7; i++) {
  //   printf("R%d: %d, ", i, reg[i]);
  // }
  // printf("\n");

  printf("Total number of executed instructions: %d\nTotal number of clock "
         "cycles: %d\nNumber of hits to local memory: %d\nTotal number of "
         "executed LD/ST instructions: %d\n",
         executedInstruction, clockCycles, localMemoryHits, totalMemoryHits);

  // clean up
  fclose(file);
  for (int i = firstLine; i < lineCount + firstLine; i++) {
    free(instructions[i]);
  }
  free(instructions);
  return 0;
}

char **getInstructionsFromFile(FILE *file, int *lineCount, int *firstLine) {
  int i = 0;
  // read how many lines there are
  char buffer[STRING_LENGTH];
  // get first line
  fgets(buffer, sizeof(buffer), file);
  sscanf(buffer, "%d ", firstLine);
  // printf("First line: %d\n", *firstLine);
  rewind(file);
  // now get line count
  while (fgets(buffer, sizeof(buffer), file)) {
    (*lineCount)++;
  }
  rewind(file);

  // then create an array and populate it based on that file
  char **instructions = malloc((*lineCount + *firstLine) * sizeof(char *));
  for (i = *firstLine; i < *lineCount + *firstLine; i++) {
    instructions[i] = malloc(STRING_LENGTH);
    if (!fgets(instructions[i], STRING_LENGTH, file)) {
      free(instructions[i]);
      break;
    }
    instructions[i][strcspn(instructions[i], "\n")] = '\0';
  }
  return instructions;
}

size_t compareInstructions(char *str1, char *str2) {
  if (strcmp(str1, str2) == 0) {
    return 1;
  } else {
    return 0;
  }
}

int toInt(char *str) { return atoi(str); }
