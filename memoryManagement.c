#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryManagement.h"
#include "list.h"


// Initialize frame table
void initialise_frame_table(int *frame_table) {
    
    // Initialize frame table with all frames free
    for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
        frame_table[i] = 0; // 0 indicates free
    }
    //printf("initialise frame table function working\n");
}

// Initialise page table
void *initialise_page_table(void) {
    page_table_entry_t *page_table = malloc(NUM_PAGES * sizeof(page_table_entry_t));
    // Check if the allocation was successful
    if (page_table == NULL) {
        fprintf(stderr, "Failed to allocate memory for page table.\n");
        return NULL;
    }
    // Initialize each entry
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].process_id = NULL;  // No process assigned yet
        page_table[i].page_number = -1;   // Invalid page number
        page_table[i].frame_number = -1;  // Invalid frame number
    }

    return page_table;  // Return the pointer to the allocated and initialized page table
}




// Allocate pages for a process
int allocate_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list) {
    // first check if there are enough empty frames
        // if there are enough, allocate the frames to the process  
        // NOT enough, call LRU function and evict ALL pages from the tail of the lru list, 
    // check again if there there are enought frames
        // if still not enough, call LRU function again and evict ALL pages from the process at the LRU list tail again 
    // keep doing so until there are enough free frames for the process
    // then allocate the frames to the process
    // update the page table and frame table and lru list, 
    // return 1 for true as all pages in the process are allocated.

    int num_pages_needed = process->memory / PAGE_SIZE; // number of pages needed for the process
    int num_frames_allocated = 0; // number of frames already allocated in the frame table 

    // Loop until all needed pages are allocated 
    while (num_frames_allocated < num_pages_needed) {
        // check if there are enough free frames
        int num_free_frames = 0; 
        for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
            if (frame_table[i] == 0) {
                num_free_frames++;
            }
        }

        // if there are not enough free frames, evict pages using LRU 
        if (num_free_frames < num_pages_needed) {
            evict_lru_pages(num_pages_needed - num_free_frames, page_table, frame_table, lru_list);
        } else {
            // Allocate frames in increasing order
            for (int frame_number = 0; frame_number < NUM_PAGES ; frame_number++) {
                if (frame_table[frame_number] == 0) {
                    // Allocate frame and update page table

                    allocate_process_id_page_table(&page_table[num_frames_allocated], process);
                    //printf("Frame allocated for process_id at address: %p\n", (void *)page_table[num_frames_allocated].process_id);
                    //printf("process process_id at address: %p\n", (void *)process->process_id);

                    page_table[num_frames_allocated].page_number = num_frames_allocated;
                    page_table[num_frames_allocated].frame_number = frame_number;
                    num_frames_allocated++;
                    
                    frame_table[frame_number] = 1; // Mark frame as occupied
                    if (num_frames_allocated == num_pages_needed) {
                        break; // All pages allocated
                    }

                }
            }
        }

    }

    update_lru(process, lru_list);
        
    return 1; // All pages allocated successfully. 
}


void evict_lru_pages(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list) {
    node_t *current = lru_list->foot; // Start from the tail (LRU)

    printf("in the evict_lru_page function ");

    while (current != NULL && num_frames_needed > 0) {
        process_t *lru_process = current->data; 

        // Evict all pages of the LRU process
        for (int i = 0; i < NUM_PAGES; i++) {
            if (strcmp(page_table[i].process_id, lru_process->process_id) == 0) {
                // update frame table
                int frame_to_free = page_table[i].frame_number; 
                frame_table[frame_to_free] = 0; // Mark frame as free. 

                // print EVICTED event
                printf("%d,EVICTED,evicted-frames=[%d]\n", lru_process->time_finished, frame_to_free); // CHANGE THIS PRINT STATEMENT

                // Update page table
                page_table[i].process_id = NULL; 
                page_table[i].page_number = -1; 
                page_table[i].frame_number = -1;

                num_frames_needed--; // Decrement needed frames count 
            }
        }

        node_t *temp = current; 
        lru_list->foot = current->prev; 

        if (lru_list->foot != NULL) {
            lru_list->foot->next = NULL;
        } else {
            lru_list->head = NULL; // List is now empty 
        }
        free_node(temp);
        current = lru_list->foot; // Move to the new tail (LRU)


    }


}


// update lru list 
void update_lru(process_t *process, list_t *lru_list) {
    //printf("Updating lru\n");
    // Find the process node in the LRU list 

    node_t *current = lru_list->head;
    node_t *previous = NULL; 
    while (current != NULL) {
        if (strcmp(current->data->process_id, process->process_id) == 0) {
            break; // found the node
        }
        previous = current; 
        current = current->next; 
    }


    // If the process is already in the list, move it to the head
    if (current != NULL) {
        if (previous != NULL) {
            // Remove the node from its current position 
            previous->next = current->next; 
            if (current->next != NULL) {
                current->next->prev = previous; // Maintains doubly linked list
            } else {
                lru_list->foot = previous; // update tail if removing the last node
            }

            // move the node to the head
            current->next = lru_list->head;
            lru_list->head->prev = current; // Maintain a doubly linked list 
            lru_list->head = current;

        }
    } else {
        // If the process is not in the list, create a new node and insert it at the head
        node_t *new_node = (node_t*)malloc(sizeof(node_t));
        if (new_node == NULL) {
            fprintf(stderr, "Failed to allocate memory for new node.\n");
            return;
        }

        // Copy the process
        new_node->data = copy_process(process);
        new_node->next = lru_list->head;
        new_node->prev = NULL;
        

        if (lru_list->head != NULL) {
            lru_list->head->prev = new_node; // Update prev of old head
        }
        lru_list->head = new_node; 
        if (lru_list->foot == NULL) {
            lru_list->foot = new_node; // if list was empty, new node is also the tail
        }
    }

}

//Free pages of a process
void free_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time) {
    // Iterate through page table and free frames. 
    //printf("free page function\n");

    char frame_numbers[256] = {0};  // Buffer to store frame numbers
    int first = 1;  // Flag to help format with commas


    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id == NULL) {
            break;  // No more entries to check
        }

        if (strcmp(page_table[i].process_id, process->process_id) == 0) {
            if (!first) {
                strcat(frame_numbers, ",");  // Add a comma before the next number except for the first
            }
            char frame_number_str[10];  // Buffer for the current frame number
            sprintf(frame_number_str, "%d", page_table[i].frame_number);
            strcat(frame_numbers, frame_number_str);  // Append current frame number to the list

            frame_table[page_table[i].frame_number] = 0; // Mark frame as free
            page_table[i].process_id = NULL;  // Mark page as unallocated
            page_table[i].page_number = -1;
            page_table[i].frame_number = -1;

            first = 0;  // Update flag after the first frame number is added
        }
    }

    if (!first) {  // Only print if at least one frame was evicted
        printf("%d,EVICTED,evicted-frames=[%s]\n", simul_time, frame_numbers);
    }

    // Remove process from LRU list
    node_t *current = lru_list->head;
    node_t *previous = NULL;
    while (current != NULL) {

        if (current->data->process_id == process->process_id) { // could potentially cause problems
            if (previous == NULL) {
                lru_list->head = current->next;
                if (lru_list->head != NULL) {
                    lru_list->head->prev = NULL; // Update prev of new head
                } else {
                    lru_list->foot = NULL; // List is now empty
                }
            } else {
                previous->next = current->next;
                if (current->next != NULL) {
                    current->next->prev = previous; // Maintain doubly linked list
                } else {
                    lru_list->foot = previous; // Update tail if removing last node
                }
            }
            free_node(current);
            break;
        }
        previous = current;
        current = current->next;
    }


}



// Allocate process_id in page_table_entry_t 
void allocate_process_id_page_table(page_table_entry_t *entry, process_t *process) {
    char *id = process->process_id;
    if (entry == NULL ||  id == NULL) {
        return; // Safety check
    }

    // Free existing process_id if it exists
    if (entry->process_id != NULL) {
        free(entry->process_id);
        entry->process_id = NULL; // Avoid dangling pointers
    }

    // Allocate memory for the new process_id
    entry->process_id = malloc(strlen(id) + 1); // +1 for the null terminator
    if (entry->process_id == NULL) {
        fprintf(stderr, "Failed to allocate memory for process_id.\n");
        return;
    }

    // Copy the new process_id into the allocated memory
    strcpy(entry->process_id, id);
}



// Copy process to put into LRU, we do this to avoid memory errors.
process_t *copy_process(process_t *process) {
    if (process == NULL) {
        return NULL; // Safety 
    }

    process_t *copy = (process_t *)malloc(sizeof(process_t));
    if (copy == NULL) {
        return NULL; // Safety
    }

    // Copy all fields 
    copy->arrival_time = process->arrival_time;
    copy->execution_time = process->execution_time;
    copy->memory = process->memory; 
    copy->state = process->state;
    copy->time_remain = process->time_remain;
    copy->time_finished = process->time_finished;

    // copy process_id string
    if (process->process_id != NULL) {
        copy->process_id = malloc(strlen(process->process_id) + 1);
        if (copy->process_id != NULL) {
            strcpy(copy->process_id, process->process_id);
        }
    } else {
        copy->process_id = NULL;
    
    }

    return copy;

}



void print_page_table(page_table_entry_t *page_table) {
    printf("Page Table:\n");
    printf("----------\n");
    printf("Page Num | Process ID | Frame Num\n");
    printf("---------|------------|----------\n");

    for (int i = 0; i < NUM_PAGES; i++) {
        // if (page_table[i].process_id != NULL) {
        //     printf("%8d | %10s | %9d\n", 
        //            page_table[i].page_number, 
        //            page_table[i].process_id, 
        //            page_table[i].frame_number); 
        // }
         printf("%8d | %10s | %9d\n", 
            page_table[i].page_number, 
            page_table[i].process_id, 
            page_table[i].frame_number); 
    }
    printf("\n");
}


void print_frame_table(int *frame_table) {
    printf("Frame Table:\n");
    printf("------------\n");
    for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
        if (frame_table[i] == 0) {
            printf("Frame %d: Free\n", i);
        } else {
            printf("Frame %d: Occupied\n", i);
        }
    }
    printf("\n"); 
}

void print_lru_list(list_t *lru_list) {
    printf("LRU List (Head to Tail):\n");
    printf("-----------------------\n");
    node_t *current = lru_list->head;
    while (current != NULL) {
        printf("Process %s\n", current->data->process_id);
        current = current->next;
    }
    printf("\n");

}


void free_page_table(page_table_entry_t *page_table) {
    if (page_table == NULL) {
        return; // If the pointer is NULL, there's nothing to free
    }

    // Free each dynamically allocated process_id
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id != NULL) {
            free(page_table[i].process_id);
        }
    }

    // Free the page table itself
    //printf("freeing the page table\n");
    free(page_table);

}