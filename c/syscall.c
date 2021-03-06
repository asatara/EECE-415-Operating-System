/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>
struct PCB* curr_running_process;

void sysputs(char* str) {
	syscall(SYS_PUTS, str);
}

void sysputs2(char* format, int a) {
	char* str = NULL;
	sprintf(str, format, a);
	syscall(SYS_PUTS, str);
}

void sysputs3(char* format, int a, int b) {
	char* str = NULL;
	sprintf(str, format, a, b);
	syscall(SYS_PUTS, str);
}

void sysputs4(char* format, int a, int b, int c) {
	char* str = NULL;
	sprintf(str, format, a, b, c);
	syscall(SYS_PUTS, str);
}

int syscreate( void (*func)(void), int stack ) {
	return syscall(SYS_CREATE, func, stack);
}

void sysyield( void ) {
	syscall(SYS_YIELD);
}

void sysstop( void ) {
	syscall(SYS_STOP);
}

extern int sysportcreate(int port) {
    return syscall(SYS_PORT_CREATE, port);
}

extern int sysportdelete(int port) {
    return syscall(SYS_PORT_DELETE, port);
}

extern int sysportsend(int port, void *msg) {
    return syscall(SYS_SEND, port, msg);
}

extern int sysportrecv(int port, void **msg) {
    return syscall(SYS_PORT_RECV, port, msg);
}

unsigned int syssleep(unsigned int milliseconds) {
	return syscall(SYS_SLEEP, milliseconds);
}

unsigned int sysgetpid(void) {
    return syscall(SYS_GET_PID);
}

int syskill(int pid, int sigNumber) {
    return syscall(SYS_KILL, pid, sigNumber);
}

int syssighandler(int signal, void (*newhandler)(void*), void (**oldhandler)(void*)) {
	return syscall(SYS_SIG_HANDLE, signal, newhandler, oldhandler);
}

void syssigreturn(void* old_sp) {
	syscall(SYS_SIG_RETURN, old_sp);
}

int syssigwait(void) {
	return syscall(SYS_SIG_WAIT);
}

int sysopen(int device_no) {
    return syscall(SYSOPEN, device_no);
}

int sysclose(int fd) {
    return syscall(SYSCLOSE, fd);
}

int syswrite(int fd, void *buff, int bufflen) {
    return syscall(SYSWRITE, fd, buff, bufflen);
}

int sysread(int fd, void *buff, int bufflen) {
    return syscall(SYSREAD, fd, buff, bufflen);
}

int sysioctl(int fd, unsigned long command, ...) {
    return syscall(SYSIOCTL, fd, command);
}

int syscall(int call, ...) {
	int result;
	va_list argv;
	va_start(argv, call);
	va_list* argp = &argv;
	#ifdef PROCESS_FLOW_LOG
		kprintf("INT 80 Called!\n");
		PAUSE;
	#endif
	__asm__ volatile(
		"movl %1, %%eax;"
		"movl %2, %%edx;"
		"INT $80;"
		"movl %%eax, %0;"
	: "=m"(result)
	: "m"(call), "m"(argp) // move call into eax, arg pointer to edx
	:"%eax", "%edx"
	);
	return result;
}
