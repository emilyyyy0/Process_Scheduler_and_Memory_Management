#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "list.h"
#include "processManager.h"
#include "memoryManagement.h"



/************************************************************** TASK 1 ***************************************************************/

void infinite(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum)
{
    int simul_time = 0;
    int process_timer = quantum;      // timer set to quantum as limit
    char *prev_process = "beginning"; // variable that holds the previous process so we can check if the process is the same one that is running in the CPU

    int num_process_left = 0; // the number of processes waiting

    // node_t* current = process_list->head;

    // Iterate through all process list until it is empty. Simulation start
    while (process_list->head != NULL)
    {
        // printf("big all process loop simulation time: %d\n", simul_time);
        //  print_list(process_list);

        // check if there is any processes that have arrived
        if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum))
        {                 // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
            simul_time++; // if no process has arrived, increase simulation time and wait.
        }

        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL && process_timer >= 0)
        {
            // printf("\n\nstarting to run process queue\n");
            // printf("The ARRIVED_LIST: \n");
            // print_list(arrived_list);

            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t *current_run = remove_head(arrived_list);
            // need to change the state to RUNNING

            current_run->state = 2; // State is changed to running
            if (strcmp(prev_process, current_run->process_id) != 0)
            {
                inf_start_process(process_list, arrived_list, current_run, &simul_time);
                prev_process = current_run->process_id;
            }
            // start_process(process_list, arrived_list, current_run, &simul_time);
            // printf("THE PROCESS CURRENTLY IN CPU: ");
            // print_process(current_run);

            while (1)
            {
                // add arrived processes to the queue, awaiting to be executed
                // printf("second arriving process: \n");
                check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                // current_run->time_remain--; // time remaining that the process needs to run in CPU
                if ((current_run->time_remain > 0))
                {
                    current_run->time_remain--;
                }

                simul_time++;    // current simulation time
                process_timer--; // the time that the process has been in the CPU

                // printf("process timer in the smallest while LOOP CHECK: %d\n", process_timer);
                // printf("current remaining time: %d\n\n", current_run->time_remain);
                //  check if process is finished, only finish the process
                // record the time it finished

                if ((current_run->time_remain == 0) && (process_timer == 0))
                {
                    current_run->state = 3; // change state to FINISHED
                    inf_process_finish(complete_list, current_run, simul_time, &num_process_left);
                    process_timer = quantum;
                    break;
                }

                // if the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0)
                {
                    // printf("The process timer has reached 0\n");
                    // printf("the third arriving process: \n");
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

        // current = current->next;
    }
    print_stats(complete_list, simul_time);
}

/************************************************************** TASK 2 ***************************************************************/

// Task 2: Round Robin Scheduling with Contiguous Memory Allocation
void first_fit(list_t *process_list, list_t *arrived_list, list_t *complete_list, m_list_t *memory_list, int quantum)
{


    int simul_time = 0;                         // The current time since the first process begins running
    int alloc_position = 0;                     // Stores the position that a memory node is allocated at for statistics
    float mem_usage = 0;                        // Stores the amount of memory (2048KB) currently used to store processes
    int percent_mem_usage;
    int process_timer = quantum;                // timer set to quantum as limit
    char *prev_process = "beginning";           // variable that holds the previous process so we can check if the process is the same one that is running in the CPU
    int allocated = 0;                          // Variable to check whether the memory has been allocated for the current process running
    int num_process_left = 0;                   // the number of processes waiting
 


    // Iterate through all process list until it is empty. Simulation start
    while (process_list->head != NULL)
    {
       

        // check if there is any processes that have arrived
        if (!first_fit_check_arriving_process(process_list, arrived_list, simul_time, &num_process_left))
        { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list

            simul_time++; // if no process has arrived, increase simulation time and wait.
        }

        // run the process_queue / process manager starts to schedule and give CPU time
        while (arrived_list->head != NULL && process_timer >= 0)
        {

            // if two processes have the same arrival time, the one that was not just executed goes first
            process_t *current_run = remove_head(arrived_list);


            // need to change the state to RUNNING
            current_run->state = 2; // State is changed to running

            // Checks if the memory has already been allocated for this process before (run on the CPU before)
            if (current_run->memoryAllocated)
            {
                allocated = 1;
            }
            // If memory hasn't been allocated yet, then allocate memory using allocateMemory()
            else
            {
                allocated = allocateMemory(memory_list, current_run->memory, current_run->process_id, &alloc_position);
                // Update the alloc position
                current_run->start_address = alloc_position;
                
            }

          
            while (1)
            {
                // add arrived processes to the queue, awaiting to be executed
       
                first_fit_check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                // If there is no memory space left then it doesnt run and goes to the tail of the queue
                if (!allocated)
                {
                 
                    first_fit_check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
                
            
                // This check ensures the same process isn't printed twice using start_process()
                if ((strcmp(prev_process, current_run->process_id) != 0))
                {   
                    mem_usage = 100 * ((float)memory_list->totalAllocated / MEMORY_CAPACITY);
                    percent_mem_usage = (int)ceil(mem_usage);
                  
                    while (simul_time%quantum != 0){
                        simul_time++;
                    }  
                 
                    mem_start_process(process_list, arrived_list, current_run, &simul_time, &percent_mem_usage, current_run->start_address);
                    
                    prev_process = current_run->process_id;
                
                  
                }
                
                // Runs a process continuously to completion as it takes up most/all of the Memory capacity
                // The first two conditions check that it is the only node stored in memory and the last condition checks
                // That there is another node that has arrived and is waiting but can't have its memory allocated currently
                else if (num_nodes(memory_list) == 2 && memory_list->foot->process_id == NULL && !is_list_empty(arrived_list)) {
                    // Checks that only one process can be fit into memory
                    if (get_head_mem_size(arrived_list) + current_run->memory > MEMORY_CAPACITY){
                        // Updates the mem_usage variables accordingly
                        mem_usage = 100 * ((float)memory_list->totalAllocated / MEMORY_CAPACITY);
                        percent_mem_usage = (int)ceil(mem_usage);
                        
                        // This skips the printing of the "RUNNING" process if the simul time is not a multiple of the quantum
                        while (simul_time%quantum != 0 && current_run->time_remain > 0){
                            simul_time++;
                            // This updates the time remaining for the process
                            current_run->time_remain--;
                        }

                        // Otherwise the process is "RUNNING" which is printed to output
                        if (simul_time%quantum == 0){
                            mem_start_process(process_list, arrived_list, current_run, &simul_time, &percent_mem_usage, current_run->start_address);
                        
                            prev_process = current_run->process_id;
                        }
                    }
                    

                }
             
                //  If the memory is successfully allocated then the memoryAllocated variable within the process is updated 
                if (allocated){
                    current_run->memoryAllocated = 1; // Creates a check in the process to verify its memory is allocated
                }
                

                // Decrements the time remaining for the process to run to completion
                if ((current_run->time_remain > 0))
                {
                    current_run->time_remain--;
                }

                simul_time++;    // current simulation time
                process_timer--; // the time that the process has been in the CPU

        

                // Check if the process has finished running after running for one quantum
                if ((current_run->time_remain == 0) && (process_timer == 0))
                {

                    current_run->state = 3; // change state to FINISHED
                    while (simul_time%quantum != 0){
                        simul_time++;
                    }  
                    // Prints the end statistics when a process has finished running
                    first_fit_check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);
                    mem_process_finish(complete_list, current_run, simul_time, &num_process_left);
                    process_timer = quantum;
                    deallocateMemory(memory_list, current_run->process_id);
                    current_run->memoryAllocated = 0;
                    break;
                }

                // if the process hasn't finished running and the quantum time is reached, reset the timer and move onto the next process
                if (process_timer == 0)
                {
                    

                    first_fit_check_arriving_process(process_list, arrived_list, simul_time, &num_process_left);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

     
    }
    print_stats(complete_list, simul_time);
}

// Function to check if there are any processes that have arrived at a particular simulation time
int first_fit_check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left) {
    

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
          
        }
        

        current = next; // Move to the next node, which we saved before any potential modification
        
        
    }
    return bool_incoming;

}

// Start current process, print the output.
void inf_start_process(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time)
{
    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));

    if (state == 0)
    {
        strcpy(state_str, "DEFAULT");
    }
    else if (state == 1)
    {
        strcpy(state_str, "READY");
    }
    else if (state == 2)
    {
        strcpy(state_str, "RUNNING");
    }

    printf("%d,%s,process-name=%s,remaining-time=%d\n", *current_time, state_str, current_process->process_id, current_process->time_remain);
    free(state_str);
}

void inf_process_finish(list_t *complete_list, process_t *current_process, int simul_time, int *num_process_left)
{

    *num_process_left = *num_process_left - 1;
    current_process->time_finished = simul_time;

    char finish_state[20] = "FINISHED";

    // need to add in number of processes remaining.
    printf("%d,%s,process-name=%s,proc-remaining=%d\n", simul_time, finish_state, current_process->process_id, *num_process_left);
    // insert finished process into finished process list.
    insert_at_foot(complete_list, current_process);
}

// Start current process, print the output.
void mem_start_process(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time, int *mem_usage, int alloc_position)
{
    int state = current_process->state;
    char *state_str = malloc(20 * sizeof(char));

    if (state == 0)
    {
        strcpy(state_str, "DEFAULT");
    }
    else if (state == 1)
    {
        strcpy(state_str, "READY");
    }
    else if (state == 2)
    {
        strcpy(state_str, "RUNNING");
    }

    printf("%d,%s,process-name=%s,remaining-time=%d,mem-usage=%d%%,allocated-at=%d\n", *current_time, state_str, current_process->process_id, current_process->time_remain, *mem_usage, alloc_position);
    free(state_str);
}


// A function to print the statistics when a process has finished running
void mem_process_finish(list_t *complete_list, process_t *current_process, int simul_time, int *num_process_left)
{

    *num_process_left = *num_process_left - 1;
    current_process->time_finished = simul_time;

    char finish_state[20] = "FINISHED";

    // need to add in number of processes remaining.
    printf("%d,%s,process-name=%s,proc-remaining=%d\n", simul_time, finish_state, current_process->process_id, *num_process_left);
    // insert finished process into finished process list.
    insert_at_foot(complete_list, current_process);
}


/************************************************************** TASK 3 and 4 ***************************************************************/

// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left, int quantum) {
    //print_list(process_list);

    // Boolean integer value, if 0, means no incoming arrival process
    // if 1, means there are incoming process that are arriving. 
    int bool_incoming = 0; 


    // iterate through process_list, if simul_time >= current process, pop the process from process_list and push onto arrived_list.
    node_t *current = process_list->head; // Start from the head of the list
    node_t *next = NULL;

    while (current != NULL) { 
        next = current->next; // Save the next node before potentially modifying the list

        if ((simul_time >= current->data->arrival_time) && (simul_time % quantum == 0)) {
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
    free(state_str);
}

// A function to print the statistics for a finished process
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
        if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
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
                check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                
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
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

    }
    // Free page table
    free_page_table(page_table);
    free_list(lru_list);
    print_stats(complete_list, simul_time);
}


// Start process for task 3
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
        if (!check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum)) { // if processes have arrived, they will be popped off the all process_list and pushed onto the arrived_list
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
                check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                
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
                    check_arriving_process(process_list, arrived_list, simul_time, &num_process_left, quantum);

                    insert_at_foot(arrived_list, current_run);

                    process_timer = quantum;
                    break;
                }
            }
        }

    }
    // Free page table
    free_page_table(page_table);
    free_list(lru_list);
    print_stats(complete_list, simul_time);

}