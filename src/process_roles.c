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

void orphaner(int order){
    log_message(LOG_ORPHAN, "ORPHAN MAKER", "I'm child #%d who will make grandchild and exit", order);
    pid_t grandchild_pid = fork(); 

    // --- Orphan Parent ---
    if (grandchild_pid > 0) {
        sleep(2); // give the child time to fork and become an orphan
        log_message(LOG_ORPHAN, "ORPHAN MAKER", "Order: %d; Forked orphan child %d.", order, grandchild_pid);
        exit(EXIT_SUCCESS); // parent exits, leaving the child as an orphan
    }
    
    // --- Orphan Child ---
    else if (grandchild_pid == 0) {
        // reset signal handlers to default in the orphan child
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        pid_t child_pid = getppid(); // get the PID of the orphan child
        int counter = 0;
        while(getppid() == child_pid){
            sleep(2);
            counter += 2;
            log_message(LOG_ORPHAN, "ORPHAN CHILD", "Order: %d; Waited: %d seconds.", order, counter);
        }
        log_message(LOG_ORPHAN, "ORPHANED CHILD", "Order: %d; Orphaned by parent %d.", order, child_pid);
        exit(EXIT_SUCCESS); // child continues running as an orphan
    }

    // --- Orphan Failed ---
    else {
        log_message(LOG_ORPHAN, "ORPHAN MAKER", "Order: %d; Failed to fork orphan child.", order);
        log_message(LOG_PROCESS, "CONTROLLER", "Order: %d; Failed to fork orphan child.", order);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief the main loop for a file worker process. Infinitely creates and encrypts files.
 */
void run_file_worker(int worker_id) {
    srand(time(NULL) ^ getpid()); // seed random number generator with time and PID

    char worker_tag[16]; 
    sprintf(worker_tag, "WORKER %d", worker_id); 

    log_message(LOG_PROCESS, worker_tag, "Worker process started with PID %d.", worker_id);

    while (1) {
        char original_path[256], 
             timestamp[64];
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", tm_info);
        snprintf(original_path, sizeof(original_path), "output/original/%s_process_%d.txt", timestamp, getpid()); // create a unique file name based on timestamp and PID

        FILE *file = fopen(original_path, "w");
        if (file){
            fprintf(file, "%d.\n– Created by %s at %s.", rand(), worker_tag, timestamp); // write a unique message to the file
            fclose(file);
            log_message(LOG_FILE_MAKING, worker_tag, "Created: %s", original_path);
        }

        // make a subdirectory for obfuscated files
        char obfuscated_dir[256];
        sprintf(obfuscated_dir, "output/obfuscated/%d", rand() % 1000); // random directory for obfuscation
        mkdir(obfuscated_dir, 0777); // create obfuscated directory with permissions

        // obfuscate the file by copying it and encrypting it to the obfuscated directory
        char obfuscated_path[512];
        sprintf(obfuscated_path, "%s/%s_process_%d.txt", obfuscated_dir, timestamp, getpid());
        
        FILE *source_file = fopen(original_path, "r");
        FILE *dest_file = fopen(obfuscated_path, "w");
        if (source_file && dest_file) {
            char character;
            while((character = fgetc(source_file)) != EOF) fputc(character, dest_file);
            fclose(source_file); 
            fclose(dest_file);
            xor_cipher_file(obfuscated_path);
            log_message(LOG_OBFUSCATION, worker_tag, "%s → %s", original_path, obfuscated_path);
        }
        sleep(10); 
    }
}