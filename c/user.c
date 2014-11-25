/* user.c : User processes
 */
#include <xeroskernel.h>
#include <kbd.h>
extern void producer(void);
void handler(void* frame);
void handler2(void* frame);
void (*p)(void) = producer;

void root(void) {
	sysputs("Greetings\n");
	char buff[10];
    int fd = sysopen(0);
	int result = sysread(fd, buff, 10);

	int fd2 = sysopen(1);
	int fd3 = sysopen(0);

	sysclose(fd);
}

void idle(void) {
	for(;;);
}

extern void producer(void) {
	int pid = sysgetpid();
	sysputs2("Process %d is alive.\n", pid);
	sysportcreate(5);
	sysyield();
	int result = syskill(0, 2);
	kprintf("syskill returned: %d\n", result);
	result = syskill(0, 3);
	kprintf("syskill returned: %d\n", result);
	sysyield();
}

void handler(void* frame) {
	kprintf("Signal For handler1 received\n");
	sysyield();
}

void handler2(void* frame) {
	kprintf("Signal For handler2 received\n");
}

