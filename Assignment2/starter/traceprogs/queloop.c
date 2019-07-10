#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

//queue object
typedef struct queu_obj {
    int d[400];
};

// aray pointer of a point, enougn memory to create queue array
struct queu_obj** q = malloc(size *sizeof(struct queu_obj));

//populate the array
void que_loop (int size) {
    int i;

    // go throught the array and populate with a queu struct    
    for (i = 0; i < size; i++) {
        q[i] = malloc(sizeof(struct queu_obj))
        q[i].d = i;    
    }
    return;
}


// free the momory that was alocated to que array
void empty_queu(int size) {
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
	FILE* marker_fp = fopen("simpleloop.marker","w");
	if(marker_fp == NULL ) {
		perror("Couldn't open marker file:");
		exit(1);
	}
	fprintf(marker_fp, "%p %p", &MARKER_START, &MARKER_END );
	fclose(marker_fp);

    MARKER_START = 33;
    que_loop(400);
    empty_queu(400);
    MARKER_END = 34;

    return 0;
}