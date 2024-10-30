#include "dummy_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

#define MAX_PROCESSES 100

// Process structure
typedef struct {
    pid_t pid;
    int completed;
    char name[256];
    long start_time;
    long last_pause;
    long end_time;
    long run_time;
    long wait_time;
} Process;

Process ready_queue[MAX_PROCESSES];
int front = 0, rear = 0;
int completed_processes = 0;

// Function prototypes
void reset_queue();
long get_current_time_ms();
void add_to_queue(pid_t pid, const char *name);
void sleep_for_timeslice(int millisec);
void print_process_statistics(Process *proc);
void schedule_processes(int ncpu, int TSLICE);
void submit_job(const char *command);

// Get the current time in milliseconds
long get_current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Add a process to the queue
void add_to_queue(pid_t pid, const char *name) {
    ready_queue[rear].pid = pid;
    ready_queue[rear].completed = 0;
    strncpy(ready_queue[rear].name, name, 256);
    ready_queue[rear].run_time = 0;
    ready_queue[rear].wait_time = 0;
    ready_queue[rear].last_pause = get_current_time_ms();
    rear = (rear + 1) % MAX_PROCESSES;
}

// Sleep for the given time slice
void sleep_for_timeslice(int millisec) {
    struct timespec req = { millisec / 1000, (millisec % 1000) * 1000000L };
    nanosleep(&req, NULL);
}

// Reset the queue after all processes are done
void reset_queue() {
    front = 0;
    rear = 0;
    completed_processes = 0;
}

// Print the statistics of a single process
void print_process_statistics(Process *proc) {
    long completion_time = proc->run_time + proc->wait_time;
    printf("\nProcess Completed: %s (PID: %d)\n", proc->name, proc->pid);
    printf("%-10s %-6s %-12s %-12s %-12s\n", 
           "Name", "PID", "Run Time", "Wait Time", "Completion Time");
    printf("%-10s %-6d %-12ld %-12ld %-12ld\n",
           proc->name, proc->pid, proc->run_time, proc->wait_time, completion_time);
}

// Schedule processes with round-robin and multi-CPU support
void schedule_processes(int ncpu, int TSLICE) {
    int total_processes = rear;

    while (completed_processes < total_processes) {
        int running = 0;

        // Run up to ncpu processes in parallel
        for (int i = 0; i < ncpu && front != rear; ++i) {
            Process *proc = &ready_queue[front];

            if (proc->completed) {
                front = (front + 1) % MAX_PROCESSES;
                continue;
            }

            if (proc->run_time > 0) {
                proc->wait_time += get_current_time_ms() - proc->last_pause;
            }

            kill(proc->pid, SIGCONT);  // Resume the process
            running++;
        }

        sleep_for_timeslice(TSLICE);  // Sleep for the time slice

        for (int i = 0; i < running; ++i) {
            Process *proc = &ready_queue[front];

            long now = get_current_time_ms();
            proc->run_time += now - proc->last_pause;
            proc->last_pause = now;

            int status;
            if (waitpid(proc->pid, &status, WNOHANG) > 0) {
                proc->end_time = now;
                proc->completed = 1;
                completed_processes++;

                // Print statistics immediately after completion
                print_process_statistics(proc);
            } else {
                // If not complete, pause the process
                kill(proc->pid, SIGSTOP);
                ready_queue[rear] = *proc;
                rear = (rear + 1) % MAX_PROCESSES;
            }

            front = (front + 1) % MAX_PROCESSES;
        }
    }
}

// Submit a job to the scheduler
void submit_job(const char *command) {
    char *args[256];
    int i = 0;

    char *token = strtok(strdup(command), " ");
    while (token != NULL && i < 255) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("Failed to execute job");
        exit(EXIT_FAILURE);
    } else {
        printf("Submitted job: %s (PID: %d)\n", command, pid);
        kill(pid, SIGSTOP);  // Pause the process initially
        add_to_queue(pid, args[0]);
    }
}

// dummy_main replaces main
int dummy_main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <NCPU> <TSLICE in milliseconds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ncpu = atoi(argv[1]);
    int TSLICE = atoi(argv[2]);

    printf("SimpleShell started with %d CPU(s) and %d ms time slice.\n", ncpu, TSLICE);

    char command[256];
    while (1) {
        printf("SimpleShell$ ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "submit ", 7) == 0) {
            submit_job(command + 7);
        } else if (strcmp(command, "start") == 0) {
            schedule_processes(ncpu, TSLICE);
            reset_queue();  // Reset the queue after each run
        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting SimpleShell.\n");
            break;
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}
#define main dummy_main
