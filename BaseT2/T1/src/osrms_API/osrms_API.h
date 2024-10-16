#pragma once
#ifndef OSRMS_API_H
#define OSRMS_API_H

#include "../osrms_File/Osrms_File.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Global memory file pointer
extern FILE *global_memory_file;

#define PCB_TABLE_SIZE 8192 // 8KB
#define PCB_ENTRY_SIZE 256  // 256B
#define PCB_TABLE_START 0
#define PROCESS_NAME_SIZE 11
#define MAX_PROCESSES (PCB_TABLE_SIZE / PCB_ENTRY_SIZE)
#define FILE_ENTRY_SIZE 23 // 23B
#define FILE_TABLE_SIZE 5
#define FILE_TABLE_OFFSET 2
#define FRAME_BITMAP_SIZE 8192          // 65536 bits = 8192 bytes
#define FRAME_COUNT 65536               // 65536 frames
#define FRAME_BITMAP_OFFSET 139392      // Offset for the Frame Bitmap based on the memory layout#define TP_BITMAP_SIZE 128      // 1024 bits = 128 bytes
#define FRAME_SIZE 32768                // 32KB per frame
#define TP_COUNT 1024                   // 1024 page tables
#define TP_BITMAP_OFFSET PCB_TABLE_SIZE // Assuming page table bitmap starts after the PCB table (e.g., at 8KB)
#define TP_BITMAP_SIZE 128              // 1024 bits = 128 bytes

void os_mount(char *memory_path);

// List all running processes in the system
void os_ls_processes(void);

// Check if a file exists for the specified process ID and file name
int os_exists(int process_id, char *file_name);

// List all files belonging to a specified process ID
void os_ls_files(int process_id);

// Print the state of the frame bitmap, showing free and occupied frames
void os_frame_bitmap(void);

// Print the state of the page table bitmap, showing free and occupied page tables
void os_tp_bitmap(void);

// Start a new process with the given process ID and name
void os_start_process(int process_id, char *process_name);

// Finish a process by freeing its resources and removing its PCB entry
void os_finish_process(int process_id);

// Free the memory allocated to the process's frames
void free_process_memory(int process_id);

// Open a file for a process, either in read or write mode

// Read file contents into the destination path (to be implemented if needed)
// int os_read_file(osrmsFile *file_desc, char *dest);  // Placeholder for read function

// Write file contents from the source path into memory (to be implemented if needed)
// int os_write_file(osrmsFile *file_desc, char *src);  // Placeholder for write function

// Close the file and free any associated resources (to be implemented if needed)
// void os_close(osrmsFile *file_desc);  // Placeholder for close function

#endif // OSRMS_API_H