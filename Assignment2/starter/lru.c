#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

typedef struct stack_node {    
    struct stack_node* next; // next node pointer
	int ind; // page frame number.
} s_node;

s_node* bottom;
s_node* top;

/*
 Note:
	I will be using Option 2 for implementing LRU using stack method. 
	Most resently used is on top of the stack and least is at the buttom.
 */

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the ind in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {

	int ind;
		// case:  there are no pages to evict
		if (bottom != NULL) {
			ind = bottom->ind;
			s_node* evicted = bottom;


			// case: there is only one node in the stack
			if (bottom->ind == top->ind) {
				bottom = NULL;
				top = NULL;
			} else {
			// case: there are more than one node in the stack
				bottom = bottom->next;
				free(evicted);
			}

			// set eviction bit value to evicted status aka 0
			coremap[ind].evic = 0;	
		}
		/* data */
	
	
	
	return ind;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	int ind = p->frame >> PAGE_SHIFT;

	// case: page hasn't been referenced before or has been evicted
	if (coremap[ind].evic == 0) {
		s_node* ref_node = (s_node *) malloc(sizeof(s_node));
		ref_node->ind = ind;		

		if (top == NULL) {
			top = ref_node;
			bottom = ref_node;
		} else {
			bottom->next = ref_node;
			bottom = bottom->next;
		}
	} else {
	// case: page has been referenced before
	// if page has been reference meanse that there is at least one node in the stack
	// if there is there is only one node (meaning their frame/ind numbers are the same) then there is no need to do anything
	s_node* curr;
	s_node* prev;

	// look for the node with desired ind/frame number
	for (curr = bottom; curr != NULL && curr->ind != ind; curr = curr->next) {
		prev = curr;
	}

	// case: there is more than 1 node in the stack
	if (prev->ind != curr->ind) {
		// unlinck found node
		prev->next = curr->next;
		curr->next = NULL;

		// place the node on top of the stack
		top->next = curr;
		// now found node is the new node on top of the stack.
		top = top->next;
	}

	}

	

	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	// initialize top and bottom of the stack
	bottom = NULL;
	top = NULL;

	// set all eviction bit vulues to 0 meaning, have been evicted
	for(int i = 0; i < memsize; ++i) {
		coremap[i].evic = 0;
	}

}
