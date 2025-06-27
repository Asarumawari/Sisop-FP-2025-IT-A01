/**
 * @file orphan.c
 * @brief Main controller for managing child for demonstrating orphan, zombie, and file workers processes
 */
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
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

    // making sure clean up can be done gracefully
    /* if (setpgid(0, 0) < 0) {
        perror("setpgid failed");
        exit(EXIT_FAILURE);
    } */

    // registering the shutdown handler
    setup_signal_handlers();

    log_message(LOG_PROCESS, "CONTROLLER", "Press CTRL+C to terminate all processes.");
    printf("[CONTROLLER] [PID: %d] :: Press CTRL+C to terminate all processes.\n", getpid());

    // --- Fork Specialized Child Processes ---
    pid_t child_pid;

    // 1. Fork Orphan Demonstrator
    if ((child_pid = fork()) == 0) {
        // reset signal handlers in child process to default
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        run_orphan_demonstrator();
    }
    
    // 2. Fork Zombie Demonstrator
    if (child_pid > 0) {
        // reset signal handlers in child process to default
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        
        if ((child_pid = fork()) == 0) {
            run_zombie_demonstrator();
        }
    }
    
    // 3. Fork Worker Spawner
    if (child_pid > 0) {
        if ((child_pid = fork()) == 0) {
            // reset signal handlers in child process to default
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
    
            run_worker_spawner();
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