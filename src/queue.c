#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL) return;
        if (q->size >= MAX_QUEUE_SIZE) {
                // Dont know what the fuck to do here
                return;
        }
        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (q == NULL || empty(q)) return NULL;
        int highest_priority = q->proc[0]->priority;
        int highest_index = 0;
        for (int i = 1; i < q->size; i++) {
                if (q->proc[i]->priority > highest_priority) {
                        highest_priority = q->proc[i]->priority;
                        highest_index = i;
                }
        }
        struct pcb_t * highest_proc = q->proc[highest_index];
        for (int i = highest_index; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        q->proc[q->size] = NULL; 
        return highest_proc;
	return NULL;
}

