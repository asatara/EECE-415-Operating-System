/* Implements each device independent call with a device specific call for the
 * keyboard device.
 */

#define BUFF_SIZE 0x4
extern struct PCB;
#include <stdarg.h>

void init_kbd(void);
int kbd_open(struct PCB* pcb);
int kbd_close(void); 
unsigned int kbd_read(void); 
int kbd_uread(struct PCB* pcb, void* buff, int len);
int kbd_write(void);
int kbd_ioctl(int commnad, va_list argv); 

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
	

char Buffer_Read(Buffer* buff);
void Buffer_write(Buffer* buff,  char* data);

