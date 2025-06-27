/**
 * @file orphan.c
 * @brief Main controller for managing child for demonstrating orphan, zombie, and file workers processes
 */
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "utils.h"
#include "process_roles.h"

int main() {
    // changed stdout to be line-buffered so printf appears immediately when encountered with '\n'
    setvbuf(stdout, NULL, _IOLBF, 0);

    // setting up system
    setup_directories(); 
    log_message(LOG_PROCESS, "CONTROLLER", "System Initialized.");

    // registering the shutdown handler
    setup_signal_handlers();

    log_message(LOG_PROCESS, "CONTROLLER", "Press CTRL+C to terminate all processes.");
    printf("[CONTROLLER] [PID: %d] :: Press CTRL+C to terminate all processes.\n", getpid());

    //  --- ORPHAN / ZOMBIE PROCESS CREATION ---
    int num_children = 0;
    char mode[10] = {0};
    int odd_even = -1;

    // loop until positive number is entered
    while (num_children <= 0) {
        printf("How many child processes (must be > 0): ");
        scanf("%d", &num_children);
        if (num_children <= 0) {
            printf("Invalid number of child processes. Please enter a positive integer.\n");
        }
    }

    // loop until 'add' or 'even' is entered
    while (odd_even == -1) {
        printf("orphan shall be (odd/even): ");
        scanf("%9s", mode);
        if (strcmp(mode, "odd") == 0) {
            odd_even = 1;
        } else if (strcmp(mode, "even") == 0) {
            odd_even = 0;
        } else {
            printf("Invalid input. Please enter 'odd' or 'even'.\n");
        }
    }

    // loop until positive number is entered
    int num_workers = 0;
    while (num_workers <= 0) {
        printf("How many worker processes to spawn (must be > 0): ");
        scanf("%d", &num_workers);
        if (num_workers <= 0) {
            printf("Invalid number of worker processes. Please enter a positive integer.\n");
        }
    }

    printf("---------------------------------------------------\n");

    for (int i = 1; i <= num_children; i++) { 
        pid_t demo_pid = fork();

        if (demo_pid < 0) {
            log_message(LOG_PROCESS, "CONTROLLER", "Failed to fork orphan/zombie child process %d.", i);
            exit(EXIT_FAILURE);
        }

        else if (demo_pid == 0) {
            // reset signal handlers in child process to default
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            
            int current_child_odd = (i % 2 != 0);
            if ((odd_even && current_child_odd) || (!odd_even && !current_child_odd)) { // if true, orphan process
                orphaner(i);
            } else { // if false, zombie process
                log_message(LOG_ZOMBIE, "ZOMBIE CHILD", "Order: %d; Becoming a zombie child.", i);
                exit(EXIT_SUCCESS); // child exits immediately, becoming a zombie
            }
            exit(EXIT_FAILURE); // shouldn't be reached
        }
        log_message(LOG_PROCESS, "CONTROLLER", "Forked child #%d, with PID %d", i, demo_pid);
        sleep(2); 
    }

    // --- WORKER ---
    for (int i = 1; i <= num_workers; i++) {
        pid_t worker_pid = fork();
        if (worker_pid == 0) {
            // reset signal handlers in child process to default
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
    
            run_file_worker(i); // run the worker process
            exit(EXIT_FAILURE); // shouldn't be reached 
        } else if (worker_pid < 0) {
            log_message(LOG_FILE_MAKING, "WORKER PARENT", "Failed to fork worker process %d.", i);
            log_message(LOG_PROCESS, "CONTROLLER", "Failed to fork worker process %d.", i);
            exit(EXIT_FAILURE);
        }
    }

    //  --- Main Controller Loop ---
    while(!shutdown_requested){
        pause(); // sleep until signal arrives
    }
    
    // --- Shutdown Sequence ---
    // below is outside the signal handler to ensure it runs only once and after the signal is received
    log_message(LOG_PROCESS, "CONTROLLER", "Shutdown requested. Terminating all child processes.");
    printf("[CONTROLLER] [PID: %d] :: Shutdown requested. Terminating all child processes.\n", getpid());
    
    // Using kill with a negative PGID is the most explicit way to signal an entire process group.
    kill(0, SIGTERM);

    // Wait for all direct children to terminate. This "reaps" them.
    while (wait(NULL) > 0);

    log_message(LOG_PROCESS, "CONTROLLER", "All child processes terminated (reaped). Cleaning up output files.");
    printf("[CONTROLLER] [PID: %d] :: All child processes terminated (reaped). Cleaning up output files.\n", getpid());
    cleanup_directory("output");

    log_message(LOG_PROCESS, "CONTROLLER", "Shutdown complete.");
    printf("[CONTROLLER] [PID: %d] :: Shutdown complete.\n", getpid());
    return 0;
}