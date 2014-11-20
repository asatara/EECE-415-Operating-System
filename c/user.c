/* user.c : User processes
 */
#include <xeroskernel.h>
extern void producer(void);
void (*p)(void) = producer;
const int num = 4;

void root(void) {
	int i = 0;
	struct Msg* msgs[num];
	int pid = sysgetpid();
	sysputs2("Root (Process %d) is alive.\n", pid);
	sysportcreate(14);
	for (i = 0; i < num; i++) {
		syscreate(p, 4000);
	}
	for (i = 0; i < num; i++) {
		sysportrecv(14, (void**)&msgs[i]);
		sysputs4("Process %d received a msg: process %d owns port %d\n", pid, msgs[i]->pid, msgs[i]->port);
	}
	syssleep(4000);
	unsigned int a = 10000;
	unsigned int b = 7000;
	unsigned int c = 20000;
	unsigned int d = 27000;
	sysportsend(msgs[2]->port, (void*)&a);
	sysportsend(msgs[0]->port, (void*)&b);
	sysportsend(msgs[1]->port, (void*)&c);
	sysportsend(msgs[3]->port, (void*)&d);
	
	sysputs3("Process %d is attempting to send a message to port %d.\n", pid, msgs[3]->port);
	int result = sysportsend(msgs[3]->port, (void*)&a);
	sysputs3("Process %d received the result from sysportsend:  %d\n", pid, result);
	sysputs3("Process %d is attempting to send a message to port %d.\n", pid, msgs[2]->port);
	result = sysportsend(msgs[2]->port, (void*)&a);
	sysputs3("Process %d received the result from sysportsend:  %d\n", pid, result);
	sysputs("Done\n");
}

void idle(void) {
	for(;;);
}



extern void producer(void) {
	int pid = sysgetpid();
	sysputs2("Process %d is alive.\n", pid);
	int port = sysportcreate(0);
	struct Msg msg;
	msg.pid = pid;
	msg.port = port;
	sysportsend(14, (void*)&msg);
	syssleep(5000);
	int* recv_buff;
	sysportrecv(port, (void**)&recv_buff);
	sysputs4("Process %d received a msg. Process %d will sleep for %d milliseconds.\n", pid, pid, *recv_buff);
	syssleep(*recv_buff);
	sysputs2("Process %d has finished sleeping. It will now terminate.\n", pid);

}
