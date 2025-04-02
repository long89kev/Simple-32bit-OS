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
        if (q->size >= MAX_QUEUE_SIZE) return; 
        q->proc[q->size] = proc; 
        q->size++;
}

struct pcb_t * queue_traversal(struct queue_t * q, int *idx) {
        if(q == NULL || q->size == 0) return NULL;
        if (idx == NULL) return NULL;
        if (*idx >= q->size) return NULL;
        struct pcb_t *proc = q->proc[*idx];
        (*idx)++;
        return proc;
}

void delete_pid(struct queue_t * q, uint32_t pid) {
        if(q == NULL || q->size == 0) return;
        for (int i = 0; i < q->size; i++) {
                if (q->proc[i]->pid == pid) {
                        for (int j = i; j < q->size - 1; j++) {
                                q->proc[j] = q->proc[j + 1];
                        }
                        q->size--;
                        return;
                }
        }
        return;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        
        #ifdef MLQ_SCHED
                if (q == NULL || q->size == 0) return NULL;

                struct pcb_t *getProc = malloc(sizeof(struct pcb_t));
                getProc = q->proc[0];

                for (int i = 0; i < q->size - 1; i++) {
                        q->proc[i] = q->proc[i + 1];
                }

                q->size--;
                return getProc;
        #else
                if (q == NULL || q->size == 0) return NULL;
                struct pcb_t *highest = malloc(sizeof(struct pcb_t));
                highest = q->proc[0];
                int highest_index = 0;
                for (int i = 1; i < q->size; i++) {
                        if (q->proc[i]->priority < highest->priority) {
                                highest = q->proc[i];
                                highest_index = i;
                        }
                }
                for (int i = highest_index; i < q->size - 1; i++) {
                        q->proc[i] = q->proc[i + 1];
                }
                q->size--;
                return highest;
        #endif

        return NULL;
}

