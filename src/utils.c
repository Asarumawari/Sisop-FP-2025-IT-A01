/**
 * @file utils.c
 * @brief impleemntation of utility functions
 */
#define _GNU_SOURCE // for killpg
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "utils.h"

// --- global shutdown flag ---
volatile sig_atomic_t shutdown_requested = 0;

// --- private function ---
static void controller_shutdown_handler(int signum); 

/** 
 * @brief sets up the required log and output directories. 
 */
void setup_directories() {
    if(mkdir("log", 0777) == -1 && errno != EEXIST) {
        perror("Failed to create log directory");
        exit(EXIT_FAILURE);
    }
    if(mkdir("output", 0777) == -1 && errno != EEXIST) {
        perror("Failed to create output directory");
        exit(EXIT_FAILURE);
    }
    if(mkdir("output/original", 0777) == -1 && errno != EEXIST) {
        perror("Failed to create original output directory");
        exit(EXIT_FAILURE);
    }
    if(mkdir("output/obfuscated", 0777) == -1 && errno != EEXIST) {
        perror("Failed to create obfuscated output directory");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Registers the signal handlers for shutdown using the robust sigaction function.
 */
void setup_signal_handlers() {
    // Using sigaction is the modern, reliable way to handle signals.
    struct sigaction sa;

    // Point to our handler function.
    sa.sa_handler = controller_shutdown_handler;
    
    // Initialize the signal mask. sa_mask specifies other signals to block
    // while this handler is executing. We don't need to block any.
    sigemptyset(&sa.sa_mask);

    // Set flags. SA_RESTART would cause certain system calls to be automatically
    // restarted if interrupted by this signal. We want the opposite, so we set
    // flags to 0 to ensure that pause() gets interrupted correctly.
    sa.sa_flags = 0;

    // Register the handler for SIGINT (Ctrl+C).
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }
    
    // Register the handler for SIGTERM (the termination signal we will send).
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error setting up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief registers the signal handlers for shutdown and cleanup.
 */
/* void setup_signal_handlers() {
    signal(SIGINT, controller_shutdown_handler); // handle Ctrl+C
    signal(SIGTERM, controller_shutdown_handler); // handle termination signal
} */

/**
 * @brief logs a message to the correct log file based on its type
 * @param type the type of log message (LogType enum)   
 * @param tag string tag for identifying the the role of the process (e.g., "CONTROLLER", "WORKER")
 * @param format string format for the log message, using format like printf
 * @param ... variable arguments for the format string (ellipsis parameter)
 */
void log_message(LogType type, const char* tag, const char* format, ...) {
    const char *filename = NULL; 
    switch (type) {
        case LOG_PROCESS:       filename = "log/process.log"; break;
        case LOG_FILE_MAKING:   filename = "log/file_making.log"; break;
        case LOG_OBFUSCATION:   filename = "log/obfuscation.log"; break;
    }

    FILE* log_file = fopen(filename, "a");
    if (!log_file) {
        if(type == LOG_PROCESS) {
            perror("Failed to open process log file");
        } else if(type == LOG_FILE_MAKING) {
            perror("Failed to open file making log file");
        } else if(type == LOG_OBFUSCATION) {
            perror("Failed to open obfuscation log file");
        }
        return;
    }

    char time_buffer[64];
    time_t raw_time;
    time(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", localtime(&raw_time));

    fprintf(log_file, "[%s] [%-16s] [PID: %5d] [PPID: %5d] [PGID: %5d] :: ", time_buffer, tag, getpid(), getppid(), getpgrp());

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fclose(log_file);
}

/**
 * @brief Signal handler that simply sets a flag.
 * @param signum The signal number received.
 * This is the SAFE way to handle signals. The handler does the bare minimum,
 * and the main program loop will detect the flag change and handle the actual shutdown logic.
 */
static void controller_shutdown_handler(int signum) {
    // The only action is to set our global flag.
    shutdown_requested = 1;
    // We add a write here just for immediate feedback in the terminal.
    // A single write() call is generally safe in a handler.
    char msg[64];
    sprintf(msg, "\nShutdown signal (%d) received, please wait...\n", signum);
    write(STDOUT_FILENO, msg, strlen(msg));
}

/**
 * @brief signal handler for shutdown and cleanup
 * @param signum the signal number
 */
/* static void controller_shutdown_handler(int signum) {
    // using write() for signal safety instead of printf or fprintf
    char msg[128]; 
    sprintf(msg, "\n[CONTROLLER] Signal %d received. Shutting down gracefully...\n", signum);
    write(STDERR_FILENO, msg, strlen(msg));

    // send SIGTERM to the entire process group. 
    if (kill(-getpgrp(), SIGTERM) == -1) {
        write(STDERR_FILENO, "Failed to send SIGTERM to process group\n", 40);
    }

    while (wait(NULL) > 0); // wait for all child processes to terminate

    write(STDERR_FILENO, "[CONTROLLER] All child processes terminated.\n", 44);
    write(STDERR_FILENO, "[CONTROLLER] Cleaning up output files...\n", 40);
    cleanup_directory("output"); 

    write(STDERR_FILENO, "[CONTROLLER] Shutdown complete\n", 31);
    _exit(EXIT_SUCCESS); // using _exit for signal safety
} */

/**
 * @brief recursively deleting a directory and its contents
 * @param path the path to the directory to be cleaned up
 */
int cleanup_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Failed to open directory for cleanup");
        return -1;
    }

    size_t path_len = strlen(path);
    int return_val = -1;

    if (dir) {
        struct dirent *entry;
        return_val = 0; 
        while (!return_val && (entry = readdir(dir))) {
            int return_val2 = -1;
            char *buf;
            size_t len; 
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue; // skip current and parent directory entries
            }

            len = path_len + strlen(entry->d_name) + 2; // +2 for '/' and '\0'
            buf = malloc(len);

            if (buf){
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, entry->d_name);
                if (!stat(buf, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        // recursively clean up subdirectory
                        return_val2 = cleanup_directory(buf);
                    } else {
                        // remove file
                        return_val2 = unlink(buf);
                    }
                } else {
                    perror("Failed to stat file during cleanup");
                }
                free(buf);
            }
            return_val = return_val2;
        }
        closedir(dir);
    }
    if (!return_val) return_val = rmdir(path); // remove the directory itself
    return return_val;
}

/**
 * @brief encrypt a file using a simple XOR cipher
 * @param file_path the path to the file to be encrypted
 */
void xor_cipher_file(const char *file_path){
    FILE *file = fopen(file_path, "r+b");
    if (!file) {
        perror("Failed to open file for cipher");
        return;
    }

    char key = 'm'; 
    int character; 
    long pos = 0; 

    fseek(file, 0, SEEK_SET);
    while ((character = fgetc(file)) != EOF) {
        fseek(file, pos, SEEK_SET); 
        fputc(character ^ key, file); // XOR Cipher
        fseek(file, pos + 1, SEEK_SET); 
        pos++;
    }
    fclose(file);
}