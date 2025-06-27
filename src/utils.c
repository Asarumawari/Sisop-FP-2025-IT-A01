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
 * @brief registers the signal handlers for shutdown using the robust sigaction function.
 */
void setup_signal_handlers() {
    // using sigaction for the reliable way to handle signals.
    struct sigaction sa;

    // point to handler function.
    sa.sa_handler = controller_shutdown_handler;
    
    // initialize the signal mask. sa_mask specifies other signals to block
    // while this handler is executing. We don't need to block any.
    sigemptyset(&sa.sa_mask);

    // set flags to 0 to ensure that pause() gets interrupted correctly.
    sa.sa_flags = 0;

    // register the handler for SIGINT (Ctrl+C).
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }
    
    // register the handler for SIGTERM (the termination signal we will send).
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error setting up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}

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
        case LOG_ORPHAN:       filename = "log/orphan.log"; break;
        case LOG_ZOMBIE:      filename = "log/zombie.log"; break;
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

    fprintf(log_file, "[%s] [%-14s] [PID: %5d] [PPID: %5d] [PGID: %5d] :: ", time_buffer, tag, getpid(), getppid(), getpgrp());

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
    // the only action is to set global flag.
    shutdown_requested = 1;
    char msg[64];
    sprintf(msg, "\nShutdown signal (%d) received, please wait...\n", signum);
    write(STDOUT_FILENO, msg, strlen(msg));
}

/**
 * @brief recursively deleting a directory and its contents
 * @param path the path to the directory to be cleaned up
 */
int cleanup_directory(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        if (errno == ENOENT) {
            // Directory does not exist, nothing to clean up
            return 0;
        }
        perror("Failed to open directory for cleanup");
        return -1;
    }

    struct dirent *entry;
    int success = 1; // assume success until proven otherwise

    while ((entry = readdir(dir)) != NULL) {
        // skip '.' and '..' directories.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        int res = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (res >= (int)sizeof(full_path)) {
            fprintf(stderr, "Error: Path is too long to clean up: %s/%s\n", path, entry->d_name);
            success = 0;
            continue; // skip this entry and mark as failed.
        }

        struct stat statbuf;
        // lstat to correctly handle symbolic links if they were to exist.
        if (lstat(full_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                // if it's a directory, recurse
                if (cleanup_directory(full_path) != 0) { // if recurse fails, recursing fails
                    success = 0;
                }
            } else {
                // if it's a file, unlink (delete)
                if (unlink(full_path) != 0) { // if unlink fails, deletion fails
                    fprintf(stderr, "Error unlinking file: %s\n", full_path);
                    perror(" > unlink");
                    success = 0;
                }
            }
        } else {
            fprintf(stderr, "Error stating file: %s\n", full_path);
            perror(" > lstat");
            success = 0;
        }
    }
    closedir(dir);

    if (rmdir(path) != 0) {
        fprintf(stderr, "Error removing directory: %s\n", path);
        perror(" > rmdir");
        success = 0;
    }
    
    return success ? 0 : -1;
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