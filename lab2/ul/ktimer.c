#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
  int ii, count = 0, BUF_LEN = 256;

  /* Check to see if the mytimer successfully has mknod run
     Assumes that ktimer is tied to /dev/ktimer */
  FILE *pFile;
  pFile = fopen("/dev/mytimer", "r+");
  if (pFile == NULL) {
    fputs("mytimer module isn't loaded\n", stderr);
    return -1;
  }

  // Check if listing
  if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    while (fgets(line, 256, pFile) != NULL) {
      printf("%s\n", line);
    }
  }
  // check if setting max concurrent
  else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
    // we do the same thing as setting a timer, just send a string
    // of length 0 so the kernel mod knows that it isn't a timer
    int seconds = atoi(argv[2]);
    char pkt[BUF_LEN];
    int off = 0;

    *(int *)(pkt + off) = 0;
    off += sizeof(int);

    *(int *)(pkt + off) = seconds;
    off += sizeof(int);

    write(fileno(pFile), pkt, off);
  }
  // Check if setting timer
  else if (argc == 4 && strcmp(argv[1], "-s") == 0) {
    int seconds = atoi(argv[2]);
    char *txt = argv[3];
    char pkt[BUF_LEN];
    int off = 0;

    *(int *)(pkt + off) = strlen(txt);
    off += sizeof(int);

    *(int *)(pkt + off) = seconds;
    off += sizeof(int);

    memcpy(pkt + off, txt, strlen(txt));
    off += strlen(txt);

    write(fileno(pFile), pkt, off);
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
