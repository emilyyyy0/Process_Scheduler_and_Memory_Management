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

// Function prototypes 
void initialize_paged_memory();
int allocate_pages(process_t *process);
void free_pages(process_t *process);
void evict_lru_pages(int num_frames_needed);
void update_lru(process_t *process);

#endif 