/*
* Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
*/

/* Sierra release
* Source Code License Grant: The authors hereby grant to Licensee
* personal permission to use and modify the Licensed Source Code
* for the sole purpose of studying while attending the course CO2018.
*/

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "string.h"
#include "queue.h"
#include <stdlib.h>

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //demo for systemkill all
    char temp_name[100];

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        // proc_name[i]= data;
        // if(data == -1) proc_name[i]='\0';
        temp_name[i] = data;
        if(data == -1) temp_name[i]='\0';
        // printf("proc_name[%d] = %c\n", i, data);
        i++;
    }
    // printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
    // demo syscall kill all
    printf("Theprocname retrieved from memregionid %d is \"%s\"\n", memrg, temp_name);

    /* TODO: Traverse proclist to terminate the proc
    *       stcmp to check the process match proc_name
    */

    //Just for demo system kill all
    proc_name[0] = '\0';
    strcat(proc_name, "input/proc/");
    strcat(proc_name, temp_name);
    printf("fullprocname = \"%s\"\n", proc_name);

    struct queue_t *running_list = caller->running_list;
    #ifdef MLQ_SCHED
        struct queue_t *mlq_ready_queue = caller->mlq_ready_queue;
    #endif
    struct pcb_t *target = NULL;
    int killed_count = 0;

    if (running_list->size > 0) {
        int n = running_list->size;
        for (int i = 0; i < n; i++) {
            target = dequeue(running_list);
            if (target == NULL) continue;

            if (target->pid == 0 || target == caller) {
                enqueue(running_list, target);
                continue;
            }

            if (strcmp(target->path, proc_name) == 0) {
                printf("terminated process with pid %d\n", target->pid);
                target->pc = target->code->size; //Set the pc to the end of code segment
                killed_count++;
            } else {
                enqueue(running_list, target);  
            }
        }
    }
#ifdef MLQ_SCHED
    if (mlq_ready_queue->size > 0) {
        int n = mlq_ready_queue->size;
        for (int i = 0; i < n; i++) {
            target = dequeue(mlq_ready_queue);
            if (target == NULL) continue;

            if (target->pid == 0 || target == caller) {
                enqueue(mlq_ready_queue, target);
                continue;
            }

            if (strcmp(target->path, proc_name) == 0) {
                printf("terminated process with pid %d\n", target->pid);
                libfree(target, memrg);  // Free memory region
                free(target);  // Free PCB
                killed_count++;
            } else {
                enqueue(mlq_ready_queue, target);  // Put back in queue
            }
        }
    }
#endif
    libfree(caller, memrg);
    printf("Total %d processes terminated\n", killed_count);
    return killed_count;
}
 