#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "mytimer"
#define BUF_LEN 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luke McCarthy");
MODULE_DESCRIPTION("Simple echo module");


static struct class *cls;

struct my_timer_holder {
        struct  timer_list timer;
        char    msg[BUF_LEN];
};

static struct my_timer_holder *my_timer; // set up timer & holder
static int max_concurrency = 1;
static int major = 61; // hard coded major number 

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
  // Define 
  int i;
  int result;

  // set up module
  result = register_chrdev(61, DEVICE_NAME, &fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"ktimer: cannot obtain major number %d\n", major);
		return result;
	}
  cls = class_create(THIS_MODULE, DEVICE_NAME);
  device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

  // set up timers
  my_timer = kzalloc(sizeof(struct my_timer_holder) * 5, GFP_KERNEL);
  if (!my_timer)
        return -ENOMEM;
  for (i = 0; i < 5; i++){
    timer_setup(&my_timer[i].timer, timer_fn, 0);
  }
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
    char buf[256];
    int charsToCopy = 0; // characters to copy over to user space
    int i;
    if (*offset) return 0;
    // If timer exists, return all of them
    for (i = 0; i < 5; i++){
      if (timer_pending(&my_timer[i].timer)){
        charsToCopy += scnprintf(buf + charsToCopy, sizeof(buf) - charsToCopy, "%s %u\n", my_timer[i].msg, jiffies_to_msecs(my_timer[i].timer.expires - jiffies)/1000);
        if (charsToCopy >= sizeof(buf)){ // overflow guardrail
          break;
        }
      }    
    } 
    return simple_read_from_buffer(buffer, len, offset, buf, charsToCopy);
}

static void timer_fn(struct timer_list *t){
  struct my_timer_holder *holder = from_timer(holder, t, timer);
    pr_info("%s\n", holder->msg);
}

static ssize_t mytimer_write(struct file *filep, const char __user *buffer,
                             size_t len, loff_t *offset) {
    // Parse the message. The setup is [[lenght], [int (seconds/concurrency)], [text]]
    int  txt_len, seconds;
    char txt[BUF_LEN];
    char txt2[BUF_LEN];
    int  off = 0;
    int  i;
    int  concurrencyCurrent = 0;

    if (len > BUF_LEN) len = BUF_LEN;
    if (copy_from_user(txt, buffer, len))
        return -EFAULT;

    txt_len = *(int *)(txt + off);  off += sizeof(int);
    seconds = *(int *)(txt + off);  off += sizeof(int);
    // In order to minimize the coms between the kernel & user space, I just assume that 
    // if the length of the message is 0 then it's actually a concurrency set call. This 
    // may not be best practice as it is possible that the userspace won't properly check 
    // the inputs and an empty string could slip through. But I have checks in the user space
    // so this should be fine for now
    if (txt_len == 0) {
      // set the new max concurrency
      max_concurrency = seconds;
    } else {
      // don't copy into the msg variable yet in case it isn't needed
      memcpy(txt2, txt + off, txt_len);
      txt2[txt_len] = '\0';
      // loop through, count how many are active. If the text of the active member 
      // matches the input text, set that clock to whatever the input is. Make sure to 
      // return here
      for (i = 0; i < 5; i++){
        if (timer_pending(&my_timer[i].timer)){
          concurrencyCurrent++;
          if (strcmp(txt2, my_timer[i].msg) == 0){
            mod_timer(&my_timer[i].timer, jiffies + msecs_to_jiffies(seconds * 1000));
            pr_info("The timer %s was updated!\n", my_timer[i].msg);
            return off + txt_len;
          }
        }
      }
      // Check concurrency
      if (concurrencyCurrent >= max_concurrency){
        pr_info("[COUNT] timer(s) already exist(s)!\n");
        return off + txt_len;
      }
      // If there is space, go and reset the closest non-active one
      for (i = 0; i < 5; i++){
        if (!timer_pending(&my_timer[i].timer)){
          memcpy(&my_timer[i].msg, txt2, txt_len);

          mod_timer(&my_timer[i].timer, jiffies + msecs_to_jiffies(seconds * 1000));
          break;
        }
      }
      return off + txt_len;
    }
    return 0;
}

module_init(mytimer_init);
module_exit(mytimer_exit);

