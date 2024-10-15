#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE *global_memory_file = NULL;

#define PCB_TABLE_SIZE 8192 // 8KB
#define PCB_ENTRY_SIZE 256 // 256B
#define PCB_TABLE_START 0
#define MAX_PROCESSES (PCB_TABLE_SIZE / PCB_ENTRY_SIZE)
#define FILE_ENTRY_SIZE 23 // 23B
#define FILE_TABLE_SIZE 5
#define FILE_TABLE_OFFSET 2


void os_mount(char* memory_path) {
    global_memory_file = fopen(memory_path, "r+b");
    if (global_memory_file == NULL) {
        perror("Error mounting memory file");
        exit(EXIT_FAILURE);
    }
    printf("Memory file mounted successfully\n");
}

void os_ls_processes() {
    if (global_memory_file == NULL) {
        perror("Memory file not mounted");
        return;
    }

    fseek(global_memory_file, 0, SEEK_SET);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);
        unsigned char process_state = pcb_entry[0];
        if (process_state == 0x01) {
            int process_id = (int)pcb_entry[1];
            char process_name[12]; //serian 11 o 12?
            strncpy(process_name, (char*)&pcb_entry[2], 11);
            process_name[11] = '\0';

            printf("Process ID: %d, Process Name: %s\n", process_id, process_name);
        }
    }
}

int os_exists(int process_id, char* file_name){
    if (global_memory_file == NULL) {
        perror("Memory file not mounted");
        return 0;
    }

    long pcb_offset = PCB_TABLE_START + (process_id * PCB_ENTRY_SIZE);
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    for (int i = 0; i < FILE_TABLE_SIZE; i++) {
        unsigned char* file_entry = &pcb_entry[FILE_TABLE_OFFSET + (i * FILE_ENTRY_SIZE)];
        unsigned char file_validity = file_entry[0];
        if (file_validity == 0x01) {
            char entry_file_name[15];
            strncpy(entry_file_name, (char*)&file_entry[1], 14);
            entry_file_name[14] = '\0';

            if (strcmp(entry_file_name, file_name) == 0) {
                return 1;
            }	
        }
    }
    
    return 0;
}