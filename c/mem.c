/* mem.c : memory manager
 */

#include <xeroskernel.h>
#include <i386.h>

struct memheader {
	unsigned long size;
	struct memheader* next;
	struct memheader* prev;
	unsigned char* sanityCheck;
	unsigned char dataStart[0];
};

// Should always be the first node in the linked list.
extern long freemem;
extern char* maxaddr;
struct PCB global_process_table[MAX_NUMBER_OF_PCBS];
int global_process_pid_table[MAX_NUMBER_OF_PCBS];

struct memheader* findFreeMemHeader(int size);
void AddToFreeList(struct memheader*); 
struct memheader* getMemheader(void* ptr);

void kmeminit(void) {
	// adjust free mem to align with 16 byte paragraphs
	freemem += 0x10 - (freemem % 0x10);

	struct memheader* memslot1 = (struct memheader*)freemem; // before hole
	struct memheader* memslot2 = (struct memheader*)HOLEEND; // after hole

	memslot1->size = HOLESTART - freemem;
	memslot1->next = memslot2;
	memslot1->prev = NULL;
    memslot1->sanityCheck = (unsigned char*)0xDDDD;

	memslot2->size = (unsigned long)maxaddr - HOLEEND;
	memslot2->prev = memslot1;
	memslot2->next = NULL;
    memslot2->sanityCheck = (unsigned char*)0xDDDD;

	kprintf_log(MEM_LOG, 0, "Initiallizing memory.\n"
		"Free memslot1 is located at %x and has size %x\n", memslot1, memslot1->size);
	kprintf_log(MEM_LOG, 0, 
		"Free memslot2 is located at %x and has size %x\n", memslot2, memslot2->size);
}


void * kmalloc(int size) {
	struct memheader* tobeAllocated = findFreeMemHeader(size);

	// no more memory left
	if (tobeAllocated == NULL) {
		return NULL;
	}
	unsigned long sizeToBeAllocated = (size/16) + ((size % 16)?1:0);
	sizeToBeAllocated = sizeToBeAllocated * 16 + sizeof(struct memheader);

	kprintf_log(MEM_LOG, 0, "Allocating new memory of size %x at address %x\n",  sizeToBeAllocated, tobeAllocated);

	struct memheader* newFreeHeader;
	struct memheader* prevFreeHeader;
	struct memheader* nextFreeHeader;

	// check to see if remaining space in free memheader is enough for more memory
	if (tobeAllocated->size > sizeToBeAllocated + sizeof(struct memheader)) {
		newFreeHeader = (struct memheader*)(((unsigned long)tobeAllocated) + sizeToBeAllocated); // move free memheader forward
		newFreeHeader->next = tobeAllocated->next;
		newFreeHeader->prev = tobeAllocated->prev;
		newFreeHeader->size = ((unsigned long)tobeAllocated + tobeAllocated->size - (unsigned long)newFreeHeader);
	} else { // if not just use next free memheader
		kprintf_log(MEM_LOG, 0, "\tSize run out in original free memheader\n");
		newFreeHeader = tobeAllocated->next;
		newFreeHeader->prev = tobeAllocated->prev;
	}
	
	kprintf_log(MEM_LOG, 0, "\tNew free header is now located at memory address %x\n", newFreeHeader);

	// if first free memheader was allocated, update freemem to point to new head
	if (tobeAllocated == (struct memheader*)freemem)
		freemem = (long)newFreeHeader;
	else {
		// adjust previous free memheader to point to new free memheader
		prevFreeHeader = tobeAllocated->prev;
		prevFreeHeader->next = newFreeHeader;
	}

	if (newFreeHeader->next != NULL) {
		nextFreeHeader = newFreeHeader->next;
		nextFreeHeader->prev = newFreeHeader;
	}
	
	tobeAllocated->size = sizeToBeAllocated;
	tobeAllocated->next = NULL;
	tobeAllocated->prev = NULL;
	tobeAllocated->sanityCheck = (unsigned char*)0xDDDD;

	return tobeAllocated->dataStart;
	
}

void printFreeMemory() {
	struct memheader* freeheader = (struct memheader*)freemem;
	int i = 0;
	kprintf("Printing Free memory...\n");
	do {
		kprintf("\tFree Memheader %d\tStart location: %x\n\tSize: %x\tNext: %x\tPrev: %x\n",
			 i, freeheader, freeheader->size, freeheader->next, freeheader->prev);
		freeheader = freeheader->next;
		i++;

	} while (freeheader != NULL);
}


struct memheader *findFreeMemHeader(int size) {
	struct memheader* freeheader = (struct memheader*)freemem;
	do {
		if (freeheader->size >= size) {
			return freeheader;
		}
		freeheader = freeheader->next;
	} while (freeheader != NULL);
    kprintf("ERROR: Not enough free memory.\n");
    return NULL;
}

void kfree(void* ptr) {   
    struct memheader* headerToFree = (struct memheader*)((unsigned long)(ptr - sizeof(struct memheader)));
    kprintf_log(MEM_LOG, 0, "kfree(%x)\n", ptr);
    kprintf_log(MEM_LOG, 0, "\tMemheader to free is at %x\n", headerToFree);
	if (headerToFree->sanityCheck == (unsigned char*)0xDDDD &&
		headerToFree->next == NULL &&
		headerToFree->prev == NULL) {
		kprintf_log(MEM_LOG, 0, "\tMemHeader Check OKAY.\n");
	} else {
		kprintf("\tMemHeader Check ERROR. Sanity Check: %x\tNext: %x\tPrev: %x\n", headerToFree->sanityCheck, headerToFree->next, headerToFree->prev);
	}

    AddToFreeList(headerToFree);          
} 

// Inserts a free memory node into the freemem linked list.
void AddToFreeList(struct memheader* headerToInsert) {
	struct memheader* currFreeHeader = (struct memheader*) freemem;
    struct memheader* nextFreeHeader = currFreeHeader->next;
	struct memheader* prevFreeHeader = currFreeHeader->prev;

	// if headerToInsert is beyond the last addr
	if ((unsigned long)headerToInsert > (unsigned long)maxaddr - sizeof(struct memheader)) {
		kprintf("ERROR: memory address %x cannot be freed because is out of range.\n", headerToInsert);
		return;
	}

	do {
		// find where headerToInsert should be inserted
		if (currFreeHeader > headerToInsert) {

			prevFreeHeader->next = headerToInsert;
			currFreeHeader->prev = headerToInsert;
			headerToInsert->next = currFreeHeader;
			headerToInsert->prev = prevFreeHeader;
			
			if (currFreeHeader == (struct memheader*)freemem) {
				freemem = (unsigned long)headerToInsert;
			}

			// if headerToInsert is exactly before a free memheader, merge the two
			if ((unsigned long)headerToInsert + headerToInsert->size == (unsigned long)currFreeHeader) {
				headerToInsert->size += currFreeHeader->size;
				headerToInsert->next = nextFreeHeader;
				nextFreeHeader->prev = headerToInsert;
				currFreeHeader = nextFreeHeader; // they become one
			}
			// if memory to be freed is exactly after a free memheader, merge the two
			if ((unsigned long)prevFreeHeader + prevFreeHeader->size == (unsigned long)headerToInsert) {
				prevFreeHeader->size += headerToInsert->size;
				prevFreeHeader->next = currFreeHeader;
				currFreeHeader->prev = prevFreeHeader;
				headerToInsert->sanityCheck = NULL;
				headerToInsert->next = NULL;
				headerToInsert->prev = NULL;
			}
			return;
		}
		currFreeHeader = nextFreeHeader;
		nextFreeHeader = currFreeHeader->next;
		prevFreeHeader = currFreeHeader->prev;
	} while (currFreeHeader != NULL);
	kprintf("ERROR: Couldn't insert free memheader into free list\n");
}

void dump_gpt() {
	int i;
	for (i = 0; i < MAX_NUMBER_OF_PCBS; i++) {
        kprintf("pcb %d is currently in state %d. Top of stack points to %x\n", 
		global_process_table[i].pid, 
		global_process_table[i].state,
		global_process_table[i].top_of_stack);
	}
}

void memManagerTest(void) {
	void * first = kmalloc(0x10);
	void * second = kmalloc(0x10);
	void * third = kmalloc(0x10);
	printFreeMemory();
	kfree(second);
	kfree(third);
	kfree(first);
	printFreeMemory();	
}

