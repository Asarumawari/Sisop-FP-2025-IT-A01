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
#include <dirent.h> 
#include <time.h>
#include <signal.h>
#include <errno.h>

#include "utils.h"

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
 * @brief registers the signal handlers for shutdown and cleanup.
 */
void setup_signal_handlers() {
    signal(SIGINT, controller_shutdown_handler); // handle Ctrl+C
    signal(SIGTERM, controller_shutdown_handler); // handle termination signal
}

/**
 * @brief logs a message to the correct log file based on its type
 * @param type the type of log message (LogType enum)   
 * @param format string format for the log message, using format like printf
 * @param ... variable arguments for the format string (ellipsis parameter)
 */
void log_message(LogType type, const char* format, ...) {
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

    fprintf(log_file, "[%s][PID:%-6d]", time_buffer, getpid());

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fprintf(log_file, "\n");
    fclose(log_file);
}

/**
 * @brief signal handler for shutdown and cleanup
 * @param signum the signal number
 */
static void controller_shutdown_handler(int signum) {
    // using write() for signal safety instead of printf or fprintf
    char msg[128]; 
    sprintf(msg, "\n[CONTROLLER] Signal %d received. Shutting down gracefully...\n", signum);
    write(STDERR_FILENO, msg, strlen(msg));

    // send SIGTERM to the entire process group. 
    if (kill(-getpgrp(), SIGTERM) == -1) {
        write(STDERR_FILENO, "Failed to send SIGTERM to process group\n", 40);
    }

    write(STDERR_FILENO, "[CONTROLLER] Cleaning up output files...\n", 40);
    cleanup_directory("output"); 

    write(STDERR_FILENO, "[CONTROLLER] Shutdown complete\n", 31);
    _exit(EXIT_SUCCESS); 
}

