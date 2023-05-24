#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct process {
    struct process* next; // linked list 
    int pid; // unique numeric process ID(1 to 10)
    float base_priority; // integer value 
    float priority; // real priority
    int arrival_time; // time when the task arrives in the unit of ms
    int burst_time; // cpu time requested by a task, in the unit of ms
    int remaining_time;
    int waiting_time; // sum of time spent waiting in the ready queue
    int response_time; // time from request to first response 
    int turnaround_time; // time for each process to complete
    int time_in_waiting; // for priority scheduling(time in ready queue)
}Process;

Process* job_front = NULL;
Process* job_rear = NULL;
Process* ready_front = NULL;
Process* ready_rear = NULL;

void init_process(Process processes[], int num_processes);
void read_process(Process processes[], char* input_filename);
void simulate(Process processes[], int quantum, float alpha, char* output_file);
void insert_process_job(Process* process);
void insert_process_ready(Process* process);
void remove_from_job(Process* process);
void remove_from_ready(Process* process);
void increase_waiting_time();


char* input_filename;
char* output_filename;
int quantum;
float alpha;

int main(int argc, char* argv[])
{
    if (argc != 5) // for the case that user does not provide the correct number of arguments
    {
        printf("Usage: %s [input_filename] [output_filename] [timequantum_for_RR] [alpha_for_PRIO]\n", argv[0]);
        return 1; // Say that program occurs an error
    }

    input_filename = argv[1];
    output_filename = argv[2];
    quantum = atoi(argv[3]);
    alpha = atof(argv[4]);

    Process processes[10];
    init_process(processes, 10);
    read_process(processes, input_filename);

    simulate(processes, quantum, alpha, output_filename);
    return 0;
}



void init_process(Process processes[], int num_processes)
{
    for (int i = 0; i < num_processes; i++)
    {
        Process p;
        p.pid = i + 1;
        p.priority = 0;
        p.base_priority = 0;
        p.arrival_time = 0;
        p.burst_time = 0;
        p.remaining_time = 0;
        p.waiting_time = 0;
        p.response_time = -1;
        p.turnaround_time = 0;
        p.next = NULL;
        p.time_in_waiting = 0;
        processes[i] = p;
    }
}


void read_process(Process processes[], char* input_filename)
{
    FILE* file = fopen(input_filename, "r");
    if (file == NULL)
    {
        perror("Error Opening File");
        exit(1);
    }

    for (int i = 0; i < 10; i++)
    {
        fscanf(file, "%d %f %d %d", &processes[i].pid, &processes[i].base_priority,
            &processes[i].arrival_time, &processes[i].burst_time);
        processes[i].remaining_time = processes[i].burst_time;
    }
    fclose(file);
}


void simulate(Process processes[], int quantum, float alpha, char* output_file)
{
    int current_time = 0;
    int completed_processes = 0;

    // Sort processes array by arrival time using bubble sort
    for (int i = 0; i < 10 - 1; i++)
    {
        for (int j = 0; j < 10 - i - 1; j++)
        {
            if (processes[j].arrival_time > processes[j + 1].arrival_time)
            {
                // Swap
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    // Insert sorted processes into job queue
    for (int i = 0; i < 10; i++)
    {
        insert_process_job(&processes[i]);
    }


    FILE* output = fopen(output_file, "w"); // open output file
    if (output == NULL) // defensive coding
    {
        perror("Error Opening Output File");
        exit(1);
    }
    float average_cpu_usage = 0;
    float average_waiting_time = 0;
    float average_response_time = 0;
    float average_turnaround_time = 0;
    int idle_time = 0;

    fprintf(output, "Scheduling : FCFS\n");
    fprintf(output, "====================================================\n");
    while (completed_processes < 10) // FCFS loop
    {
        Process* current_process = ready_front;
        Process* arrived_process = job_front;
        if (current_process == NULL && ready_front == NULL && job_front->arrival_time != current_time)
        {
            fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
            idle_time++;
        }

        bool same_arrival = false; // bool variable that tells whether multiple processes at a same arrival time

        if (job_front != NULL && job_front->arrival_time == current_time)
        {
            fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
            remove_from_job(arrived_process);
            insert_process_ready(arrived_process);
            current_process = ready_front;
            if(job_front != NULL)
	        {
		        arrived_process = job_front;
	        }
            if(arrived_process->arrival_time == current_time && arrived_process->next != NULL)
            {
                same_arrival = true;
                while (same_arrival == true)
                {
                    fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
                    remove_from_job(arrived_process);
                    insert_process_ready(arrived_process);
                    current_process = ready_front;
		            if(job_front != NULL)
            	    {
                	arrived_process = job_front;
                    }
		            else
		            {
			            same_arrival = false;
			            break;
		            }
                    if (job_front->arrival_time != current_time)
                    {
                        same_arrival = false; 
                    }
                }
            }
        }

        if (current_process != NULL) // running state(FCFS)
        {


            if (current_process->response_time == -1) // when response time not set yet
            {
                current_process->response_time = (current_time - 1) - current_process->arrival_time;
            }


            if (current_process != NULL && current_process->remaining_time == 0)
            {
                // average_waiting_time += (current_time - current_process->arrival_time) - current_process->burst_time;
                average_waiting_time += current_process->response_time;
                average_response_time += current_process->response_time;
                average_turnaround_time += (current_time) - current_process->arrival_time;

                completed_processes++;
                fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);

                if (completed_processes < 10 && ready_front->next != NULL)
                {
                    fprintf(output, "------------------------------ (Context-Switch)\n");
		            fprintf(output, "<time %d> process %d is running\n",current_time,current_process->next->pid);
                }
                else if (completed_processes < 10 && ready_front->next == NULL)
                {
                    fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
                    idle_time++;
                }
                else
                {
                    fprintf(output, "<time %d> all processes finish\n", current_time);
                }

                remove_from_ready(current_process);

                if (ready_front != NULL)
                {
                    current_process = ready_front;
                }
                else
                {
                    current_process = NULL;
                }
            }
            else
            {
                fprintf(output, "<time %d> process %d is running\n", current_time, current_process->pid);
            }

        }
        if (current_process != NULL)
        {
            current_process->remaining_time--;
        }
        current_time++;
    }
    average_cpu_usage = ((float)(current_time - 1 - idle_time)) /(current_time-1) * 100;
    average_waiting_time /= 10.0;
    average_response_time /= 10.0;
    average_turnaround_time /= 10.0;
    fprintf(output, "====================================================\n");
    fprintf(output, "Average cpu usage : %.2f %%\n", average_cpu_usage);
    fprintf(output, "Average waiting time : %.2f \n", average_waiting_time);
    fprintf(output, "Average response time : %.2f \n", average_response_time);
    fprintf(output, "Average turnaround time : %.2f \n", average_turnaround_time);
    fprintf(output, "*********************************************************************************\n");

    fprintf(output, "Scheduling : RR\n");
    fprintf(output, "====================================================\n");

    init_process(processes, 10);
    read_process(processes, input_filename);

    // Sort processes array by arrival time using bubble sort
    for (int i = 0; i < 10 - 1; i++)
    {
        for (int j = 0; j < 10 - i - 1; j++)
        {
            if (processes[j].arrival_time > processes[j + 1].arrival_time)
            {
                // Swap
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    // Initialize variables
    int quantum_cnt = 0;
    completed_processes = 0;
    current_time = 0;
    idle_time = 0;
    average_cpu_usage = 0;
    average_waiting_time = 0;
    average_response_time = 0;
    average_turnaround_time = 0;

    // Insert sorted processes into job queue
    for (int i = 0; i < 10; i++)
    {
        insert_process_job(&processes[i]);
    }

    if (quantum < 1) // defensive coding
    {
        perror("Error; Quantum is positive integer");
        exit(1);
    }

    while (completed_processes < 10) // RR loop
    {
        Process* current_process = ready_front;
        Process* arrived_process = job_front;

        if (current_process == NULL && ready_front == NULL && (job_front == NULL || job_front->arrival_time != current_time))
        {
            fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
            idle_time++;
        }

        bool same_arrival = false; // bool variable that tells whether multiple processes at a same arrival time

        if (job_front != NULL && job_front->arrival_time == current_time)
        {
            fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
            remove_from_job(arrived_process);
            insert_process_ready(arrived_process);
            current_process = ready_front;
	        if(ready_front->next == NULL)
	        {
		        current_process->remaining_time++;
	        }
            if (job_front != NULL)
            {
                arrived_process = job_front;
            }
            if (arrived_process->arrival_time == current_time && arrived_process->next != NULL)
            {
                same_arrival = true;
                while (same_arrival == true)
                {
                    fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
                    remove_from_job(arrived_process);
                    insert_process_ready(arrived_process);
                    current_process = ready_front;
                    if (job_front != NULL)
                    {
                        arrived_process = job_front;
                    }
                    else
                    {
                        same_arrival = false;
                        break;
                    }
                    if (job_front->arrival_time != current_time)
                    {
                        same_arrival = false;
                    }
                }
            }
        }

    
        if (current_process != NULL)
        {
            quantum_cnt++;

            if (current_process->response_time == -1) // when response time not set yet
            {
                current_process->response_time = (current_time - 1) - current_process->arrival_time;
    	    }

            if(current_process->remaining_time == 1 && ready_front->next != NULL)
            {
                average_waiting_time += (current_time)-current_process->arrival_time - current_process->burst_time;
                average_response_time += current_process->response_time;
                average_turnaround_time += (current_time)-current_process->arrival_time;
                completed_processes++;

                if (completed_processes != 10)
                {
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    current_process = ready_front;
                    quantum_cnt = 0;
                    fprintf(output, "------------------------------ (Context-Switch)\n");
                    fprintf(output, "<time %d> process %d is running\n", current_time, current_process->next->pid);
                }
                else
                {
                    quantum_cnt = 0;
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output, "<time %d> all processes finish\n", current_time);
                }
                remove_from_ready(current_process);
            }
	        else if(current_process->remaining_time == 1 && ready_front->next ==  NULL)
	        {
		        average_waiting_time += (current_time)-current_process->arrival_time - current_process->burst_time;
                average_response_time += current_process->response_time;
                average_turnaround_time += (current_time)-current_process->arrival_time;
                completed_processes++;

                if (completed_processes != 10)
                {
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    current_process = ready_front;
                    quantum_cnt = 0;
                    fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
                    idle_time++;
                }
                else
                {
                    quantum_cnt = 0;
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output, "<time %d> all processes finish\n", current_time);
                }
                remove_from_ready(current_process);
            }
            else if (quantum_cnt >= quantum && current_process->remaining_time != 0)
            {
		        if(ready_front->next == NULL)
		        {
			        fprintf(output, "<time %d> process %d is running\n", current_time, current_process->pid);
                    		current_process->remaining_time--;
		        }
		        else if(ready_front->next != NULL)
                	{
                    current_process->remaining_time--;
                    remove_from_ready(current_process);
                    insert_process_ready(current_process);
                    current_process = ready_front;
                    quantum_cnt = 0;
                    fprintf(output, "------------------------------ (Context-Switch)\n");
                    fprintf(output, "<time %d> process %d is running\n", current_time, current_process->pid);
                     // Reset quantum counter
		        }
            }
            else
            {
                current_process->remaining_time--;
                fprintf(output, "<time %d> process %d is running\n", current_time, current_process->pid);
            }
        }
        current_time++;
    }

    
    average_cpu_usage = ((float)(current_time-1 - idle_time)) / (current_time-1) * 100;
    average_waiting_time /= 10.0;
    average_response_time /= 10.0;
    average_turnaround_time /= 10.0;
    fprintf(output, "====================================================\n");
    fprintf(output, "Average cpu usage : %.2f %%\n", average_cpu_usage);
    fprintf(output, "Average waiting time : %.2f \n", average_waiting_time);
    fprintf(output, "Average response time : %.2f \n", average_response_time);
    fprintf(output, "Average turnaround time : %.2f \n", average_turnaround_time);
    fprintf(output, "*********************************************************************************\n");

    fprintf(output, "Scheduling : Preemptive Priority Scheduling with Aging\n");
    fprintf(output, "====================================================\n");

    init_process(processes, 10);
    read_process(processes, input_filename);

    // Sort processes array by arrival time using bubble sort
    for (int i = 0; i < 10 - 1; i++)
    {
        for (int j = 0; j < 10 - i - 1; j++)
        {
            if (processes[j].arrival_time > processes[j + 1].arrival_time)
            {
                // Swap
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    completed_processes = 0;
    current_time = 0;
    idle_time = 0;
    average_cpu_usage = 0;
    average_waiting_time = 0;
    average_response_time = 0;
    average_turnaround_time = 0;

    // Insert sorted processes into job queue
    for (int i = 0; i < 10; i++)
    {
        insert_process_job(&processes[i]);
    }

    if (alpha < 0 || alpha > 1) // defensive coding
    {
        perror("Error; alpha range[0~1]");
        exit(1);
    }

    while (completed_processes < 10) // Priority loop
    {
        Process* current_process = ready_front;
        Process* arrived_process = job_front;

        increase_waiting_time();

        if (current_process == NULL && ready_front == NULL && job_front->arrival_time != current_time)
        {
            fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
            idle_time++;
        }

        bool same_arrival = false;

        if (job_front != NULL && job_front->arrival_time == current_time)
        {
            fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
	        remove_from_job(arrived_process);
            insert_process_ready(arrived_process);
	        arrived_process->priority = arrived_process->base_priority;
	        if(current_process != NULL)
	        {
		            if((arrived_process->priority > current_process->priority) && (ready_front != NULL) && (ready_front->next != NULL))
                    {
                        Process* temp_p = ready_front;
                    	Process* prev = temp_p;
                    	while(temp_p != arrived_process)
                    	{
                            prev = temp_p;
                            temp_p = temp_p->next;
                            remove_from_ready(prev);
                            insert_process_ready(prev);
                            temp_p = ready_front;
                    	}
			            current_process = temp_p;
			            if (current_process->response_time == -1) // when response time not set yet
           		        {
                		    current_process->response_time = (current_time-2)-current_process->arrival_time;
            		    }	
			            fprintf(output, "------------------------------ (Context-Switch)\n");
            	     }
	                 else
		             {
			            current_process = ready_front;
		             }	     
            }	    
	        else
            {
                current_process =ready_front;
            }

            if (job_front != NULL)
            {
                arrived_process = job_front;
                if (arrived_process->arrival_time == current_time && arrived_process->next != NULL)
                {
                    same_arrival = true;
                    while (same_arrival == true)
                    {
                        fprintf(output, "<time %d> [new arrival] process %d\n", current_time, arrived_process->pid);
                        remove_from_job(arrived_process);
                        insert_process_ready(arrived_process);
                        arrived_process->priority = arrived_process->base_priority;
                        if (current_process != NULL)
                        {
                            if ((arrived_process->priority > current_process->priority) && (ready_front != NULL) && (ready_front->next != NULL))
                            {
                                Process* temp_p = ready_front;
                                Process* prev = temp_p;
                                while (temp_p != arrived_process)
                                {
                                    prev = temp_p;
                                    temp_p = temp_p->next;
                                    remove_from_ready(prev);
                                    insert_process_ready(prev);
                                    temp_p = ready_front;
                                }
                                current_process = temp_p;
                                if (current_process->response_time == -1) // when response time not set yet
                                {
                                    current_process->response_time = (current_time - 2) - current_process->arrival_time;
                                }
                                fprintf(output, "------------------------------ (Context-Switch)\n");
                            }
                            else
                            {
                                current_process = ready_front;
                            }
                        }
                        else
                        {
                            current_process = ready_front;
                        }

                        
                        if (job_front != NULL)
                        {
                            arrived_process = job_front;
                        }
                        else
                        {
                            same_arrival = false;
                            break;
                        }
                        if (job_front->arrival_time != current_time)
                        {
                            same_arrival = false;
                        }
                    }
                }
            }
        }

        if (current_process != NULL) // running state(Priority)
        {
            
            if (current_process->response_time == -1) // when response time not set yet
            {
                current_process->response_time = (current_time-2)-current_process->arrival_time;
            }

            if (current_process->remaining_time == 0 && ready_front->next != NULL)
            {
                // Process has completed
                average_waiting_time += (current_time)-current_process->arrival_time - current_process->burst_time;
                average_response_time += current_process->response_time;
                average_turnaround_time += (current_time)-current_process->arrival_time;
                completed_processes++;

                if (completed_processes != 10)
                {
                    fprintf(output, "<time %d> process %d is finished[priority %.2f]\n", current_time, current_process->pid, current_process->priority);
                    Process* p = ready_front->next;
                    Process* highest_priority_process = p;
                    while (p != NULL)
                    {
                        if (p->priority > highest_priority_process->priority)
                        {
                            highest_priority_process = p;
                        }
                        p = p->next;
                    }
                    // Move all processes in front of highest_priority_process to the end of the ready queue
		            remove_from_ready(current_process);
                    while (ready_front != highest_priority_process)
                    {
                        Process* moving_process = ready_front;
                        remove_from_ready(moving_process);
                        insert_process_ready(moving_process);
                    }

                    // Now, the highest_priority_process is at the front of the ready queue
                    current_process = highest_priority_process;

                    fprintf(output, "------------------------------ (Context-Switch)\n");
                    fprintf(output, "<time %d> process %d is running[priority %.2f]\n", current_time, current_process->pid, current_process->priority);
                    current_process->remaining_time--;
                }
                else
                {
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output, "<time %d> all processes finish\n", current_time);
                    remove_from_ready(current_process);
                }
                
            }
            else if(current_process->remaining_time == 0 && ready_front->next == NULL)
            {
                // Process has completed
                average_waiting_time += (current_time)-current_process->arrival_time - current_process->burst_time;
                average_response_time += current_process->response_time;
                average_turnaround_time += (current_time)-current_process->arrival_time;
                completed_processes++;

                if (completed_processes != 10)
                {
                    fprintf(output, "<time %d> process %d is finished[priority %.2f]\n", current_time, current_process->pid, current_process->priority);
                    current_process = ready_front;
                    fprintf(output, "<time %d> ---- system is idle ----\n", current_time);
                    idle_time++;
                }
                else
                {
                    fprintf(output, "<time %d> process %d is finished\n", current_time, current_process->pid);
                    fprintf(output, "<time %d> all processes finish\n", current_time);
                }
                remove_from_ready(current_process);
            }
            else if (current_process->remaining_time != 0)
            {
                if (ready_front->next == NULL)
                {
                    fprintf(output, "<time %d> process %d is running[priority %.2f]\n", current_time, current_process->pid, current_process->priority);
                    current_process->remaining_time--;
                }
                else
                {
                    Process* p = ready_front->next;
                    Process* highest_priority_process = p;
                    while (p != NULL)
                    {
                        if (p->priority > highest_priority_process->priority)
                        {
                            highest_priority_process = p;
                        }
                        p = p->next;
                    }

                    if (current_process->priority < highest_priority_process->priority)
                    {
                        // Move all processes in front of highest_priority_process to the end of the ready queue
                        while (ready_front != highest_priority_process)
                        {
                            Process* moving_process = ready_front;
                            remove_from_ready(moving_process);
                            insert_process_ready(moving_process);
                        }

                        // Now, the highest_priority_process is at the front of the ready queue
                        current_process = highest_priority_process;

                        fprintf(output, "------------------------------ (Context-Switch)\n");
                        fprintf(output, "<time %d> process %d is running[priority %.2f]\n", current_time, current_process->pid,current_process->priority);
                        current_process->remaining_time--;
                    }
                    else
                    {
                        fprintf(output, "<time %d> process %d is running[priority %.2f]\n", current_time, current_process->pid, current_process->priority);
                        current_process->remaining_time--;
                    }
                }
            }

        }
        current_time++;
    }
    average_cpu_usage = ((float)(current_time-1 - idle_time)) / (current_time-1) * 100;
    average_waiting_time /= 10.0;
    average_response_time /= 10.0;
    average_turnaround_time /= 10.0;
    fprintf(output, "====================================================\n");
    fprintf(output, "Average cpu usage : %.2f %%\n", average_cpu_usage);
    fprintf(output, "Average waiting time : %.2f \n", average_waiting_time);
    fprintf(output, "Average response time : %.2f \n", average_response_time);
    fprintf(output, "Average turnaround time : %.2f \n", average_turnaround_time);
    fprintf(output, "*********************************************************************************\n");
    fclose(output);
}

void increase_waiting_time() // Increase value of time in ready queue
{
    if (ready_front == NULL || ready_front->next == NULL)
    {
        return;
    }

    Process* temp = ready_front->next; // Start from the second process in the queue

    while (temp != NULL)
    {
        temp->time_in_waiting += 1;
        temp->priority = temp->base_priority + (alpha * temp->time_in_waiting);
        temp = temp->next;
    }
}


void insert_process_job(Process* process)
{
    if (process == NULL)
    {
        fprintf(stderr, "Error: Process is NULL\n");
        return;
    }

    if (job_front == NULL)
    {
        job_front = process;
        job_rear = process;
    }
    else
    {
        job_rear->next = process;
        job_rear = process;
    }
}

void insert_process_ready(Process* process)
{
    if (process == NULL)
    {
        fprintf(stderr, "Error: Process is NULL\n");
        return;
    }

    process->next = NULL;

    if (ready_front == NULL)
    {
        ready_front = process;
        ready_rear = process;
    }
    else
    {
        ready_rear->next = process;
        ready_rear = process;
    }
}

void remove_from_job(Process* process)
{
    if (process == NULL)
    {
        fprintf(stderr, "Error: Process is NULL\n");
        return;
    }

    if (job_front == process)
    {
        job_front = job_front->next;

        if (job_front == NULL)
        {
            job_rear = NULL;
        }
    }
}

void remove_from_ready(Process* process)
{
    if (process == NULL)
    {
        fprintf(stderr, "Error: Process is NULL\n");
        return;
    }

    if (ready_front == process)
    {
        ready_front = ready_front->next;

        if (ready_front == NULL)
        {
            ready_rear = NULL;
        }
    }
    else
    {
        Process* temp = ready_front;
        while (temp->next != NULL && temp->next != process)
        {
            temp = temp->next;
        }
        if (temp->next != NULL)
        {
            temp->next = temp->next->next;
            if (temp->next == NULL)
            {
                ready_rear = temp;
            }
        }
    }
}
