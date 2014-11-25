/* Device independant calls
*/

#include <xeroskernel.h>

int getFdbyDeviceno(int device_no);
void printFDT(struct PCB *pcb);


/* Returns the file descriptor for the device the process opened. If the open
 * failed, return -1.
 */
int di_open(struct PCB* pcb, int device_no) {
    int i;
    for(i = 0; i < FDT_SIZE; i++) {
       if(pcb->fdt[i] == 0) {
           devsw device = device_table[device_no];
           kprintf("Opening device: %s\n", device.dvname);
           device.dvopen();
           pcb->fdt[i] = &device;
           printFDT(pcb);
           return i;
       }
    }
    return -1;
}

/* Return 0 if the file is successfully closed. Return -1 on error.
 */
int di_close(struct PCB *pcb, int fd) {
    if(fd >= 0 && fd < FDT_SIZE) {
        pcb->fdt[fd] = 0;
        printFDT(pcb);
        return 0;
    } else {
        return -1;
    }
    
}

void di_write(void) {

}

void di_read(void) {

}

void di_ioctl(void) {

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


void Buffer_Read(Buffer* buff, char* target){

	if (buff->isFull)
		buff->isFull = FALSE;

	*target = buff->buff[buff->tail];
	if (buff->head != buff->tail) {
		buff->tail++;
		buff->tail = buff->tail % (BUFF_SIZE);
	}
	return;
}

void Buffer_Write(Buffer* buff, char data) {
	if (buff->isFull)
		return;

	buff->buff[buff->head] = data;

	if (buff->tail == ( (buff->head + 1) % BUFF_SIZE))
		buff->isFull = TRUE;
	else {
		buff->head++;
		buff->head = buff->head % (BUFF_SIZE);
	}
}
