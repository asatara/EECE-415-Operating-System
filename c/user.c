/* user.c : User processes
 */
#include <xeroskernel.h>
extern void producer(void);
void handler(void* frame);
void handler2(void* frame);
void (*p)(void) = producer;

void root(void) {
	int pid = sysgetpid();
	sysputs2("Root (Process %d) is alive.\n", pid);
	syscreate(producer, 4000);
	void (*old_handler)(void*);
	void (*new_handler)(void*) = handler;
	void (*new_handler2)(void*) = handler2;
	int result = syssighandler(1, new_handler, &old_handler);
	kprintf("syssighandler returned: %d\n", result);
	result = syssighandler(0, new_handler2, &old_handler);
	kprintf("syssighandler returned: %d\n", result);
	result = syssigwait();
	kprintf("Process 0: syssigwait returned: %d\n", result);
	for(;;);
	sysputs("Done\n");
}

void idle(void) {
	for(;;);
}



extern void producer(void) {
	int pid = sysgetpid();
	sysputs2("Process %d is alive.\n", pid);
	int result = syskill(0, 0);
	kprintf("syskill returned: %d\n", result);
	result = syskill(0, 1);
	kprintf("syskill returned: %d\n", result);
	sysyield();
	
}

void handler(void* frame) {
	kprintf("Signal For handler1 received\n");
	syssigwait();
}

void handler2(void* frame) {
	kprintf("Signal For handler2 received\n");
}




