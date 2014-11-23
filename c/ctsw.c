/* ctsw.c : context switcher
 */

#include <xeroskernel.h>

extern void _ISREntryPoint(void);
extern void _CommonEntryPoint(void);
extern void _TimerEntryPoint(void);

static void* k_stack;
static unsigned int ESP;
static int rc;
static int interrupt;

void context_init(void) {
	set_evec(80, (unsigned long)_ISREntryPoint);
	set_evec(32, (unsigned long)_TimerEntryPoint);
}

system_call contextswitch(struct PCB* p) {
	curr_running_process = p;
	p->state = RUNNING;
	p->next = NULL;
	p->context->eax = p->rc;
	ESP = p->context->esp;
	__asm__ volatile(
		"pushf;"
		"pusha;"
		"movl %%esp, k_stack;"
		"movl ESP, %%esp;"
		"popa;"
		"iret;"
	"_TimerEntryPoint:" 
		"cli;" // turn off interrupts
		"pusha;" // save registers
		"movl $1, %%ecx;" // indicate interrupt
		"movl $9, %%eax;" // indicate TIMER_INT
		"jmp _CommonEntryPoint;"
	"_ISREntryPoint:"
		"cli;"
		"pusha;"
		"movl $0, %%ecx;" // indicate syscall
	"_CommonEntryPoint:"
		"movl %%esp, ESP;"
		"movl k_stack, %%esp;"
		"movl %%ecx, interrupt;"
		"movl %%eax, rc;" // move return value to rc
		"popa;"
		"popf;"
	:
	: 
	:"%eax", "%ecx"
	); 
	p->context = (struct context_frame*)ESP;
	p->context->esp = ESP;
	#ifdef PROCESS_FLOW_LOG
		kprintf("Switched back to kernel from process %d. Retuning to dispatcher\n", p->pid);
	#endif

	if (interrupt) {
		// preserve the value of eax so we can restore it later
		p->rc = p->context->eax;
	}

	if (FALSE)
		k_stack = 0;

	return rc;
}
