/* user.c : User processes
 */
#include <xeroskernel.h>
#include <kbd.h>
extern void producer(void);
void handler(void* frame);
void handler2(void* frame);
void (*p)(void) = producer;

void root(void) {
    int fd = sysopen(0);
    sysputs2("Root got fd %d\n", fd);
	char buff[10];
	sysread(fd, buff, 10);
	sysputs2("Buffer received: %s\n", buff);
    sysclose(fd);
	sysputs("Done\n");
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

