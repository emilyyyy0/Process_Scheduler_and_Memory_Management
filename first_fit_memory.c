#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "first_fit_memory.h"

// create an empty memory list with one node that represents all the memory
m_list_t *make_memory_list(void) {
    m_list_t *list;
    list = (m_list_t*)malloc(sizeof(*list));
    assert(list != NULL);
    list->totalAllocated = -1;
    list->head = list->foot = NULL;


    return list;
}


// Function to check if the linked list is empty
int is_mem_list_empty(m_list_t *list) {
    if (list == NULL || list->head == NULL) {
        return 1; // The list is empty 
    }
    return 0; // The list is not empty
}

// Function to traverse the memory list and find the first fit memory node block for a specific process size
mem_node_t* traverse_memory(mem_node_t *head, int memorySize) {
    mem_node_t *current = head;
   
    // Traverses the whole list
    while (current != NULL) {
       
        // Checks the memory is free and of a large enough size
    
        if (!current->isAllocated && (current->size >= memorySize)) {
            return current; // Returns the suitable block
        }
       
        // Otherwise it continues to traverse the list
        current = current->next;
    }
    return NULL; // No suitable block found
}

// Allocates the memory for a process in a linked list node if there is space available
// It is inserted at the lowest available memory address
int allocateMemory(m_list_t *list, int p_memory, char *p_id, int *alloc_position) {
    assert(list != NULL);
    mem_node_t *free_block = NULL;
    mem_node_t *new = NULL;
    
    // Checks if there is any more room in the memory to store more processes
    if ((list->totalAllocated + p_memory) > MEMORY_CAPACITY) {
        
        return 0;
    }
  
    free_block = traverse_memory(list->head, p_memory);
  
    
    // If no suitable free block is found, create a new block and add it at the end.
    if (free_block == NULL) {
        
        new = (mem_node_t*)malloc(sizeof(*new));
        assert(new != NULL);
        new->start_address = list->totalAllocated + 1; // Because it is inserted at the end of the list this is the starting address
        
        new->size = p_memory;
        new->isAllocated = 1;
        new->process_id = p_id;
        new->next = NULL;
        *alloc_position = new->start_address;
      
        if (list->head == NULL) {
            list->head = list->foot = new;
            // This runs when the first node is allocated and doesn't fulfill the total memory capacity
            if (list->head->size != MEMORY_CAPACITY){
                mem_node_t *new2 = NULL;
                new2 = (mem_node_t*)malloc(sizeof(*new2));
                assert(new2 != NULL);
                new2->start_address = new->size; // Because it is inserted at the end of the list this is the starting address
                
                new2->size = MEMORY_CAPACITY - p_memory;
                new2->isAllocated = 0;
                new2->process_id = NULL;
                new2->next = NULL;
                list->head->next = new2;
                list->foot = new2;
              
            }
        } 
        else {
            list->foot->next = new;
            list->foot = new;
            
        }

        // Update the totalAllocated variable
        list->totalAllocated += new->size;
       
        return 1;
    } 
    else {
        // If the process fits exactly into the free block allocate accordingly
        if (free_block->size == p_memory){
        
            free_block->isAllocated = 1;
            free_block->process_id = p_id;
            list->totalAllocated += p_memory;
            *alloc_position = free_block->start_address;
            

        }
        // If the process fits into the free block, but there is excess that needs to be cut off and become a hole
        else {
          
            // Create a new node to represent the unallocated memory left over...
            // after cutting down the free_block to allocate the exact size of the process to memory
            splitMemory(free_block, p_memory);
         
            // Cuts the free block down to size to perfectly fit the process
            free_block->size = p_memory;
            free_block->isAllocated = 1;
            *alloc_position = free_block->start_address;
            free_block->process_id = p_id;
            list->totalAllocated += p_memory;
            
        }
        
        return 1;
    }
    
    return 0;
}

// Used when a process has completely finished its execution and is removed from memory
void deallocateMemory(m_list_t *memory_list, char *p_id){
    mem_node_t* current = memory_list->head;
    // Traverse the list till you find the correct node (with the right start address)
    while (current != NULL) {
        if (current->process_id == p_id) {
            current->isAllocated = 0;
            memory_list->totalAllocated -= current->size;
            current->process_id = NULL;
            // Merge adjacent free blocks of memory if they exist
            mergeMemory(memory_list->head);
            return;
        }
        current = current->next;
    }
    printf("Memory deallocation failed\n");
    return;
}

// Frees nodes in the memory list
void free_mem_node(mem_node_t **head, mem_node_t *node_to_remove) {
    mem_node_t *current = *head;
    mem_node_t *prev = NULL;
    
    // Searches the whole memory list till it finds the mathcing stored node to the node_to_remove
    while (current != NULL) {
        if (current == node_to_remove) {
            // Rearranges the pointers to effectively "remove" the node from the list, only runs once
            if (prev != NULL) {
                prev->next = current->next; // Removes any dangling pointers
            } 
            else {
                *head = current->next; // Updates the head pointer if the head node is being removed
            }
            free(current); // Frees the node
            break; 
        }
        prev = current;
        current = current->next;
    }
}

// Merges adjacent memory nodes that are free/unallocated
void mergeMemory(mem_node_t *head){
    // Search begins from the newly allocated block to merge any adjacent free nodes BEFORE or AFTER
    mem_node_t *current = head;
    // Checks two adjacent memory nodes exist and are being checked currently
    while (current!= NULL && current->next != NULL){
        // Checks the adjacent blocks both have no allocated memory/ 'are free'
        if (!(current->isAllocated) && !(current->next->isAllocated)){
            
            current->size += current->next->size; // Merge the size of the 2 memory blocks into the current memory block
            mem_node_t *temp = current->next; // Temporary pointer to the node being removed
            current->next = current->next->next; // Skip over the node being merged
            free(temp); // Free the removed node
          

        }
        else {
            current = current->next;
        }
        
    }

}

// Splits one free block of memory into two separate memory nodes, one with allocated memory and the other with unallocated memory
// This is used when a free/hole of memory is found that fits a new process and is thus used (as per the first fit algorithm)
void splitMemory(mem_node_t *free_block, int allocatedSize){
    mem_node_t *new = (mem_node_t*)malloc(sizeof(*new));
    assert(new != NULL);
    new->start_address = free_block->start_address + allocatedSize;
    new->size = free_block->size - allocatedSize;
    new->isAllocated = 0;
    // New free node points to the node previously immediately after the 'free_block' node
    new->next = free_block->next;
    // And now the 'free_block' node's next pointer points to the newly created node
    free_block->next = new;
    // Merges any free blocks adjacent and AFTER the free block
    // We do not pass through the head node of the list because we know the nodes directly before the 'free_block' node are unchanged
    // so we only need to merge any free nodes AFTER the 'free_block' node
    mergeMemory(new);

}


// A Function to print information stored in memory nodes, used by print_memory_list()
void print_mem_nodes(mem_node_t *current){
    printf("process_id = %s, start address = %d, size = %d, allocated = %d\n", current->process_id, current->start_address, current->size, current->isAllocated);
}

// A Function to print the memory list and all the nodes, including holes and stored processes
void print_memory_list(mem_node_t *head){
    mem_node_t *current = head; // Start from the head of the list

    while (current != NULL) { 
        print_mem_nodes(current); 
        current = current->next; 
    }

}

// Function to find the number of nodes in the list
int num_nodes(m_list_t *list) {
    int count = 0;
    mem_node_t *current = list->head; // Start from the head of the list

    while (current != NULL) { // Traverse the list until reaching the end
        count++; // Increment the count for each node encountered
        current = current->next; // Move to the next node
    }

    return count;
}

// A function to free the memory list
void free_memory_list(m_list_t *memory_list){
    if (memory_list != NULL){
        // Traverse the list and free each node
        mem_node_t *current = memory_list->head;
        mem_node_t *next;

        while (current != NULL){
            next = current->next;
            free(current->process_id);
            free(current);
            current = next;
        }
        free(memory_list);
    }
}