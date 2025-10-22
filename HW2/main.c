#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STRING_LENGTH 256

// Instructions:
// 0: MOV Rn, <num>
// 1: ADD Rn, Rm
// 2: ADD Rn, <num>
// 3: CMP Rn, Rm
// 4: JE  <addr>
// 5: JMP <addr>
// 6: LD  Rn, [Rm]
// 7: ST  [Rm], Rn
// 8: Invalid Instruction
typedef struct {
  uint8_t instruction;
  int arg1;
  int arg2;
} Instruction;
static const char *mnemonic[] = {"MOV", "ADD", "ADD", "CMP",
                                 "JE",  "JMP", "LD",  "ST"};

void printInstruction(const Instruction *inst);
Instruction *getInstructionsFromFile(FILE *file, int *lineCount,
                                     int *firstLine);
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
  Instruction *instructions =
      getInstructionsFromFile(file, &lineCount, &firstLine);

  // set up registers
  int8_t reg[7];
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
    Instruction inst = instructions[i];
    // printf("%d: , instruction: %d ", i, inst.instruction);
    // printInstruction(&inst);
    // Now do heuristics
    // 0: MOV Rn, <num>
    if (inst.instruction == 0) {
      reg[inst.arg1] = inst.arg2;
      clockCycles++;
      // 1: ADD Rn, Rm
    } else if (inst.instruction == 1) {
      reg[inst.arg1] = reg[inst.arg1] + reg[inst.arg2];
      clockCycles++;
      // 2: ADD Rn, <num>
    } else if (inst.instruction == 2) {
      reg[inst.arg1] += inst.arg2;
      // printf("adding %d to %d\n", inst.arg2, reg[inst.arg1]);
      clockCycles++;
      // 3: CMP Rn, Rm
    } else if (inst.instruction == 3) {
      if (reg[inst.arg1] == reg[inst.arg2]) {
        equal = 1;
      } else {
        equal = 0;
      }
      clockCycles++;
      // 4: JE  <addr>
    } else if (inst.instruction == 4) {
      if (equal) {
        // jump to that spot
        int toJump = inst.arg1;
        // you gotta subtract one because it'll iterate on the next go
        i = toJump - 1;
      }
      clockCycles++;
      // 5: JMP <addr>
    } else if (inst.instruction == 5) {
      // jump to that spot
      int toJump = inst.arg1;
      // you gotta subtract one because it'll iterate on the next go
      i = toJump - 1;
      clockCycles++;
      // 6: LD  Rn, [Rm]
    } else if (inst.instruction == 6) {
      uint8_t addr = reg[inst.arg2] & 0xFF;
      if (!touched[addr]) { // first ever reference
        touched[addr] = 1;
        clockCycles += 50;
      } else { // local hit
        localMemoryHits++;
        clockCycles += 2;
      }
      reg[inst.arg1] = memory[addr];
      totalMemoryHits++;

      // 7: ST  [Rm], Rn
    } else if (inst.instruction == 7) {
      uint8_t addr = reg[inst.arg1] & 0xFF;
      if (!touched[addr]) { // first ever reference
        touched[addr] = 1;
        clockCycles += 50;
      } else { // local hit
        localMemoryHits++;
        clockCycles += 2;
      }
      memory[addr] = reg[inst.arg2];
      totalMemoryHits++;
    } else {
      printf("Unknown Instruction: %d\n", inst.instruction);
    }
    // print regs:
    // for (j = 1; j < 7; j++) {
    //   printf("R%d: %d, ", j, reg[j]);
    // }
    // printf("\n");
    // for (j = 0; j < 256; j++) {
    //   printf("M%d: %d, ", j, memory[j]);
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
  free(instructions);
  return 0;
}

Instruction *getInstructionsFromFile(FILE *file, int *lineCount,
                                     int *firstLine) {
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
  Instruction *instructions =
      malloc((*lineCount + *firstLine) * sizeof(Instruction));
  for (i = *firstLine; i < *lineCount + *firstLine; i++) {
    fgets(buffer, sizeof(buffer), file);
    buffer[strcspn(buffer, "\n")] = '\0';
    int dummyLine;
    char inst[STRING_LENGTH], arg1[STRING_LENGTH], arg2[STRING_LENGTH];
    sscanf(buffer, "%d %s %[^,], %s", &dummyLine, inst, arg1, arg2);
    // convert the string instruction to a object that can be used later without
    // having to use strcmp or atoi (which are slow)
    // 0: MOV Rn, <num>
    if (compareInstructions(inst, "MOV")) {
      instructions[i].instruction = 0;
      instructions[i].arg1 = toInt(&arg1[1]);
      instructions[i].arg2 = toInt(arg2);
      // 1-2: ADD
    } else if (compareInstructions(inst, "ADD")) {
      // There are two types of add, check which one is present
      // 1: ADD Rn, Rm
      if (arg2[0] == 'R') {
        instructions[i].instruction = 1;
        instructions[i].arg1 = toInt(&arg1[1]);
        instructions[i].arg2 = toInt(&arg2[1]);
        // 2: ADD Rn, <num>
      } else {
        instructions[i].instruction = 2;
        instructions[i].arg1 = toInt(&arg1[1]);
        instructions[i].arg2 = toInt(arg2);
      }
      // 3: CMP Rn, Rm
    } else if (compareInstructions(inst, "CMP")) {
      instructions[i].instruction = 3;
      instructions[i].arg1 = toInt(&arg1[1]);
      instructions[i].arg2 = toInt(&arg2[1]);
      // 4: JE  <addr>
    } else if (compareInstructions(inst, "JE")) {
      // printf("line=%d  inst=%s  arg1=%s  arg2=%s\n", dummyLine, inst, arg1,
      //        arg2);
      instructions[i].instruction = 4;
      instructions[i].arg1 = toInt(arg1);
      // printf("instruction[%d] is now %d\n", i, instructions[i].instruction);
      // 5: JMP <addr>
    } else if (compareInstructions(inst, "JMP")) {
      // printf("line=%d  inst=%s  arg1=%s  arg2=%s\n", dummyLine, inst, arg1,
      //        arg2);
      instructions[i].instruction = 5;
      instructions[i].arg1 = toInt(arg1);
      // printf("instruction[%d] is now %d\n", i, instructions[i].instruction);
      // 6: LD  Rn, [Rm]
    } else if (compareInstructions(inst, "LD")) {
      instructions[i].instruction = 6;
      instructions[i].arg1 = toInt(&arg1[1]);
      instructions[i].arg2 = toInt(&arg2[2]);
      // 7: ST  [Rm], Rn
    } else if (compareInstructions(inst, "ST")) {
      instructions[i].instruction = 7;
      instructions[i].arg1 = toInt(&arg1[2]);
      instructions[i].arg2 = toInt(&arg2[1]);
    } else {
      instructions[i].instruction = 8;
    }
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
void printInstruction(const Instruction *inst) {
  uint8_t op = inst->instruction;
  if (op > 7) { /* catch the “Invalid Instruction” case */
    printf("Invalid Instruction\n");
    return;
  }

  switch (op) {
  case 0: /* MOV Rn, imm */
    printf("%s R%d, %d\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  case 1: /* ADD Rn, Rm */
    printf("%s R%d, R%d\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  case 2: /* ADD Rn, imm */
    printf("%s R%d, %d\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  case 3: /* CMP Rn, Rm */
    printf("%s R%d, R%d\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  case 4: /* JE addr */
    printf("%s %d\n", mnemonic[op], inst->arg1);
    break;
  case 5: /* JMP addr */
    printf("%s %d\n", mnemonic[op], inst->arg1);
    break;
  case 6: /* LD Rn, [Rm] */
    printf("%s R%d, [R%d]\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  case 7: /* ST [Rm], Rn */
    printf("%s [R%d], R%d\n", mnemonic[op], inst->arg1, inst->arg2);
    break;
  }
}
