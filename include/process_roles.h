#ifndef PROCESS_ROLES_H
#define PROCESS_ROLES_H

/**
 * @file process_roles.h
 * @brief header file for specialized process role implementation
 * 
 * declares the functions that define the behavior of different child processes. 
 * child processes types: orphan/zombie demonstrators, and file workers
 */

/**
 * @brief this function is called by a child process. It will fork its own
 * child (a grandchild) and then exit, orphaning the grandchild.
 * @param order the sequential number of this child process for logging purposes.
 */
void orphaner(int order);

/** 
 * @brief this function is called by a child process. It works to make new files
 * with random content with timestamps and it will then obfuscate the newly
 * generated files
 * @param worker_id the sequential number of this child process for logging purposes
 */
void run_file_worker(int worker_id);

#endif // PROCESS_ROLES_H