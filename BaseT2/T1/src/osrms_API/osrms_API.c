#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE *global_memory_file = NULL;

#define PCB_TABLE_SIZE 8192 // 8KB
#define PCB_ENTRY_SIZE 256  // 256B
#define PCB_TABLE_START 0
#define PROCESS_NAME_SIZE 11
#define MAX_PROCESSES (PCB_TABLE_SIZE / PCB_ENTRY_SIZE)
#define FILE_ENTRY_SIZE 23 // 23B
#define FILE_TABLE_SIZE 5
#define FILE_TABLE_OFFSET 2
#define FRAME_BITMAP_SIZE 8192     // 65536 bits = 8192 bytes
#define FRAME_COUNT 65536          // 65536 frames
#define FRAME_BITMAP_OFFSET 139392 // Offset for the Frame Bitmap based on the memory layout#define TP_BITMAP_SIZE 128      // 1024 bits = 128 bytes
#define TP_COUNT 1024              // 1024 page tables
#define TP_BITMAP_OFFSET 8192      // Assuming page table bitmap starts after the PCB table (e.g., at 8KB)

void os_mount(char *memory_path)
{
    global_memory_file = fopen(memory_path, "r+b");
    if (global_memory_file == NULL)
    {
        perror("Error mounting memory file");
        exit(EXIT_FAILURE);
    }
    printf("Memory file mounted successfully\n");
}

void os_ls_processes()
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    fseek(global_memory_file, 0, SEEK_SET);
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);
        unsigned char process_state = pcb_entry[0];
        if (process_state == 0x01)
        {
            int process_id = (int)pcb_entry[1];
            char process_name[12]; // serian 11 o 12?
            strncpy(process_name, (char *)&pcb_entry[2], 11);
            process_name[11] = '\0';

            printf("Process ID: %d, Process Name: %s\n", process_id, process_name);
        }
    }
}

int os_exists(int process_id, char *file_name)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return 0;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        unsigned char *file_entry = &pcb_entry[FILE_TABLE_OFFSET + (i * FILE_ENTRY_SIZE)];
        unsigned char file_validity = file_entry[0];
        if (file_validity == 0x01)
        {
            char entry_file_name[15];
            strncpy(entry_file_name, (char *)&file_entry[1], 14);
            entry_file_name[14] = '\0';

            if (strcmp(entry_file_name, file_name) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

void os_ls_files(int process_id)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    printf("Files for process ID%d:\n", process_id);

    int files_found = 0;
    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        unsigned char *file_entry = &pcb_entry[FILE_TABLE_OFFSET + (i * FILE_ENTRY_SIZE)];
        unsigned char file_validity = file_entry[0];
        if (file_validity == 0x01)
        {
            char file_name[15];
            strncpy(file_name, (char *)&file_entry[1], 14);
            file_name[14] = '\0';

            unsigned int file_size = *(unsigned int *)&file_entry[15];

            printf("  - File Name: %s, Size: %u bytes\n", file_name, file_size);
            files_found++;
        }
    }

    if (files_found == 0)
    {
        printf("  No files found for this process.\n");
    }
}

void os_frame_bitmap()
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    unsigned char frame_bitmap[FRAME_BITMAP_SIZE];
    fread(frame_bitmap, FRAME_BITMAP_SIZE, 1, global_memory_file);

    int free_frames = 0;
    int occupied_frames = 0;

    printf("Frame Bitmap:\n");

    for (int i = 0; i < FRAME_COUNT; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (frame_bitmap[byte_index] & (1 << bit_index))
        {
            printf("Frame %d: Occupied\n", i);
            occupied_frames++;
        }
        else
        {
            printf("Frame %d: Free\n", i);
            free_frames++;
        }
    }

    printf("Total Occupied Frames: %d\n", occupied_frames);
    printf("Total Free Frames: %d\n", free_frames);
}

void os_tp_bitmap()
{
}

void os_start_process(int process_id, char *process_name)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE] = {0};
    pcb_entry[0] = 0x01;
    pcb_entry[1] = (unsigned char)process_id;

    strncpy((char *)&pcb_entry[2], process_name, PROCESS_NAME_SIZE);

    for (int i = strlen(process_name); i < PROCESS_NAME_SIZE; i++)
    {
        pcb_entry[2 + i] = '\0';
    }

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    fwrite(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    printf("Process with ID %d and name %s started successfully.\n", process_id, process_name);
}

void os_finish_process(int process_id)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    if (pcb_entry[0] != 0x01)
    {
        printf("Process with ID %d is not running.\n", process_id);
        return;
    }

    free_process_memory(process_id);

    pcb_entry[0] = 0x00;

    memset(&pcb_entry[1], 0, PCB_ENTRY_SIZE - 1);

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    fwrite(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    printf("Process with ID %d finished successfully.\n", process_id);
}

void free_process_memory(int process_id)
{
    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    unsigned char frame_bitmap[FRAME_BITMAP_SIZE];
    fread(frame_bitmap, FRAME_BITMAP_SIZE, 1, global_memory_file);

    for (int i = 0; i < FRAME_COUNT; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (frame_bitmap[byte_index] & (1 << bit_index))
        {
            frame_bitmap[byte_index] &= ~(1 << bit_index);
        }
    }

    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    fwrite(frame_bitmap, FRAME_BITMAP_SIZE, 1, global_memory_file);

    printf("Memory freed for process ID %d.\n", process_id);
}

osrmsFile *os_open(int process_id, char *file_name, char mode)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return NULL;
    }

    if (mode != 'r' && mode != 'w')
    {
        perror("Invalid file mode. Use 'r' for read or 'w' for write.");
        return NULL;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        unsigned char *file_entry = &pcb_entry[FILE_TABLE_OFFSET + (i * FILE_ENTRY_SIZE)];
        unsigned char file_validity = file_entry[0];
        char entry_file_name[15];
        strncpy(entry_file_name, (char *)&file_entry[1], 14);
        entry_file_name[14] = '\0';

        if (mode == 'r' && file_validity == 0x01 && strcmp(entry_file_name, file_name) == 0)
        {
            osrmsFile *file = (osrmsFile *)malloc(sizeof(osrmsFile));
            strncpy(file->file_name, entry_file_name, 15);
            file->process_id = process_id;
            file->size = *(unsigned int *)&file_entry[15];
            file->vaddr = *(unsigned int *)&file_entry[19];
            file->mode = mode;
            return file;
        }

        if (mode == 'w' && file_validity == 0x01 && strcmp(entry_file_name, file_name) == 0)
        {
            return NULL;
        }

        if (mode == 'w' && file_validity == 0x00)
        {
            file_entry[0] = 0x01;
            strncpy((char *)&file_entry[1], file_name, 14);

            // Initialize the file size and virtual address (for now, size = 0)
            unsigned int initial_size = 0;
            unsigned int initial_vaddr = 0x0; // Assuming this is the starting address (needs adjustment based on the system)

            memcpy(&file_entry[15], &initial_size, 4);
            memcpy(&file_entry[19], &initial_vaddr, 4);

            // Write the updated PCB entry back into memory
            fseek(global_memory_file, pcb_offset, SEEK_SET);
            fwrite(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

            // Allocate memory for the osrmsFile structure and return it
            osrmsFile *file = (osrmsFile *)malloc(sizeof(osrmsFile));
            strncpy(file->file_name, file_name, 15);
            file->process_id = process_id;
            file->size = initial_size;
            file->vaddr = initial_vaddr;
            file->mode = mode;
            return file;
        }
    }

    // If no matching file is found in 'r' mode, or no space for a new file in 'w' mode, return NULL
    return NULL;
}