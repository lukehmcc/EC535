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
  int reg[6];
  for (i = 0; i < 6; i++) {
    reg[i] = 0;
  }
  // set up flags
  int equal = 0;

  // Now move through instructions
  for (i = 0; i < lineCount; i++) {
    printf("Running Instruction: %d: %s\n", i, instructions[i]);
    // beep boop
    char inst[STRING_LENGTH], arg1[STRING_LENGTH], arg2[STRING_LENGTH];
    sscanf(instructions[i], "%s %[^,], %s", inst, arg1, arg2);
    // Now do heuristics
    if (strcmp(inst, "MOV") == 0) {
      int r1 = atoi(&arg1[1]);
      reg[r1] = atoi(arg2);
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
    } else if (strcmp(inst, "CMP") == 0) {
      int r1 = atoi(&arg1[1]);
      int r2 = atoi(&arg2[1]);
      if (reg[r1] == reg[r2]) {
        equal = 1;
      } else {
        equal = 0;
      }

    } else {
      printf("Unknown Instruction: %s\n", inst);
    }
  }

  // print regs:
  for (i = 0; i < 6; i++) {
    printf("R%d: %d, ", i, reg[i]);
  }
  printf("\n");

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
