/* Necessary includes for device drivers */
#include "linux/kern_levels.h"
#include "linux/timer.h"
#include <asm/system_misc.h> /* cli(), *_flags */
#include <asm/uaccess.h>     /* copy_from/to_user */
#include <linux/errno.h>     /* error codes */
#include <linux/fcntl.h>     /* O_ACCMODE */
#include <linux/fs.h>        /* everything... */
#include <linux/init.h>
#include <linux/jiffies.h> /* jiffies */
#include <linux/kernel.h>  /* printk() */
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched/signal.h> // for sig sending
#include <linux/seq_file.h>     // signle_open
#include <linux/signal.h>
#include <linux/slab.h>  /* kmalloc() */
#include <linux/types.h> /* size_t */
#include <linux/uaccess.h>
#include <linux/vmalloc.h> // For vmalloc

#include <linux/gpio.h> // for GPIO control
#include <linux/interrupt.h> // for IRQ handling

#define GPIO_RED 67     // Red LED
#define GPIO_YELLOW 68  // Yellow LED
#define GPIO_GREEN 44   // Green LED
#define GPIO_BTN0 26    // Button 0
#define GPIO_BTN1 46    // Button 1

#define BUF_LEN 256
#define MAX_PRINT_LENGTH PAGE_SIZE
#define MAX_TIMERS 2 // one more than Necessary to handle overflow
#define DEVICE_NAME "mytraffic"

MODULE_LICENSE("Dual BSD/GPL");

// define the timer holder struct
struct my_timer_holder {
  struct timer_list timer;
  int state;
};

static int mytraffic_open(struct inode *inode, struct file *filp);
static int mytraffic_release(struct inode *inode, struct file *filp);
static ssize_t mytraffic_write(struct file *filp, const char *buf, size_t count,
                               loff_t *f_pos);
static ssize_t mytraffic_read(struct file *filep, char __user *buffer,
                              size_t len, loff_t *offset);
static int mytraffic_init(void);
static void mytraffic_exit(void);
static void timer_handler(struct timer_list *);
struct my_timer_holder *find_by_pid(pid_t pid);

/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations mytraffic_fops = {
    .write = mytraffic_write,
    .read = mytraffic_read,
    .open = mytraffic_open,
    .release = mytraffic_release,
};

/* Declaration of the init and exit functions */
module_init(mytraffic_init);
module_exit(mytraffic_exit);

// all the variables
static int mytraffic_major = 61; /* be sure to run mknod with this major num! */
static struct my_timer_holder *my_timer;
int currentFrequency = 1; // defaults to a 1x multiplyer
int currentState = 0;
int lights[3] = {0, 0, 0};
int pedestrian_requested = 0;
int pedestrian_phase_active = 0;
int pedestrian_cycles = 0;
static int irq_btn0 = 0;
static int irq_btn1 = 0;

// interrupt handler for BTN0 (mode cycle)
static irqreturn_t btn0_isr(int irq, void *dev_id) {
  printk(KERN_ALERT "BUTTON0 TRIGGERED\n");
  currentState = (currentState + 1) % 3;
  my_timer->state = -1; // reset timer state when changing modes
  mod_timer(&my_timer->timer, jiffies + msecs_to_jiffies(10));
  printk(KERN_ALERT "BTN0 pressed, mode changed to %d\n", currentState);
  return IRQ_HANDLED;
}

// interrupt handler for BTN1 (pedestrian button)
static irqreturn_t btn1_isr(int irq, void *dev_id) {
  printk(KERN_ALERT "BUTTON1 TRIGGERED\n");
  if (currentState == 0) {
    pedestrian_requested = 1;
    printk(KERN_ALERT "BTN1 pressed, pedestrian request registered\n");
  } else {
    printk(KERN_ALERT "BTN1 pressed but ignored (not in normal mode, currentState = %d)\n", currentState);
  }
  return IRQ_HANDLED;
}

static int mytraffic_init(void) {
  // First set up the timer
  int result;
  int ret = 0;

  /* Register Timer  */
  my_timer = kzalloc(sizeof(struct my_timer_holder), GFP_KERNEL);
  if (!my_timer) {
    kfree(my_timer);
    return -ENOMEM;
  }

  // set state to default blinking pattern
  // -1 is a hack because you +1 at the start of each loops so I want the first
  // thing to run to be 0, not 1
  my_timer->state = -1;
  // then set up timer and run it at the current frequency
  timer_setup(&my_timer->timer, timer_handler, 0);
  mod_timer(&my_timer->timer,
            jiffies +
                msecs_to_jiffies(currentFrequency * 10)); // immediately start

  // configure GPIO pins
  result = gpio_request(GPIO_GREEN, "traffic_green");
  if (result) {
    printk(KERN_ALERT "Failed to request GPIO %d\n", GPIO_GREEN);
    // goto err_gpio;
    return result;
  }
  gpio_direction_output(GPIO_GREEN, 0);

  result = gpio_request(GPIO_YELLOW, "traffic_yellow");
  if (result) {
    printk(KERN_ALERT "Failed to request GPIO %d\n", GPIO_YELLOW);
    // goto err_gpio;
    return result;
  }
  gpio_direction_output(GPIO_YELLOW, 0);

  result = gpio_request(GPIO_RED, "traffic_red");
  if (result) {
    printk(KERN_ALERT "Failed to request GPIO %d\n", GPIO_RED);
    // goto err_gpio;
    return result;
  }
  gpio_direction_output(GPIO_RED, 0);

  result = gpio_request(GPIO_BTN0, "traffic_btn0");
  if (result) {
    printk(KERN_ALERT "Failed to request GPIO %d\n", GPIO_BTN0);
    // goto err_gpio;
    return result;
  }
  gpio_direction_input(GPIO_BTN0);

  result = gpio_request(GPIO_BTN1, "traffic_btn1");
  if (result) {
    printk(KERN_ALERT "Failed to request GPIO %d\n", GPIO_BTN1);
    // goto err_gpio;
    return result;
  }
  gpio_direction_input(GPIO_BTN1);

  // request IRQs for les buttons
  irq_btn0 = gpio_to_irq(GPIO_BTN0);
  result = request_irq(irq_btn0, btn0_isr, IRQF_TRIGGER_FALLING, "traffic_btn0", NULL);
  if (result) {
    printk(KERN_ALERT "Failed to request IRQ for BTN0\n");
    return result;
  }

  irq_btn1 = gpio_to_irq(GPIO_BTN1);
  result = request_irq(irq_btn1, btn1_isr, IRQF_TRIGGER_FALLING, "traffic_btn1", NULL);
  if (result) {
    printk(KERN_ALERT "Failed to request IRQ for BTN1\n");
    return result;
  }

  printk("mytraffic loaded.\n");

  /* Registering device */
  result = register_chrdev(mytraffic_major, "mytraffic", &mytraffic_fops);
  if (result < 0) {
    printk(KERN_ALERT "mytraffic: cannot obtain major number %d\n",
           mytraffic_major);
    return result;
  }

  return ret;
}

static void mytraffic_exit(void) {
  /* Freeing the major number */
  if (my_timer) {
    del_timer_sync(&my_timer->timer);
    kfree(my_timer);
    my_timer = NULL;
  }

  // freedom
  if (irq_btn0)
    free_irq(irq_btn0, NULL);
  if (irq_btn1)
    free_irq(irq_btn1, NULL);

  gpio_free(GPIO_RED);
  gpio_free(GPIO_YELLOW);
  gpio_free(GPIO_GREEN);
  gpio_free(GPIO_BTN0);
  gpio_free(GPIO_BTN1);

  unregister_chrdev(mytraffic_major, "mytraffic");
  printk(KERN_ALERT "Removing mytraffic module\n");
}

static int mytraffic_open(struct inode *inode, struct file *filp) { return 0; }

static int mytraffic_release(struct inode *inode, struct file *filp) {
  return 0;
}

static ssize_t mytraffic_read(struct file *filep, char __user *buffer,
                              size_t len, loff_t *offset) {
  char buf[512] = {0};
  int count = 0;

  if (*offset)
    return 0;

  // print to buffer
  count += scnprintf(buf + count, sizeof(buf) - count,
                     "Current Frequency: %dHz\n", currentFrequency);

  // TODO: Print which lights are active
  count += scnprintf(buf + count, sizeof(buf) - count, "Green: %s\n",
                     (lights[0] == 1) ? "On" : "Off");
  count += scnprintf(buf + count, sizeof(buf) - count, "Yellow: %s\n",
                     (lights[1] == 1) ? "On" : "Off");
  count += scnprintf(buf + count, sizeof(buf) - count, "Red: %s\n",
                     (lights[2] == 1) ? "On" : "Off");

  count +=
      scnprintf(buf + count, sizeof(buf) - count, "Pedestrian Present: N/A\n");
  // then sent that to the user
  if (copy_to_user(buffer, buf, count))
    return -EFAULT;

  *offset = count;
  return count;
}

static ssize_t mytraffic_write(struct file *filp, const char *buf, size_t count,
                               loff_t *f_pos) {
  // do something on write
  char kbuf[BUF_LEN];
  int value;
  int rc;

  // leave room for \0
  if (count >= BUF_LEN)
    count = BUF_LEN - 1;

  if (copy_from_user(kbuf, buf, count))
    return -EFAULT;

  // add null termination
  kbuf[count] = '\0';
  rc = kstrtoint(kbuf, 10, &value); // convert ot int
  // if not zero something is wrong
  if (rc)
    return -EINVAL;

  // now process that
  // TODO: Handler is here to do btn1 & ped call
  if (value == 0) {
    currentState = ((currentState + 1) % 3); // wrap around 0, 1, 2
    del_timer(&my_timer->timer);
    mod_timer(&my_timer->timer,
              jiffies +
                  msecs_to_jiffies(currentFrequency * 10)); // immediately fire
    printk(KERN_ALERT "State is now %d\n", currentState);
  } else if (value == 1) {
    printk(KERN_ALERT "Button 1 pressed\n");
  } else {
    printk(KERN_ALERT "Value is not supported\n");
  }
  return count;
}

static void timer_handler(struct timer_list *t) {
  // printk(KERN_ALERT "Timer state: %d (mode: %d)\n", my_timer->state, currentState); // debug print
  // TODO: On each timer: jump the interal counter depending on state

  // Normal: 3 green, 1 yellow, 2 red
  // or extended red if there's a pedestrian
  if (currentState == 0) {

    // check if we're entering red phase and pedestrian was requested before incrementing
    if (my_timer->state == 3 && pedestrian_requested) {
      pedestrian_phase_active = 1;
      pedestrian_requested = 0;
      pedestrian_cycles = 0;
      printk(KERN_ALERT "Pedestrian phase starting\n");
    }
    
    if (pedestrian_phase_active && my_timer->state == 5) { // don't increment if in pedestrian phase
      pedestrian_cycles++;
      if (pedestrian_cycles >= 4) {
        // done with pedestrian phase, reset and move to next state
        pedestrian_phase_active = 0;
        pedestrian_cycles = 0;
        my_timer->state = 0;
      } else {
        // stay at state 5 while in pedestrian phase
        my_timer->state = 5;
      }
    } else {
      my_timer->state = (my_timer->state + 1) % 6;
    }
    
    if (my_timer->state == 0 || my_timer->state == 1 || my_timer->state == 2) { // green cycle
      lights[0] = 1;
      lights[1] = 0;
      lights[2] = 0;
      gpio_set_value(GPIO_GREEN, 1);
      gpio_set_value(GPIO_YELLOW, 0);
      gpio_set_value(GPIO_RED, 0);
      printk(KERN_ALERT "GREEN");
    } else if (my_timer->state == 3) { // yellow cycle
      lights[0] = 0;
      lights[1] = 1;
      lights[2] = 0;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 1);
      gpio_set_value(GPIO_RED, 0);
      printk(KERN_ALERT "YELLOW");
    } else if (my_timer->state == 4) { // red cycle
      lights[0] = 0;
      lights[1] = 0;
      lights[2] = 1;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 0);
      gpio_set_value(GPIO_RED, 1);
      printk(KERN_ALERT "RED");
    } else if (my_timer->state == 5) { // pedestrian phase
      // in pedestrian phase show red & yellow, otherwise just red
      if (pedestrian_phase_active) {
        lights[0] = 0;
        lights[1] = 1;
        lights[2] = 1;
        gpio_set_value(GPIO_GREEN, 0);
        gpio_set_value(GPIO_YELLOW, 1);
        gpio_set_value(GPIO_RED, 1);
        printk(KERN_ALERT "RED+YELLOW");
      } else {
        lights[0] = 0;
        lights[1] = 0;
        lights[2] = 1;
        gpio_set_value(GPIO_GREEN, 0);
        gpio_set_value(GPIO_YELLOW, 0);
        gpio_set_value(GPIO_RED, 1);
        printk(KERN_ALERT "RED");
      }
    }
    // Flashing red: 1 red, 1 off
  } else if (currentState == 1) {
    my_timer->state = (my_timer->state + 1) % 2;
    if (my_timer->state == 0) {
      lights[0] = 0;
      lights[1] = 0;
      lights[2] = 1;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 0);
      gpio_set_value(GPIO_RED, 1);
      printk(KERN_ALERT "RED");
    } else if (my_timer->state == 1) {
      lights[0] = 0;
      lights[1] = 0;
      lights[2] = 0;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 0);
      gpio_set_value(GPIO_RED, 0);
      printk(KERN_ALERT "OFF");
    }
    // Flashing Yellow: 1 yellow, 1 off
  } else if (currentState == 2) {
    my_timer->state = (my_timer->state + 1) % 2;
    if (my_timer->state == 0) {
      lights[0] = 0;
      lights[1] = 1;
      lights[2] = 0;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 1);
      gpio_set_value(GPIO_RED, 0);
      printk(KERN_ALERT "YELLOW");
    } else if (my_timer->state == 1) {
      lights[0] = 0;
      lights[1] = 0;
      lights[2] = 0;
      gpio_set_value(GPIO_GREEN, 0);
      gpio_set_value(GPIO_YELLOW, 0);
      gpio_set_value(GPIO_RED, 0);
      printk(KERN_ALERT "OFF");
    }
  } else {
    // undefined state, panic back to 0
    gpio_set_value(GPIO_GREEN, 0);
    gpio_set_value(GPIO_YELLOW, 0);
    gpio_set_value(GPIO_RED, 0);
    printk(KERN_ALERT "Undefined state... uh oh\n");
    currentState = 0;
  }
  // now trigger the next timer to deal with later
  mod_timer(&my_timer->timer,
            jiffies + msecs_to_jiffies(currentFrequency * 1000));
}
