#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile; 



//need to replace the page that will be referenced the furthest in the future, or not at all

//linked list of addresses
typedef struct addr{
    addr_t vaddr;
    struct addr *next;
} addr_list;

addr_list *head;

/* Helper function: Returns the number of frames between the next referenced frame
 * given page table entry
 * ie. if it returns 0 then the frame is referenced in the next frame
 * if no other referenced frame, return -1
 */
int is_next_frame(pgtbl_entry_t *p) {
    int num = 0;
    //create a current pointer
    addr_list *curr = head;

    while(curr) {
        if (curr->vaddr == p->vaddr) {
            return num;
        }
        curr= curr->next;
        num++;
    }
    //frame was not referenced anywhere else after
    return -1;
}

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
    
    int longest = -1;
    int pfn;
    int i;
	//loop through frames
    for (i=0; i<memsize; i++) {
        // get pte for this frame
        //get frame's next occurence if there is
        int current = is_next_frame(coremap[i].pte);

        // not referenced anywhere else
        if (current == -1) {
            return i;
        } else {
            if (current > longest) {
                // change new longest distance of frames
                longest = current;
                pfn = i;
                //loop will continue to see if there is another longest
            }
        }
    }
	return pfn;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
    addr_list *current = head;
    head = head->next;
    free(current);
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
    //using code from sim.c
    // need to read tracefile, set vaddrs in linked list struct
    char buf[MAXLINE];
    addr_t vaddr = 0;
	char type;
    head=NULL;
    addr_list *next_node = NULL;
    //open trace file
    FILE *tfp;

	if((tfp = fopen(tracefile, "r")) == NULL) {
		perror("Error opening tracefile:");
		exit(1);
	}
    while(fgets(buf, MAXLINE, tfp) != NULL) {
        if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
            addr_list *new = malloc(sizeof(addr_list));
            //need to initialize a head
			if (head==NULL){
                head = new;
                head->vaddr = vaddr;
                head->next = NULL;
            } else {
                next_node->next = new;
                next_node->vaddr = vaddr;
                next_node->next=NULL;
                next_node=new;
            }
		} else {
			continue;
		}
    }
    fclose(tfp);
}

