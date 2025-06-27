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
    log_message(LOG_PROCESS, "ORPHAN-PARENT", "Creating grandchild process.");
    pid_t child_pid = fork();

    // --- GRANDCHILD PROCESS ---
    if (child_pid == 0) {
        log_message(LOG_PROCESS, "GRANDCHILD", "I am a grandchild process. My parents are going to buy some milk.");

        // loop for a few seconds to observe the change in parent PID
        for (int i = 0; i < 5; i++){
            sleep(3); 
            pid_t prev_ppid = getppid(); // store previous parent PID
            sleep(3);
            pid_t current_ppid = getppid(); // get current parent PID
            if (prev_ppid == current_ppid){
                log_message(LOG_PROCESS, "GRANDCHILD", "I have been waiting for %d seconds for my parents to buy some milk.", i*5);
            } 
            else {
                log_message(LOG_PROCESS, "ORPHAN", "My parents have left to buy for some milk");
            }
        }
        log_message(LOG_PROCESS, "ORPHAN", "I am now an orphan process. My new parent is PID %d (init).", getppid());
        exit(EXIT_SUCCESS);
    }

    // --- CHILD PROCESS ---
    else if (child_pid > 0) {
        sleep(5);
        log_message(LOG_PROCESS, "ORPHAN-PARENT", "I am the parent process and I will now leave to buy some milk.");
        exit(EXIT_SUCCESS);
    }

    // --- FORK FAILURE ---
    else {
        log_message(LOG_PROCESS, "ORPHAN-PARENT", "Fork failed to create grandchild process.");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief creates a zombie process
 * forks a child process that exits immediately. Parent doesn't call wait() on the child.
 */
void run_zombie_demonstrator() {
    log_message(LOG_PROCESS, "ZOMBIE-PARENT", "Creating child process that will become a zombie.");
    pid_t child_pid = fork(); 

    // --- CHILD PROCESS ---
    if (child_pid == 0) {
        log_message(LOG_PROCESS, "ZOMBIE-TO-BE", "I am a child process and I'm soon to be zombified.");
        exit(EXIT_SUCCESS);
    }

    // --- PARENT PROCESS ---
    else if (child_pid > 0) {
        sleep(10); // wait for the child to exit and become a zombie
        log_message(LOG_PROCESS, "ZOMBIE-PARENT", "I am the parent process. My child (%d) should have exited by now and I will not call wait().");
        sleep(30); // Give time to observe the zombie state
        log_message(LOG_PROCESS, "ZOMBIE-PARENT", "I will now exit and some system will reap the zombie (%d) on their own.", child_pid);
        exit(EXIT_SUCCESS);
    }

    // --- FORK FAILURE ---
    else {
        log_message(LOG_PROCESS, "ZOMBIE-PARENT", "Fork failed to create child process.");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief spawns several file worker processes that run in an infinite loop
 * Each worker will run their own process of creating files, copying, and encrypting them.
 */
void run_worker_spawner() {
    log_message(LOG_PROCESS, "WORKER-SPAWNER", "Starting to spawn workers.");
    
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
    log_message(LOG_PROCESS, "WORKER", "Worker process started with PID %d.", my_pid);

    while (1) {
        char original_path[256], 
             timestamp[64];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", tm_info);
        snprintf(original_path, sizeof(original_path), "output/original/%s_process_%d.txt", timestamp, my_pid); // create a unique file name based on timestamp and PID

        FILE *file = fopen(original_path, "w");
        if (file){
            fprintf(file, "%d.\n– Created by %d at %s.", rand(), my_pid, timestamp); // write a unique message to the file
            fclose(file);
            log_message(LOG_FILE_MAKING, "WORKER", "Created file: %s", original_path);
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
            while((character = fgetc(source_file)) != EOF) fputc(character, dest_file);
            fclose(source_file); 
            fclose(dest_file);
        }
        log_message(LOG_FILE_MAKING, "WORKER", "Copied file from %s to %s", original_path, obfuscated_path);

        xor_cipher_file(obfuscated_path); 
        log_message(LOG_OBFUSCATION, "WORKER", "Obfuscated file: %s", obfuscated_path);

        sleep(10); 
    }
}