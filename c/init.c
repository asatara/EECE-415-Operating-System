/* initialize.c - initproc */

#include <i386.h>
#include <xeroskernel.h>
#include <xeroslib.h>
#include <kbd.h>

extern	int	entry( void );  /* start of kernel image, use &start    */
extern	int	end( void );    /* end of kernel image, use &end        */
extern  long	freemem; 	/* start of free memory (set in i386.c) */
extern char	*maxaddr;	/* max memory address (set in i386.c)	*/

/************************************************************************/
/***				NOTE:				      ***/
/***								      ***/
/***   This is where the system begins after the C environment has    ***/
/***   been established.  Interrupts are initially DISABLED.  The     ***/
/***   interrupt table has been initialized with a default handler    ***/
/***								      ***/
/***								      ***/
/************************************************************************/

/*------------------------------------------------------------------------
 *  The init process, this is where it all begins...
 *------------------------------------------------------------------------
 */
 
// initially root and idle process
void (*root_process_entry)(void) = root;
void (*idle_process_entry)(void) = idle;

void init_queues(void);
void init_pcb_table(void);
void init_port_table(void);
void init_device_table(void);
void initKeyboard(void);
void initKeyboardEcho(void);


/* The beginning */
void initproc( void ) {
	kmeminit();
	context_init();
    init_queues();
	init_pcb_table();
	init_port_table();
	initPIT(10);
	create(root_process_entry, 0x1000);
	create(idle_process_entry, 0x1000);
    init_device_table();
	dispatch();
}

void init_queues(void) {
    ready_queue = NULL;
    blocked_queue = NULL;
}

void init_pcb_table(void) {
	int i;
	for (i = 0; i < MAX_NUMBER_OF_PCBS; i++) {
		global_process_table[i].pid = -1;
		global_process_table[i].state = STOPPED;
		global_process_pid_table[i] = -1;

        // Initialize all entries in fdt to zero.
        int k;
        for(k = 0; k < FDT_SIZE; k++) {
            global_process_table[i].fdt[k] = 0;
        }
	}
}

void init_port_table(void) {
	int i;
	for (i = 0; i < MAX_NUMBER_OF_PORTS; i++) {
		global_port_table[i].in_use = FALSE;
        global_port_table[i].next = NULL; // Not part of a proc's linked list of ports.
	}
}


void init_device_table(void) {
    devsw device_table[DEVICE_TABLE_SIZE];
    initKeyboard();
    initKeyboardEcho();
}

void initKeyboard(void) {
   devsw *d = &device_table[0];
   d->dvnum = 0;
   d->dvname = "keyboard";
   d->dvopen = kbd_open; 
   d->dvclose = kbd_close;
   d->dvread = kbd_uread;
   d->dvwrite = kbd_write;
   d->dvioctl = kbd_ioctl; 
   devswToString(&device_table[0]);
}

void initKeyboardEcho(void) {
   devsw *d = &device_table[1];
   d->dvnum = 1;
   d->dvname = "keyboardecho";
   d->dvopen = &kbd_open; 
   d->dvclose = &kbd_close;
   d->dvread = &kbd_uread;
   d->dvwrite = &kbd_write;
   d->dvioctl = &kbd_ioctl;
}

