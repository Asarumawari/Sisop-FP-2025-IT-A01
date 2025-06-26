#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "utils.h"
#include "process_roles.h"

#define LOG_FILE_PATH "../log" // path to log file
#define LOG_FILE_NAME "orphan_logging.log" // name of the log file

FILE *log_file = NULL; // global log file pointer so parent and child can log to the same file

/**
 * @brief gets the current time and formats it into a string.
 * @param buffer a character array to store the formatted time string.
 * @param buffer_size the size of the buffer.
 */
void get_current_time(char *buffer, size_t buffer_size) {
    time_t raw_time;
    struct tm *time_info;

    time(&raw_time); // getting the current time in seconds since the epoch
    time_info = localtime(&raw_time); // convert to local time

    // time format: "YYYY-MM-DD HH:MM:SS"
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", time_info);
}

/**
 * @brief Sets up logging by creating a directory and opening a log file.
 * if the directory already exists, it simply opens the log file in append mode.
 * 
 * NOTE: assuming the program is ran in src/ directory
 */
void setup_logging() {
    const char *dir_path = LOG_FILE_PATH;
    const char *file_name = LOG_FILE_NAME;
    char full_path[256];

    if (mkdir(dir_path, 0777) == -1) {
        // if mkdir fails, check if it's because directory already exists
        if (errno != EEXIST) {
            perror("mkdir() failed, log directory isn't there and cannot be created");
            exit(EXIT_FAILURE);
        }
        // if directory already exists (errno is EEXIST), the program continues

        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, file_name);
        log_file = fopen(full_path, "a"); // open the log file in append mode
        if (log_file == NULL) {
            perror("fopen() failed, log file cannot be opened");
            exit(EXIT_FAILURE);
        }
    }
}

/** 
 * @brief logs a message to console and to log file using variadic function
 * @param format string format for the log message, using format like pintf
 * @param ... variable arguments for the format string (ellipsis parameter)
 */
void log_message(const char*format, ...){
    va_list args_console; 
    va_list args_file;
    va_start(args_console, format);
    va_start(args_file, format);
    
    vprintf(format, args_console); // print to console
    if (log_file) {
        vfprintf(log_file, format, args_file); // print to log file
        fflush(log_file); // ensure the log is written immediately to prevent mixed-up logs
    }

    va_end(args_console);
    va_end(args_file);
}

int main() {
    pid_t child_pid;

    // --- setup logging ---
    setup_logging();
    log_message("--- Log Session Started ---\n");

    // create new process
    child_pid = fork();
    if (child_pid < 0) { // if it so happens that fork() fails
        log_message("Fork failed: %s\n", strerror(errno));
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }

    // --- parent process logic ---
    if (child_pid > 0) {
        log_message("[PARENT][PID: %d]: Created a child with PID %d.\n", getpid(), child_pid);
        log_message("[PARENT][PID: %d]: Exiting in 2 seconds, orphaning (%d) child.\n", getpid(), child_pid);
        sleep(2); // sleep for 2 seconds to allow child to log its PPID
        log_message("[PARENT][PID: %d]: Exiting now.\n", getpid());
    }

    // --- child process logic ---
    else {
        log_message("[CHILD][PID: %d]: Child of parent with PID %d.\n", getpid(), getppid());
        
        // logging time time for 20 seconds
        for (int i = 0; i < 20; i++) {
            char time_buffer[100];
            get_current_time(time_buffer, sizeof(time_buffer));

            // log PID, PPID, and the timestamp.
            log_message("[CHILD LOG][PID: %d][PPID: %d][Time: %s]\n", getpid(), getppid(), time_buffer);
            sleep(1); // wait for one second before the next log entry
        }
    }

    // --- cleanup and exit ---
    if (log_file) {
        log_message("--- Log Session Ended ---\n");
        fclose(log_file); // close the log file
    }
    return EXIT_SUCCESS;
}
