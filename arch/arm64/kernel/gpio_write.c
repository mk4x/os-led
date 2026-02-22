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
		printk(KERN_ERR "gpio_write: Invalid pin %d (Must be 0-53)", pin);
		return -EINVAL;
	}

	// Validate value (must be 0 or 1)
	if (value != 0 && value != 1) {
		printk(KERN_ERR "gpio_write: Invalid value %d (Must be 0 or 1)", value);
		return -EINVAL;
	}

	// Remap GPIO registers into kernel's virtual memory
	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_write: Failed to map GPIO memory\n");
		return -ENOMEM;
	}

	// Set or clear the pin based on value
	if (value == 1) {
		// Set pin HIGH using GPSET register
		iowrite32(1 << pin, gpio_base + GPSET0);
		printk(KERN_INFO "gpio_write: Pin %d set HIGH\n", pin);
	} else {
		// Set pin LOW using GPCLR register
		iowrite32(1 << pin, gpio_base + GPCLR0);
		printk(KERN_INFO "gpio_write: Pin %d set LOW\n", pin);
	}

	// Unmap the memory
	iounmap(gpio_base);

	return 0;
}
