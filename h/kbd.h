/* 
 * Implements each device independent call with a device specific call for the
 * keyboard device.
 */

#define BUFF_SIZE 0x4
extern struct PCB;
#include <stdarg.h>

// Device specific open function for the keyboard and keyboardecho devices
// Enables keyboard interrupts.
int kbd_open(struct PCB* pcb);

// Disables keyboard interrupts.
int kbd_close(void); 

// Lower half device specific open.
unsigned int kbd_read(void); 

// Upper half keyboard read.
int kbd_uread(struct PCB* pcb, void* buff, int len, int e);

// Invalid operation for keyboard. Returns -1.
int kbd_write(void);

// Allows application to set EOF character.

void main(void);
int kbd_ioctl(int command); 

// Buffer struct for lower and upper half.
typedef struct {
	char buff[BUFF_SIZE];
	int head;
	int tail;
	int nb;

} Buffer;

typedef enum {
	NONE,
	READ,
	IOCTL
} request_type;
	
// Read buffer for the lower half of the keyboard device driver.
char Buffer_Read(Buffer* buff);

// Upper half device driver buffer.
void Buffer_Write(Buffer* buff,  char data);

