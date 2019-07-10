#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct queu_obj {
    int d[400];
};

struct queu_obj** q = malloc(size *sizeof(struct queu_obj));

void que_loop (int size) {
    int i;
        
    for (i = 0; i < size; i++) {
        q[i] = malloc(sizeof(struct queu_obj))
        q[i].d = i;    
    }
    return;
}

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