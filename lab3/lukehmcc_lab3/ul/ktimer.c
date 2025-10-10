#define _GNU_SOURCE
#define BUF_LEN 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// define communication struct
struct comStruct {
  // 0: -s
  // 1: -r 
  // 2: -m
  int command; 
  int seconds;
  int pid;
  int count; // for changing the count from 1 to 2
  char msg[BUF_LEN];
};

static void sighandler(int sig);
void printManPage();

// message top level
// this will only be useful once multiple timers have 
// been implemented
char *txt;

int main(int argc, char **argv) {
	int pFile, procFile, oflags;
	struct sigaction action;

	// Opens to device file
	pFile = open("/dev/mytimer", O_RDWR);
	if (pFile < 0) {
		fprintf (stderr, "mytimer module isn't loaded\n");
		return 1;
	}
	procFile = open("/proc/mytimer", O_RDWR);
	if (pFile < 0) {
		fprintf (stderr, "mytimer proc isn't loaded\n");
		return 1;
	}

	// Setup signal handler
	memset(&action, 0, sizeof(action));
	action.sa_handler = sighandler;
	action.sa_flags = SA_SIGINFO;
	sigemptyset(&action.sa_mask);
	sigaction(SIGIO, &action, NULL);
	fcntl(pFile, F_SETOWN, getpid());
	oflags = fcntl(pFile, F_GETFL);

    // Check if listing
  if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    char buf[512];
    ssize_t n;
    while ((n = read(pFile, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, n);
  }
  
  // Check if setting timer
  else if (argc == 4 && strcmp(argv[1], "-s") == 0) {
    int pid = getpid();
    struct comStruct req = {
      .command = 0,
      .seconds = atoi(argv[2]),
      .pid = pid,
    };
    strncpy(req.msg, argv[3], BUF_LEN); // copy to struct
    txt = argv[3]; // and copy locally

	 ssize_t n = write(pFile, &req, sizeof(req));
   // TODO fix this
    if (n < 0 && errno == ENOSPC) {
      fprintf(stderr, "No free timer slots\n");
      exit(1);
    } else if (n < 0 && errno == EBUSY){
      printf("The timer %s was updated!\n", txt);
      exit(0);
    }
    fcntl(pFile, F_SETFL, oflags | FASYNC); // run this here to register
    pause();
    printf("%s\n", txt);
    // Printing is actually handled by the sighandler so this really just
    // freezes the userspace applicaiton and waits.
  }

  // check if deleting that timer
  else if (argc == 2 && strcmp(argv[1], "-r") == 0) {
    struct comStruct req = {
      .command = 1,
    };

    write(pFile, &req, sizeof(req));
    // Printing is actually handled by the sighandler so this really just
    // freezes the userspace applicaiton and waits.
  }

  // check if changing count
  else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
    struct comStruct req = {
      .command = 2,
      .count = atoi(argv[2]),
    };

    ssize_t n = write(pFile, &req, sizeof(req));
    if (n < 0 && errno == EINVAL){
      printf("Incorrect count number. Accepted range is 1-2.\n");
      exit(1);
    }
  }

  // Otherwise invalid
  else {
    printManPage();
  }

	close(pFile);
  close(procFile);
	return 0;
	
}

// print the manpage
void printManPage() {
  printf("Error: invalid use.\n");
  printf(" ktimer [-flag] [message]\n");
  printf(" -l: list out current timers\n");
  printf(" -r: remove all current timers\n");
  printf(" -s [SEC] \"[MSG]\": add timer with message\n");
  printf(" -m [COUNT]\": change maximum concurrency\n");
}


// SIGIO handler
static void sighandler(int sig) {
}
