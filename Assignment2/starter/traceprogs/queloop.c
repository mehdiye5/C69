#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//queue object
typedef struct queu_obj {
    int d[400];
} Queu;


//populate the array
void que_loop (int size, Queu ** q) {
    int i;

    // go throught the array and populate with a queu struct    
    for (i = 0; i < size; i++) {
        q[i] = malloc(sizeof(Queu));    
    }
    return;
}


// free the momory that was alocated to que array
void empty_queu(int size,  Queu ** q) {
    int i;
    for (i= 0; i < size; i++) {
        free(q[i]);
    }
    return;
}
int main(int argc, char ** argv) {

    /* Markers used to bound trace regions of interest */
	volatile char MARKER_START, MARKER_END;
	

    /* Record marker addresses */
	FILE* marker_fp = fopen("queloop.marker","w");
	if(marker_fp == NULL ) {
		perror("Couldn't open marker file:");
		exit(1);
	}
	fprintf(marker_fp, "%p %p", &MARKER_START, &MARKER_END );
	fclose(marker_fp);

       // aray pointer of a point, enougn memory to create queue array
	Queu ** q = malloc(400 *sizeof(Queu *));

    MARKER_START = 33;
    que_loop(400, q);
    MARKER_END = 34;

    empty_queu(400, q);
    return 0;
}
