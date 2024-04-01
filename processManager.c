#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "list.h"
#include "processManager.h"

void infinite(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {
    int simul_time = 0;
    int process_timer = quantum; // timer set to quantum as limit 

    node_t* current = process_list->head;

    // Iterate through all process list until it is empty. Simulation start 
    while (current != NULL && simul_time <= 30) {
        printf("big all process loop simulation time: %d\n", simul_time);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process(process_list, arrived_list, simul_time)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            printf("\n\nstarting to run process queue\n");
            printf("The ARRIVED_LIST: \n");
            print_list(arrived_list);


            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            // need to change the state to RUNNING
            current_run->state = 2; // State is changed to running 
            printf("\nOUTPUT: \n");
            start_process(process_list, arrived_list, current_run, &simul_time);
            printf("THE PROCESS CURRENTLY IN CPU: ");
            print_process(current_run);


            
            while (1) {
                printf("process timer in the smallest while LOOP CHECK: %d\n", process_timer);
                // add arrived processes to the queue, awaiting to be executed
                printf("second arriving process: \n");
                check_arriving_process(process_list, arrived_list, simul_time);
                
                current_run->time_remain--;
                simul_time++;
                process_timer--;
                // give process cpu 

                // check if process is finished
                if(current_run->time_remain == 0) {
                    current_run->state = 3; // change state to FINISHED
                    process_finish(complete_list, current_run, simul_time );
                    process_timer = quantum;
                    break;
                }


                
                // if the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0) {
                    printf("The process timer has reached 0\n");
                    printf("the third arriving process: \n");
                    check_arriving_process(process_list, arrived_list, simul_time);


                    insert_at_foot(arrived_list, current_run);
                   
                    

                    process_timer = quantum;
                    break;
                }
            }
        }
        
        //current = current->next;

    }


    printf("infinite memory task \n");
    //print_list(process_list);
    printf("quantum: %d, simulation time: %d\n", quantum, simul_time);

    printf("length of list: %d\n", len_list(process_list));
}


// Task 2: Round Robin Scheduling with Contiguous Memory Allocation 
void first_fit() {
    printf("first fit memory strat\n");
}



// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time) {

    // Boolean integer value, if 0, means no incoming arrival process
    // if 1, means there are incoming process that are arriving. 
    int bool_incoming = 0; 


    // iterate through process_list, if simul_time >= current process, pop the process from process_list and push onto arrived_list.
    node_t *current = process_list->head; // Start from the head of the list

    while (current != NULL) { 

        if (simul_time >= current->data->arrival_time) {
            // add to arrived_list;
            insert_at_head(arrived_list, current->data);
            printf("the process that has arrived= %s at time %d \n", current->data->process_id, simul_time);

            // then pop the process off of the all process list 
            remove_head(process_list);

            // could sort for safe measure

            bool_incoming = 1;

        }
        current = current->next; 
        
    }

    return bool_incoming;

}

// Start current process, print the output.
void start_process(list_t *process_list, list_t *arrived_list, process_t *current_process, int* current_time) {
    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));

    if (state == 0) {
        strcpy(state_str,"DEFAULT");
    } else if (state == 1) {
        strcpy(state_str,"READY");
    } else if (state == 2) {
        strcpy(state_str, "RUNNING");
    }

    printf("%d,%s,process-name=%s,remaining-time=%d  TESTING OUTPUT\n", *current_time, state_str, current_process->process_id, current_process->time_remain);
}


void process_finish(list_t* complete_list, process_t* current_process, int simul_time) {

    char finish_state[20] = "FINISHED";

    // need to add in number of processes remaining. 
    printf("%d,%s,process-name=%s,proc-remaining\n", simul_time, finish_state, current_process->process_id);
    // insert finished process into finished process list.
    insert_at_foot(complete_list, current_process);

}