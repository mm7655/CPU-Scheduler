#include "oslabs.h"
#include <stdio.h>
#include <limits.h> 
#include <string.h>

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
        new_process.execution_endtime = timestamp + new_process.total_bursttime;
        new_process.remaining_bursttime = new_process.total_bursttime;
        ready_queue[(*queue_cnt)++] = new_process;
        return new_process;
    }

    // Check if new process should preempt
    if (new_process.process_priority < current_process.process_priority) {
        // Preempt the current process
        current_process.remaining_bursttime -= (timestamp - current_process.execution_starttime);
        current_process.execution_starttime = -1; 
        current_process.execution_endtime = 0; 

        // Add preempted process to the queue
        ready_queue[(*queue_cnt)++] = current_process; 

        //Start new process
        new_process.execution_starttime = timestamp;
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
    ready_queue[insertIndex] = new_process; // Insert preempted process
  
    return current_process; 
}

struct PCB handle_process_completion_pp(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp) {
    // Handle empty queue
    if (*queue_cnt == 0) {
        struct PCB null_PCB = {0, 0, 0, 0, 0, 0, 0}; // total_bursttime = 0
        return null_PCB;
    }

    int highestPriorityIndex = 0;
    for (int i = 1; i < *queue_cnt; i++) {
        if (ready_queue[i].process_priority < ready_queue[highestPriorityIndex].process_priority) {
            highestPriorityIndex = i;
        }
    }

    struct PCB next_process = ready_queue[highestPriorityIndex];

    for (int i = highestPriorityIndex; i < (*queue_cnt) - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }
    (*queue_cnt)--;

    next_process.execution_starttime = timestamp;
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime;
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
    // Handle empty queue
    if (*queue_cnt == 0) {
        struct PCB null_PCB = {0, 0, 0, 0, 0, 0, 0};
        return null_PCB; // Return a properly initialized null PCB
    }

    // Find the process with the shortest remaining burst time
    int shortestRemainingTimeIndex = findShortestRemainingTime(ready_queue, *queue_cnt);
    
    // If no process with remaining burst time is found, return a null PCB
    if (shortestRemainingTimeIndex == -1) {
        struct PCB null_PCB = {0, 0, 0, 0, 0, 0, 0};
        return null_PCB;
    }

    struct PCB next_process = ready_queue[shortestRemainingTimeIndex];

    // Shift elements to remove the completed process from the ready queue
    for (int i = shortestRemainingTimeIndex; i < (*queue_cnt) - 1; i++) {
        ready_queue[i] = ready_queue[i + 1];
    }

    // Decrease the queue count
    (*queue_cnt)--;

    // Update the execution start and end times for the selected process
    next_process.execution_starttime = timestamp; 
    next_process.execution_endtime = timestamp + next_process.remaining_bursttime; 

    // Return the process with the shortest remaining burst time to be executed next
    return next_process;
}


// **** ROUND ROBIN ****

struct PCB handle_process_arrival_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, struct PCB current_process, struct PCB new_process, int timestamp, int time_quantum) {
   
   // Handle full queue 
    if (*queue_cnt == QUEUEMAX) {
        printf("Ready queue is full. Dropping new process.\n");
        return current_process; 
    }
    
    // Insert the new process into the queue based on priority
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



struct PCB handle_process_completion_rr(struct PCB ready_queue[QUEUEMAX], int *queue_cnt, int timestamp, int time_quantum) {
    if (*queue_cnt == 0) {
        struct PCB null_PCB = {0, 0, 0, 0, 0, 0, 0};
        return null_PCB;
    }

    // Remove the completed/preempted process from the front
    struct PCB completed_process = ready_queue[0]; 
    for (int i = 0; i < *queue_cnt - 1; i++) {
        ready_queue[i] = ready_queue[i + 1]; 
    }
    (*queue_cnt)--; // Decrement queue count

    // Adjust execution times if process didn't complete in time quantum
    if (completed_process.remaining_bursttime > 0) {
        completed_process.remaining_bursttime -= time_quantum;
        completed_process.execution_starttime = -1;

        // Re-insert ONLY if there's space in the queue and if the process hasn't finished
        if (*queue_cnt < QUEUEMAX && completed_process.remaining_bursttime > 0) {
            // Add the preempted process to the end of the ready queue
            ready_queue[(*queue_cnt)++] = completed_process;
        } else {
            // Process completed, update execution_endtime if it hasn't been updated already (in case of preemption)
            if (completed_process.execution_endtime == 0) {
                completed_process.execution_endtime = timestamp;
            }
        }
    } else {
        // Process completed, update execution_endtime
        completed_process.execution_endtime = timestamp;
    }
    
    //Check for the case where there is only one item in the queue and it is a process that has been preempted before and did not finish
    if (*queue_cnt == 1 && ready_queue[0].execution_starttime == -1) {
        ready_queue[0].execution_starttime = timestamp;
    }

    // Update execution_starttime of the next process (if any)
    if (*queue_cnt > 0 && ready_queue[0].execution_starttime != -1) {
        ready_queue[0].execution_starttime = timestamp;
    }

    return (*queue_cnt > 0) ? ready_queue[0] : (struct PCB){-1, -1, -1, -1, -1, -1, -1}; 
}





