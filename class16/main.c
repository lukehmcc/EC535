// Luke McCarthy: U80112684
// Jackson Clary: U15109211
#include "actors.h"
#include "fifo.h"

int main() {
  fifo_t input1;
  fifo_t input2;
  fifo_t output1;
  fifo_t output2;
  init_fifo(&input1);
  init_fifo(&input2);
  init_fifo(&output1);
  // create two initial tokens
  put_fifo(&input1, 0);
  put_fifo(&input2, 24);
  // iterate the system schedule 100 times
  for (unsigned i = 0; i < 10; i++) {
    actor_inc(&input1, &output1);
    actor_fork(&output1, &input1, &output2);
    actor_mul(&output2, &input2, &output1);
    actor_print(&output1);
  }
  // after this completes, the output1 queue
  // will contain a single token with value 100
  return 0;
}
