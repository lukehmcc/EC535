#include "bits.h"
#include <stdio.h>
int main() {
  // Read in the ints to a list
  FILE *inFile = fopen("./input.txt", "r");
  if (inFile == NULL) {
    perror("Error opening inFile");
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

  // Then close it out by writing those values out to a outFile
  FILE *outFile = fopen("./output.txt", "w");
  if (outFile == NULL) {
    perror("Error opening outFile");
    return 1;
  }

  // write out those values
  for (int i = 0; i < lengthOfInput; i++) {
    fprintf(outFile, "%d  > %d\n", mirrors[i], countedSequences[i]);
  }

  fclose(outFile);
  return 0;
}
