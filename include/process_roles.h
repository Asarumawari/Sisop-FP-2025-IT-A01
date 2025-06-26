#ifndef PROCESS_ROLES_H
#define PROCESS_ROLES_H

/**
 * @file process_roles.h
 * @brief header file for specialized process role implementation
 * 
 * declares the functions that define the behavior of different child processes. 
 * child processes types: orphan/zombie demonstrators, and file workers
 */

void run_orphan_demonstrator(); 
void run_zombie_demonstrator(); 
void run_worker_spawner();
void run_file_worker();

#endif // PROCESS_ROLES_H