/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#include <stdarg.h>
/* Symbolic constants used throughout Xinu */

typedef	char    Bool;   /* Boolean type                  */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */


/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

#define MAX_NUMBER_OF_PCBS  0xf
#define MAX_NUMBER_OF_PORTS 0xf
#define MAX_NUMBER_OF_SIGS 0x20
#define FDT_SIZE 0x4
#define DEVICE_TABLE_SIZE 0x2

#define PAUSE int z;for(z=0;z < 2000000;z++)
#define PAUSE10 int y;for(y=0;y < 10000000;y++)

// Logging levels
#define MEM_LOG    0
#define DISP_LOG   1
#define FLOW_LOG   0
#define QUEUE_lOG  0
#define TABEL_LOG  0
#define CREATE_LOG 0
#define DEMO_LOG   1
#define PORT_LOG   0
#define SLEEP_LOG  0
#define PTR_LOG    0
#define SIG_LOG    1

int temp22;
/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
int            kprintf_log(int log_level, int pause, char * fmt, ...);
int			   sprintf(char* str, char* fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);
void           set_evec(unsigned int xnum, unsigned long handler);

typedef enum {
    RUNNING,
    READY,
    STOPPED,
    BLOCKED,
	SLEEP,
	SIG_WAIT
} Process_state;

typedef enum {
	SYS_YIELD,
	SYS_CREATE,
	SYS_STOP,
	SYS_PUTS,
    SYS_PORT_CREATE,
    SYS_PORT_DELETE,
    SYS_SEND,
    SYS_PORT_RECV,
	SYS_SLEEP,
	TIMER_INT,
    SYS_GET_PID,
    SYS_KILL,
	SYS_SIG_HANDLE,
	SYS_SIG_RETURN,
	SYS_SIG_WAIT,
    SYSOPEN,
    SYSCLOSE,
    SYSWRITE,
    SYSREAD,
    SYSIOCTL,
    KBD_INT
} system_call;

// used for testing
typedef enum {
	SHORT,
	LONG
} pause_length;

struct Devsw;

struct PCB {
    int pid;
    Process_state state;
    struct context_frame* context;
    struct PCB *next; // next process in a queue
	int top_of_stack; // where stack memory begins
	int rc; // return content
    struct Port* ports; // Linked list of ports owned by the process.
	int ticks; // number of sleep ticks left when sleeping
	void* msg; // used for IPC
	struct PCB** blocked_queue; // If blocked, pcb is in this blocked_queue
	int signal_controller;
	void* signal_table[MAX_NUMBER_OF_SIGS];
	Bool is_in_signal;
    struct Devsw* fdt[FDT_SIZE];  // Fixed size file descriptor table.
};

/* This struct represents a device in the 'upper half' of the device driver.
 * DII (device independant interface) functions are mapped to upper half
 * device specific functions.
 */
typedef struct Devsw {
    int dvnum;
    char *dvname;
    int (*dvopen) (struct PCB* pcb);  // TODO: decide on the parameters for these function and add them in.
    int (*dvclose) (void);
    int (*dvread) (struct PCB* pcb, void* buff, int len);
    int (*dvwrite) (void);
    int (*dvioctl) (int command, va_list argv);
} devsw;

struct Port {
    int in_use; 
	int portNumber;
    struct Port* next;
	struct PCB* owner;
	struct PCB* blocked_list; // list of processes blocked waiting for this port
};

// for demo
struct Msg {
	int pid;
	int port;
};

struct context_frame {
    unsigned int edi;
	unsigned int esi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int ebx;
	unsigned int edx;
	unsigned int ecx;
	unsigned int eax;
	unsigned int iret_eip;
	unsigned int iret_cs;
	unsigned int eflags;
};

// Defined in mem.c
extern void  kmeminit(void);
extern void* kmalloc(int size);
extern void  kfree(void* ptr);
extern void  printFreeMemory(void);
extern void  dump_gpt(void);
extern void  memManagerTest(void);

// Defined in create.c
extern int   create(void (*func)(void), int stack_size);

// Defined in disp.c
extern void  dispatch(void);

/* addToQueue
 * Adds a PCB to the end of the specified queue
 */
extern void  addToQueue(struct PCB** queue, struct PCB* process);

/* removeFromQueue
 * Removes the specified pcb from the specific queue and returns in
 * If process is not found in queue, returns NULL
 * If input pcb is NULL, returns first pcb in queue
 */
extern struct PCB*  removeFromQueue(struct PCB** queue, struct PCB* process);

/* unblockProcess
 * Used by ksysportrecv
 * Checks to see if a sending process is on a ports blocked queue. If it is
 *		it removes it from the blocked queue and adds it to the ready queue
 *
 * Returns if whether process was removed from the queue
 */

extern Bool unblockProcess(struct PCB** queue, int port);
extern Bool doesProcExist(int pid);
extern void kkillproc(struct PCB* proc);
extern void  queue_dump(void);
extern struct PCB* find_pcb(int pid);


// Defined in ctsw.c
extern system_call contextswitch(struct PCB* p);
extern void context_init(void);

// Defined in user.c
extern void  root(void);
extern void  idle(void);

// Defined in syscall.c
extern int   syscall(int call, ...);
extern void  sysyield(void);
extern void  sysstop(void);
extern int   syscreate( void (*func)(void), int stack );
extern void  sysputs(char* str); 
extern void  sysputs2(char* str, int); 
extern void  sysputs3(char* str, int, int); 
extern void  sysputs4(char* str, int, int, int); 
extern int   sysportcreate(int port);
extern int   sysportdelete(int port);
extern int   sysportsend(int port, void *msg);
extern int   sysportrecv(int port, void **msg);
extern unsigned int syssleep(unsigned int milliseconds);
extern unsigned int sysgetpid(void);
extern int   syssighandler(int signal, void (*newhandler)(void*), void (**oldhandler)(void*));
extern void  syssigreturn(void* old_sp);
extern int   syskill(int pid, int sigNumber);
extern int   syssigwait(void);
extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void *buff, int bufflen);
extern int sysread(int fd, void *buff, int bufflen);
extern int sysioctl(int fg, unsigned long command, ...);


// Defined in msg.c
extern int  kcreateport(int port, struct PCB* pcb);
extern int  kdestroyport(int port, struct PCB* pcb);
extern void kdestroyallports(struct PCB* pcb);
extern int  ksendtoport(int port, void *msg, struct PCB* proc);
extern int  krecvfromport(int port, void **msg, struct PCB *proc);
extern struct PCB * findProcByPort(struct PCB** queue, int portnum);
extern struct Port global_port_table[MAX_NUMBER_OF_PORTS];

// Defined in sleep.c
extern void sleep(struct PCB* process);
extern void tick(void);

// Defined in signal.c
extern void sigtramp(void (*handler)(void *), void *context, void *old_sp);
extern int signal(int pid, int signalNum);
extern void prepare_sigtramp(struct PCB* pcb);

// Defined in di_calls.c
extern int di_open(struct PCB* pcb, int device_no);
extern int di_close(struct PCB *pcb, int fd);
extern void di_write(void);
extern int di_read(struct PCB* pcb, int fd, void* buff, int len);
extern int di_ioctl(struct PCB* pcb, int fd, int command, va_list argv);
void devswToString(devsw *d); 


// The global process table has 32 spaces so there can be 32 process in the system at once.
// Static so that it is viewable be the whole compilation unit.
struct PCB global_process_table[MAX_NUMBER_OF_PCBS];

// This table is used to help assign PIDs to processes. 
// Read find_next_free_pcb(int* pid) in create.c for more info on
// how it works.
int global_process_pid_table[MAX_NUMBER_OF_PCBS];

// Kernel's device table. Only contains 2 entries for assignment 3.
devsw device_table[DEVICE_TABLE_SIZE];

struct PCB* ready_queue;
struct PCB* blocked_queue;
struct PCB* curr_running_process;
struct PCB* idle_process;
struct PCB* sleep_queue;
