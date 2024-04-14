#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryManagement.h"
#include "list.h"

// Global variables for page table, frame table, and LRU list
page_table_entry_t page_table[NUM_PAGES];
int frame_table[TOTAL_MEMORY / PAGE_SIZE];
list_t *lru_list;

// Initialize paged memory system
void initialize_paged_memory() {
    // Initialize page table entries
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].process_id = NULL; // NULL indicates unallocated
        page_table[i].page_number = -1;
        page_table[i].frame_number = -1;
    }

    // Initialize frame table with all frames free
    for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
        frame_table[i] = 0; // 0 indicates free
    }

    // Create an empty LRU list
    lru_list = make_empty_list();
}

// Allocate pages for a process
int allocate_pages(process_t *process) {
    int num_pages_needed = process->memory / PAGE_SIZE;
    int num_frames_allocated = 0;
    int frame_number = 0;

    // Find free frames and allocate them
    while (num_frames_allocated < num_pages_needed && frame_number < TOTAL_MEMORY / PAGE_SIZE) {
        if (frame_table[frame_number] == 0) {
            // Allocate frame and update page table
            page_table[num_frames_allocated].process_id = process->process_id;
            page_table[num_frames_allocated].page_number = num_frames_allocated;
            page_table[num_frames_allocated].frame_number = frame_number;

            // Update frame table
            frame_table[frame_number] = 1; // Mark frame as occupied

            num_frames_allocated++;
        }
        frame_number++;
    }

    // Update LRU list
    update_lru(process);

    // Return 1 if all pages allocated, 0 otherwise
    return (num_frames_allocated == num_pages_needed);
}

// Free pages of a process
void free_pages(process_t *process) {
    // Iterate through page table and free frames
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id == process->process_id) {
            frame_table[page_table[i].frame_number] = 0; // Mark frame as free
            page_table[i].process_id = NULL; // Mark page as unallocated
            page_table[i].page_number = -1;
            page_table[i].frame_number = -1;
        }
    }

    // Remove process from LRU list
    node_t *current = lru_list->head;
    node_t *previous = NULL;
    while (current != NULL) {
        if (current->data->process_id == process->process_id) {
            if (previous == NULL) {
                lru_list->head = current->next;
            } else {
                previous->next = current->next;
            }
            free_node(current);
            break;
        }
        previous = current;
        current = current->next;
    }
}

// Evict pages using LRU policy
void evict_lru_pages(int num_frames_needed) {
    node_t *current = lru_list->foot; // Start from the tail (LRU)
    int num_frames_freed = 0;

    while (current != NULL && num_frames_freed < num_frames_needed) {
        process_t *lru_process = current->data;

        // Free all pages of the LRU process
        for (int i = 0; i < NUM_PAGES; i++) {
            if (strcmp(page_table[i].process_id, lru_process->process_id) == 0) {
                // Update frame table
                int frame_to_free = page_table[i].frame_number;
                frame_table[frame_to_free] = 0; // Mark frame as free
                
                // Print EVICTED event
                printf("%d,EVICTED,evicted-frames=[%d]\n", 
                       lru_process->time_finished, frame_to_free); 
                // Update page table
                page_table[i].process_id = NULL; 
                page_table[i].page_number = -1; 
                page_table[i].frame_number = -1; 
                
                num_frames_freed++;
                if (num_frames_freed == num_frames_needed) {
                    break; // Enough frames freed
                }
            }
        }

        // Remove LRU process from the list 
        lru_list->foot = current->prev; 
        if (lru_list->foot != NULL) {
            lru_list->foot->next = NULL; 
        } else {
            lru_list->head = NULL; // List becomes empty
        }
        free_node(current);
        current = lru_list->foot; // Move to the new tail (LRU) 
    }
}

// Update LRU list 
void update_lru(process_t *process) {
    // Find the process node in the LRU list
    node_t *current = lru_list->head;
    node_t *previous = NULL;
    while (current != NULL) {
        if (current->data->process_id == process->process_id) {
            break; // Found the node
        }
        previous = current;
        current = current->next;
    }

    // If the process is already in the list, move it to the head
    if (current != NULL) {
        if (previous != NULL) {
            previous->next = current->next;
            current->next = lru_list->head;
            lru_list->head = current;
        } 
    } else {
        // If the process is not in the list, create a new node and insert it at the head
        node_t *new_node = (node_t*)malloc(sizeof(node_t));
        new_node->data = process;
        new_node->next = lru_list->head;
        lru_list->head = new_node;
        if (lru_list->foot == NULL) {
            lru_list->foot = new_node;
        }
    }
}