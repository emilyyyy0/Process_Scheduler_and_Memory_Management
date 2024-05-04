/* Solution for W3.8, comp20003 workshop Week 3 */

/*-------------------------------------------------------------- 
..Project: qStud
  list.h :  
          = the implementation of module linkedList of the project

  NOTES:
        - this module is polymorphic
        - the data component in a list node is of type void*
----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

void error(char *name) {
	fprintf(stderr, "Function %s not yet implemented.\n", name);
	exit(EXIT_FAILURE);
}

// Data definitions, internal within list. >>>>>>>>>>>>>>>>>>>>>>>>>>
// A list node 
typedef struct node node_t;
struct node {
	void *data;              // Points to the data element of the node
	node_t *next;            // Points to the next node in the list
};

// A linked list is defined as a couple of pointers
struct list {
	node_t *head;  // Points to the first node of the list
	node_t *tail;  // Points to the last node of the list 
	size_t n;               // Number of elements in the list
};


// Generous functions >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// Creates & returns an empty linked list
list_t *listCreate() {
	list_t *list = malloc(sizeof(*list));
	assert(list);
	list->head = list->tail = NULL;
	list->n = 0;
	return list;
}

// Free the list
/* Free all memory used by a list, including used by data component
    Requires argument dataFree which is a function.
    The parameter can be declared as function pointer, which can be read as broken down here:
           (*dataFree)           : dataSize is a pointer to
           (*dataFree)(void *)   :  a function that has "void *" as argument
      void (*dataFree)(void *)   :  and that returns void
    (the above break-down is application of the right-left rule, 
      see https://cseweb.ucsd.edu/~gbournou/CSE131/rt_lt.rule.html)
*/
void listFree(list_t *list, void (*dataFree)(void *)){
	assert(list != NULL);
	node_t *curr = list->head; // curr points to the first node
	while (curr) {             // While curr not reaching the end of list
		node_t *tmp = curr;
		curr = curr->next;     // Advance curr to the next next node 
		dataFree(tmp->data);       // Frees the previous node including
		free(tmp);             //    Freeing the space used by data
	}
	free(list);                // Free the list itself
} 

// Returns 1 if the list is empty, 0 otherwise
int listIsEmpty(list_t *list) {
	assert(list);
	return list->head == NULL;
}

// Returns the number of elements in list
int listCount(list_t *list){
	assert(list);
	return list->n;
}

// Functions for insertion >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// Inserts a new node with value "data" to the front of "list"
void listPrepend(list_t *list, void *data) {
	assert(list);

	// Creates a new node and set data stored to given data value
	node_t *new = malloc(sizeof(*new));
	assert(new);
	new->data = data;
	
	// Links the new node to the list
	new->next = list->head;     // Connects "new" to the head node
	list->head = new;           // Repairs the pointer "head"

	// Checks special initial condition of inserting into an empty list
	if (list->tail == NULL) {       // Original list is empty
		list->tail = new;   // So "new" is also the tail node
	}
	// Updates the number of elements in the list
	(list->n)++;
}

// Inserts a new node with value "data" to the end of "list" 
void listAppend(list_t *list, void *data) {
	assert(list);

	// Creates a new node and set data to provided value
	node_t *new = malloc(sizeof(*new));
	assert(new);
	new->data = data;
	new->next = NULL;                // "new" is the terminating node!
	if (list->head == NULL) {          // If the original list is empty
		// "new" becomes the only node of the list
		list->head = list->tail = new;
	} else {
		list->tail->next = new;  // Connects the tail node to "new"
		list->tail = new;        // Repairs the pointer "tail"
	}

	// Updates the number of elements in the list
	(list->n)++;
}


// Functions for deletion >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// Removes the first node of the list and returns the node's data
void *listDeleteHead(list_t *list) {
	assert(list && list->head);
	node_t *tmp = list->head;      // The first, being-removed node 
	void *data = tmp->data;        // The returned data

	// Remove the node from the list
	list->head = list->head->next; // Links "head" to the second node
	if (list->head == NULL) {       // If the list becomes empty 
		list->tail = NULL;     //   Then the "tail" must also be set to NULL
	}
	free(tmp);                    // Free the removed node

	// Updates the number of elements in the list
	(list->n)--;

	return data;
} 

// Removes the last node of the list and returns the node's data
void *listDeleteTail(list_t *list){
	assert(list && list->head);
	node_t *tmp = list->tail;      // The last, being-removed node 
	void *data = tmp->data;        // The returned data

	// Remove the node from the list
	if (list->n == 1) {                     // If the original list has just one node 
		list->head = list->tail = NULL; //   it becomes empty
	} else {                      // Here the list has >1 nodes 
		// Step 1: Find "prev", the second last node, which points to the "tail" value
		node_t *prev = list->head;
		while (prev->next != list->tail) {
			prev = prev->next;
		}

		// Step 2: Makes prev become the last node
		prev->next = NULL;       // prev does not have a following element anymore
		list->tail = prev;       // and tail now points to the new last node
	}

	free(tmp);                      // Free the removed node

	// Updates the number of elements in the list
	(list->n)--;

	return data;
}


// Functions for processing over the whole list >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// Applies the function "f" to the data of each node in the list.
void listApplyIntFunc(list_t *list, void (*f)(void *)) {
	assert(list);
	for (node_t *p = list->head; p; p = p->next) {
		f(p->data);
	}
}

// Returns the (first-appearing) maximal data, using "compar" for comparing pairs of data
void *listMax(list_t *list, int (*compar) (const void *, const void *)){
	assert(list);
	if (list->head == NULL) return NULL;

	void *max = list->head;
	for (node_t *p = list->head->next; p; p = p->next) {
		if (compar(p->data, max) > 0) {
			max = p->data;
		}
	}
	return max;
}

// Performs linear search in "list", returns found data or NULL 
// Here we assume that key is an int (a limited assumption for sake of simplicity!)
void *listSearch(list_t *list, int key, int *comps, int (*dataGetKey)(void *)) {
	void *soln = NULL;
	*comps = 0;
	for (node_t *p = list->head; p; p = p->next) {
		(*comps)++;
		if (key == dataGetKey(p->data)) {
			soln = p->data;
			break;
		}
	}
	return soln;
}


/* =====================================================================
   This skeleton/program is compiled by the comp20003 teaching team,
   the university of Melbourne.
   Last Updated: 03/08/2023
   ================================================================== */