/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

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

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


#define MAX_PROC        64
#define KERNEL_INT      80
#define TIMER_INT       (TIMER_IRQ + 32)
#define PROC_STACK      (4096 * 4)

#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_SLEEP     2

#define SYS_STOP        0
#define SYS_YIELD       1
#define SYS_CREATE      2
#define SYS_TIMER       3
#define SYS_GETPID      100
#define SYS_PUTS       103
#define SYS_SLEEP       105

typedef void    (*funcptr)(void);

typedef struct struct_pcb pcb;
struct struct_pcb {
    pcb         *next;
    pcb         *prev;
    long        esp;
    int         state;
    int         pid;
    int         otherpid;
    void        *buffer;
    int         bufferlen;
    int         ret;
    int         sleepdiff;
    long        args;
};


/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

typedef struct context_frame {
  unsigned int        edi;
  unsigned int        esi;
  unsigned int        ebp;
  unsigned int        esp;
  unsigned int        ebx;
  unsigned int        edx;
  unsigned int        ecx;
  unsigned int        eax;
  unsigned int        iret_eip;
  unsigned int        iret_cs;
  unsigned int        eflags;
  unsigned int        stackSlots[0];
} context_frame;



void     kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( int size );
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, int stack );
void     printCF (void * stack);
void     root( void );
void     set_evec(unsigned int xnum, unsigned long handler);
void     sleep(pcb *, int);
int      syscall(int call, ...);
int      syscreate( funcptr fp, int stack );
int      sysgetpid( void );
unsigned int syssleep( unsigned int t );
int      sysstop( void );
void     sysputs( char *str );
int      sysyield( void );
void     tick( void );





