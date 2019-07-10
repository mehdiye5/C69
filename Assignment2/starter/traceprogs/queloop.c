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
    que_loop(400);
    empty_queu(400);
}