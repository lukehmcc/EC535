#include "bits.h"
#include <stdio.h>
#include <stdlib.h>

char *UIntToBinary(unsigned int n);
unsigned int BinaryToUInt(const char *binary);

unsigned int BinaryMirror(unsigned int input) {
  // First convert number into binary string (32 bit)
  char *binary = UIntToBinary(input);
  // give em a flip
  char binaryFlipped[33];
  for (int i = 31; i >= 0; i--) {
    binaryFlipped[31 - i] = binary[i];
  }
  binaryFlipped[32] = '\0';
  // Convert back into an int
  unsigned int flipped = BinaryToUInt(binaryFlipped);
  // return
  return flipped;
}
unsigned int CountSequence(unsigned int input) { // first
  char *binary = UIntToBinary(input);
  int count = 0;
  for (int i = 0; binary[i + 2] != '\0'; i++) {
    if (binary[i] == '0' && binary[i + 1] == '1' && binary[i + 2] == '0')
      count++;
  }
  return count;
}

char *UIntToBinary(unsigned int n) {
  // LITTLE endian binary representation
  char *constructor = (char *)malloc(33 * sizeof(char));
  unsigned int toDeconstruct = n;
  for (int i = 31; i >= 0; i--) {
    if (toDeconstruct >= (1U << i)) {
      toDeconstruct -= (1U << i);
      constructor[31 - i] = '1';
    } else {
      constructor[31 - i] = '0';
    }
  }
  constructor[32] = '\0';

  return constructor;
}

unsigned int BinaryToUInt(const char *binary) {
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

char *UIntToAscii(unsigned int n) {
  char *buf = (char *)malloc(11 * sizeof(char));
  if (!buf)
    return NULL;
  sprintf(buf, "0x%08X", n); // use 8 digits because that can store 32 bits
  return buf;
}
