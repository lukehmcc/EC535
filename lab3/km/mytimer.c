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
#include <linux/seq_file.h> // signle_open
#include <linux/signal.h>
#include <linux/slab.h>  /* kmalloc() */
#include <linux/types.h> /* size_t */
#include <linux/uaccess.h>
#include <linux/vmalloc.h> // For vmalloc
#include <linux/sched/signal.h> // for sig sending

#define BUF_LEN 256
#define MAX_PRINT_LENGTH PAGE_SIZE
#define MAX_TIMERS 2 // one more than Necessary to handle overflow

MODULE_LICENSE("Dual BSD/GPL");

// define communication struct
struct comStruct {
  // 0: -s
  // 1: -r
  // 2: -m
  int command;
  int seconds;
  int pid;
  int count;
  char msg[BUF_LEN];
};

// define the timer holder struct
struct my_timer_holder {
  struct timer_list timer;
  char msg[BUF_LEN];
  int pid;
  struct fasync_struct *async_queue; // async queue with just one guy
};

static int mytimer_fasync(int fd, struct file *filp, int mode);
static int mytimer_open(struct inode *inode, struct file *filp);
static int mytimer_proc_open(struct inode *inode, struct file *filp);
static int mytimer_proc_show(struct seq_file *m, void *v);
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
  write : mytimer_write,
  read : mytimer_read,
  open : mytimer_open,
  release : mytimer_release,
  fasync : mytimer_fasync
};

// File operations with proc device
static const struct file_operations mytimer_proc_fops = {
    .owner = THIS_MODULE,
    .open = mytimer_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

/* Declaration of the init and exit functions */
module_init(mytimer_init);
module_exit(mytimer_exit);

// all the variables
static int mytimer_major = 61; /* be sure to run mknod with this major num! */
static char *list_pot; // Space for the proc output
static struct proc_dir_entry *proc_entry; // the proc entry
static u64 start_jiffies;
static struct my_timer_holder *my_timer[MAX_TIMERS];
int currentMaxTimers = 1;

static int mytimer_init(void) {
  // First set up the timer
  int result, i;
  int ret = 0;
  list_pot = (char *)vmalloc(MAX_PRINT_LENGTH);
  if (!list_pot)
        return -ENOMEM;

  /* Register Timers  */
  for (i = 0; i < MAX_TIMERS; ++i) {
        my_timer[i] = kzalloc(sizeof(struct my_timer_holder), GFP_KERNEL);
        if (!my_timer[i]) {
            while (--i >= 0)
                kfree(my_timer[i]);
            vfree(list_pot);
            return -ENOMEM;
        }
        timer_setup(&my_timer[i]->timer, timer_handler, 0);
    }

  printk("mytimer loaded.\n");

  /* Registering device */
  result = register_chrdev(mytimer_major, "mytimer", &mytimer_fops);
  if (result < 0) {
    printk(KERN_ALERT "mytimer: cannot obtain major number %d\n",
           mytimer_major);
    return result;
  }


  // Now set up the proc file
  if (!list_pot) {
    ret = -ENOMEM;
  } else {
    memset(list_pot, 0, MAX_PRINT_LENGTH);
    proc_entry = proc_create("mytimer", 0644, NULL, &mytimer_proc_fops);

    if (proc_entry == NULL) {
      ret = -ENOMEM;
      vfree(list_pot);
      printk(KERN_INFO "fortune: Couldn't create proc entry\n");
    } else {
      printk(KERN_INFO "fortune: Module loaded.\n");
    }
  }

  // and get the start fiffies
  start_jiffies = get_jiffies_64();
  return ret;
}

static void mytimer_exit(void) {
  /* Freeing the major number */
  int i;
  unregister_chrdev(mytimer_major, "mytimer");
  for (i = 0; i < MAX_TIMERS; ++i) {
    kfree(my_timer[i]);
    my_timer[i] = NULL;
  }
  printk(KERN_ALERT "Removing mytimer module\n");

  // free proc file
  remove_proc_entry("mytimer", NULL);
  vfree(list_pot);
  printk(KERN_INFO "mytimer: Module unloaded.\n");
}

static int mytimer_proc_show(struct seq_file *m, void *v) {
  int i = 0;
  u64 elapsed = jiffies_to_msecs(get_jiffies_64() - start_jiffies);
  seq_printf(m, "Module Name: mytimer\nSince Module Loaded: %ums\n",
             jiffies_to_msecs(elapsed));
  for (i = 0; i < MAX_TIMERS; i++) {
    if (my_timer[i] && timer_pending(&my_timer[i]->timer)) {
      seq_printf(m, "PID: %u\nCMD: %s\nSec: %u\n", my_timer[i]->pid, my_timer[i]->msg,
                jiffies_to_msecs(my_timer[i]->timer.expires - jiffies) / 1000);

    }
  }
  return 0;
}

static int mytimer_proc_open(struct inode *inode, struct file *filp) {
  return single_open(filp, mytimer_proc_show, NULL);
}

static int mytimer_open(struct inode *inode, struct file *filp) {
  return 0;
}

static int mytimer_release(struct inode *inode, struct file *filp) {
  mytimer_fasync(-1, filp, 0);
  return 0;
}

static ssize_t mytimer_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset){
    char buf[512];
    int count = 0, i;

    if (*offset)
        return 0;

    for (i = 0; i < MAX_TIMERS; ++i) {
        if (my_timer[i] && timer_pending(&my_timer[i]->timer)) {
            unsigned long left = my_timer[i]->timer.expires - jiffies;
            count += scnprintf(buf + count, sizeof(buf) - count,
                               "%s %u\n", my_timer[i]->msg,
                               jiffies_to_msecs(left) / 1000U);
        }
    }
    if (copy_to_user(buffer, buf, count))
        return -EFAULT;

    *offset = count;
    return count;
}

static ssize_t mytimer_write(struct file *filp, const char *buf, size_t count,
                             loff_t *f_pos) {
  // Parse the mess;
  struct comStruct req;
  int slot = -1;
  int i;
  int currentActive = 0;

  // Copy that struct into the the buffer
  if (count > BUF_LEN)
    count = BUF_LEN;
  if (copy_from_user(&req, buf, sizeof(req)))
    return -EFAULT;

  // Remember this
  // 0: -s
  // 1: -r
  // then set up that timer
  if (req.command == 0) {
    // first check if the message is duplicate
    for (i = 0; i < MAX_TIMERS; ++i) {
        // if it does exist, just mod the expiration date and return
        if (timer_pending(&my_timer[i]->timer) && strcmp(req.msg, my_timer[i]->msg) == 0) {
          mod_timer(&my_timer[i]->timer,
                    jiffies + msecs_to_jiffies(1000 * req.seconds)); // For vmalloc
          return -EBUSY; // incorrect error code (I am tired)
        }
    }

    // Check how many currently active
    for (i = 0; i < MAX_TIMERS; ++i) {
        if (timer_pending(&my_timer[i]->timer)) {
          currentActive++;
        }
    }

    // look for a free slot
    for (i = 0; i < MAX_TIMERS; ++i) {
        if (!timer_pending(&my_timer[i]->timer)) {
            slot = i;
            break;
        }
    }
    // if there's not free slot return an error for no space
    if (currentActive >= currentMaxTimers || slot < 0)          
        return -ENOSPC;
    
    // now insert timer to that slot
    mod_timer(&my_timer[slot]->timer,
              jiffies + msecs_to_jiffies(1000 * req.seconds)); // For vmalloc
    strscpy(my_timer[slot]->msg, req.msg, strlen(req.msg)+1);
    my_timer[slot]->pid = req.pid;

    // Nuke the timers
  } else if (req.command == 1) {
    for (i = 0; i < MAX_TIMERS; ++i) {
      if (my_timer[i] && timer_pending(&my_timer[i]->timer)) {
        del_timer(&my_timer[i]->timer);
      }
    }
  } else if (req.command == 2){
    if (req.count < 1 || req.count > 2){
      return -EINVAL;
    } else {
      currentMaxTimers = req.count;
    }
  }

  return sizeof(req);
}

static int mytimer_fasync(int fd, struct file *filp, int mode) {
  struct my_timer_holder *holder = find_by_pid(current->pid);
  if (holder){
    return fasync_helper(fd, filp, mode, &holder->async_queue);
  } 
  return -1;
}

static void timer_handler(struct timer_list *t) {
  struct my_timer_holder *holder = from_timer(holder, t, timer);
  if (holder->async_queue) {
    kill_fasync(&holder->async_queue, SIGIO, POLL_IN);
  }
}

// helper funciton to grab the holder by pid
struct my_timer_holder *find_by_pid(pid_t pid){
  int i;
  for (i = 0; i < MAX_TIMERS; ++i) {
    if (my_timer[i] && my_timer[i]->pid == pid)
        return my_timer[i];
  }
  return NULL;
}
