#include "mylist.h"
#include <stdlib.h>
#include <string.h>
MyList *create_MyList_node(unsigned int input, unsigned int mirror, int counted,
                           char *binary, char *ASCII, MyList *next) {
  MyList *head = (MyList *)malloc(sizeof(MyList));
  head->input = input;
  head->mirror = mirror;
  head->counted = counted;
  head->binary = binary;
  head->ASCII = ASCII;
  head->next = next;
  return head;
}

MyList *insert_MyList_sorted(MyList *head, MyList *node) {
  // protect against bad nodes
  if (node == NULL)
    return head;

  if (head == NULL) {
    node->next = NULL;
    return node;
  }
  // if it already is the smallest, return it
  if (strcmp(node->ASCII, head->ASCII) <= 0) {
    node->next = head;
    return node;
  }
  MyList *current = head;
  while (current->next && strcmp(node->ASCII, current->next->ASCII) > 0) {
    current = current->next;
  }
  node->next = current->next;
  current->next = node;
  return head;
}
