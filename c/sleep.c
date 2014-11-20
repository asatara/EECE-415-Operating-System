/* sleep.c : sleep device (assignment 2)
 */

#include <xeroskernel.h>

void sleep(struct PCB* process) {

	kprintf_log(SLEEP_LOG, NONE, "Adding pid %d to sleep queue\n", process->pid);
	if (sleep_queue == NULL) {
		sleep_queue = process;
		return;
	}

	struct PCB* iterator = sleep_queue;
	struct PCB* prev = NULL;

	while (iterator != NULL) {
		if (process->ticks < iterator->ticks) {
			break;
		}
		prev = iterator;
		iterator = iterator->next;
	}

	if (prev == NULL) {
		process->next = iterator;
		sleep_queue = process;
	} else {
		prev->next = process;
		process->next = iterator;
	}
	return;
}

void tick(void) {
	if (sleep_queue == NULL)
		return;

	struct PCB* iterator = sleep_queue;
	while (iterator != NULL) {
		iterator->ticks--;
		if (iterator->ticks < 0) {
			kprintf_log(SLEEP_LOG, 0, "PID %d finished sleeping. moving to ready queue\n", iterator->pid);
			sleep_queue = iterator->next;
			iterator->next = NULL;
			addToQueue(&ready_queue, iterator);
			iterator = sleep_queue;
		} else {
			iterator = iterator->next;
		}
	}
}
