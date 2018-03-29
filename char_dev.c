#include <linux/init.h>
#include <linux/module.h>

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("eecsninja");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_LICENSE("GPL");

#define DEVICE_NAME "char_dev"
#define CLASS_NAME "dummy_class"

#define MESSAGE_BUFFER_SIZE    1024

// Contains all static variables used in this driver.
// Zero-initialized by default.
static struct {
	int major_number;

	// Number of times the device was opened.
	int open_count;

	// Memory for data passed in from user space.
	char message_buffer[MESSAGE_BUFFER_SIZE];
	int message_length;

	struct class *device_class;
	struct device *device;
} driver;

// Contains pointers to functions that handle all required file operations.
// TODO: Implement these.
static struct file_operations file_ops;

//////////// FILE OPERATION FUNCTIONS ////////////////

static int dev_open(struct inode *inodep, struct file *filep)
{
	++driver.open_count;
	printk(KERN_INFO "%s: Device has been opened %d time(s)\n", DEVICE_NAME,
	       driver.open_count);
	return 0;
}

static ssize_t dev_read(struct file *filep,
			char *buffer,
			size_t len,
			loff_t *offset)
{
	int ret = 0;

	if (driver.message_length < len)
		len = driver.message_length;
	ret = copy_to_user(buffer, driver.message_buffer, len);

	if (ret) {
		printk(KERN_ERR "%s: Could not read %zu bytes of data from "
		       "userspace.\n", DEVICE_NAME, len);
		ret = -1;
		goto exit;
	}
	printk("%s: Sent %zu characters to userspace.\n", DEVICE_NAME, len);
	ret = len;

exit:
	return ret;
}

static ssize_t dev_write(struct file *filep,
			 const char *buffer,
			 size_t count,
			 loff_t *offset)
{
	// Read message from buffer.
	snprintf(driver.message_buffer, sizeof(driver.message_buffer),
		 "%s (%zu)", buffer, count);
	driver.message_length = strlen(driver.message_buffer) + 1;

	printk(KERN_INFO "%s: Received %zu characters from userspace; read "
	       "%d.\n", DEVICE_NAME, count, driver.message_length);
	return count;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
   printk(KERN_INFO "%s: Device successfully closed\n", DEVICE_NAME);
   return 0;
}

///////////// MODULE FUNCTIONS ////////////////

// Module initialization function.
static int __init char_dev_init(void)
{
	int ret = 0;

	// Specify file operation functions.
	file_ops.open = dev_open;
	file_ops.read = dev_read;
	file_ops.write = dev_write;
	file_ops.release = dev_release;

	printk(KERN_INFO "Initializing char driver for %s\n", DEVICE_NAME);

	// Allocate major number for device.
	ret = register_chrdev(0, DEVICE_NAME, &file_ops);
	if (ret < 0) {
		printk(KERN_ERR "Failed to register a major number for %s\n",
		       DEVICE_NAME);
		goto exit_device;
	}
	driver.major_number = ret;

	// Register device class.
	driver.device_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(driver.device_class)) {
		printk(KERN_ERR "Failed to register device class %s\n",
		       CLASS_NAME);
		ret = -1;
		goto exit_class;
	}

	// Register device driver.
	driver.device = device_create(driver.device_class, NULL,
				      MKDEV(driver.major_number, 0), NULL,
				      DEVICE_NAME);
	if (IS_ERR(driver.device)) {
		printk(KERN_ERR "Failed to create device %s\n", DEVICE_NAME);
		ret = -1;
		goto exit_driver;
	}

	printk(KERN_INFO "Device driver for %s was created successfully.\n",
	       DEVICE_NAME);
	return 0;

exit_driver:
	class_destroy(driver.device_class);
exit_class:
	unregister_chrdev(driver.major_number, DEVICE_NAME);
exit_device:
	return ret;
}

// Module cleanup function.
// TODO: Can this be called from init when an error occurs?
static void __exit char_dev_exit(void)
{
	device_destroy(driver.device_class, MKDEV(driver.major_number, 0));
	class_unregister(driver.device_class);
	class_destroy(driver.device_class);
	unregister_chrdev(driver.major_number, DEVICE_NAME);

	printk(KERN_INFO "Exiting from driver for %s\n", DEVICE_NAME);
}

module_init(char_dev_init);
module_exit(char_dev_exit);
