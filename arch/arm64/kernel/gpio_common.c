#include <linux/kernel.h>
#include <linux/io.h>
#include "gpio_common.h"

void __iomem *gpio_base;

// Helper function to keep one global gpio_base address
int gpio_hw_init(void)
{
	if (gpio_base)
		return 0; // already mapped

	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base) {
		printk(KERN_ERR "gpio_hw_init: Failed to map GPIO\n");
		return -ENOMEM;
	}

	return 0;
}

// Unmap the gpio
void gpio_hw_exit(void)
{
	if (gpio_base) {
		iounmap(gpio_base);
		gpio_base = NULL;
	}
}
