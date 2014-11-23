/* user.c : User processes
 */
#include <xeroskernel.h>
extern void producer(void);
void handler(void* frame);
void handler2(void* frame);
void (*p)(void) = producer;

void root(void) {
	sysputs("hi\n");
	int pid = sysgetpid();
	sysputs2("Root (Process %d) is alive.\n", pid);
	syscreate(producer, 4000);
	sysportcreate(10);
	sysyield();
	void (*old_handler)(void*);
	void (*new_handler)(void*) = handler;
	void (*new_handler2)(void*) = handler2;
	int result = syssighandler(1, new_handler, &old_handler);
	kprintf("syssighandler returned: %d\n", result);
	result = syssighandler(0, new_handler2, &old_handler);
	kprintf("syssighandler returned: %d\n", result);
	void* buff;
	result = sysportsend(5, buff);
	kprintf("Process 0: syssigwait returned: %d\n", result);
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




