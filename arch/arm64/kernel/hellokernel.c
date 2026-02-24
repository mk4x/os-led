#include "linux/kernel.h"
#include "linux/syscalls.h"

SYSCALL_DEFINE1(hellokernel, long, param)
{
	printk(KERN_EMERG "Your kernel says hello %d times!", (int)param);
	return 0;
}