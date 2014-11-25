/* kbd.c: keyboard device driver
 */

#include "kbd.h"
#include <xeroskernel.h>
#define KEY_UP   0x80            /* If this bit is on then it is a key   */
                                 /* up event instead of a key down event */

/* Control code */
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38

#define LCTL    0x1d
#define CAPSL   0x3a


/* scan state flags */
#define INCTL           0x01    /* control key is down          */
#define INSHIFT         0x02    /* shift key is down            */
#define CAPSLOCK        0x04    /* caps lock mode               */
#define INMETA          0x08    /* meta (alt) key is down       */
#define EXTENDED        0x10    /* in extended character mode   */

#define EXTESC          0xe0    /* extended character escape    */
#define NOCHAR          256


static int state; /* the state of the keyboard */

static Buffer buffer;
request_type request = NONE;
static char* requestBuffer;
static int requestLen;
static int requestInd;
static struct PCB* requestProcess;
static unsigned int _EOF = 10;
static int ECHO;

/*  Normal table to translate scan code  */
unsigned char   kbcode[] = { 0,
          27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
         '0',  '-',  '=', '\b', '\t',  'q',  'w',  'e',  'r',  't',
         'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',
         's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',
         '`',    0, '\\',  'z',  'x',  'c',  'v',  'b',  'n',  'm',
         ',',  '.',  '/',    0,    0,    0,  ' ' };

/* captialized ascii code table to tranlate scan code */
unsigned char   kbshift[] = { 0,
           0,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',
         ')',  '_',  '+', '\b', '\t',  'Q',  'W',  'E',  'R',  'T',
         'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',
         'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',
         '~',    0,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',
         '<',  '>',  '?',    0,    0,    0,  ' ' };
/* extended ascii code table to translate scan code */
unsigned char   kbctl[] = { 0,
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
           0,   31,    0, '\b', '\t',   17,   23,    5,   18,   20,
          25,   21,    9,   15,   16,   27,   29, '\n',    0,    1,
          19,    4,    6,    7,    8,   10,   11,   12,    0,    0,
           0,    0,   28,   26,   24,    3,   22,    2,   14,   13 };




static int
extchar(code)
unsigned char code;
{
    state &= ~EXTENDED;
}




unsigned int kbtoa( unsigned char code )
{
  unsigned int ch;
  
  if (state & EXTENDED)
    return extchar(code);
  if (code & KEY_UP) {
    switch (code & 0x7f) {
    case LSHIFT:
    case RSHIFT:
      state &= ~INSHIFT;
      break;
    case CAPSL:
      kprintf("Capslock off detected\n");
      state &= ~CAPSLOCK;
      break;
    case LCTL:
      state &= ~INCTL;
      break;
    case LMETA:
      state &= ~INMETA;
      break;
    }
    
    return NOCHAR;
  }
  
  
  /* check for special keys */
  switch (code) {
  case LSHIFT:
  case RSHIFT:
    state |= INSHIFT;
    kprintf("shift detected!\n");
    return NOCHAR;
  case CAPSL:
    state |= CAPSLOCK;
    kprintf("Capslock ON detected!\n");
    return NOCHAR;
  case LCTL:
    state |= INCTL;
    return NOCHAR;
  case LMETA:
    state |= INMETA;
    return NOCHAR;
  case EXTESC:
    state |= EXTENDED;
    return NOCHAR;
  }
  
  ch = NOCHAR;
  
  if (code < sizeof(kbcode)){
    if ( state & CAPSLOCK )
      ch = kbshift[code];
	  else
	    ch = kbcode[code];
  }
  if (state & INSHIFT) {
    if (code >= sizeof(kbshift))
      return NOCHAR;
    if ( state & CAPSLOCK )
      ch = kbcode[code];
    else
      ch = kbshift[code];
  }
  if (state & INCTL) {
    if (code >= sizeof(kbctl))
      return NOCHAR;
    ch = kbctl[code];
  }
  if (state & INMETA)
    ch += 0x80;
  return ch;
}


main() {
  kbtoa(LSHIFT);
  kprintf("45 = %c\n", kbtoa(45));
  kbtoa(LSHIFT | KEY_UP);
  kprintf("45 = %c\n", kbtoa(45));
}

// keyboard microcontroller: port 0x60
// onboard microcontroller: port 0x64
int kbd_open(struct PCB* pcb) {
    kprintf("Executing kbd_open.\n");
	requestProcess = pcb;
    enable_irq(1, 0);
	buffer.head = 0;
	buffer.tail = 0;
	buffer.buff[0] = 0;
	buffer.nb = 0;
	request = NONE;
    kprintf("Onboard controller status(port 0x64): %x\n", inb(0x64));
    kprintf("Keyboard controller status(port 0x60): %x\n", inb(0x60));
    return 0;
}

int kbd_close(void) {
    kprintf("Executing kbd_close.\n");
    enable_irq(1, 1);
    return 0;
}
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38

#define LCTL    0x1d
#define CAPSL   0x3a
unsigned int kbd_read(void) {  
    unsigned char code = inb(0x60);
    unsigned int ascii = kbtoa(code);
	switch (request) {
		case(NONE): {
			Buffer_Write(&buffer, (char)ascii);
			break;
		}
		case(READ): {
			if (!(code & KEY_UP) && code != 0x2a && code != 0x36 && code !=0x38 && code != 0x1d
					&& code != 0x3a) {
				requestBuffer[requestInd] = (char)ascii;
				requestInd++;
			}
			if (requestInd == requestLen) {
				removeFromQueue(&blocked_queue, requestProcess);
				addToQueue(&ready_queue, requestProcess);
				requestProcess->rc = requestInd;
				request = NONE;
			} else if(code == 0x20 && state & INCTL) {
				removeFromQueue(&blocked_queue, requestProcess);
				addToQueue(&ready_queue, requestProcess);
				requestProcess->rc = requestInd;
				request = NONE;
			} else if(10 == ascii) {
				removeFromQueue(&blocked_queue, requestProcess);
				addToQueue(&ready_queue, requestProcess);
				requestProcess->rc = requestInd;
				request = NONE;
			}
			break;
		}
		case(IOCTL): {
			if (!(code & KEY_UP) && code != 0x2a && code != 0x36 && code !=0x38 && code != 0x1d
					&& code != 0x3a) {
				_EOF = ascii;
				removeFromQueue(&blocked_queue, requestProcess);
				addToQueue(&ready_queue, requestProcess);
				requestProcess->rc = 0;
			}
		}
	}
    if(ECHO) {
	    kprintf("%c", ascii);
    }
	return ascii;
}


int kbd_uread(struct PCB* pcb, void* buff, int len, int e) {
	request = READ;
	requestBuffer = (char*)buff;
	requestLen = len;
	requestInd = 0;
	requestProcess = pcb;
    ECHO = e;
	kprintf("buffer addr is %d\n", requestBuffer);

	while (buffer.nb != 0) {
		requestBuffer[requestInd] = Buffer_Read(&buffer);
		requestInd++;
		if (requestInd == requestLen) {
			removeFromQueue(&blocked_queue, requestProcess);
			addToQueue(&ready_queue, requestProcess);
			requestProcess->rc = requestInd;
		}
	}
	return 0;
}


int kbd_write(void) {
    return -1;
}


int kbd_ioctl(int command, va_list argv) {
	if (command != 53)
		return -1;
	
	request = IOCTL;
    return 0;
}

char Buffer_Read(Buffer* buff){
	if (buff->nb == 0)
		return;
	
	char data = buff->buff[buff->tail];
	buff->nb--;
	buff->tail++;
	buff->tail = buff->tail % (BUFF_SIZE);
	return data;
}

void Buffer_Write(Buffer* buff, char data) {
	if (buff->nb == BUFF_SIZE)
		return;

	buff->buff[buff->head] = data;
	buff->nb++;
	buff->head++;
	buff->head = buff->head % (BUFF_SIZE);
}
