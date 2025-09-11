#include "bits.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[]) {
  // Create help message
  if (argc != 3) {
    printf("Error: Incorrect arguments provided.\nUsage: "
           "MyBitApp <inputFile> <outputFile>\n"
           "  - inputFile: Path to the input text file\n"
           "  - outputFile: Path to the output text file\n");
  }
  // Set up the files
  FILE *inFile = fopen(argv[1], "r");
  if (inFile == NULL) {
    printf("Error: Failed to open inFile: %s\n", argv[1]);
    return 1;
  }
  FILE *outFile = fopen(argv[2], "w");
  if (outFile == NULL) {
    printf("Error: Failed to open outFile: %s\n", argv[2]);
    return 1;
  }
  // There are only 14 ints, were just going to assume it's less than 100
  int integers[100];

  // Loop through and grab the values
  int i = 0;
  int num;
  while (fscanf(inFile, "%d", &num) > 0) {
    integers[i] = num;
    i++;
  }
  fclose(inFile);

  // We now know how long it is so assign two lists to handle the new vals
  int lengthOfInput = i;
  int mirrors[lengthOfInput];
  int countedSequences[lengthOfInput];

  for (int i = 0; i < lengthOfInput; i++) {
    mirrors[i] = BinaryMirror(integers[i]);
    countedSequences[i] = CountSequence(integers[i]);
  }

  // write out those values
  for (int i = 0; i < lengthOfInput; i++) {
    fprintf(outFile, "%u  > %u\n", mirrors[i], countedSequences[i]);
  }

  fclose(outFile);
  return 0;
}
