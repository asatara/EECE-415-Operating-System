/* signal.c : (assignment 3)
 */

#include <xeroskernel.h>

int hibit(unsigned int n);

void sigtramp(void (*handler)(void *), void *context, void *old_sp) {
	handler(context);
	syssigreturn(old_sp);
}

int signal(int pid, int signalNum) {
	struct PCB* pcb = find_pcb(pid);
	kprintf("signaling %d\n", signalNum);

	// check if pid is valid
	if (pcb->pid == -1 || pcb->pid != pid)
		return -1;
	
	if (signalNum < 0 || signalNum > MAX_NUMBER_OF_SIGS - 1)
		return -2;

	if (pcb->signal_table[signalNum] == NULL) {
		kprintf("No signal handler found\n");
		return 0;
	}

	
	if (pcb->state == BLOCKED) {
		struct PCB* result = removeFromQueue(pcb->blocked_queue, pcb);
		if (result == NULL) {
			kprintf("ERROR: pcb status is blocked by cannot find pcb in blocked queue\n");			
		}

		pcb->rc = -452;
		pcb->state = READY;
		addToQueue(&ready_queue, pcb);
		kprintf("Adding pid %d to ready queue\n", pcb->pid);
	}

	
	
	if (pcb->state == SIG_WAIT) {
		pcb->state = READY;
		kprintf("Adding pid %d to ready queue\n", pcb->pid);
		addToQueue(&ready_queue, pcb);
		pcb->rc = signalNum;
	}

	// toggle signal bit on
	pcb->signal_controller |= 1 << signalNum;

	return 0;

}

void prepare_sigtramp(struct PCB* pcb) {
	int signal = hibit(pcb->signal_controller); // get highest bit
	pcb->signal_controller &= ~(1 << signal); // toggle signal off
	void* handler = pcb->signal_table[signal];
	kprintf("Preparing for signal %d\n", signal);
	int* old_sp;
	old_sp = (int*)pcb->context->esp;

	int* sp = old_sp;

	// push return value
	sp -= 1;
	*sp = pcb->rc;

	// push old_sp
	sp -= 1;
	*sp = (int)old_sp;

	// push context
	sp -= 1;
	*sp = (int)pcb->context;

	// push handler
	sp -= 1;
	*sp = (int)handler;

	// push return addr (never reached)
	sp -= 1;
	*sp = 0;

	// push sigtramp context
	struct context_frame* sigtramp_context;
	sp -= sizeof(struct context_frame) / sizeof(struct context_frame*);
	sigtramp_context = (struct context_frame*) sp;

	// initialize sigtramp context
	sigtramp_context->iret_eip = (int)sigtramp;
	sigtramp_context->esp = (int)old_sp;
	sigtramp_context->iret_cs = getCS();
	//sigtramp_context->eflags = 0x00003200;
	sigtramp_context->eflags = 0;
	sigtramp_context->edi = 0;
	sigtramp_context->esi = 0;
	sigtramp_context->ebp = 0;
	sigtramp_context->ebx = 0;
	sigtramp_context->edx = 0;
	sigtramp_context->ecx = 0;
	sigtramp_context->eax = 0;
	pcb->context->esp = (int)sp;
	
	pcb->is_in_signal = TRUE;

}

// returns index of highest bit set in integer
/*
int hibit(unsigned int n) {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1) - 1;
}
*/

int hibit(unsigned int n) {
	unsigned r = 0;

	while (n >>= 1) {
		    r++;
	}
	return r;
}
