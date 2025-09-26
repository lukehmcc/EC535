#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mytimer"
#define BUF_LEN 132

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luke McCarthy");
MODULE_DESCRIPTION("Simple echo module");

static int major;
static struct class *cls;
static struct timer_list my_timer; // set up timer list
static char msg[BUF_LEN];

static int mytimer_open(struct inode *, struct file *);
static int mytimer_release(struct inode *, struct file *);
static ssize_t mytimer_write(struct file *, const char __user *, size_t,
                             loff_t *);
static ssize_t mytimer_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset);
static void timer_fn(struct timer_list *t);

static struct file_operations fops = {.open = mytimer_open,
                                      .release = mytimer_release,
                                      .write = mytimer_write,
                                      .read = mytimer_read,
                                      .owner = THIS_MODULE};

static int __init mytimer_init(void) {
  major = register_chrdev(0, DEVICE_NAME, &fops);
  cls = class_create(THIS_MODULE, DEVICE_NAME);
  device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
  timer_setup(&my_timer, timer_fn, 0);
  return 0;
}

static void __exit mytimer_exit(void) {
  device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, DEVICE_NAME);
}

static int mytimer_open(struct inode *inodep, struct file *filep) { return 0; }

static int mytimer_release(struct inode *inodep, struct file *filep) {
  return 0;
}

static ssize_t mytimer_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    char buf[64];
    int ret;
    if (*offset) return 0;
    // If timer exists, return it
    if (timer_pending(&my_timer)){
      ret = scnprintf(buf, sizeof(buf), "%s: %u\n", msg, jiffies_to_msecs(my_timer.expires - jiffies)/1000);
      return simple_read_from_buffer(buffer, len, offset, buf, ret);
    // otherwise do nothing
    } else {
      return 0;
    }     
}


static void timer_fn(struct timer_list *t)
{
    pr_info("mytimer: %s\n", msg);
    del_timer(&my_timer);
}

static ssize_t mytimer_write(struct file *filep, const char __user *buffer,
                             size_t len, loff_t *offset) {
  int bytes = len > BUF_LEN ? BUF_LEN : len;
  copy_from_user(msg, buffer, bytes);
  msg[bytes] = '\0';

  mod_timer(&my_timer, jiffies + msecs_to_jiffies(2000));

  return bytes;
}

module_init(mytimer_init);
module_exit(mytimer_exit);

