// Variable to store the total size of memory
#define MEMORY_CAPACITY 2048

// linked list memory_node
typedef struct mem_node mem_node_t;

struct mem_node
{
    int start_address;
    int size;
    int isAllocated;  // 0 = not allocated, 1 = allocated
    char *process_id; // ID of the process being stored currently
    mem_node_t *next;
};

// construct a linked list
typedef struct
{
    int totalAllocated;
    mem_node_t *head;
    mem_node_t *foot;
} m_list_t;

// Creates an empty memory list
m_list_t *make_memory_list(void);

// Checks if the memory list is empty
int is_mem_list_empty(m_list_t *list);

// Traverses the memory list to find the first free memory block (via first fit algorithm)
mem_node_t *traverse_memory(mem_node_t *head, int memorySize);

// Allocates a process to the memory list
int allocateMemory(m_list_t *list, int p_memory, char *p_id, int* alloc_position);

// Deallocates a process from the memory list
void deallocateMemory(m_list_t *memory_list, char *p_id);

// Frees a memory node once the process has completed execution
void free_mem_node(mem_node_t **head, mem_node_t *node_to_remove);

// Merges adjacent free memory blocks
void mergeMemory(mem_node_t *head);

// Splits a free memory block into an allocated and unallocated memory block to fit a new process exactly
void splitMemory(mem_node_t *free_block, int deallocateSize);

// Prints the memory nodes stored in the memory list
void print_mem_nodes(mem_node_t *current);

// Prints the memory list
void print_memory_list(mem_node_t *head);

// Prints the number of nodes in the memory list currently
int num_nodes(m_list_t *list);

// Function to free memory list
void free_memory_list(m_list_t *memory_list);
