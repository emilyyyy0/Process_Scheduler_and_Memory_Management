
void infinite(list_t *process_list, list_t *arrived_list, list_t *complete_list, int quantum);

void first_fit();



// Function to check if there are any processes that have arrived at a particular simulation time
int check_arriving_process(list_t *process_list, list_t *arrived_list, int simul_time);

// Start current process, print the output.
void start_process(list_t *process_list, list_t *arrived_list, process_t *current_process, int *current_time);

// Process has finished using the CPU. 
void process_finish(list_t* complete_list, process_t* current_process, int simul_time);