#include <stdio.h>
#include <string.h>

/******************************************************
  Prepared by Matthew Yee

  Usage:
    ./ktimer [flag] [message]

        -r (read)
        -w (write)

  Examples:
        ./ktimer -r
                Print whatever message that the ktimer module is holding

        ./ktimer -w ThisIsAMessage
                Write the string "ThisIsAMessage" to the ktimer module

******************************************************/

void printManPage(void);

int main(int argc, char **argv) {
  char line[256];
  int ii, count = 0;

  /* Check to see if the mytimer successfully has mknod run
     Assumes that ktimer is tied to /dev/ktimer */
  FILE *pFile;
  pFile = fopen("/dev/mytimer", "r+");
  if (pFile == NULL) {
    fputs("mytimer module isn't loaded\n", stderr);
    return -1;
  }

  // Check if in read mode
  if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    while (fgets(line, 256, pFile) != NULL) {
      printf("%s\n", line);
    }
  }

  // Check if in write mode
  else if (argc == 4 && strcmp(argv[1], "-s") == 0) {
    fputs(argv[3], pFile);
  }

  // Otherwise invalid
  else {
    printManPage();
  }

  fclose(pFile);
  return 0;
}

void printManPage() {
  printf("Error: invalid use.\n");
  printf(" ktimer [-flag] [message]\n");
  printf(" -l: list out current timers\n");
  printf(" -s [SEC] \"[MSG]\": add timer with message\n");
  printf(" -m [COUNT]: set max timers\n");
}
