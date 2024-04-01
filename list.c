#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "list.h"

// create an empty list 
list_t *make_empty_list(void) {
    list_t *list;
    list = (list_t*)malloc(sizeof(*list));
    assert(list != NULL);
    list->head = list->foot = NULL;
    //printf("list successfully created\n");
    return list;

}

// Function to check if the linked list is empty
int is_list_empty(list_t *list) {
    if (list == NULL || list->head == NULL) {
        return 1; // The list is empty 
    }
    return 0; // The list is not empty
}

// Insert a node at the foot of the linked list 
// Inserts a new node with value "process" to the end of "list" 
void insert_at_foot(list_t *list, process_t *process) {

    // Creates a new node with process inside of it
    node_t *new = (node_t*)malloc(sizeof(*new));
    assert(list != NULL && new != NULL);
    new->data = process;
    new->next = NULL; // this points to NULL as it is the end of the list

    if (list->head == NULL) {
        // First insert into the list 
        // new becomes only node in the list
        list->head = list->foot = new;
    } else {
        list->foot->next = new; // old tail connected to "new"
        list->foot = new; // new foot of the list 
    }

    //printf("inserted at foot %s\n", process->process_id);

}

// Function to insert a process into the front of the arrived_list.
void insert_at_head(list_t *list, process_t *process) {

    // Creates a new node 
    node_t *new = (node_t*)malloc(sizeof(*new));
    assert(list != NULL && new != NULL);
    new->data = process;

    if (list->foot == NULL) {
        list->head = list->foot = new;
    } else {
        new->next = list->head;
        list->head = new;
    }

}



// Delete the head node of the linked list 
// Removes the first node of the list and returns the node's data (process_t)
void *remove_head(list_t *list) {
    assert(list && list->head);
    node_t *tmp = list->head;      // The first, being-removed node 
    void *data = tmp->data;        // The returned data

    //process_t *p = tmp->data;

    // Remove the node from the list
    list->head = list->head->next; // Links "head" to the second node
    if (list->head == NULL) {       // If the list becomes empty 
        list->foot = NULL;     //   Then the "tail" must also be set to NULL
    }
    //free(tmp);                    // Free the removed node
    //printf("remove_head function works");
    //print_process(p);

    return data;
} 

//free linked list

//print list function 
void print_process(process_t *process) {
    printf("process_id: %s, arrival time: %d, execution time: %d, state: %d, memory: %d, time-remaining: %d \n", process->process_id, process->arrival_time, process->execution_time, process->state, process->memory, process->time_remain);
}

// Function to traverse and print the linked list
void print_list(list_t *list) {
    node_t *current = list->head; // Start from the head of the list

    while (current != NULL) { 
        print_process(current->data); 
        current = current->next; 
    }
}

// Function to free a process struct
void free_process(process_t *process) {
    if (process != NULL) {
        free(process->process_id);
        free(process);
    }
}

// Function to free the process linked list node
void free_node(node_t *node) {
    if (node != NULL) {
        free_process(node->data);
        free(node);
    }
}

// Function to free the entire linked list
void free_list(list_t *list) {
    if (list != NULL) {
        // Traverse the list and free each node
        node_t *current = list->head;
        node_t *next;

        while (current != NULL) {
            next = current->next;
            free_node(current);  
            current = next;
        }
    
        free(list);
    }
}


int len_list(list_t *list) {
    int length = 0;

    node_t *current = list->head; // Start from the head of the list

    while (current != NULL) { 
        current = current->next; 
        length++;
    }

    return length;

}