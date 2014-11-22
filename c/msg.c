/* msg.c : messaging system (assignment 2)
 */

#include <xeroskernel.h>

struct Port global_port_table[MAX_NUMBER_OF_PORTS];
void pcbPortDump(struct PCB* pcb);
void portDump(int port_num);
void addPortToPCB(struct Port *port, struct PCB *pcb);
void portTableDump(void);
void destroyPort(struct PCB* pcb);
Bool procOwnsPort(struct PCB *proc, int port);
void getMsg(int port, void **msg);
int findNextFreePort(void);
void dumpMsgs(struct PCB* proc);
void unblockPort(struct Port* port); 


int kcreateport(int portNumber, struct PCB* pcb) {
    if(portNumber < 0 || portNumber > MAX_NUMBER_OF_PORTS) {
        kprintf("\tERROR: Invalid port number.\n"
				"\tPort must be greater than or equal to 0 and less than %d\n",
				MAX_NUMBER_OF_PORTS);
        return -2;
    } else if(global_port_table[portNumber].in_use == TRUE && portNumber != 0) {
        kprintf("\tERROR: Port %d is already in use.\n", portNumber);
        return -1; 
    } else {
		if (portNumber == 0) {
			portNumber = findNextFreePort();
			if (portNumber == -1) {
				kprintf("\tERROR: No more available port slots. Please free a port.\n");
				return -3;
			}
		}
		struct Port* port = &global_port_table[portNumber];
		port->in_use = TRUE;
		port->portNumber = portNumber;
		port->next = NULL;
		port->blocked_list = NULL;
        addPortToPCB(port, pcb);
		kprintf_log(PORT_LOG, 0, "\tPort %d created successfully\n", portNumber);
        return portNumber;
    }
    return -3;
}

int ksendtoport(int portNumber, void* msg, struct PCB *sending_proc) {
	struct Port* recv_port = &global_port_table[portNumber];

    if(portNumber <= 0 || portNumber > MAX_NUMBER_OF_PORTS || !recv_port->in_use) {
        kprintf_log(PORT_LOG, 0, "ERROR: Port %d doesn't exist or hasn't been created yet.\n",
			portNumber);
        return -1;
	}
	else if(procOwnsPort(sending_proc, portNumber)) {	
        kprintf("ERROR: cannot send to a port owned by the sender.\n");
        return -3;
    } else {
		struct PCB* recv_proc = recv_port->owner;
		// if receiver is waiting on the blocked_queue with same port number
		if(unblockProcess(&blocked_queue, recv_port->portNumber) == TRUE) {
			*((int**)recv_proc->msg) = (int*)msg;
			addToQueue(&ready_queue, recv_proc);
			recv_proc->rc = portNumber;
			return 1;
		} else {
			// add sending process to port blocked queue
			kprintf_log(PORT_LOG, 0, "Receiver unavailable. Blocking pid %d\n", 
				sending_proc->pid);
			sending_proc->state = BLOCKED;
			sending_proc->msg = msg;
			addToQueue(&recv_port->blocked_list, sending_proc);
			sending_proc->blocked_queue = recv_port->blocked_list;
			return -2;
		}

    }
}

int krecvfromport(int portNumber, void **msg, struct PCB *recv_proc) {
	struct Port* recv_port = &global_port_table[portNumber];
    if(portNumber < 0 || portNumber > MAX_NUMBER_OF_PORTS) {
        kprintf("ERROR: Invalid port number.\n");
        return -1;
	} else if (!recv_port->in_use) {
		kprintf("ERROR: Port %d does not exist yet.\n", portNumber);
		return -1;
	} else if (!procOwnsPort(recv_proc, portNumber)) {
		kprintf("ERROR: Process %d does not own port %d.\n", recv_proc->pid, portNumber);
		return -1;
	} else {
		struct PCB* send_proc;
		// if port number is 0, check blocked queue for every port
		if (portNumber == 0) {
			struct Port* iterator = recv_proc->ports;
			while(iterator != NULL) {
				send_proc = removeFromQueue(&iterator->blocked_list, NULL);
				if (send_proc != NULL)
					break;
				iterator = iterator->next;
			}
		} else {
			send_proc = removeFromQueue(&recv_port->blocked_list, NULL);
		}
		// if there is a sending process blocked on the poer queue
		if (send_proc != NULL) {
			addToQueue(&ready_queue, send_proc);
			send_proc->rc = 1;
			*msg = send_proc->msg;
			return portNumber;
		}
		else {
			kprintf_log(PORT_LOG, 0, "No sender available. Blocking pid %d\n", recv_proc->pid);
			recv_proc->msg = msg;
			recv_proc->state = BLOCKED;
			addToQueue(&blocked_queue, recv_proc);
			recv_proc->blocked_queue = blocked_queue;
			return -2;
		}
    }
}

int kdestroyport(int portNumber , struct PCB* pcb) {	
	struct Port* iterator = pcb->ports;
	struct Port* prev = NULL;
	struct Port* port_to_destroy = &global_port_table[portNumber];
	if (iterator == NULL || port_to_destroy->owner != pcb) {
		return -1;
	}

	do {
		if (iterator->portNumber == portNumber) {
			iterator->in_use = FALSE;
			if (prev == NULL) {
				pcb->ports = iterator->next;
			} else {
				prev->next = iterator->next;
			}
			iterator->next = NULL;
			unblockPort(iterator);
			return 1;
		}
		iterator = iterator->next;
	} while (iterator != NULL);
	return -1;
}

void kdestroyallports(struct PCB* pcb) {
	struct Port* iterator = pcb->ports;
	struct Port* prev = iterator;

	if (iterator == NULL)
		return;
	
	do {
		iterator->in_use = FALSE;
		unblockPort(iterator);
		iterator = iterator->next;
		prev->next = NULL;
		prev = iterator;
	} while ( iterator != NULL);
	prev->next = NULL;
	return;
}


/* 
 * Helper functions for kernel IPC calls.
 */

void pcbPortDump(struct PCB* pcb) {
	kprintf("Printing ports for pid %d\n", pcb->pid);
	struct Port* iterator = pcb->ports;
	while (iterator != NULL) {
		kprintf("\tPort %d\n", iterator->portNumber);
		iterator = iterator->next;
	}
}

void portDump(int port_num) {
    if(port_num < 0 || port_num > MAX_NUMBER_OF_PORTS) {
        kprintf("ERROR: Tried to dump invalid port number! PORT: %d", port_num);
    } else {
        struct Port port = global_port_table[port_num];
		kprintf("PORT: %d\tin_use: %d\tnext: %d\towner: %d\n", port_num, port.in_use, 
															 port.next, port.owner->pid);   
    }
}

void portTableDump(void) {
	kprintf("Dumpbing Port Table\n");
	int i;
	for (i = 0; i < MAX_NUMBER_OF_PORTS; i++) {
		struct Port port = global_port_table[i];
		kprintf("\tslot %d - in use %d\n", i, port.in_use);
	}
}

void addPortToPCB(struct Port *port, struct PCB *pcb) {
	port->owner = pcb;
	if (pcb->ports == NULL) {
		pcb->ports = port;
		return;
	}
	struct Port* iterator = pcb->ports;
	while (iterator->next != NULL) {
		iterator = iterator->next;
	}
	iterator->next = port;
	port->next = NULL;
}


// Returns TRUE if a port is owned by a process. Returns FALSE otherwise.
// Traverses the linked list of owned ports in a proc's pcb.
Bool procOwnsPort(struct PCB *proc, int portNumber) {
	if (portNumber == 0) 
		return TRUE;
	struct Port* port = &global_port_table[portNumber];
	if (port->owner == proc) {
		return TRUE;
	}
	else
		return FALSE;
}

int findNextFreePort(void) {
	int i;
	for (i = 1; i < MAX_NUMBER_OF_PORTS; i++) {
		if (global_port_table[i].in_use == FALSE)
			return i;
	}
	return -1;
}

void unblockPort(struct Port* port) {
	struct PCB* p;
	do {
		p = removeFromQueue(&port->blocked_list, NULL);
		if (p != NULL)
			addToQueue(&ready_queue, p);
	} while (p != NULL);
}

	

