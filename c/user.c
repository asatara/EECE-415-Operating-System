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
	fd = sysopen(1);

	void (*oldhandler)(void*);
	void (*newhandler)(void*) = handler;
	syssighandler(18, newhandler, &oldhandler);
	syscreate(p, 3000);
	result = sysread(fd, buff, 10);
	sysputs2("The result of sysread is %d\n", result);

	sysclose(fd);
}

void idle(void) {
	for(;;);
}

extern void producer(void) {
	syssleep(1000);
	syskill(0, 20);
	syskill(0, 18);
	}

void handler(void* frame) {
	kprintf("Signal 18 received\n");
}

void handler2(void* frame) {
	kprintf("Signal For handler2 received\n");
}

