#include "oslabs.h"
#include <stdio.h>
#include <limits.h> 

int findShortestRemainingTime(struct PCB ready_queue[QUEUEMAX], int queue_cnt) {
    int shortest_index = -1;
    int shortest_time = INT_MAX;

    for (int i = 0; i < queue_cnt; i++) {
        if (ready_queue[i].remaining_bursttime < shortest_time && ready_queue[i].remaining_bursttime > 0) {
            shortest_time = ready_queue[i].remaining_bursttime;
            shortest_index = i;
        }
    }
    return shortest_index;
}

// **** PRIORITY-BASED PREEMPTIVE ****

struct PCB handle_process_arrival_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
   
    // Handle full queue
    if (*queue_cnt == QUEUEMAX) {
        printf("Ready queue is full. Dropping new process.\n");
        return current_process; 
    }

    // Handle initial arrival (no process currently running)
    if (current_process.process_id == -1) {
        new_process.execution_starttime = timestamp;
        ready_queue[(*queue_cnt)++] = new_process;
        return new_process; 
    }

    // Check if new process should preempt
    if (new_process.process_priority < current_process.process_priority) {
        // Preempt the current process
        current_process.remaining_bursttime -= (timestamp - current_process.execution_starttime);
        current_process.execution_starttime = -1; // Mark as not running
        
        // Add the preempted process to the ready queue
        ready_queue[(*queue_cnt)++] = current_process;

        // Update new process start time
        new_process.execution_starttime = timestamp;

        // Update the execution_endtime of the new process
        new_process.execution_endtime = timestamp + new_process.total_bursttime;

        return new_process; 
    }

    // No preemption, insert the new process into the queue based on priority
    int insertIndex = *queue_cnt;
    for (int i = 0; i < *queue_cnt; i++) { 
        if (new_process.process_priority < ready_queue[i].process_priority) {
            insertIndex = i;
            break;
        }
    }
    for (int i = (*queue_cnt)++; i > insertIndex; i--) {
        ready_queue[i] = ready_queue[i - 1];
    }
    ready_queue[insertIndex] = new_process; 

    return current_process; 
}


struct PCB handle_process_completion_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
   if (*queue_cnt == 0) {
        struct PCB null_PCB = {-1, -1, -1, -1, -1, -1, -1};
        return null_PCB;
    }
    
    int highestPriorityIndex = 0;
    for (int i = 1; i < *queue_cnt; i++) {
        if (ready_queue[i].process_priority < ready_queue[highestPriorityIndex].process_priority) {
            highestPriorityIndex = i;
        }
    }

    struct PCB next_process = ready_queue[highestPriorityIndex];

    // Shift elements to remove the highest priority process
    for (int i = highestPriorityIndex; i < (*queue_cnt) - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;

    next_process.execution_starttime = timestamp; // Set the start time for the next process
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime; //Set end time for the process
    return next_process;
}



// **** SRTF ****

struct PCB handle_process_arrival_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp) {
    if (*queue_cnt == QUEUEMAX) {
        printf("Ready queue is full. Dropping new process.\n");
        return current_process;
    }
    if (current_process.process_id == -1) { // If no process is running
        new_process.execution_starttime = timestamp;
        ready_queue[(*queue_cnt)++] = new_process;
        return new_process; 
    }

    // Check if new process should preempt (based on remaining burst time)
    if (new_process.total_bursttime < current_process.remaining_bursttime) {
        current_process.remaining_bursttime -= (timestamp - current_process.execution_starttime);
        ready_queue[(*queue_cnt)++] = current_process;
        new_process.execution_starttime = timestamp;
        return new_process;
    }
    ready_queue[(*queue_cnt)++] = new_process;
    return current_process;
}

struct PCB handle_process_completion_srtp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    if (*queue_cnt == 0) { // Handle empty queue
        struct PCB null_PCB = {-1, -1, -1, -1, -1, -1, -1};
        return null_PCB;
    }

    int next_process_index = findShortestRemainingTime(ready_queue, *queue_cnt); 
    struct PCB next_process = ready_queue[next_process_index];

    // Shift elements to remove the completed process
    for (int i = next_process_index; i < (*queue_cnt) - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;

    next_process.execution_starttime = timestamp; 
    return next_process;
}


// **** ROUND ROBIN ****

struct PCB handle_process_arrival_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp, int time_quantum)
{
    if (*queue_cnt < QUEUEMAX) {
        ready_queue[*queue_cnt] = new_process;
        (*queue_cnt)++;
    } 
    return current_process; // No preemption in RR on arrival
}

struct PCB handle_process_completion_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp, int time_quantum) {
    if (*queue_cnt == 0) { // Handle empty queue
        struct PCB null_PCB = {-1, -1, -1, -1, -1, -1, -1};
        return null_PCB;
    }

    struct PCB completed_process = ready_queue[0];
    for (int i = 0; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;

    // Check if process used its full time quantum
    if (completed_process.remaining_bursttime > 0) {
        completed_process.remaining_bursttime -= time_quantum;
        completed_process.execution_starttime = -1; // Mark as not running
        ready_queue[*queue_cnt] = completed_process; // Add back to the end of the queue
        (*queue_cnt)++;
    }

    // Update the execution start time of the next process (if any)
    if (*queue_cnt > 0) {
        ready_queue[0].execution_starttime = timestamp;
    }
    
    // Return the next process or a null PCB
    return (*queue_cnt > 0) ? ready_queue[0] : (struct PCB){-1, -1, -1, -1, -1, -1, -1};
}

