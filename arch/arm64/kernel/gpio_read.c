#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>
#include "gpio_common.h"

/*
gpio_read
Reads the current value of a GPIO pin

@pin: int number (0-53)

returns 0 or 1 on success, negative error code on fail
*/
SYSCALL_DEFINE1(gpio_read, int, pin)
{
	__u32 level_reg;
	int value;

	// Validate pin number
	if (pin < 0 || pin > 53) {
		printk(KERN_ERR "gpio_read: Invalid pin %d (Must be 0-53)",
		       pin);
		return -EINVAL;
	}

	// Remap GPIO registers into kernel's virtual memory
	// This function will initialize the gpio_base and a pointer to the virtual
	// memory address can then be used
	if (gpio_hw_init())
		return -ENOMEM;

	// Read the level register which contains state of all pins
	if (pin < 32)
		level_reg = ioread32(gpio_base + GPLEV0);
	else
		level_reg = ioread32(gpio_base + GPLEV1);

	// Extract the specific pin's value (0 or 1)
	value = (level_reg >> (pin % 32)) & 1;

	printk(KERN_INFO "gpio_read: Pin %d = %d\n", pin, value);

	return value;
}
