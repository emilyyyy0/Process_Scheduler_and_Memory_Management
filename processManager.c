#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "list.h"
#include "processManager.h"
#include "memoryManagement.h"


// Task 1: Round-Robin Scheduling with Infinite Memory 
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
        if (!check_arriving_process_2(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
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


// Task 3: Round-Robin Scheduling with Paged Memory Allocation
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

    int frames_allocated = 0;
    


    // Iterate through all process list until it is empty. Simulation start 
    while (process_list->head != NULL) {
        // print_list(process_list);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process_2(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            
            // Allocate memory for the process 
            if (allocate_pages(current_run, page_table, frame_table, lru_list, &frames_allocated, simul_time)) {
                current_run->state = RUNNING; // State is changed to running 
                if (strcmp(prev_process, current_run->process_id) != 0) {
                    start_process_paged(process_list, arrived_list, current_run, &simul_time, page_table, frames_allocated); // prints to stdout
                    //print_page_table(page_table);
                    prev_process = current_run->process_id;
                }
                //printf("frames allocated: %d\n", frames_allocated);
                
            }

            
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

                    free_pages(current_run, page_table, frame_table, lru_list, simul_time, &frames_allocated);
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


// Start process for task 3 and 4
void start_process_paged(list_t *process_list, list_t *arrived_list, process_t *current_process, int* current_time, page_table_entry_t *page_table, int total_num_frames_current) {
    //printf("in starting process paged function\n");
    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));



    // Need to dynamically reallocate to avoid buffer overflow 
    int capacity = 256; 
    char *frame_numbers = malloc(capacity * sizeof(char));
    frame_numbers[0] = '\0'; // Start with an empty string
    //char frame_numbers[256] = {0};  // Buffer to store frame numbers
    
    
    int first = 1;  // Flag to help format with commas


    if (state == 0) {
        strcpy(state_str,"DEFAULT");
    } else if (state == 1) {
        strcpy(state_str,"READY");
    } else if (state == 2) {
        strcpy(state_str, "RUNNING");
    }


    int memUsage = divide_and_round_up( total_num_frames_current * 100 ,NUM_PAGES);
    
    for (int i = 0; i < NUM_PAGES; i++) {

        // Check if there is enough space in frame_numbers, if not realloc
        if (strlen(frame_numbers) + 10 >= capacity) {
            capacity *= 2; // Double the capacity 
            char *new_frame_numbers = realloc(frame_numbers, capacity * sizeof(char));
                if (!new_frame_numbers) {
                    perror("Failed to reallocate frame_numbers");
                    free(state_str);
                    free(frame_numbers);
                    return;
                }
            frame_numbers = new_frame_numbers;
        }
        
        if ((page_table[i].process_id != NULL) && (strcmp(page_table[i].process_id, current_process->process_id) == 0)) {
            if (!first) {
                strcat(frame_numbers, ",");  // Add a comma before the next number except for the first
            }

            char frame_number_str[10];  // Buffer for the current frame number
            sprintf(frame_number_str, "%d", page_table[i].frame_number);
            strcat(frame_numbers, frame_number_str);  // Append current frame number to the list

            first = 0; // Update flag after the first frame number is added. 
        }
        
    }

    if (!first) {  // Only print if at least one frame was evicted
        printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%d%%,mem-frames=[%s]\n", *current_time, state_str, current_process->process_id, current_process->time_remain, memUsage, frame_numbers);

    }
    free(state_str);
    free(frame_numbers);
    
}


// Task 4: Round-Robin Scheduling with Virtual Memory Allocation 
void virtual(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {
    int simul_time = 0;
    int process_timer = quantum; // timer set to quantum as limit 
    char *prev_process = "beginning"; //variable that holds the previous process so we can check if the process is the same one that is running in the CPU
    int num_process_left = 0; // the number of processes waiting 

    // Initialise paged memory structures (page table, frame table)
    page_table_entry_t* page_table = initialise_page_table();
    int frame_table[TOTAL_MEMORY / PAGE_SIZE];
    list_t *lru_list = make_empty_list();
    initialise_frame_table(frame_table); 

    int frames_allocated = 0;
    


    // Iterate through all process list until it is empty. Simulation start 
    while (process_list->head != NULL) {
        // print_list(process_list);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process_2(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            
            // Allocate memory for the process 
            if (allocate_pages_virtual(current_run, page_table, frame_table, lru_list, &frames_allocated, simul_time)) {
                current_run->state = RUNNING; // State is changed to running 
                if (strcmp(prev_process, current_run->process_id) != 0) {
                    start_process_paged(process_list, arrived_list, current_run, &simul_time, page_table, frames_allocated); // prints to stdout
                    //print_page_table(page_table);
                    prev_process = current_run->process_id;
                }
                //printf("frames allocated: %d\n", frames_allocated);
                
            }

            
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

                    free_pages(current_run, page_table, frame_table, lru_list, simul_time, &frames_allocated);
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


// Task 2: Round Robin Scheduling with Contiguous Memory Allocation 
void first_fit(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum) {

    int simul_time = 0;
    int process_timer = quantum; // timer set to quantum as limit 
    char *prev_process = "beginning"; //variable that holds the previous process so we can check if the process is the same one that is running in the CPU
    int num_process_left = 0; // the number of processes waiting 

    // Initialise head of blocked memory
    memory_block_t *memory_head = NULL; 
    initialise_memory_block(&memory_head);

    


    // Iterate through all process list until it is empty. Simulation start 
    while (process_list->head != NULL) {
        // print_list(process_list);
        
        // check if there is any processes that have arrived
        if (!check_arriving_process_2(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait. 
        }


        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL &&  process_timer >= 0) {
            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t* current_run = remove_head(arrived_list);
            //printf("\n\nprocess_timer in the second while loop: %d\n", process_timer);
            
            // Allocate memory for the process 
            // if cant allocate move to back of queue, remain in READY state
            if (allocate_block(current_run, current_run->memory, memory_head)) {
                //printf("simul time: %d\n", simul_time);
                current_run->state = RUNNING; // State is changed to running 
                if (strcmp(prev_process, current_run->process_id) != 0) {
                    start_process_block(process_list, arrived_list, current_run, &simul_time, memory_head); // prints to stdout
                    prev_process = current_run->process_id;
                }

                
                //printf("frames allocated: %d\n", frames_allocated);
                
            } else {
                // if not allocated, put the process to back of the queue, 
                if (strcmp(prev_process, current_run->process_id) != 0) {
                    insert_at_foot(arrived_list, current_run);

                }
            }

            
            while (1) {
                //printf("in while loop 1 simul time: %d\n", simul_time);

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

                    //printf("finishe the process\n");
                    // when a process finishes, we free the block. And merge free blocks to avoid fragmentation
                    free_block(current_run, memory_head);
                    //print_process(tmp);
                    process_finish(complete_list, current_run, simul_time, &num_process_left);
                    process_timer = quantum;
                    break;
                }


                
                // if the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0) {
                    //printf("END OF QUANTUM\n");
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);
                    //print_list(arrived_list);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

    }
    // Free page table
    free_memory_blocks(memory_head);
    print_stats(complete_list, simul_time);

}

// Task 2 start
void start_process_block(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time, memory_block_t *memory_head) {
    // Find the allocated block for the process
    
    memory_block_t *current_block = memory_head; 
    while (current_block != NULL) {
        if (current_block->status == ALLOCATED && strcmp(current_block->process_id, current_process->process_id) == 0) {
            break; // FOUND the block
        }
        current_block = current_block->next;
    }

    // Calculate memory usage percentage
    int total_memory_used = 0;
    memory_block_t *block = memory_head; 
    while (block != NULL) {
        if (block != NULL) {
            if (block ->status == ALLOCATED) {
                total_memory_used += block->size; 
            }
        }
        block = block->next; 
    }


    // calcualte memory usage percentage 
    int mem_usage = divide_and_round_up(total_memory_used * 100, TOTAL_MEMORY);

    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));

    if (state == 0) {
        strcpy(state_str,"DEFAULT");
    } else if (state == 1) {
        strcpy(state_str,"READY");
    } else if (state == 2) {
        strcpy(state_str, "RUNNING");
    }

    if (current_block != NULL) {
        printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", 
                *current_time, state_str, current_process->process_id, current_process->time_remain, mem_usage, current_block->start_address);
    } else {
        // Handle the case where the block is not found (shouldn't happen if allocation was successful)
        printf("Error: Block not found for process %s\n", current_process->process_id);
    }

}



// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process_2(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left, int quantum) {
    //print_list(process_list);

    // Boolean integer value, if 0, means no incoming arrival process
    // if 1, means there are incoming process that are arriving. 
    int bool_incoming = 0; 


    // iterate through process_list, if simul_time >= current process, pop the process from process_list and push onto arrived_list.
    node_t *current = process_list->head; // Start from the head of the list
    node_t *next = NULL;

    while (current != NULL) { 
        next = current->next; // Save the next node before potentially modifying the list

        if ((simul_time >= current->data->arrival_time) && (simul_time % quantum == 0)){
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