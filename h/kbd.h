/* Implements each device independent call with a device specific call for the
 * keyboard device.
 */

void init_kbd(void);
int kbd_open(void); 
int kbd_close(void); 
int kbd_read(void); 
int kbd_write(void);
int kbd_ioctl(void); 
