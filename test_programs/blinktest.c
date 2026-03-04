#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

/*
 * Blink test for GPIO syscalls
 *
 * NOTE: The Pi's onboard LED (pin 29) may have system triggers enabled
 * by default (like disk activity). If blinking is erratic, disable with:
 *   sudo sh -c "echo none > /sys/class/leds/ACT/trigger"
 *
 * This allows our syscalls to have sole control of the LED.
 */

#define __NR_gpio_configure 464
#define __NR_gpio_write 465

#define GPIO_OUTPUT 1

int main() {
    if (geteuid() != 0) {
        fprintf(stderr, "Must run as root\n");
        return 1;
    }

    long ret = syscall(__NR_gpio_configure, 29, GPIO_OUTPUT);
    if (ret < 0) {
        fprintf(stderr, "configure failed: %s\n", strerror(errno));
        return 1;
    }

    printf("Blinking pin 29. Ctrl+C to stop.\n");
    while (1) {
        syscall(__NR_gpio_write, 29, 1);
        sleep(1);
        syscall(__NR_gpio_write, 29, 0);
        sleep(1);
    }

    return 0;
}
