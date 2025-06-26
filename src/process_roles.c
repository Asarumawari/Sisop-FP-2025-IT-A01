/**
 * @file process_roles.c
 * @brief implementation file for specialized process role functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <time.h>

#include "process_roles.h"
#include "utils.h"

/**
 * @brief creates an orphan process
 * forks a child (grandchild) process that will become an orphan after the parent exits.
 * grandchild will be adopted by 'init' (PID 1) process.
 */
void run_orphan_demonstrator() {
    pid_t child_pid = fork(); 
    // --- CHILD PROCESS ---
    if (child_pid == 0) {
        int prev_ppid = getppid();
        log_message(LOG_PROCESS, "[ORPHAN (to be)][PID:%d][PPID:%d][PGID:%d]: My parent (%d) is going to leave some milk", getpid(), getppid(), getppid(), getpgrp());
        for (int i = 0; i < 5; i++){
            sleep(1); 
            if (prev_ppid != getppid()){
                log_message(LOG_PROCESS, "[ORPHAN (to be)][PID:%d][PPID:%d][PGID:%d]: %d seconds in and I'm still waiting for my parents to buy some milk.", getpid(), getppid(), getpgrp(), i + 1);
            } 
            else {
                log_message(LOG_PROCESS, "[ORPHAN-ized][PID:%d][PPID:%d][PGID:%d]: %d seconds in and my parents has left to get some milk. Now, I'm with this parent:%d", getpid(), getppid(), getpgrp(), i + 1, getppid());
            }
        }
        log_message(LOG_PROCESS, "[ORPHAN][PID:%d][PPID:%d][PGID:%d]: Work is completed. Exiting.", getpid(), getppid(), getpgrp());
        exit(EXIT_SUCCESS);
    }

    // --- PARENT PROCESS ---
    else if (child_pid > 0) {
        sleep(1);
        log_message(LOG_PROCESS, "[ORPHAN DEMO PARENT][PID:%d][PPID:%d][PGID:%d]: I'm going to leave some milk, leaving child %d alone", getpid(), getppid(), getpgrp(), child_pid);
        exit(EXIT_SUCCESS);
    }

    // --- FORK FAILURE ---
    else {
        log_message(LOG_PROCESS, "[ORPHAN DEMO] fork failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief creates a zombie process
 * forks a child process that exits immediately. Parent doesn't call wait() on the child.
 */
void run_zombie_demonstrator() {
    pid_t child_pid = fork(); 
    // --- CHILD PROCESS ---
    if (child_pid == 0) {
        log_message(LOG_PROCESS, "[ZOMBIE (to be)][PID:%d][PPID:%d][PGID:%d]: I'm a zombie-to-be. Exiting immediately.", getpid(), getppid(), getpgrp());
        exit(EXIT_SUCCESS);
    }

    // --- PARENT PROCESS ---
    else if (child_pid > 0) {
        sleep(5); // wait for the child to exit and become a zombie
        log_message(LOG_PROCESS, "[ZOMBIE DEMO PARENT][PID:%d][PPID:%d][PGID:%d]: I have created a zombie child with PID %d. I'm going to sleep without calling wait().", getpid(), getppid(), getpgrp(), child_pid);
        sleep(20); // Give time to observe the zombie state
        log_message(LOG_PROCESS, "[ZOMBIE DEMO PARENT][PID:%d][PPID:%d][PGID:%d]: Exiting now. I'll let the system clean up my mess (zombie child)", getpid(), getppid(), getpgrp());
        exit(EXIT_SUCCESS);
    }

    // --- FORK FAILURE ---
    else {
        log_message(LOG_PROCESS, "[ZOMBIE DEMO] fork failed");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief spawns several file worker processes that run in an infinite loop
 * Each worker will run their own process of creating files, copying, and encrypting them.
 */
void run_worker_spawner() {
    log_message(LOG_PROCESS, "[WORKER SPAWNER][PID:%d][PPID:%d][PGID:%d]: spawning 3 file workers.", getpid(), getppid(), getpgrp());

    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
                run_file_worker();
        }
        sleep(1); // giving some time between worker creations
    }
    
    // so the workers can work forever, process will wait indefinitely
    while(1) {
        sleep(60); // sleep to avoid busy waiting
    }
}

/**
 * @brief the main loop for a file worker process. Infinitely creates and encrypts files.
 */
void run_file_worker() {
    srand(time(NULL) ^ getpid()); // seed random number generator with time and PID
    pid_t my_pid = getpid();
    log_message(LOG_PROCESS, "[WORKER][PID:%d][PPID:%d][PGID:%d]: I'm a file worker. Starting my infinite loop.", my_pid, getppid(), getpgrp());

    while (1) {
        char original_path[256], 
             timestamp[64];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", tm_info);
        snprintf(original_path, "output/original/%s_process_%d.txt", timestamp, my_pid); // create a unique file name based on timestamp and PID

        FILE *file = fopen(original_path, "w");
        if (file){
            fprintf(file, "%d.\n– Created by %d at %s.", rand(), my_pid, timestamp); // write a unique message to the file
            fclose(file);
            log_message(LOG_FILE_MAKING, "[WORKER][PID:%d][PPID:%d][PGID:%d]: Created original file: %s", my_pid, getppid(), getpgrp(), original_path);
        }

        // make a subdirectory for obfuscated files
        char obfuscated_dir[256];
        sprintf(obfuscated_dir, "output/obfuscated/%d", rand() % 1000); // random directory for obfuscation
        mkdir(obfuscated_dir, 0777); // create obfuscated directory with permissions

        // obfuscate the file by copying it and encrypting it to the obfuscated directory
        char obfuscated_path[512];
        sprintf(obfuscated_path, "%s/%s_process_%d.txt", obfuscated_dir, timestamp, my_pid);
        
        FILE *source_file = fopen(original_path, "r");
        FILE *dest_file = fopen(obfuscated_path, "w");
        if (source_file && dest_file) {
            char character;
            while((character = fgetc(source_file)) != EOF) fputc(dest_file, character);
            fclose(source_file); 
            fclose(dest_file);
        }
        log_message(LOG_OBFUSCATION, "[WORKER][PID:%d][PPID:%d][PGID:%d]: Copied %s to %s", my_pid, getppid(), getpgrp(), original_path, obfuscated_path);

        xor_cipher_file(obfuscated_path); 
        log_message(LOG_OBFUSCATION, "[WORKER][PID:%d][PPID:%d][PGID:%d]: Encrypted: %s", my_pid, getppid(), getpgrp(), obfuscated_path);

        sleep(rand() % 5 + 2); 
    }
}