// in-class exercise, EC535
// Submitted by Luke McCarthy (lukehmcc) and Roger Brown (rogerjb)

#include "stdio.h"

int two(int limit) { return limit; }

int one(int limit) {
  int i, a[50];

  for (i = limit - 50; i < limit; i++)
    a[i % 50] = i + two(i);
  return a[49];
}

int main() {
  int j, a;
  a = 0;
  for (j = 0; j < 1000; j++)
    a = a + one(j);
  printf("The result is %d\n", a);
  return 0;
}
