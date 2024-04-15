#ifndef MEMORYMANAGEMENT_H
#define MEMORYMANAGEMENT_H

#define PAGE_SIZE 4
#define TOTAL_MEMORY 2048
#define NUM_PAGES 512

#include "list.h"

// Structure to represent a page table entry
typedef struct {
    char *process_id;       // ID of the process owning the page
    int page_number;     // Page number within the process 
    int frame_number;    // Frame number allocated to the page
} page_table_entry_t;

/* Functions to initialise data structures for paged memory. Page table and frame table */

// Function to initialise frame table
void initialise_frame_table(int *frame_table);

// Function to initialise page table
void *initialise_page_table(void);

int allocate_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lry_list);

void evict_lru_pages(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list);

void free_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list);


// Update the lru
void update_lru(process_t *process, list_t *lru_list);

// Print page table
void print_page_table(page_table_entry_t *page_table);

// Print frame table
void print_frame_table(int *frame_table);

// Print the lru_list
void print_lru_list(list_t *lru_list);




#endif 