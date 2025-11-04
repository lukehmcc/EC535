#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define UNLIMIT
#define MAXARRAY                                                               \
  60000 /* this number, if too large, will cause a seg. fault!! */
#define BUFSZ (1000 * 1000) // size of output

struct my3DVertexStruct {
  int x, y, z;
  double distance;
  // // lukehmcc: should not use double for distance as it is unnecesary to
  // // compute exact vector lengths
  // uint64_t distance;
};

int compare(const void *elem1, const void *elem2) {
  /* D = [(x1 - x2)^2 + (y1 - y2)^2 + (z1 - z2)^2]^(1/2) */
  /* sort based on distances from the origin... */

  double distance1, distance2;

  distance1 = (*((struct my3DVertexStruct *)elem1)).distance;
  distance2 = (*((struct my3DVertexStruct *)elem2)).distance;
  return (distance1 > distance2) ? 1 : ((distance1 == distance2) ? 0 : -1);

  // // lukehmcc: remove branch, just subtract.
  // const struct my3DVertexStruct *a = elem1;
  // const struct my3DVertexStruct *b = elem2;
  //
  // // branch-free, overflow-safe, returns {-1,0,1}
  // return (a->distance > b->distance) - (a->distance < b->distance);
}

int main(int argc, char *argv[]) {
  // int j;
  // for (int j = 0; j < 100; j++) {
  struct my3DVertexStruct array[MAXARRAY];
  FILE *fp;
  int i, count = 0;
  int x, y, z;

  if (argc < 2) {
    fprintf(stderr, "Usage: qsort_large <file>\n");
    exit(-1);
  } else {
    fp = fopen(argv[1], "r");

    while ((fscanf(fp, "%d", &x) == 1) && (fscanf(fp, "%d", &y) == 1) &&
           (fscanf(fp, "%d", &z) == 1) && (count < MAXARRAY)) {
      array[count].x = x;
      array[count].y = y;
      array[count].z = z;
      array[count].distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
      // // lukehmcc: part of the edit to the struct, don't need to sqrt every
      // // time and so they can be just an int
      // array[count].distance = x * x + y * y + z * z;
      count++;
    }
  }
  printf("\nSorting %d vectors based on distance from the origin.\n\n", count);
  qsort(array, count, sizeof(struct my3DVertexStruct), compare);

  // for (i = 0; i < count; i++)
  //   printf("%d %d %d\n", array[i].x, array[i].y, array[i].z);

  // lukehmcc The biggest optimization. Buffer the output before printing
  // it to reduce write times.
  char buf[BUFSZ];
  size_t used = 0;

  for (size_t i = 0; i < count; ++i) {
    int n = snprintf(buf + used, BUFSZ - used, "%d %d %d\n", array[i].x,
                     array[i].y, array[i].z);
    if ((size_t)n >= BUFSZ - used) { /* would overflow */
      fwrite(buf, 1, used, stdout);  /* flush */
      used = 0;                      /* reset */
      n = snprintf(buf, BUFSZ, "%d %d %d\n", array[i].x, array[i].y,
                   array[i].z);
    }
    used += n;
  }
  fwrite(buf, 1, used, stdout); /* final flush */
  // }
  return 0;
}
