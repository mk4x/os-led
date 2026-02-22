#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>

// GPIO config for PI 2 Zero
#define GPIO_BASE 0x3F000000
#define GPIO_SIZE 0x1000

// GPIO register offsets
#define GPFSEL0 0x00 // (pins 0-9)
#define GPFSEL1 0x04 // (pins 10-19)
#define GPFSEL2 0x08 // (pins 20-29)
#define GPFSEL3 0x0C // (pins 30-39)
#define GPFSEL4 0x10 // (pins 40-49)
#define GPFSEL5 0x14 // (pins 50-53)
#define GPSET0 0x1C // Set pins high (0-31 pins)
#define GPSET1 0x20 // Set pins high (32-53 pins)
#define GPCLR0 0x28 // Set pins low (0-31 pins)
#define GPCLR1 0x2C // Set pins low (0-53 pins)

/*
gpio_write
Sets a GPIO pin HIGH (1) or LOW (0)

@pin: int number (0-53)
@value: 0 or 1

returns 0 on success, negative error code on fail
*/
SYSCALL_DEFINE2(gpio_write, int, pin, int, value)
{
	void __iomem *gpio_base;

	// Validate pin number
	if (pin < 0 || pin > 53) {
		printk(KERN_ERR "gpio_write: Invalid pin %d (Must be 0-53)",
		       pin);
		return -EINVAL;
	}

	// Validate value (must be 0 or 1)
	if (value != 0 && value != 1) {
		printk(KERN_ERR "gpio_write: Invalid value %d (Must be 0 or 1)",
		       value);
		return -EINVAL;
	}

	// Remap GPIO registers into kernel's virtual memory
	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_write: Failed to map GPIO memory\n");
		return -ENOMEM;
	}
	// Set bit, wraparound if more than 32 to next GP function
	u32 bit = 1 << (pin % 32);

	// Set or clear the pin based on value
	if (pin < 32) {
		if (value)
			iowrite32(bit, gpio_base + GPSET0);
		else
			iowrite32(bit, gpio_base + GPCLR0);
	} else {
		if (value)
			iowrite32(bit, gpio_base + GPSET1);
		else
			iowrite32(bit, gpio_base + GPCLR1);
	}

	// Unmap the memory
	iounmap(gpio_base);

	return 0;
}
