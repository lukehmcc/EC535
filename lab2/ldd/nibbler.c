
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
#include <asm/system_misc.h> /* cli(), *_flags */
#include <linux/uaccess.h>
#include <asm/uaccess.h> /* copy_from/to_user */

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of memory.c functions */
static int ktimer_open(struct inode *inode, struct file *filp);
static int ktimer_release(struct inode *inode, struct file *filp);
static ssize_t ktimer_read(struct file *filp,
		char *buf, size_t count, loff_t *f_pos);
static ssize_t ktimer_write(struct file *filp,
		const char *buf, size_t count, loff_t *f_pos);
static void ktimer_exit(void);
static int ktimer_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations ktimer_fops = {
	read: ktimer_read,
	write: ktimer_write,
	open: ktimer_open,
	release: ktimer_release
};

/* Declaration of the init and exit functions */
module_init(ktimer_init);
module_exit(ktimer_exit);

static unsigned capacity = 128;
static unsigned bite = 128;
module_param(capacity, uint, S_IRUGO);
module_param(bite, uint, S_IRUGO);

/* Global variables of the driver */
/* Major number */
static int ktimer_major = 61;

/* Buffer to store data */
static char *ktimer_buffer;
/* length of the current message */
static int ktimer_len;

static int ktimer_init(void)
{
	int result;

	/* Registering device */
	result = register_chrdev(ktimer_major, "ktimer", &ktimer_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"ktimer: cannot obtain major number %d\n", ktimer_major);
		return result;
	}

	/* Allocating ktimer for the buffer */
	ktimer_buffer = kmalloc(capacity, GFP_KERNEL); 
	if (!ktimer_buffer)
	{ 
		printk(KERN_ALERT "Insufficient kernel memory\n"); 
		result = -ENOMEM;
		goto fail; 
	} 
	memset(ktimer_buffer, 0, capacity);
	ktimer_len = 0;

	printk(KERN_ALERT "Inserting ktimer module\n"); 
	return 0;

fail: 
	ktimer_exit(); 
	return result;
}

static void ktimer_exit(void)
{
	/* Freeing the major number */
	unregister_chrdev(ktimer_major, "ktimer");

	/* Freeing buffer memory */
	if (ktimer_buffer)
	{
		kfree(ktimer_buffer);
	}

	printk(KERN_ALERT "Removing ktimer module\n");

}

static int ktimer_open(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "open called: process id %d, command %s\n",
		current->pid, current->comm);
	/* Success */
	return 0;
}

static int ktimer_release(struct inode *inode, struct file *filp)
{
	printk(KERN_INFO "release called: process id %d, command %s\n",
		current->pid, current->comm);
	/* Success */
	return 0;
}

static ssize_t ktimer_read(struct file *filp, char *buf, 
							size_t count, loff_t *f_pos)
{ 
	int temp;
	char tbuf[256], *tbptr = tbuf;

	/* end of buffer reached */
	if (*f_pos >= ktimer_len)
	{
		return 0;
	}

	/* do not go over then end */
	if (count > ktimer_len - *f_pos)
		count = ktimer_len - *f_pos;

	/* do not send back more than a bite */
	if (count > bite) count = bite;

	/* Transfering data to user space */ 
	if (copy_to_user(buf, ktimer_buffer + *f_pos, count))
	{
		return -EFAULT;
	}

	tbptr += sprintf(tbptr,								   
		"read called: process id %d, command %s, count %d, chars ",
		current->pid, current->comm, count);

	for (temp = *f_pos; temp < count + *f_pos; temp++)					  
		tbptr += sprintf(tbptr, "%c", ktimer_buffer[temp]);

	printk(KERN_INFO "%s\n", tbuf);

	/* Changing reading position as best suits */ 
	*f_pos += count; 
	return count; 
}

static ssize_t ktimer_write(struct file *filp, const char *buf,
							size_t count, loff_t *f_pos)
{
	int temp;
	char tbuf[256], *tbptr = tbuf;

	/* end of buffer reached */
	if (*f_pos >= capacity)
	{
		printk(KERN_INFO
			"write called: process id %d, command %s, count %d, buffer full\n",
			current->pid, current->comm, count);
		return -ENOSPC;
	}

	/* do not eat more than a bite */
	if (count > bite) count = bite;

	/* do not go over the end */
	if (count > capacity - *f_pos)
		count = capacity - *f_pos;

	if (copy_from_user(ktimer_buffer + *f_pos, buf, count))
	{
		return -EFAULT;
	}

	tbptr += sprintf(tbptr,								   
		"write called: process id %d, command %s, count %d, chars ",
		current->pid, current->comm, count);

	for (temp = *f_pos; temp < count + *f_pos; temp++)					  
		tbptr += sprintf(tbptr, "%c", ktimer_buffer[temp]);

	printk(KERN_INFO "%s\n", tbuf);

	*f_pos += count;
	ktimer_len = *f_pos;

	return count;
}

