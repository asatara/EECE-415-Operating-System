/* create.c : create a process
 */

#include <xeroskernel.h>



unsigned short getCS( void );

// Returns a PCB pointer to the next free pcb in the global process table.
struct PCB* find_next_free_pcb(int* pid);

/* func is a function pointer to the start of the process. stack_size is the
 * size of stack to be allocated to the process. If successful return the PID
 * of the new process, else return -1.
 */

int create(void (*func)(void), int stack_size) {
	int process_id = -1;

	struct PCB* pcb = find_next_free_pcb(&process_id);
	if (pcb == (struct PCB*)SYSERR) {
		return SYSERR;
	}
	
	unsigned long top_of_stack = (unsigned long)kmalloc(stack_size);
	pcb->top_of_stack = top_of_stack;
	
	// align with 16 bytes
	unsigned long real_stack_size =  ((stack_size/16) + ((stack_size % 16)?1:0)) * 16;

	void (*_sysstop)(void) = sysstop;

	// put context at bottom of stack
	unsigned long context_ptr = top_of_stack + (unsigned long)real_stack_size - sizeof(struct context_frame) - sizeof(unsigned long);

	// make return addr point to sysstop
	unsigned long* return_addr;
	return_addr = (unsigned long*)(context_ptr + sizeof(struct context_frame));
	*return_addr = (unsigned long)_sysstop;

	pcb->context = (struct context_frame*) context_ptr;
	pcb->context->edi = 0;
	pcb->context->esi = 0;
	pcb->context->ebp = 0;
	pcb->context->esp = (unsigned int)context_ptr;
	pcb->context->ebx = 0;
	pcb->context->edx = 0;
	pcb->context->ecx = 0;
	pcb->context->eax = 0;
	pcb->context->iret_eip = (unsigned int)func;
	pcb->context->iret_cs = getCS();
	pcb->context->eflags = 0x00003200;
	//pcb->context->eflags = 0;
	pcb->state = READY;
	pcb->pid = process_id;
	pcb->next = NULL;
    pcb->ports = NULL;

	// idle process doesn't live in ready queue
	if (func == idle) {
		idle_process = pcb;
	} else {
		addToQueue(&ready_queue, pcb);
	}

	kprintf_log(CREATE_LOG, 0, "Creating new process with pid %d and size %d. "
			"Adding process to ready queue.\n", process_id, real_stack_size);
	return process_id;

}


// finds the next free pcb
// the pid variable is passed in and updated
// the pid of each process is the form is PXX, where P is the index 
//		of the process in the GPT and XX varies.
//		Each time a new process is created in an index slot, the 
//		value of XX increases until it resets when it reaches 100.
//		This provides us a fast mechanism to search for PCBs by pid
//		while ensuring no lifetime issues
struct PCB* find_next_free_pcb(int* pid) {
	int i;
	for (i = 0; i < MAX_NUMBER_OF_PCBS; i++) {
		if (global_process_table[i].pid == -1) {
			global_process_pid_table[i] = (global_process_pid_table[i] + 1) % 100;
			*pid = i * 100 + global_process_pid_table[i];
			return &global_process_table[i];
		}
	}
	kprintf("ERROR: Max number of processes reached.\n");
	return (struct PCB*)SYSERR;
}


