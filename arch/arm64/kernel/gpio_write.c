#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>
#include "gpio_common.h"

/*
gpio_write
Sets a GPIO pin HIGH (1) or LOW (0)

@pin: int number (0-53)
@value: 0 or 1

returns 0 on success, negative error code on fail
*/
SYSCALL_DEFINE2(gpio_write, int, pin, int, value)
{
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
	// This function will initialize the gpio_base and a pointer to the virtual
	// memory address can then be used
	if (gpio_hw_init())
		return -ENOMEM;

	// Set bit, wraparound if more than 32 to next GP function
	__u32 bit = 1 << (pin % 32);

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

	return 0;
}