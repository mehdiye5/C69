#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int ind;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the ind in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	// ind to be evicted
	int evicted = ind;

	// new ind location
	// we use mod, becase if ind = memsize then ind will equal to 0 (memsize % memsize equals to 0)
	ind = (ind + 1) % memsize;
	
	return evicted;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {

	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	// initialize ind at 0
	ind = 0;
}
