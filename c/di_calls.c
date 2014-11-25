/* Device independant calls
*/

#include <xeroskernel.h>


int getFdbyDeviceno(int device_no);
void printFDT(struct PCB *pcb);

extern struct PCB* find_next_ready_process(void);

/* Returns the file descriptor for the device the process opened. If the open
 * failed, return -1.
 */
int di_open(struct PCB* pcb, int device_no) {
    int i;
	int index = -1;
    for(i = 0; i < FDT_SIZE; i++) {
       if(pcb->fdt[i] == 0 && index == -1) {
           devsw device = device_table[device_no];
           kprintf("\tOpening device: %s\n", device.dvname);
           device.dvopen(pcb);
           pcb->fdt[i] = &device_table[device_no];
           index = i;
		   continue;
       }
	   if (pcb->fdt[i] != 0 ) {
		   kprintf("\tERROR: One device is already in use\n");
		   return -1;
	   }
    }

    return index;
}

/* Return 0 if the file is successfully closed. Return -1 on error.
 */
int di_close(struct PCB *pcb, int fd) {
    if(fd >= 0 && fd < FDT_SIZE) {
        pcb->fdt[fd] = 0;
        return 0;
    } else {
        return -1;
    }
    
}

void di_write(void) {

}

int di_read(struct PCB* pcb, int fd, void* buff, int len) {
	if (fd < 0 || fd > FDT_SIZE - 1)
		return -1;
	
	if (pcb->fdt[fd] == NULL)
		return -1;

	if (len < 0)
		return -1;

	devsw* d = pcb->fdt[fd];
	addToQueue(&blocked_queue, pcb);
	pcb->blocked_queue = &blocked_queue;
	pcb->state = BLOCKED;
	return (d->dvread)(pcb, buff, len, d->dvnum);

}

int di_ioctl(struct PCB* pcb, int fd, int command, va_list argv) {
	if (fd < 0 || fd > FDT_SIZE - 1)
		return -1;
	
	if (pcb->fdt[fd] == NULL)
		return -1;

	devsw* d = pcb->fdt[fd];
	int ret = (d->dvioctl)(command, argv);

	if (ret == 0) {
		addToQueue(&blocked_queue, pcb);
		pcb = find_next_ready_process();
	}

	return 0;
}


/* Helper functions */

// Translate the user supplied device_no into a fd.
int getFdbyDeviceno(int device_no) {
    if(device_no >= 0 && device_no < DEVICE_TABLE_SIZE) {
        return device_no;
    } else {
        return -1;
    }
}

void printFDT(struct PCB *pcb) {
    kprintf("Printing file descriptor table for proc %d\n", pcb->pid);
    int k;
    for(k = 0; k < FDT_SIZE; k++) {
        devsw *d = pcb->fdt[k];
        kprintf("\tfd %d = %d", k, d);
        if(d > 0) {
            kprintf("(%s)\n",d->dvname);
        } else {
            kprintf("\n");
        }
    }
}

void devswToString(devsw *d) {
   kprintf("devsw\n"); 
   kprintf("\tdvnum = %d\tdvname = %s\tdvopen = %d\tdvclose = %d\n",
        d->dvnum, d->dvname, d->dvopen, d->dvclose);
   kprintf("\tdvwrite = %d\tdvread = %d\tdvioctl = %d\n",
        d->dvwrite, d->dvread, d->dvioctl);
}



