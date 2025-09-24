// Luke McCarthy: lukehmcc@bu.edu
// Roger Brown: rogerjb@bu.edu
#include "bits.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
  // Create help message
  if (argc != 2) {
    printf("Error: Incorrect arguments provided.\nUsage: "
           "MyBitApp <32 bit>\n"
           "  - inputFile: Path to the input text file\n"
           "  - outputFile: Path to the output text file\n");
    return 0;
  }

  printf("Arg: %u\n", atoi(argv[1]));
  BinaryMirror(atoi(argv[1]));

  return 0;
}
