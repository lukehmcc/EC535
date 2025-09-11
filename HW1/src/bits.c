#include "bits.h"
#include <math.h>
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
  char *binary = intToString(input);
  int count = 0;
  for (int i = 0; binary[i + 2] != '\0'; i++) {
    if (binary[i] == '0' && binary[i + 1] == '1' && binary[i + 2] == '0')
      count++;
  }
  return count;
}

char *intToString(unsigned int n) {
  // LITTLE endian binary representation
  char *constructor = (char *)malloc(33 * sizeof(char));
  unsigned int toDeconstruct = n;
  for (int i = 31; i >= 0; i--) {
    if (toDeconstruct >= pow(2, i)) {
      toDeconstruct -= pow(2, i);
      constructor[31 - i] = '1';
    } else {
      constructor[31 - i] = '0';
    }
  }
  constructor[32] = '\0';

  return constructor;
}

unsigned int stringToInt(const char *binary) {
  unsigned int result = 0;
  // just walk through and reconstruct the number
  for (int i = 31; i >= 0; i--) {
    char current = binary[31 - i];
    if (current == '1') {
      result += (1U << i);
    } else {
      // do nothing
    }
  }

  return result;
}
