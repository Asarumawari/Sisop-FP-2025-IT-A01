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
    if (child_pid == 0) {
        log_message(LOG_PROCESS)
    }
}