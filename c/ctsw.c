/* ctsw.c : context switcher
 */

#include <xeroskernel.h>

extern void _ISREntryPoint(void);
extern void _CommonEntryPoint(void);
extern void _TimerEntryPoint(void);
extern void _KeyboardEntryPoint(void);

static void* k_stack;
static unsigned int ESP;
static int rc;
static int interrupt;

void context_init(void) {
	set_evec(80, (unsigned long)_ISREntryPoint);
	set_evec(32, (unsigned long)_TimerEntryPoint);
    set_evec(33, (unsigned long)_KeyboardEntryPoint);
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
    "_KeyboardEntryPoint:" 
		"cli;" // turn off interrupts
		"pusha;" // save registers
        "movl $1, %%ecx;"  // indicate interrupt
        "movl $1, %%eax;"  // keyboard interrupt  
        "jmp _CommonEntryPoint;"
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
		kprintf("Switched back to kernel from process %d. Returning to dispatcher\n", p->pid);
	#endif

	if (interrupt) {
		// preserve the value of eax so we can restore it later
		p->rc = p->context->eax;
	}
    
    if(rc == 1 && interrupt == 1) {
       kprintf("\n\nKeyboard interrupt!\n\n"); 
    }

	if (FALSE)
		k_stack = 0;

	return rc;
}
