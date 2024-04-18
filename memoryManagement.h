#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#define PAGE_SIZE 4
#define TOTAL_MEMORY 2048
#define NUM_PAGES 512

// Defines for blocked memory 
#define FREE 0
#define ALLOCATED 1

#include "list.h"

// Structure to represent a page table entry
typedef struct {
    char *process_id;       // ID of the process owning the page
    int page_number;     // Page number within the process 
    int frame_number;    // Frame number allocated to the page
} page_table_entry_t;


// Struct for Blocked Memory
typedef struct memory_block {
    int start_address; // starting address of the block 
    int size; // the size of block in KB
    int status; // 0 for FREE and 1 for ALLOCATED
    char *process_id; // ID of the process occupying the block (if allocated) otherwise if unallocated, it is NULL
    struct memory_block *next; // this poitns to the next memory block in the linked list 
} memory_block_t;

/************************** Functions to initialise data structures for paged memory. Page table and frame table *****************************/

// Function to initialise frame table
void initialise_frame_table(int *frame_table);

// Function to initialise page table
void *initialise_page_table(void);

// Function creates the initial state of the memory block list with a single free block reprsenting all available memory
void initialise_memory_block(memory_block_t **head);


/********************************* Allocate, Evict, Free Memory Functions ******************************************************/

int allocate_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lry_list, int* total_frames_allocated, int simul_time);

int evict_lru_pages(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time);

void free_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time, int* frames_allocated);

// Allocate pages for virtual - Task 4
int allocate_pages_virtual(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int* total_frames_allocated, int simul_time);

// Evict least recently used pages in process - Task 4
int evict_lru_pages_virtual(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time);


// Update the lru
void update_lru(process_t *process, list_t *lru_list);

// Allocate process_id to page_table_entry process id to avoid same memory address
void allocate_process_id_page_table(page_table_entry_t *entry, process_t *process);

// Copy the process to put into LRU_list
process_t *copy_process(process_t *process);

// Function to check if process has already been allocated paged memory
int check_process_allocated(process_t* process, page_table_entry_t *page_table);

// Function to count number of pages allocated in a process
int count_num_allocated(process_t* process, page_table_entry_t *page_table);


// Allocate block for Contiguous memory - Task 2
int allocate_block(process_t *process, int size, memory_block_t *memory_head);

// Free blocks and merge if there are holes next to it - Task 2
void free_block(process_t *process, memory_block_t *memory_head);

/************************************************* Printing Functions *********************************************************/

// Print page table
void print_page_table(page_table_entry_t *page_table);

// Print frame table
void print_frame_table(int *frame_table);

// Print the lru_list
void print_lru_list(list_t *lru_list);


/************************************************* Freeing Functions *********************************************************/

// Free page table
void free_page_table(page_table_entry_t *page_table);

// Free memory blocks 
void free_memory_blocks(memory_block_t *head);


#endif 