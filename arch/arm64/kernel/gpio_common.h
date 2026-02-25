#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#include <linux/io.h>

// GPIO config for PI 2 Zero
#define GPIO_BASE 0x3F200000
#define GPIO_SIZE 0x1000

// GPIO register offsets
#define GPFSEL0 0x00 // (pins 0-9)
#define GPFSEL1 0x04 // (pins 10-19)
#define GPFSEL2 0x08 // (pins 20-29)
#define GPFSEL3 0x0C // (pins 30-39)
#define GPFSEL4 0x10 // (pins 40-49)
#define GPFSEL5 0x14 // (pins 50-53)

// Pin set functions
#define GPSET0 0x1C // Set pins high (0-31 pins)
#define GPSET1 0x20 // Set pins high (32-53 pins)
#define GPCLR0 0x28 // Set pins low (0-31 pins)
#define GPCLR1 0x2C // Set pins low (0-53 pins)

// Read pin level
#define GPLEV0 0x34 // Read pin levels (0-31)
#define GPLEV1 0x38 // Read pin levels (32-53)

// Pin modes
#define GPIO_INPUT 0
#define GPIO_OUTPUT 1

extern void __iomem *gpio_base;

int gpio_hw_init(void); // ioremap
void gpio_hw_exit(void); // iounmap

#endif