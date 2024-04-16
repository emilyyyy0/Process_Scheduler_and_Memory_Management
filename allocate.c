#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "allocate.h"
#include "list.h"
#include "processManager.h"
#include "memoryManagement.h"


int main(int argc, char **argv) {
    
    char *filename; // path to file
    int quantum;
    char *memory_strat;


    //Read in standard input 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], FILENAME) == 0) {
            filename = argv[i+1];
        } else if (strcmp(argv[i], QUANTUM) == 0) {
            quantum = atoi(argv[i+1]);
        } else if (strcmp(argv[i], MEMORY_STRAT) == 0) {
            memory_strat = argv[i+1];
        }
    }
    //printf("The test filename = %s \n the quantum  = %d\n memory strategy = %s \n\n\n", filename, quantum, memory_strat);


    // List of all processes    
    list_t* process_list = make_empty_list();

    // List of all arrived processes
    list_t* arrived_list = make_empty_list();

    // List of completed processes
    list_t* complete_list = make_empty_list();

    // Read in the file from stdin
    FILE *file = fopen(filename, "r");

    
    /* Read and handle the text file
     * Create process struct and store in all processes list */

    // Error handling if the filename is unreadable
    if (file == NULL) {
        perror("Unable to open file");
        return 1;
    }

    char line[MAX_LINE_LEN]; // Temporary buffer to store the current line/process being read
    char process_id_buffer[MAX_PROCESS_LEN]; // Temporary buffer to hold the process ID

    while (fgets(line, sizeof(line), file)) {
        // Initialize process_id in the process struct to NULL 
        process_t* process = malloc(sizeof(process_t));
        process->process_id = NULL;
        process->state = 0;
        
        
        // Use sscanf with a temporary buffer to parse the line into the process structure
        if (sscanf(line, "%d %8s %d %d", &process->arrival_time, process_id_buffer, &process->execution_time, &process->memory) == 4) {

            // Allocate memory for the process_id and copy the string from the buffer
            process->process_id = malloc(strlen(process_id_buffer) + 1);

            // Error handling if the process_id memory allocation fails
            if (process->process_id == NULL) {
                perror("Memory allocation failed");
                fclose(file); 
                return 1;
            }

            // If memory allocation is successful copy the temporary process_id buffer into the process struct
            strcpy(process->process_id, process_id_buffer);

            // Code that adds the process to the end of the queue
            insert_at_foot(process_list, process);

        } else {
            // Error handling if the text file is not of expected output
            fprintf(stderr, "Unexpected line format: %s", line);
        }

        process->time_remain = process->execution_time;
    }

    fclose(file);


    //Execute process manager depending on which task 
    if (strcmp(memory_strat, INFINITE) == 0) {
        infinite(process_list, arrived_list, complete_list, quantum);
    } else if (strcmp(memory_strat, FIRST_FIT) == 0) {
        first_fit();
    } else if (strcmp(memory_strat, PAGED ) == 0) {
        paged(process_list, arrived_list, complete_list, quantum);
    }
    else if (strcmp(memory_strat, VIRTUAL) == 0) {
        virtual();
    }




    return 0;
}


//free(process.process_id);

    // Traverse and print the whole process queue
  /*   printf("PRINT\n");
    print_list(process_queue);
    printf("END\n"); */



//