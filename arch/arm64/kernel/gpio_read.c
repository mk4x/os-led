#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>

// GPIO config for PI 2 Zero
#define GPIO_BASE 0xFE200000
#define GPIO_SIZE 0x1000

// GPIO register offsets
#define GPFSEL0 0x00 // (pins 0-9)
#define GPFSEL1 0x04 // (pins 10-19)
#define GPFSEL2 0x08 // (pins 20-29)
#define GPFSEL3 0x0C // (pins 30-39)
#define GPFSEL4 0x10 // (pins 40-49)
#define GPFSEL5 0x14 // (pins 50-53)
#define GPSET0  0x1C // Set pins high
#define GPCLR0  0x28 // Set pins low
#define GPLEV0  0x34 // Read pin levels

/*
gpio_read
Reads the current value of a GPIO pin

@pin: int number (0-53)

returns 0 or 1 on success, negative error code on fail
*/
SYSCALL_DEFINE1(gpio_read, int, pin)
{
	void __iomem *gpio_base;
	u32 level_reg;
	int value;

	// Validate pin number
	if (pin < 0 || pin > 53) {
		printk(KERN_ERR "gpio_read: Invalid pin %d (Must be 0-53)", pin);
		return -EINVAL;
	}

	// Remap GPIO registers into kernel's virtual memory
	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_read: Failed to map GPIO memory\n");
		return -ENOMEM;
	}

	// Read the level register which contains state of all pins
	level_reg = ioread32(gpio_base + GPLEV0);

	// Extract the specific pin's value (0 or 1)
	value = (level_reg >> pin) & 1;

	// Unmap the memory
	iounmap(gpio_base);

	printk(KERN_INFO "gpio_read: Pin %d = %d\n", pin, value);

	return value;
}
