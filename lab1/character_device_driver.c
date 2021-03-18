#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yaremko Roman & Kuprianov Arthur");
MODULE_DESCRIPTION("Linux module for calculations with storing the result");
MODULE_VERSION("3.14159265");

static dev_t char_dev;
static struct cdev char_class;
static struct class *cl;

static struct proc_dir_entry * entry;

static int dev_open(struct inode *i, struct file *f){
	printk(KERN_INFO "Open file\n");
	return 0;
}

static int dev_close(struct inode *i, struct file *f){
	printk(KERN_INFO "Close file\n");
	return 0;
}

static char string_buf[1024];

static ssize_t dev_read(struct file * f, char __user * buf, size_t len, loff_t * off){
	printk(KERN_INFO "Read file\n");
	return simple_read_from_buffer(buf, len, off, string_buf, 1024);
}

static ssize_t dev_write(struct file * f, const char __user * buf, size_t len, loff_t * off){
	printk(KERN_INFO "Write file\n");
	if (copy_from_user(&string_buf, buf, len) != 0)
		return -EFAULT;
	else
		return len;
}

static ssize_t proc_read(struct file * file, char __user * buf, size_t len, loff_t * off){
	printk(KERN_INFO "Read proc file\n");
	return simple_read_from_buffer(buf, len, off, string_buf, 1024);
}

static ssize_t proc_write(struct file * file, const char __user * buf, size_t len, loff_t * off){
	printk(KERN_DEBUG "Attempt to write into proc file");
	return -1;
}

static struct file_operations pugs_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.read = dev_read,
	.write = dev_write
};

static const struct proc_ops proc_fops = {
	.proc_read = proc_read,
	.proc_write = proc_write
};

static int __init mod_init(void){
	printk(KERN_INFO "Initialization");
	entry = proc_create("var2", 0444, NULL, &proc_fops);
	//string_buf = kmalloc()
	if (alloc_chrdev_region(&char_dev, 0, 1, "Laba") < 0){
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "chardriver")) == NULL){
		unregister_chrdev_region(char_dev, 1);
		return -1;
	}
	if (device_create(cl, NULL, char_dev, NULL, "var2") == NULL){
		class_destroy(cl);
		unregister_chrdev_region(char_dev, 1);
		return -1;
	}
	cdev_init(&char_class, &pugs_fops);
	if (cdev_add(&char_class, char_dev, 1) == -1){
		device_destroy(cl, char_dev);
		class_destroy(cl);
		unregister_chrdev_region(char_dev, 1);
		return -1;
	}
	return 0;
}

static void __exit mod_exit(void){
	printk(KERN_INFO "Exit module");
	cdev_del(&char_class);
	device_destroy(cl, char_dev);
	class_destroy(cl);
	unregister_chrdev_region(char_dev, 1);
}

module_init(mod_init);
module_exit(mod_exit);
