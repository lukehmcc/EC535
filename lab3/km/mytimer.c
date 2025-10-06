/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/jiffies.h> /* jiffies */
#include <asm/system_misc.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

static int mytimer_fasync(int fd, struct file *filp, int mode);
static int mytimer_open(struct inode *inode, struct file *filp);
static int mytimer_release(struct inode *inode, struct file *filp);
static ssize_t mytimer_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static int mytimer_fasync(int fd, struct file *filp, int mode);
static int mytimer_init(void);
static void mytimer_exit(void);
static void timer_handler(struct timer_list*);


/*
 * The file operations for the pipe device
 * (some are overlayed with bare scull)
 */
struct file_operations mytimer_fops = {
write:
    mytimer_write,
open:
    mytimer_open,
release:
    mytimer_release,
fasync:
    mytimer_fasync
};

/* Declaration of the init and exit functions */
module_init(mytimer_init);
module_exit(mytimer_exit);

static int mytimer_major = 61; /* be sure to run mknod with this major num! */
struct fasync_struct *async_queue; /* structure for keeping track of asynchronous readers */
static struct timer_list * fasync_timer; /* structure for keeping track of timer */

static int mytimer_init(void) {
    int result;

    /* Registering device */
    result = register_chrdev(mytimer_major, "mytimer", &mytimer_fops);
    if (result < 0)
    {
        printk(KERN_ALERT
               "mytimer: cannot obtain major number %d\n", mytimer_major);
        return result;
    }

    /* Allocating buffers */
    fasync_timer = (struct timer_list *) kmalloc(sizeof(struct timer_list), GFP_KERNEL);

    /* Check if timer  */
    if (!fasync_timer)
    {
        printk(KERN_ALERT "Insufficient kernel memory\n");
        result = -ENOMEM;
        goto fail;
    }

    printk("mytimer loaded.\n");
    return 0;

fail:
    mytimer_exit();
    return result;
}

static void mytimer_exit(void) {
    /* Freeing the major number */
    unregister_chrdev(mytimer_major, "mytimer");
    if (fasync_timer)
        kfree(fasync_timer);

    printk(KERN_ALERT "Removing mytimer module\n");

}

static int mytimer_open(struct inode *inode, struct file *filp) {
    return 0;
}

static int mytimer_release(struct inode *inode, struct file *filp) {
    mytimer_fasync(-1, filp, 0);
    return 0;
}

static ssize_t mytimer_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
    timer_setup(fasync_timer, timer_handler, 0);
    mod_timer(fasync_timer, jiffies + msecs_to_jiffies(10000));
    return count;
}

static int mytimer_fasync(int fd, struct file *filp, int mode) {
    return fasync_helper(fd, filp, mode, &async_queue);
}

static void timer_handler(struct timer_list *data) {
    if (async_queue)
        kill_fasync(&async_queue, SIGIO, POLL_IN);

    del_timer(fasync_timer);
}

