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
MODULE_VERSION("123");

static dev_t char_dev;
static struct cdev char_class;
static struct class *cl;

static struct proc_dir_entry * entry;

static char result_buf[4096];
static size_t size;

static int atoi(char *str){
	if (*str == "\0")
		return 0;
	
	int res = 0;
	int sign = 1;
	int i = 0;

	if (str[0] == "-") {
		sign = -1;
		i++;
	}

	for (; str[i] != '\0'; i++) {
		if (str[i] <= '0' || str[i] >= '9')
			return 0;
		res = res*10 + str[i] - '0';
	}

	return sign * res;
}

static void reverse(char s[]){
    int i, j;
    char c;
 
    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

static void itoa(int n, char s[]){
	int i, sign;
 
    if ((sign = n) < 0)
    	n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

static ssize_t length(int num) {
	ssize_t count = 0;
	if (num < 0)
		count++;
	while (num != 0) {
		count++;
		num /= 10;
	}
	return count;
}

static int dev_open(struct inode *i, struct file *f){
	printk(KERN_INFO "Open file\n");
	return 0;
}

static int dev_close(struct inode *i, struct file *f){
	printk(KERN_INFO "Close file\n");
	return 0;
}

static int calculator(char * term){
	if (strstr(term, "+")) {
		char * first_str = strsep(&term, "+");
		int first = atoi(first_str);
		int second = atoi(term);
		return first + second;
	} else if (strstr(term, "-")) {
		char * first_str = strsep(&term, "-");
		int first = atoi(first_str);
		int second = atoi(term);
		return first - second;
	} else if (strstr(term, "*")) {
		char * first_str = strsep(&term, "*");
		int first = atoi(first_str);
		int second = atoi(term);
		return first * second;
	} else if (strstr(term, "/")) {
		char * first_str = strsep(&term, "/");
		int first = atoi(first_str);
		int second = atoi(term);
		return first / second;
	}
	return 0;
}

static ssize_t dev_read(struct file * f, char __user * buf, size_t len, loff_t * off){
	printk(KERN_INFO "Read file len %d\n", len);
	return simple_read_from_buffer(buf, len, off, result_buf, 4096);
}

static ssize_t proc_read(struct file * file, char __user * buf, size_t len, loff_t * off){
	printk(KERN_INFO "Read proc file\n");
	return simple_read_from_buffer(buf, len, off, result_buf, 4096);
}

static ssize_t dev_write(struct file * f, const char __user * buf, size_t len, loff_t * off){
	char tmp[len];
	if (copy_from_user(tmp, buf, len) != 0)
		return -EFAULT;
	else {
		tmp[len-1] = '\0';
		int res = calculator(tmp);

		size_t new_len = length(res);
		char res_str[new_len];
		itoa(res, res_str);

		int i;
		for (i = 0; i < new_len; i++) {
			result_buf[size + i] = res_str[i];
		}

		printk(KERN_DEBUG "RESULT IS %d\n", res);
		printk(KERN_INFO "Write file %d %s\n", len, tmp);
		size = size + len;
		return len;
	}
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
	entry = proc_create("var2", 0777, NULL, &proc_fops);
	size = 0;
	if (alloc_chrdev_region(&char_dev, 0, 1, "Laba") < 0){
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "chardriver")) == NULL){
		unregister_chrdev_region(char_dev, 1);
		return -1;
	}
	if (device_create(cl, NULL, char_dev, NULL, "var2") == NULL){ // echo "1+1" > /dev/var2
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