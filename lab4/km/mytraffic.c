/* Necessary includes for device drivers */
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

#define BUF_LEN 256
#define MAX_PRINT_LENGTH PAGE_SIZE
#define MAX_TIMERS 2 // one more than Necessary to handle overflow
#define DEVICE_NAME "mytraffic"

MODULE_LICENSE("Dual BSD/GPL");

// define the timer holder struct
struct my_timer_holder {
  struct timer_list timer;
};

static int mytimer_open(struct inode *inode, struct file *filp);
static int mytimer_release(struct inode *inode, struct file *filp);
static ssize_t mytimer_write(struct file *filp, const char *buf, size_t count,
                             loff_t *f_pos);
static ssize_t mytimer_read(struct file *filep, char __user *buffer, size_t len,
                            loff_t *offset);
static int mytimer_init(void);
static void mytimer_exit(void);
static void timer_handler(struct timer_list *);
struct my_timer_holder *find_by_pid(pid_t pid);

/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations mytimer_fops = {
    .write = mytimer_write,
    .read = mytimer_read,
    .open = mytimer_open,
    .release = mytimer_release,
};

/* Declaration of the init and exit functions */
module_init(mytimer_init);
module_exit(mytimer_exit);

// all the variables
static int mytimer_major = 61; /* be sure to run mknod with this major num! */
static struct my_timer_holder *my_timer;

static int mytimer_init(void) {
  // First set up the timer
  int result;
  int ret = 0;

  /* Register Timer  */
  my_timer = kzalloc(sizeof(struct my_timer_holder), GFP_KERNEL);
  if (!my_timer) {
    kfree(my_timer);
    return -ENOMEM;
  }
  timer_setup(&my_timer->timer, timer_handler, 0);

  printk("mytimer loaded.\n");

  /* Registering device */
  result = register_chrdev(mytimer_major, "mytimer", &mytimer_fops);
  if (result < 0) {
    printk(KERN_ALERT "mytimer: cannot obtain major number %d\n",
           mytimer_major);
    return result;
  }

  return ret;
}

static void mytimer_exit(void) {
  /* Freeing the major number */
  unregister_chrdev(mytimer_major, "mytimer");
  kfree(my_timer);
  my_timer = NULL;
  printk(KERN_ALERT "Removing mytimer module\n");

  // free proc file
  remove_proc_entry("mytimer", NULL);
  printk(KERN_INFO "mytimer: Module unloaded.\n");
}

static int mytimer_open(struct inode *inode, struct file *filp) { return 0; }

static int mytimer_release(struct inode *inode, struct file *filp) { return 0; }

static ssize_t mytimer_read(struct file *filep, char __user *buffer, size_t len,
                            loff_t *offset) {
  char buf[512] = {0};
  int count = 0;

  if (*offset)
    return 0;

  if (my_timer && timer_pending(&my_timer->timer)) {
    count += scnprintf(buf + count, sizeof(buf) - count, "Yo nerd\n");
  }
  if (copy_to_user(buffer, buf, count))
    return -EFAULT;

  *offset = count;
  return count;
}

static ssize_t mytimer_write(struct file *filp, const char *buf, size_t count,
                             loff_t *f_pos) {
  // do something on write
  return 0;
}

static void timer_handler(struct timer_list *t) {
  struct my_timer_holder *holder = from_timer(holder, t, timer);
  // do something on timer
}
