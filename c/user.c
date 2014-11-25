/* user.c : User processes
 */
#include <xeroskernel.h>
#include <kbd.h>
extern void producer(void);
void p2(void);
void p3(void);
void handler(void* frame);
void handler2(void* frame);
void (*p)(void) = producer;
void (*_p2)(void) = p2;
void (*_p3)(void) = p3;

void root(void) {
	sysputs("Greetings\n");
	char buff[10];
    int fd = sysopen(1);
	int result = sysread(fd, buff, 10);

	sysopen(1);
	sysopen(0);
	sysclose(fd);
	fd = sysopen(0);

	void (*oldhandler)(void*);
	void (*newhandler)(void*) = handler;
	syssighandler(18, newhandler, &oldhandler);
	syscreate(p, 3000);
	result = sysread(fd, buff, 10);
	sysputs2("Process 0: The result of sysread is %d\n", result);
	newhandler = handler2;
	syssighandler(18, newhandler, &oldhandler);
	syscreate(_p2, 2000);
	result = sysread(fd, buff, 10);
	sysputs2("Process 0: The result of sysread is %d\n", result);
	void (*oldhandler2)(void*);
	syssighandler(20, oldhandler, &oldhandler2);
	syscreate(_p3, 2000);
	result = sysread(fd, buff, 10);
	sysputs2("Process 0: The result of sysread is %d\n", result);
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

void p2(void) {
	syssleep(5000);
	syskill(0, 18);
}

void p3(void ) {
	syssleep(5000);
	syskill(0, 20);
}

void handler(void* frame) {
	kprintf("Process 0: Signal 18 received on handler 1\n");
}

void handler2(void* frame) {
	kprintf("Process 0: Signal 18 received on handler 2\n");
}

