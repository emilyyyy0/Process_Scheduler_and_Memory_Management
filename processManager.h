#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "memoryManagement.h"


// Task 1: Round Robin Scheduling with Infinite Memory 
void infinite(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum);

// Task 2: Round-Robin Scheduling with Contiguous Memory Allocation
void first_fit(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum);

// Task 3: Round-Robin Scheduling with Paged Memory Allocation
void paged(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum);

// Task 4: Round-Robin with Virtual Memory Allocation
void virtual(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum);



// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left);

// Start current process, print the output.
void start_process(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time);

// Start the current process, task 3, paged
void start_process_paged(list_t *process_list, list_t *arrived_list, process_t *current_process, int* current_time, page_table_entry_t *page_table, int total_num_frames_current);


// Start the contiguous memory allocation task 2 
void start_process_block(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time, memory_block_t *memory_head);


// Process has finished using the CPU. 
void process_finish(list_t* complete_list, process_t* current_process, int simul_time, int* num_process_left);

// process function prototype, checks if sumul time is on time
int check_arriving_process_2(list_t *process_list, list_t *arrived_list, int simul_time, int* num_process_left, int quantum);


#endif 