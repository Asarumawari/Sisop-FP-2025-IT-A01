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
    log_message(LOG_PROCESS, "--- System Initialized ---");

    // making sure clean up can be done gracefully
    if (setpgid(0, 0) < 0) {
        perror("setpgid failed");
        exit(EXIT_FAILURE);
    }

    // registering the shutdown handler
    setup_signal_handlers();

    log_message(LOG_PROCESS, "[CONTROLLER][PID:%d][PPID:%d][PGID:%d]: Press CTRL+C to terminate all process.", getpid(), getppid(), getpgrp());
    printf("[CONTROLLER][PID:%d][PPID:%d][PGID:%d]: Press CTRL+C to terminate all process.", getpid(), getppid(), getpgrp());

    // --- Fork Specialized Child Processes ---
    pid_t child_pid;

    // 1. Fork Orphan Demonstrator
    if ((child_pid = fork()) == 0) {
        run_orphan_demonstrator();
        log_message(LOG_PROCESS, "[ORPHAN DEMO][PID:%d][PPID:%d][PGID:%d]: Forked Orphan Demonstrator", getpid(), getppid(), getpgrp());
    }

    // 2. Fork Zombie Demonstrator
    else if (child_pid > 0) {
        if ((child_pid = fork()) == 0) {
            run_zombie_demonstrator();
            log_message(LOG_PROCESS, "[ZOMBIE DEMO][PID:%d][PPID:%d][PGID:%d]: Forked Zombie Demonstrator", getpid(), getppid(), getpgrp());
        }
    }

    // 3. Fork Worker Spawner
    else if (child_pid > 0) {
        if ((child_pid = fork()) == 0) {
            run_worker_spawner();
            log_message(LOG_PROCESS, "[WORKER SPAWNER][PID:%d][PPID:%d][PGID:%d]: Forked Worker Spawner", getpid(), getppid(), getpgrp());
        }
    }

    // controller process to wait for all children to finish and 'reap' them
    while(1){
        sleep(15); // sleep to allow child processes to run
        waitpid(-1, NULL, WNOHANG); // wait for any child process to finish
    }

    return 0;
}