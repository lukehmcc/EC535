#ifndef MYLIST_H
#define MYLIST_H

typedef struct MyList {
  struct MyList *next;
  unsigned int input;
  unsigned int mirror;
  int counted; // coutned sequences
  char *binary;
  char *ASCII;
} MyList;

MyList *create_MyList_node(unsigned int input, unsigned int mirror, int counted,
                           char *binary, char *ASCII, MyList *next);

MyList *insert_MyList_sorted(MyList *head, MyList *node);

#endif // MYLIST_H
