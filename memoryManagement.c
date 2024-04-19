#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryManagement.h"
#include "list.h"

/******************************************************************** INITIALISE FUNCTIONS ****************************************************************************************************/


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

// Intialise Memory Block 
void initialise_memory_block(memory_block_t **head) {
    *head = malloc(sizeof(memory_block_t));
    if (*head == NULL) {
        perror("Memory allocation failed for initial memory block");
        exit(1); // Handles allocation failutre
    }

    (*head)->start_address = 0; 
    (*head)->size = TOTAL_MEMORY;
    (*head)->status = FREE;
    (*head)->process_id = NULL;
    (*head)->next = NULL;

}



/******************************************************************** ALLOCATE, EVICT, FREE, UPDATE FUNCTIONS ****************************************************************************************************/


// Allocate pages for a process - Task 3
int allocate_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int* total_frames_allocated, int simul_time) {
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
    int remainder = process->memory % PAGE_SIZE; // need to allocate an extra page if not enough
    num_pages_needed += remainder;
    // printf("modulo: %d\n", process->memory % PAGE_SIZE);

    // printf("number of pages needed: %d\n", num_pages_needed);


    int num_frames_allocated = 0; // number of frames allocated in the frame table 

    // NEED TO CHECK IF PROCESS HAS ALREADY BEEN ALLOCATED
    // if already been allocated, continue running and update lru_list 
    if (check_process_allocated(process, page_table)) {
        update_lru(process, lru_list);
        return 1;
    }

    
    //printf("\n\n\n\nallocating memory function:\n process_id: %s\n number of pages needed: %d\n total number of frames occuped in memory: %d\n ", process->process_id, num_pages_needed, *total_frames_allocated);
    


    int num_free_frames = 0; // the number of free frames in memory right now
    int num_evicted = 0; // the number of frames evicted if there are not enough to allocate for process

    // Loop until all needed pages are allocated 
    while (num_frames_allocated < num_pages_needed) {
        // check if there are enough free frames
        //int num_free_frames = 0; 
        for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
            if (frame_table[i] == 0) {
                num_free_frames++;
            }
        }
        

        // if there are not enough free frames, evict pages using LRU 
        if (num_free_frames < num_pages_needed) {
            num_evicted = evict_lru_pages(num_pages_needed - num_free_frames, page_table, frame_table, lru_list, simul_time);
            // if frames are evicted, change the total number of frames allocated
            *total_frames_allocated = *total_frames_allocated - num_evicted;
        } else {
            // Allocate frames in increasing order
            for (int frame_number = 0; frame_number < NUM_PAGES ; frame_number++) { 
                
                if (frame_table[frame_number] == 0) {
                    // Allocate frame and update page table

                    allocate_process_id_page_table(&page_table[frame_number], process);

                    page_table[frame_number].page_number = frame_number; // allocate page number 
                    page_table[frame_number].frame_number = frame_number; // allocate frame number 
                    num_frames_allocated++;
                    
                    frame_table[frame_number] = 1; // Mark frame as occupied
                    if (num_frames_allocated == num_pages_needed) {
                        break; // All pages allocated
                    }
                }
            }
        }

    }
    //printf("ALLOCATED\n");
    //print_page_table(page_table);

    update_lru(process, lru_list);

    // num_free_frames + num_evicted = current number of free frames
    // then current_number of free frames - num_pages needed  
    //int total_num_frames_allocated = (num_free_frames + num_evicted) - num_pages_needed;
    //printf("num frames allocated to this process: %d\n", num_frames_allocated);
    *total_frames_allocated = *total_frames_allocated + num_frames_allocated;
    //printf("total num frames currently allocated: %d\n", *total_frames_allocated );

    // total_frames_allocated = *total_num_frames_allocated + total_num_frames_allocated;
        
    return 1; // All pages allocated successfully. 
}

// Evict pages in process that was least recently used task 3
int evict_lru_pages(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time) {
    node_t *current = lru_list->foot; // Start from the tail (LRU)

    // Dynamic memory to avoid buffer overflow for printing 
    int capacity = 256; 
    char *frame_numbers = malloc(capacity * sizeof(char)); 
    frame_numbers[0] = '\0'; // Start with an empty string

    int first = 1; // Flag to help format with commas

    //printf("in the evict_lru_page function\n");
    int num_evicted = 0;

    while (current != NULL && num_frames_needed > 0) {
        process_t *lru_process = current->data; 

        // Evict all pages of the LRU process
        for (int i = 0; i < NUM_PAGES; i++) {
            

            // Check if there is enough space in frame_numbers, if not realloc
            if (strlen(frame_numbers) + 10 >= capacity) {
                capacity *= 2; // Double the capacity 
                char *new_frame_numbers = realloc(frame_numbers, capacity * sizeof(char));
                    
                frame_numbers = new_frame_numbers;
            }



            if ((page_table[i].process_id != NULL) && (strcmp(page_table[i].process_id, lru_process->process_id) == 0)) {

                 if (!first) {
                    strcat(frame_numbers, ",");  // Add a comma before the next number except for the first
                }

                char frame_number_str[10];  // Buffer for the current frame number
                sprintf(frame_number_str, "%d", page_table[i].frame_number);
                strcat(frame_numbers, frame_number_str);  // Append current frame number to the list

                // update frame table
                int frame_to_free = page_table[i].frame_number; 
                frame_table[frame_to_free] = 0; // Mark frame as free. 

                // print EVICTED event
                //printf("%d,EVICTED,evicted-frames=[%d]\n",simul_time, frame_to_free); // CHANGE THIS PRINT STATEMENT

                // Update page table
                page_table[i].process_id = NULL; 
                page_table[i].page_number = -1; 
                page_table[i].frame_number = -1;

                num_frames_needed--; // Decrement needed frames count 
                num_evicted++;

                first = 0; // Update flag after the first frame number is added
            }
        }

        // Print the EVICTED frames
        if (!first) {  // Only print if at least one frame was evicted
            printf("%d,EVICTED,evicted-frames=[%s]\n", simul_time, frame_numbers);
        }


        // Change LRU_List
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

    free(frame_numbers);
    return num_evicted;

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
void free_pages(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time, int* frames_allocated) {
    // Iterate through page table and free frames. 
    //printf("free page function\n");

    // Dynamic memory to avoid buffer overflow
    int capacity = 256; 
    char *frame_numbers = malloc(capacity * sizeof(char));
    frame_numbers[0] = '\0'; // Start with an empty string

    //char frame_numbers[256] = {0};  // Buffer to store frame numbers
    
    int first = 1;  // Flag to help format with commas


    for (int i = 0; i < NUM_PAGES; i++) {

        // Check if there is enough space in frame_numbers, if not realloc
        if (strlen(frame_numbers) + 10 >= capacity) {
            capacity *= 2; // Double the capacity 
            char *new_frame_numbers = realloc(frame_numbers, capacity * sizeof(char));
                if (!new_frame_numbers) {
                    perror("Failed to reallocate frame_numbers");
                    free(frame_numbers);
                    return;
                }
            frame_numbers = new_frame_numbers;
        }

    
        if ((page_table[i].process_id != NULL) && strcmp(page_table[i].process_id, process->process_id) == 0) {
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

            *frames_allocated = *frames_allocated - 1;
            
            first = 0; // Update flag after the first frame number is added. 

        }
    }

    // Print the EVICTED frames
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

    free(frame_numbers);

}



// Allocate pages for a process - Task 4
int allocate_pages_virtual(process_t *process, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int* total_frames_allocated, int simul_time) {
    // first check if there are at least 4 empty frames
    // num_frames_free = 512 frames - total_frames_allocated
        // if num_frames_free >= 4, allocate the frames to the process  (num_frames is at least 4)
        // else not enough (num_frames_free < 4)
            // we call LRU function and evict (4-x) pages out of a process
 
    // then allocate the frames to the process
        // update the page table and frame table 
    // update lru_list
    // update number of frames allocated.
    // return 1 for true as all pages in the process are allocated.


    // think about edge case for when less than 4 pages are needed. check how many pages are needed for the process.
    // if num_pages_needed < 4, then allocate normally

    
    int num_pages_needed = process->memory / PAGE_SIZE; 
    int remainder = process->memory % PAGE_SIZE; // need to allocate an extra page if not enough
    num_pages_needed += remainder; // Number of pages needed for the process

    //int total_pages_process = num_pages_needed; // this the the max number of pages a process can have 

    int num_FREE_frames = NUM_PAGES - *total_frames_allocated;

    int num_pages_already_allocated = count_num_allocated(process, page_table);

    //printf("\n\n\nBEFORE\nprocess_id: %s\nnumber of pages needed: %d\nnumber of FREE frames: %d\nnumber of pages allocated already: %d\n", process->process_id, num_pages_needed, num_FREE_frames, num_pages_already_allocated);

    // If the number of pages allocated in a process is >= 4, and the number of pages needed is >= 4 then we continue running, as do not need to allocate more memory.
    // Update the lru_list as process has been used. 
    if ((num_pages_already_allocated >= 4) && (num_pages_needed >= 4)) {
        //printf("************************** EXITING THE ALLOCATE MEMORY, already enough memory ***************************\n");
        //print_page_table(page_table);
        update_lru(process, lru_list);
        return 1;
    }

    // Function handles edge case, if process requires less than 4 pages, and if all pages already allocated, then we can run.
    // processes with < 4 pages, all pages must be allocated 
    if ((num_pages_needed < 4) && (num_pages_already_allocated == num_pages_needed)) {
        printf("exiting the allocate memory, already enough memory, pages < 4\n");
        update_lru(process, lru_list);
        return 1; 
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Now we find number of pages needed to allocate in order to run the process
    int num_frames_to_evict = 0; 

    // If there are more free frames than pages needed, then we can allocate all pages to a frame 
    // e.g. 512 frames, 500 pages needed by a process. 
    if (num_FREE_frames >= num_pages_needed) {
        //printf("all pages can be allocated\n");
        //num_pages_needed = num_FREE_frames - num_pages_needed;
        // num to evict = 0; 
    } else if ((num_pages_already_allocated < 4) && (num_FREE_frames >= num_pages_needed)) { 
        // If there are less than 4 pages allocated in a process but more frames than needed so no frames need to be victed. 
        int curr_num_pages_needed = 4 - num_pages_already_allocated; 
        printf("inside the else if\n");

        // but if number of free frames is more than curr_num_pages_needed, then the number of pages we can allocate = num_FREE_frames - num_already_allocated
        if (num_FREE_frames > curr_num_pages_needed) {
            num_pages_needed = num_FREE_frames - num_pages_already_allocated; 
            printf("num pages needed HERE: %d\n", num_pages_needed);
        } 

        // if number of free frames is < curr_num_pages_needed, then we must evict 
        if (num_FREE_frames < curr_num_pages_needed) {
            num_frames_to_evict = curr_num_pages_needed - num_FREE_frames;
        }
    } else if ((num_FREE_frames >= 4) && (num_pages_needed > num_FREE_frames)) {
        // there are more than 4 free frmaes, but less free frames than pages needed
        //printf("there are more than 4 free frames, but less free frames than pages needed\n"); 
        num_pages_needed = num_FREE_frames;
    } else if ((num_pages_already_allocated < 4) && (num_FREE_frames < 4)) {
        // number of pages we allocate is less than 4, and there are less than 4 free frames, so we need to evict 
        num_pages_needed = 4 - num_pages_already_allocated;

        // if the number of free pages is less than the number of pages we needed that we just calcualted 
        // e.g. 3 free frames but we need 4
        if (num_FREE_frames < num_pages_needed ) {
            num_frames_to_evict = num_pages_needed - num_FREE_frames;
        } else if (num_FREE_frames == num_pages_needed) {
            // e.g. we need 4 pages, and have 4 frames 
            num_frames_to_evict = 0; 
        } 
        // else if (num_FREE_frames > num_pages_needed) {
        //     // e.g. we need 2 pages, 3 free frames 
        //     num_pages_needed = 
        // }
        //printf("we need to evict: %d\n", num_frames_to_evict);
    }
    


    int num_frames_allocated = 0; // number of frames allocated in the frame table for this new process

  
    int num_free_frames = 0; // the number of free frames in memory right now
    int num_evicted = 0; // the number of frames evicted if there are not enough to allocate for process

    // Loop until all needed pages are allocated 
    // loop until there are 
    while (num_frames_allocated < num_pages_needed) {
        // check if there are enough free frames
        //int num_free_frames = 0; 
        for (int i = 0; i < TOTAL_MEMORY / PAGE_SIZE; i++) {
            if (frame_table[i] == 0) {
                num_free_frames++;
            }
        }
        

        // if there are not enough free frames, evict pages using LRU 
        if (num_free_frames < num_pages_needed) {
            num_evicted = evict_lru_pages_virtual(num_frames_to_evict, page_table, frame_table, lru_list, simul_time);
            // if frames are evicted, change the total number of frames allocated
            *total_frames_allocated = *total_frames_allocated - num_evicted;
        } else {
            
            for (int frame_number = 0; frame_number < NUM_PAGES ; frame_number++) { 
                // if num_evicted > 0 then we have to start allocating from the first frame free beginning 

                if (num_evicted > 0) {
                    if (frame_table[frame_number] == 0) {
                        // we have a free frame, frame_number = the frame that is free so we allocate to frame that is free
                        // allocate frame and update page table
                        allocate_process_id_page_table(&page_table[frame_number], process); // allocate process_id at place where we are allocating frame
                        page_table[frame_number].page_number = frame_number; // want the page num to be same as frame num
                        page_table[frame_number].frame_number = frame_number;  // frame number where free is where we put frame number, we want to keep page table array in order of frames
                        num_frames_allocated++;

                        frame_table[frame_number] = 1; // Mark frame as occupied 
                        if (num_frames_allocated == num_pages_needed) {
                            break; // All pages allocated
                        }

                    }

                }
            

                // else allocate as normal
                if (frame_table[frame_number] == 0) {
                    // Allocate frame and update page table

                    allocate_process_id_page_table(&page_table[frame_number], process);

                    page_table[frame_number].page_number = frame_number; // allocate page number 
                    page_table[frame_number].frame_number = frame_number; // allocate frame number 
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

    *total_frames_allocated = *total_frames_allocated + num_frames_allocated;
    //printf("\nAFTER\nnum pages needed at end of allocation: %d\ntotal num frames currently allocated at end of allocation function: %d\n\n",num_pages_needed, *total_frames_allocated );
    //print_page_table(page_table);
    //print_lru_list(lru_list);
    //printf("*************************** END OF THE ALLOCATION FUNCTION **********************************\n\n");
        
    return 1; // All pages allocated successfully. 
}


// Evict pages in process that was least recently used - Task 4
int evict_lru_pages_virtual(int num_frames_needed, page_table_entry_t *page_table, int *frame_table, list_t *lru_list, int simul_time) {
    node_t *current = lru_list->foot; // Start from the tail (LRU)

    // Dynamic memory to avoid buffer overflow for printing 
    int capacity = 256; 
    char *frame_numbers = malloc(capacity * sizeof(char)); 
    frame_numbers[0] = '\0'; // Start with an empty string

    int first = 1; // Flag to help format with commas

    //printf("\nIN THE EVICT_LRU_PAGES_VIRTUAL FUNCTION \n");
    int num_evicted = 0;

    while (current != NULL && num_frames_needed > 0) {
        
        process_t *lru_process = current->data; 

        //printf("Process being evicted: %s\n ", lru_process->process_id);

        // Evict pages of the LRU process until enough 
        for (int i = 0; i < NUM_PAGES; i++) {
            //printf("number evicted: %d\n", num_evicted);
            
            
            // Check if there is enough space in frame_numbers, if not realloc
            if (strlen(frame_numbers) + 10 >= capacity) {
                capacity *= 2; // Double the capacity 
                char *new_frame_numbers = realloc(frame_numbers, capacity * sizeof(char));
                    
                frame_numbers = new_frame_numbers;
            }



            if ((page_table[i].process_id != NULL) && (strcmp(page_table[i].process_id, lru_process->process_id) == 0) && (num_frames_needed > 0)) {

                 if (!first) {
                    strcat(frame_numbers, ",");  // Add a comma before the next number except for the first
                }

                char frame_number_str[10];  // Buffer for the current frame number
                sprintf(frame_number_str, "%d", page_table[i].frame_number);
                strcat(frame_numbers, frame_number_str);  // Append current frame number to the list

                // update frame table
                int frame_to_free = page_table[i].frame_number; 
                frame_table[frame_to_free] = 0; // Mark frame as free. 

                // print EVICTED event
                //printf("%d,EVICTED,evicted-frames=[%d]\n",simul_time, frame_to_free); // CHANGE THIS PRINT STATEMENT

                // Update page table
                page_table[i].process_id = NULL; 
                page_table[i].page_number = -1; 
                page_table[i].frame_number = -1;

                num_frames_needed--; // Decrement needed frames count 
                num_evicted++;

                first = 0; // Update flag after the first frame number is added
            }
        }

        // Print the EVICTED frames
        if (!first) {  // Only print if at least one frame was evicted
            printf("%d,EVICTED,evicted-frames=[%s]\n", simul_time, frame_numbers);
        }


        // Change LRU_List
        // only if all pages are deallocated we remove from the list 
        int num_pages_left = count_num_allocated(lru_process, page_table);

        if (num_pages_left <= 0) {
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
    //print_page_table(page_table); 
    //print_lru_list(lru_list);
    free(frame_numbers);
    //printf("num_evicted: %d\nEXIT EVICT_LRU_FUNCTION\n", num_evicted);
    return num_evicted;

}


// Allocate blocks - Task 2
int allocate_block(process_t *process, int size, memory_block_t *memory_head) {
    //printf("in allocate_block: process_id %s\n", process->process_id);
    //print_process(process);
    // size = the size of memory the process needs to be allocated to run 

    // Need to check if process has already been allocated
    if (is_block_process_allocated(process, memory_head)) {
        return 1;
    }

    // Implement first firt memory allocation 
    memory_block_t *current = memory_head; 

    // iterate through the memory_block_t list and find the first free block that is large enough to accomodate the requested size. 
        // if the block is larger than the memory required, we split the block, one allocated and the free block 
        // return 1 if allocation is successfull, 0 otherwise. 
        
    while (current != NULL) {
        if (current->status == FREE && current->size >= size) {
            // Allocate the block 
            current->status = ALLOCATED;
            current->process_id = process->process_id; 

            // Split block if necessary 
            int remaining_size = current->size - size; 
            if (remaining_size > 0) {
                memory_block_t *new_block = (memory_block_t *)malloc(sizeof(memory_block_t));
                new_block->start_address = current->start_address + size; 
                new_block->size = remaining_size; 
                new_block->status = FREE;
                new_block->process_id = NULL; 
                new_block->next = current->next;
                current->next = new_block;
                current->size = size; 
            }
            //print_memory_block_list(memory_head);
            //printf("allocation successful\n");
            return 1; // Allocation successful
        }

        current = current->next;

    }

    //print_memory_block_list(memory_head);
    return 0; // Allocation failed (no suitable block found)

}


// Free blocks at end of process and merge 
void free_block(process_t *process, memory_block_t *memory_head) {
    memory_block_t *current = memory_head; 
    memory_block_t *previous = NULL;

    while (current != NULL) {
        if (current->status == ALLOCATED && strcmp(current->process_id, process->process_id) == 0) {
            // Free the block 
            current->status = FREE;
            //free(current->process_id); 
            current->process_id = NULL;


            // Merge with previous block if its free
            if (previous != NULL && previous->status == FREE) {
                previous->size += current->size; 
                previous->next = current->next;
                free(current); 
                current = previous;
            }

            // Merge with next block if it's free
            if (current->next != NULL && current->next->status == FREE) {
                current->size += current->next->size; 
                memory_block_t *temp = current->next;
                current->next = current->next->next; 
                free(temp);
            }
            break;
        }
        previous = current;
        current = current->next;
    }
}


/******************************************************************** HELPER FUNCTIONS ***************************************************************************************************************************/

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

// Print the page table
void print_page_table(page_table_entry_t *page_table) {
    printf("Page Table:\n");
    printf("----------\n");
    printf("Page Num | Process ID | Frame Num\n");
    printf("---------|------------|----------\n");

    int numPage = 0;

    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id != NULL) {
            printf("%8d | %10s | %9d\n", 
                   page_table[i].page_number, 
                   page_table[i].process_id, 
                   page_table[i].frame_number); 
            numPage++;
        }
        //  printf("%8d | %10s | %9d\n", 
        //     page_table[i].page_number, 
        //     page_table[i].process_id, 
        //     page_table[i].frame_number); 
    }
    double mem_usage = (((double)(numPage / (double) NUM_PAGES)) * 100);
    printf("numpages = %d, NUM_PAGES = %d, memusage = %lf\n", numPage, NUM_PAGES, mem_usage);
    printf("\n");
}

// Check if the process has already been allocated memory in task 3
int check_process_allocated(process_t* process, page_table_entry_t *page_table) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id != NULL && 
            strcmp(page_table[i].process_id, process->process_id) == 0) {
            return 1; // Process has at least one page allocated
        }
    }
    return 0; // Process has no pages allocated 
}

// Count number of pages allocated to a process
int count_num_allocated(process_t* process, page_table_entry_t *page_table) {
    int num_allocated = 0; // number of pages allocated in a process
    for (int i = 0; i < NUM_PAGES; i++) {
        if (page_table[i].process_id != NULL &&
            strcmp(page_table[i].process_id, process->process_id) == 0) {
                num_allocated++;
            }
    }
    return num_allocated;
}

// Print the frame table
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

// Print lru_list
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

// Print memory_block list
void print_memory_block_list(memory_block_t *memory_head) {
    printf("Memory Block List:\n");
    printf("------------------\n");

    memory_block_t *current = memory_head;
    while (current != NULL) {
        printf("Address: %d, Size: %d KB, Status: %s", 
               current->start_address, current->size, 
               (current->status == FREE) ? "FREE" : "ALLOCATED");

        if (current->status == ALLOCATED) {
            printf(", Process: %s", current->process_id);
        }
        printf("\n");

        current = current->next;
    }

    printf("\n"); 


}


// Function to free the page_table
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

// Free blocked memory once we are done using it
void free_memory_blocks(memory_block_t *head) {
    memory_block_t *current = head; 
    while (current != NULL) {
        // Free process_id if it was allocated
        if (current->process_id != NULL) {
            free(current->process_id);
        } 

        memory_block_t *temp = current; 
        current = current->next; 
        free(temp); // Free the memory_block_t structure
    }
}

// Function to check if a process has been allocated block memory
int is_block_process_allocated(process_t *process, memory_block_t *memory_head) {
    memory_block_t *current = memory_head;
    while (current != NULL) {
        if (current->status == ALLOCATED && strcmp(current->process_id, process->process_id) == 0) {
            return 1; // Process is already allocated 
        }
        current = current->next;
    }
    return 0; // Process is not allocated 
}