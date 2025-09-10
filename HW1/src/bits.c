#include "bits.h"
#include <stdio.h>
#include <stdlib.h>

char *intToString(unsigned int n);
unsigned int stringToInt(const char *binary);

unsigned int BinaryMirror(unsigned int input) {
  // First convert number into binary string (32 bit)
  char *binary = intToString(input);
  // give em a flip
  char binaryFlipped[33];
  for (int i = 31; i >= 0; i--) {
    binaryFlipped[31 - i] = binary[i];
  }
  binaryFlipped[32] = '\0';
  // Convert back into an int
  unsigned int flipped = stringToInt(binaryFlipped);
  // return
  return flipped;
}
unsigned int CountSequence(unsigned int input) { // first
  return 1;
}

char *intToString(unsigned int n) {
  int c, k, i;
  char *binary = (char *)malloc(33 * sizeof(char));
  i = 0;

  for (c = 31; c >= 0; c--) {
    k = n >> c;

    if (k & 1) {
      binary[i] = '1';
    } else {
      binary[i] = '0';
    }
    i++;
  }

  binary[i] = '\0';

  return binary;
}

unsigned int stringToInt(const char *binary) {
  unsigned int result = 0;
  int i = 0;

  while (binary[i] == ' ' || binary[i] == '\t') {
    i++;
  }

  while (binary[i] != '\0') {
    result <<= 1;

    if (binary[i] == '1') {
      result |= 1;
    } else if (binary[i] != '0') {
      printf("Error: Invalid binary digit '%c' at position %d\n", binary[i], i);
      return 0;
    }

    i++;
  }

  return result;
}
