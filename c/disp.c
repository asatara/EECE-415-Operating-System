/* disp.c : dispatcher
 */

#include <xeroskernel.h>
#include <stdarg.h>

// return a pointer to the next process in the ready queue
struct PCB* find_next_ready_process(void);

// Prints out the list of pcbs found in each queue.
void queue_dump(void);
void _queue_dump(struct PCB* queue);
struct PCB* find_pcb(int pid);
extern void end_of_intr(void);

extern void dispatch(void) {
	int request;
	kprintf_log(DISP_LOG, 0, "Entering dispatcher.\n");
	struct PCB* process = find_next_ready_process();
	request = contextswitch(process);
	
	for(;;) {
		switch(request) {
			case(SYS_YIELD): {
				kprintf_log(DISP_LOG, SHORT, "Process %d requested service SYS_YIELD."
					" Adding process to ready queue\n",process->pid);
				addToQueue(&ready_queue, process);
				process = find_next_ready_process();
				request = contextswitch(process);
				break;
			}
			case(SYS_CREATE): {
				kprintf_log(DISP_LOG, SHORT, "Process %d requested service SYS_CREATE.\n", process->pid);
				va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				void (*new_process)(void) = va_arg(argv, unsigned long);
				int size = va_arg(argv, int);
				int pid = create(new_process, size);
				process->rc = pid;
				kprintf_log(DEMO_LOG, NONE, "Process %d requested service SYS_CREATE. Process %d has been created.\n", process->pid, pid);
				request = contextswitch(process);
				break;
			}
			case(SYS_STOP): {
				kprintf_log(DISP_LOG, SHORT, "Process %d requested service SYS_STOP. Terminating process\n", process->pid);
                kkillproc(process);
				process = find_next_ready_process();
				request = contextswitch(process);
				break;
			}
			case(SYS_PUTS): {
				va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				char* str = va_arg(argv, char*);
				kprintf("%s", str);
				request = contextswitch(process);
				break;
			}
			case(TIMER_INT): {
				tick();
				process->state = READY;
				addToQueue(&ready_queue, process);
				process = find_next_ready_process();
				end_of_intr();
				request = contextswitch(process);
				break;
			}
            case(SYS_PORT_CREATE): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_PORT_CREATE.\n", process->pid);
                va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				int port = va_arg(argv, int);
                process->rc = kcreateport(port, process);
				request = contextswitch(process);
                break;
            }
            case(SYS_PORT_DELETE): {
				kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_PORT_DELETE.\n", process->pid);
                va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				int port = va_arg(argv, int);
				process->rc = kdestroyport(port, process);
				request = contextswitch(process);
                break;
            }
            case(SYS_SEND): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_SEND.\n", process->pid);
                va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				int port = va_arg(argv, int);
                void *msg = va_arg(argv, void*);
                process->rc = ksendtoport(port, msg, process);
				if (process->rc == -2) {
					process = find_next_ready_process();
				}
					request = contextswitch(process);
                break;
            }
            case(SYS_PORT_RECV): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_PORT_RECV.\n", process->pid);
                va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				int port = va_arg(argv, int);
                void **msg = va_arg(argv, void**);
                process->rc = krecvfromport(port, msg, process);
				if (process->rc == -2) {
					process = find_next_ready_process();
				}
				request = contextswitch(process);
                break;
            }
			case(SYS_SLEEP): {
				va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				unsigned int milliseconds = va_arg(argv, unsigned int);
				kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_SLEEP for %d milliseconds.\n", process->pid, milliseconds);
				if (milliseconds > 0 ) {
					process->ticks = milliseconds / 10;
					process->state = SLEEP;
					sleep(process);
					process = find_next_ready_process();
				}
				request = contextswitch(process);
				break;
			}
            case(SYS_GET_PID): {
				kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_GET_PID.\n", process->pid);
                process->rc = process->pid;
                request = contextswitch(process);
                break;
			}
            case(SYS_KILL): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYS_KILL.\n", process->pid);
                va_list* argp = (va_list*)process->context->edx;
				va_list argv = *argp;
				unsigned int target_pid = va_arg(argv, unsigned int);
                int ret = -1;
                if(target_pid == process->pid) {
                    kprintf("ERROR: Process tried to kill itself!");
                    ret = -2;
                } else if(doesProcExist((int)target_pid) == FALSE) {
                    kprintf("ERROR: Tried to kill a process that doesn't exist!");
                    dump_gpt();
                    ret = -1;
                } else {
                    kprintf("Killing process %d.\n", target_pid);
                    kkillproc(find_pcb(target_pid));
                    ret = 1;
                }
                process->rc = ret;
                request = contextswitch(process);
                break;
            }
            case(SYSOPEN): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYSOPEN.\n", process->pid);
                break;
            }
            case(SYSCLOSE): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYSCLOSE.\n", process->pid);
                break;
            }
            case(SYSWRITE): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYSWRITE.\n", process->pid);
                break;
            }
            case(SYSREAD): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYSREAD.\n", process->pid);
                break;
            }
            case(SYSIOCTL): {
                kprintf_log(DISP_LOG, SHORT,"Process %d requested system call SYSIOCTL.\n", process->pid);
                break;
            }
			default: {
				kprintf("Incorrect SYS_CALL %d. Returning to process.\n", request);
				PAUSE;
				request = contextswitch(process);
			}
		}

	}

}

struct PCB* find_next_ready_process(void) {
	if (ready_queue == NULL)
		return idle_process;
	
	struct PCB* head = ready_queue;
	ready_queue = head->next;
	head->next = NULL;
	kprintf_log(DISP_LOG, SHORT,"Next ready process is process %d."
		" Switching to process %d\n", head->pid, head->pid);
	return head;
}



struct PCB* find_pcb(int pid) {
	int index = (pid / 100);
	return &global_process_table[index];
}

Bool doesProcExist(int pid) {
    if(pid < 0) {
        return FALSE;
    }
    struct PCB *proc = find_pcb(pid);
    if(pid == proc->pid) {
        return TRUE;
    }
    return FALSE;
}
		
Bool unblockProcess(struct PCB** queue, int portNumber) {
	struct Port* port = &global_port_table[portNumber];
    struct PCB *proc = port->owner;
	if (proc == NULL) 
		return FALSE;
    struct PCB* result = removeFromQueue(queue, proc);

	if (result == NULL)
		return FALSE;
	else {
		kprintf_log(PORT_LOG, SHORT, "Unblocking pid %d\n", result->pid);
		return TRUE;
	}
}

// Kill a process and clean up its owned resources.
void kkillproc(struct PCB *proc) {
    proc->state = STOPPED;
    proc->pid = -1;
    kdestroyallports(proc);
    kfree((void *)proc->top_of_stack);
}

void _queue_dump(struct PCB* queue) {
	if (queue == NULL) {
		kprintf("queue empty.\n");
		return;
	}
	struct PCB* iterator = queue;
	int i = 0;
	do {
		if (queue == sleep_queue)
			kprintf("\t%d - pcb %d - ticks %d\n", i, iterator->pid, iterator->ticks);
		else
			kprintf("\t%d - pcb %d.\n", i, iterator->pid);
		i++;
		iterator = iterator->next;
	} while (iterator != NULL);
}

void queue_dump() {
	kprintf("Printing ready queue...\n");
	_queue_dump(ready_queue);
	kprintf("Printing sleep queue...\n");
	_queue_dump(sleep_queue);
	kprintf("Printing blocked queue...\n");
	_queue_dump(blocked_queue);
	kprintf("Current running process %d\n", curr_running_process->pid);
}

void addToQueue(struct PCB** queue, struct PCB* process) {
	if (*queue == NULL) {
		*queue = process;
		return;
	}

	if (process == idle_process)
		return;

	if (*queue == ready_queue) {
		process->state = READY;
	}

	if (*queue == blocked_queue) {
		process->state = BLOCKED;
	}
	
	struct PCB* iterator = *queue;
	while (iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = process;
	process->next = NULL;
}

struct PCB* removeFromQueue(struct PCB** queue, struct PCB* process) {
	if (process == idle_process) {
		return NULL;
    }
    
	if (*queue == NULL) {
		return NULL;
	}

    struct PCB* iterator = *queue;
    struct PCB* prev = NULL;
    while(iterator != NULL) {
        if(iterator == process || process == NULL) {
            if(prev == NULL) { // 'process' is the first node in the list.
                *queue = iterator->next;
            } else { // 'process' is somewhere in the middle or on the end.
                prev->next = iterator->next;
            }
            iterator->next = NULL;
            return iterator;
        }
        prev = iterator;
        iterator = iterator->next;
    }
    return NULL;
}

