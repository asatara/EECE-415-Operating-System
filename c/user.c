/* user.c : User processes
 */
#include <xeroskernel.h>
const int num = 4;

void root(void) {
    int fd = sysopen(0);
    sysputs2("Root got fd %d\n", fd);
	sysputs("Done\n");
}

void idle(void) {
	for(;;);
}
