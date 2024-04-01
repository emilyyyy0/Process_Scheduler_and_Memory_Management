#define DEFAULT 0
#define READY 1
#define RUNNING 2
#define FINISHED 3
#define EVICTED 4

typedef struct process {
    int arrival_time; // time arrived
    char *process_id; // process name
    int execution_time; // execution time
    int memory; // memory required
    int state; // DEFAULT = 0, READY = 1, RUNNING = 2, FINISHED = 3, EVICTED = 4
    int time_remain; // Remaining time needed in CPU to finish
    
} process_t;


// linked list node
typedef struct node node_t;

struct node {
    process_t *data;
    node_t *next;
};

// construct a linked list
typedef struct {
    node_t *head;
    node_t *foot;
}list_t;

// typedef enum {
//     DEFAULT = 0,
//     READY = 1,
//     RUNNING = 2,
//     FINISHED = 3, 
//     EVICTED = 4
// } State;




// create an empty linked list 
list_t *make_empty_list(void);

// function to get head of the list 
void *remove_head(list_t *list);

// function to append node to end of the list. Append to foot
void insert_at_foot(list_t *list, process_t *process);

// function to append node to the front of the list. 
void insert_at_head(list_t *list, process_t *process); 

// print process
void print_process(process_t *process);

// prints the whole list
void print_list(list_t *list);

// Function to free a process struct
void free_process(process_t *process);

// Function to free the process linked list node
void free_node(node_t *node);

// Function to free the entire linked list
void free_list(list_t *list);

// Function that finds length of list.
int len_list(list_t *list);

// Function that checks if list is empty
int is_list_empty(list_t *list);