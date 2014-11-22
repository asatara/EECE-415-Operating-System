/* Device independant calls
*/

#include <xeroskernel.h>

int getFdbyDeviceno(int device_no);
void printFDT(struct PCB *pcb);


/* Returns the file descriptor for the device the process opened. If the open
 * failed, return -1.
 */
int di_open(struct PCB* pcb, int device_no) {
    devsw device = device_table[device_no];
    int i;
    for(i = 0; i < FDT_SIZE; i++) {
       if(pcb->fdt[i] == 0) {
           pcb->fdt[i] = &device;
           printFDT(pcb);
           return i;
       }
    }
    return -1;
}

void di_close(void) {

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
    if(device_no >= 0 || device_no < DEVICE_TABLE_SIZE) {
        return device_no;
    } else {
        return -1;
    }
}

void printFDT(struct PCB *pcb) {
    kprintf("Printing file descriptor table for proc %d\n", pcb->pid);
    int k;
    for(k = 0; k < FDT_SIZE; k++) {
        kprintf("\tfd %d = %d\n", k, pcb->fdt[k]);
    }
}
