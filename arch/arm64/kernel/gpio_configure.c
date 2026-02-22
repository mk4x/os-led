#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>

// GPIO config for PI 2 Zero
#define GPIO_BASE 0x3F000000
#define GPIO_SIZE 0x1000

// GPIO register offsets (function select)
#define GPFSEL0 0x00 // (pins 0-9)
#define GPFSEL1 0x04 // (pins 10-19)
#define GPFSEL2 0x08 // (pins 20-29)
#define GPFSEL3 0x0C // (pins 30-39)
#define GPFSEL4 0x10 // (pins 40-49)
#define GPFSEL5 0x14 // (pins 50-53)

// Pin modes
#define GPIO_INPUT 0
#define GPIO_OUTPUT 1

/*
gpio_configure
Sets the GPIO pin as INPUT or OUTPUT

@pin: int number (0-53)
@mode: 0, 1 (input / output)

returns 0 on success, negative error code on fail
*/
SYSCALL_DEFINE2(gpio_configure, int, pin, int, mode)
{
	void __iomem *gpio_base;
	u32 reg_offset, bit_shift, value;

	// Validate pin number
	if (pin < 0 || pin > 53) {
		printk(KERN_ERR "gpio_configure: Invalid pin %d (Must be 0-53)",
		       pin);
		return -EINVAL;
	}

	// Validate mode
	if (mode != GPIO_INPUT && mode != GPIO_OUTPUT) {
		printk(KERN_ERR "gpio_configure: Invalid mode %d (must be 0,1)",
		       mode);
		return -EINVAL;
	}

	// Remap GPIO registers into kernels virtual memory
	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_configure: Failed to map GPIO memory\n");
		return -ENOMEM;
	}

	// Now calculate which GPFSEL register to use
	// Each register controls 10 pins
	// Each register is 4 bytes apart
	reg_offset = (pin / 10) * 4;

	// Calculate bit position in the register
	// Each pin has 3 bits for function select
	// 000 is input, 001 is output, rest are others
	bit_shift = (pin % 10) * 3;

	// Read what is in the register beforehand
	value = ioread32(gpio_base + reg_offset);

	// Clear the 3 bits there
	value &= ~(7 << bit_shift); // 7 = 111 in binary

	// Set mode bits to the argument mode
	value |= (mode << bit_shift);

	// Write modified value to memory
	iowrite32(value, gpio_base + reg_offset);

	// Unmap the memory
	iounmap(gpio_base);

	printk(KERN_INFO "gpio_configure: Pin %d configured as %s\n", pin,
	       mode == GPIO_OUTPUT ? "OUTPUT" : "INPUT");

	return 0;
}
