#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "list.h"
#include "processManager.h"
#include "memoryManagement.h"



void infinite(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {
    int simul_time = 0;
    int process_timer = quantum; // timer set to quantum as limit 
    char *prev_process = "beginning"; //variable that holds the previous process so we can check if the process is the same one that is running in the CPU

    int num_process_left = 0; // the number of processes waiting 


    //node_t* current = process_list->head;

    // Iterate through all process list until it is empty. Simulation start 
    while (process_list->head != NULL) {
        //printf("big all process loop simulation time: %d\n", simul_time);
        // print_list(process_list);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            // printf("\n\nstarting to run process queue\n");
            // printf("The ARRIVED_LIST: \n");
            //print_list(arrived_list);


            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            // need to change the state to RUNNING
            
            
            current_run->state = 2; // State is changed to running 
            if (strcmp(prev_process, current_run->process_id) != 0) {
                start_process(process_list, arrived_list, current_run, &simul_time);
                prev_process = current_run->process_id;
            }
            //start_process(process_list, arrived_list, current_run, &simul_time);
            //printf("THE PROCESS CURRENTLY IN CPU: ");
            //print_process(current_run);


            
            while (1) {
                // add arrived processes to the queue, awaiting to be executed
                //printf("second arriving process: \n");
                check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);
                
                //current_run->time_remain--; // time remaining that the process needs to run in CPU
                if ((current_run->time_remain > 0)) {
                    current_run->time_remain--;
                }

                simul_time++; // current simulation time 
                process_timer--; // the time that the process has been in the CPU
                
                //printf("process timer in the smallest while LOOP CHECK: %d\n", process_timer);
                //printf("current remaining time: %d\n\n", current_run->time_remain);
                // check if process is finished, only finish the process 
                //record the time it finished 
               

                if ((current_run->time_remain == 0) && (process_timer == 0)) {
                    current_run->state = 3; // change state to FINISHED
                    process_finish(complete_list, current_run, simul_time, &num_process_left);
                    process_timer = quantum;
                    break;
                }


                
                // if the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0) {
                   // printf("The process timer has reached 0\n");
                    //printf("the third arriving process: \n");
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }
        
        //current = current->next;

    }
    print_stats(complete_list, simul_time);
}


// Task 2: Round Robin Scheduling with Contiguous Memory Allocation 
void first_fit() {
    printf("first fit memory strat\n");
}



// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left) {
    //print_list(process_list);

    // Boolean integer value, if 0, means no incoming arrival process
    // if 1, means there are incoming process that are arriving. 
    int bool_incoming = 0; 


    // iterate through process_list, if simul_time >= current process, pop the process from process_list and push onto arrived_list.
    node_t *current = process_list->head; // Start from the head of the list
    node_t *next = NULL;

    while (current != NULL) { 
        next = current->next; // Save the next node before potentially modifying the list

        if (simul_time >= current->data->arrival_time) {
            // add to arrived_list;
            insert_at_foot(arrived_list, current->data);
            //printf("the process that has arrived= %s at time %d \n", current->data->process_id, simul_time);

            // then pop the process off of the all process list 
            remove_head(process_list);

            // could sort for safe measure
            *num_process_left = *num_process_left + 1;

            bool_incoming = 1;
            // printf("checking arrived processes: ");
            // print_process(current->data);

        }
        
        //current = current->next; 
        current = next; // Move to the next node, which we saved before any potential modification
        
        
    }
    return bool_incoming;

}

// Start task 1 current process, print the output.
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

    
    printf("%d,%s,process-name=%s,remaining-time=%d\n", *current_time, state_str, current_process->process_id, current_process->time_remain);
}


void process_finish(list_t* complete_list, process_t* current_process, int simul_time, int* num_process_left) {

    *num_process_left = *num_process_left - 1;
    current_process->time_finished = simul_time;


    char finish_state[20] = "FINISHED";
    //print_process(current_process);

    // need to add in number of processes remaining. 
    printf("%d,%s,process-name=%s,proc-remaining=%d\n", simul_time, finish_state, current_process->process_id, *num_process_left);
    // insert finished process into finished process list.
    insert_at_foot(complete_list, current_process);

}



// Task 4: Round-Robin Scheduling with Virtual Memory Allocation 
void virtual() {
    printf("Round-Robin Scheduling with Paged Memory Allocation\n");
}



// Task 3: Round-Robin Scheduling with Paged Memory Allocation 
// void paged(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {
//     int simul_time = 0;
//     int process_timer = quantum;
//     int num_process_left = 0;
//     // Initialize paged memory structures (page table, frame table, LRU list)
//     initialise_paged_memory();
//     while (process_list->head != NULL) {
//         // Check for arriving processes and add them to the arrived list
//         if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left)) {
//             simul_time++;
//         }
//         while (arrived_list->head != NULL && process_timer >= 0) {
//             process_t *current_process = remove_head(arrived_list);
//             // Attempt to allocate memory for the process
//             if (allocate_pages(current_process)) {
//                 current_process->state = RUNNING;
//                 start_process(process_list, arrived_list, current_process, &simul_time);
//                 while (1) {
//                     // Check for arriving processes
//                     check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);
//                     if (current_process->time_remain > 0) {
//                         current_process->time_remain--;
//                     }
//                     simul_time++;
//                     process_timer--;
//                     // Process finished execution
//                     if (current_process->time_remain == 0 && process_timer == 0) {
//                         current_process->state = FINISHED;
//                         process_finish(complete_list, current_process, simul_time, &num_process_left);
//                         free_pages(current_process); // Free pages of the finished process
//                         process_timer = quantum;
//                         break;
//                     }
//                     // Quantum time reached
//                     if (process_timer == 0) {
//                         check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);
//                         insert_at_foot(arrived_list, current_process);
//                         process_timer = quantum;
//                         break;
//                     }
//                 }
//             } else {
//                 // Memory allocation failed, move process to the back of the queue
//                 insert_at_foot(arrived_list, current_process);
//             }
//         }
//     }
//     print_stats(complete_list, simul_time);
// }


void paged(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {
    int simul_time = 0;
    int process_timer = quantum; // timer set to quantum as limit 
    char *prev_process = "beginning"; //variable that holds the previous process so we can check if the process is the same one that is running in the CPU
    int num_process_left = 0; // the number of processes waiting 

    // Initialise paged memory structures (page table, frame table)
    page_table_entry_t* page_table = initialise_page_table();
    int frame_table[TOTAL_MEMORY / PAGE_SIZE];
    list_t *lru_list = make_empty_list();
    initialise_frame_table(frame_table); 
    


    // Iterate through all process list until it is empty. Simulation start 
    while (process_list->head != NULL) {
        // print_list(process_list);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            
            // Allocate memory for the process 
            if (allocate_pages(current_run, page_table, frame_table, lru_list)) {
                current_run->state = RUNNING; // State is changed to running 
                if (strcmp(prev_process, current_run->process_id) != 0) {
                    start_process_paged(process_list, arrived_list, current_run, &simul_time, page_table); // prints to stdout
                    //print_page_table(page_table);
                    prev_process = current_run->process_id;
                }
                
            }
            
            // current_run->state = RUNNING; // State is changed to running 
            // if (strcmp(prev_process, current_run->process_id) != 0) {
            //     start_process(process_list, arrived_list, current_run, &simul_time); // prints to stdout
            //     prev_process = current_run->process_id;
            // }

            
            while (1) {
                // add arrived processes to the queue, awaiting to be executed
                check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                
                //current_run->time_remain--; // time remaining that the process needs to run in CPU
                if ((current_run->time_remain > 0)) {
                    current_run->time_remain--;
                }

                simul_time++; // current simulation time 
                process_timer--; // the time that the process has been in the CPU


                if ((current_run->time_remain == 0) && (process_timer == 0)) {
                    current_run->state = 3; // change state to FINISHED
                
                    //process_finish(complete_list, current_run, simul_time, &num_process_left);

                    free_pages(current_run, page_table, frame_table, lru_list, simul_time);
                    //print_process(tmp);
                    process_finish(complete_list, current_run, simul_time, &num_process_left);
                    process_timer = quantum;
                    break;
                }


                
                // if the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0) {
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

    }
    // Free page table
    free_page_table(page_table);
    print_stats(complete_list, simul_time);
}


// Start process for task 3
void start_process_paged(list_t *process_list, list_t *arrived_list, process_t *current_process, int* current_time, page_table_entry_t *page_table) {
    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));

    if (state == 0) {
        strcpy(state_str,"DEFAULT");
    } else if (state == 1) {
        strcpy(state_str,"READY");
    } else if (state == 2) {
        strcpy(state_str, "RUNNING");
    }


    
    printf("%d,%s,process-name=%s,remaining-time=%d, mem-usage=,mem-frames=[", *current_time, state_str, current_process->process_id, current_process->time_remain);
    for (int i = 0; i < NUM_PAGES; i++) {
        
        if (page_table[i].process_id != NULL) {
            if (i == 0) {
                printf("%d",page_table[i].frame_number); 
                continue;
            }
            printf(",");
            printf("%d",page_table[i].frame_number); 
        }
        
    }
    printf("]\n");
}