#ifndef UTILS_H
#define UTILS_H

#include <signal.h>

/**
 * @file utils.h
 * @brief header file for utility functions
 * 
 * declares common helper functions for logging, file/directory management, signal handling, and encryption
 */

//  --- GLOBAL SHUTDOWN FLAG ---
/**
 * @brief global flag for the main loop to start a graceful shutdown
 * 'volatile' to prevent compiler optimizations that could skip checking this variable
 * 'sig_atomic_t' to ensure atomic access in signal handlers
 */
extern volatile sig_atomic_t shutdown_requested;

//  --- ENUM FOR STRUCTURED LOGGING ---
 typedef enum {
    LOG_PROCESS,        // for process lifecycle events (fork, exit, orphan, zombie)
    LOG_ORPHAN,         // for file copying and encryption events
    LOG_ZOMBIE,        // for file copying and encryption events
    LOG_FILE_MAKING,    // for original file creation events
    LOG_OBFUSCATION     // for file copying and encryption events
} LogType;

// --- FUNCTION DECLARATIONS ---
void setup_directories(); 
void setup_signal_handlers(); 
void log_message(LogType type, const char* tag, const char* format, ...);
int cleanup_directory(const char *path); 
void xor_cipher_file(const char *file_path);

#endif // UTILS_H