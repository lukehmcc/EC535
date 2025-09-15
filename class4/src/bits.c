#include "bits.h"
#include <stdio.h>

char *UIntToBinary(unsigned int n);
unsigned int BinaryToUInt(const char *binary);

unsigned int BinaryMirror(unsigned int input) {
  // convert to hex
  char buf[9] = {0}; /* 8 chars + terminating '\0' */
  snprintf(buf, sizeof buf, "%08x", input);
  // flip hex
  char bufflipped[9];
  for (int i = 7; i >= 0; i--) {
    bufflipped[7 - i] = buf[i];
  }
  bufflipped[8] = '\0';

  printf("hex: %s, hex flipped: %s\n", buf, bufflipped);

  // hex back to int
  return 0;
}
