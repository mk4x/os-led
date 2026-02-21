#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <string.h>

// Syscalls
#define __NR_gpio_configure 464
#define __NR_gpio_write 465
#define __NR_gpio_read 466

// IO
#define GPIO_INPUT 0
#define GPIO_OUTPUT 1

// Counters for tests
int tests_succeed = 0, tests_failed = 0;

// Helper to test syscall from its return value
int assert_syscall(const char *test_desc, long syscall_result,
		   int should_succeed)
{
	if (should_succeed) {
		if (syscall_result >= 0) {
			tests_succeed++;
			printf("PASSED (%s) returned (%ld)\n", test_desc,
			       syscall_result);
		} else {
			tests_failed++;
			printf("FAILED (%s) returned (%ld)\n", test_desc,
			       syscall_result);
		}
	} else {
		if (syscall_result < 0) {
			tests_succeed++;
			printf("PASSED (correct reject) (%s) returned (%ld)\n",
			       test_desc, syscall_result);
		} else {
			tests_failed++;
			printf("FAILED (should be rejected) (%s) returned (%ld)\n",
			       test_desc, syscall_result);
		}
	}
	return 0;
}

void test_gpio_configure()
{
	printf("Testing gpio_configure()\n");
	// Valids
	assert_syscall("Configure pin 29 as OUTPUT",
		       syscall(__NR_gpio_configure, 29, GPIO_OUTPUT), 1);

	assert_syscall("Configure pin 17 as INPUT",
		       syscall(__NR_gpio_configure, 17, GPIO_INPUT), 1);

	assert_syscall("Configure pin 0 as OUTPUT",
		       syscall(__NR_gpio_configure, 0, GPIO_OUTPUT), 1);

	assert_syscall("Configure pin 53 as INPUT",
		       syscall(__NR_gpio_configure, 53, GPIO_INPUT), 1);

	// Invalids
	assert_syscall("Reject negative pins",
		       syscall(__NR_gpio_configure, -1, GPIO_OUTPUT), 0);

	assert_syscall("Reject pin > 53",
		       syscall(__NR_gpio_configure, 100, GPIO_OUTPUT), 0);

	assert_syscall("Reject invalid mode",
		       syscall(__NR_gpio_configure, 29, 5), 0);
	printf("Finished testing gpio_configure()");
}
void test_gpio_write()
{
	printf("Testing gpio_write() - Not yet implemented\n");
}

void test_gpio_read()
{
	printf("Testing gpio_read() - Not yet implemented\n");
}

void test_blink()
{
	printf("Testing blink - Not yet implemented\n");
}

int main()
{
	printf("Test Suite for GPIO\n");

	// Check if running as root
	if (geteuid() != 0) {
		fprintf(stderr, "Test Suite not running as root\n");
		exit(EXIT_FAILURE);
	}

	// Run all tests
	test_gpio_configure();
	test_gpio_write();
	test_gpio_read();
	test_blink();

	// Final results
	printf("Final Results: Test success/failed: %d / %d\n", tests_succeed,
	       tests_failed);

	// return 0 on success
	return tests_failed == 0 ? 0 : 1;
}
