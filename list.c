#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

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
    printf("process_id: %s, arrival time: %d, execution time: %d, state: %d, memory: %d, time-remaining: %d, time-finished: %d \n", process->process_id, process->arrival_time, process->execution_time, process->state, process->memory, process->time_remain, process->time_finished);
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


//Print Performance Statistics 
void print_stats(list_t* complete_list, int simul_time) {
    //print_list(complete_list);
    // Turnaround time 
    print_turnaround(complete_list);

    // Time overhead
    print_time_overhead(complete_list);

    // Makespan 
    printf("Makespan %d\n", simul_time);
}

//print average turnaround time 
// turnaround time is the time elapsed between the arrival and the completion of a process
void print_turnaround(list_t* process_list) {
    node_t* current = process_list->head;

    int total_turnaroud_time = 0;
    int num_processes = 0;

    while(current != NULL) {
        total_turnaroud_time += (current->data->time_finished - current->data->arrival_time);
        num_processes++;
        current = current->next;
    }

    printf("Turnaround time %d\n", (int)ceil((double)total_turnaroud_time/num_processes));

}

//print maximum and average Time Overhead 
// Time overhead of a process is defined as its turnaround time divided by its service time.
void print_time_overhead(list_t* process_list) {
    node_t* current = process_list->head;

    double total_time_overhead = 0;
    int num_process = 0;

    double maximum; 

    while(current != NULL) {
        double current_overhead = ((current->data->time_finished - current->data->arrival_time) / (double)(current->data->execution_time));
        total_time_overhead += current_overhead;
        num_process++;

        if (num_process == 0) {
            maximum =  current_overhead;
        } else {
            if (maximum < current_overhead) {
                maximum = current_overhead;
            }
        }

        current = current->next;
    }

    printf("Time overhead %.2f %.2f\n", maximum, total_time_overhead/num_process);
}