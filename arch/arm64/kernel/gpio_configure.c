#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/io.h>
#include <linux/errno.h>
#include "gpio_common.h"

/*
gpio_configure
Sets the GPIO pin as INPUT or OUTPUT

@pin: int number (0-53)
@mode: 0, 1 (input / output)

returns 0 on success, negative error code on fail
*/
SYSCALL_DEFINE2(gpio_configure, int, pin, int, mode)
{
	__u32 reg_offset, bit_shift, value;

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

	// Remap GPIO registers into kernels virtual memory using helper function
	// This function will initialize the gpio_base and a pointer to the virtual
	// memory address can then be used
	if (gpio_hw_init())
		return -ENOMEM;

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

	printk(KERN_INFO "gpio_configure: Pin %d configured as %s\n", pin,
	       mode == GPIO_OUTPUT ? "OUTPUT" : "INPUT");

	return 0;
}
