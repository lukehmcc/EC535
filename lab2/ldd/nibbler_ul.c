#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************
  Prepared by Matthew Yee
  
  Usage:
    ./ktimer_ul [flag] [message]
	 
	-r (read)
	-w (write)
	
  Examples:
	./ktimer_ul -r
		Print whatever message that the ktimer module is holding

	./ktimer_ul -w ThisIsAMessage
		Write the string "ThisIsAMessage" to the ktimer module
	
******************************************************/

void printManPage(void);

int main(int argc, char **argv) {
	char line[256];
	int ii, count = 0;
	
	/* Check to see if the ktimer successfully has mknod run
	   Assumes that ktimer is tied to /dev/ktimer */
	FILE * pFile;
	pFile = fopen("/dev/ktimer", "r+");
	if (pFile==NULL) {
		fputs("ktimer module isn't loaded\n",stderr);
		return -1;
	}

	// Check if in read mode
	if (argc == 2 && strcmp(argv[1], "-r") == 0) {
		while (fgets(line, 256, pFile) != NULL) {
			printf("%s\n", line);
		}
	}

	// Check if in write mode
	else if (argc == 3 && strcmp(argv[1], "-w") == 0) {
		fputs(argv[2], pFile);
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
	printf(" ktimer_ul [-flag] [message]\n");
	printf(" -r: read from the ktimer module\n");	
	printf(" -w: write [message] to the ktimer module\n");
}
