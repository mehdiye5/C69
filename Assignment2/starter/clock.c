#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"



extern int memsize;

extern int debug;

extern struct frame *coremap;

int clock;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	//sweep through pages in circular order
    //PG_REF is set if page has been referenced
    
    while (1) {
        // check if clock hand is less than the end
        if (clock < memsize) {
            // if this page's reference bit is set/turned on
            if (coremap[clock].pte->frame & PG_REF) {
                //give it a second chance
                //set reference bit off/ turn it off
                // &= removes bits
                coremap[clock].pte->frame &= ~PG_REF;
                // continue with next page
            } else {
                //ref bit is off, has not been used recently
                // evict this page
                return clock;
            }
        } else {
            //clock is at end so reset back to 0
            clock = 0;
        }
        clock++;

    }
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
    // set reference bit
    // |= adds bits
    p->frame |= PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
    clock = 0;
}
