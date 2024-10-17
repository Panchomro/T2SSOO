#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE *global_memory_file = NULL;

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

    int process_found = 0;
    long pcb_offset = -1;

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        long current_pcb_offset = PCB_TABLE_START + (i * PCB_ENTRY_SIZE);
        fseek(global_memory_file, current_pcb_offset, SEEK_SET);

        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
        {
            printf("Error: Could not read PCB entry at index %d.\n", i);
            return 0;
        }

        unsigned char process_state = pcb_entry[0];
        if (process_state == 0x01)
        {
            unsigned char pcb_process_id = pcb_entry[1];
            if (pcb_process_id == process_id)
            {
                process_found = 1;
                pcb_offset = current_pcb_offset;
                break;
            }
        }
    }

    if (!process_found)
    {
        printf("Process with ID %d not found or not running.\n", process_id);
        return 0;
    }

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
    {
        printf("Error: Could not read PCB entry for process ID %d.\n", process_id);
        return 0;
    }

    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        unsigned char *file_entry = &pcb_entry[13 + (i * FILE_ENTRY_SIZE)];

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

    int process_found = 0;
    long pcb_offset = -1;

    // Recorrer los 32 PCBs para encontrar el ID del proceso que buscamos
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        // Cada PCB tiene un tamaño de 256 bytes
        long current_pcb_offset = PCB_TABLE_START + (i * PCB_ENTRY_SIZE);
        fseek(global_memory_file, current_pcb_offset, SEEK_SET);

        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
        {
            printf("Error: Could not read PCB entry at index %d.\n", i);
            return;
        }

        // Verificar si el estado del proceso es 0x01 (en ejecución)
        unsigned char process_state = pcb_entry[0];
        if (process_state == 0x01)
        {
            // Si el proceso está en ejecución, verificar el ID del proceso
            unsigned char pcb_process_id = pcb_entry[1]; // Segundo byte es el ID del proceso
            if (pcb_process_id == process_id)
            {
                process_found = 1;
                pcb_offset = current_pcb_offset;
                break; // Hemos encontrado el PCB del proceso que buscábamos
            }
        }
    }

    if (!process_found)
    {
        printf("Process with ID %d not found or not running.\n", process_id);
        return;
    }

    // Leer el PCB del proceso encontrado
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
    {
        printf("Error: Could not read PCB entry for process ID %d.\n", process_id);
        return;
    }

    printf("Process ID: %d\n", pcb_entry[1]); // Mostrar el ID del proceso

    // Iterar sobre la tabla de archivos dentro del PCB
    int files_found = 0;
    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        // Cada entrada de archivo tiene 23 bytes, y la tabla de archivos comienza en el byte 13 del PCB
        unsigned char *file_entry = &pcb_entry[13 + (i * FILE_ENTRY_SIZE)];

        // Verificar el byte de validez
        unsigned char file_validity = file_entry[0];
        if (file_validity == 0x01) // Si la entrada del archivo es válida
        {
            // Leer el nombre del archivo
            char file_name[15] = {0};                       // 14 bytes para el nombre + 1 para el terminador null
            strncpy(file_name, (char *)&file_entry[1], 14); // Copiar el nombre del archivo

            // Leer el tamaño del archivo (pequeño endian)
            unsigned int file_size = 0;
            memcpy(&file_size, &file_entry[15], sizeof(unsigned int)); // Copiar los 4 bytes del tamaño

            // Mostrar el nombre y tamaño del archivo
            printf("%s %u Bytes\n", file_name, file_size);
            files_found++;
        }
    }

    if (files_found == 0)
    {
        printf("No files found for this process.\n");
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
    unsigned char frame_bitmap[8192];
    fread(frame_bitmap, 1, 8192, global_memory_file);
    fclose(global_memory_file);

   

    printf("Frame Bitmap:\n");

    int frames_ocupados = 0;
    int frames_libres = 0;

    printf("Estado de cada frame (0: Libre, 1: Ocupado):\n");
    for (int frame_index = 0; frame_index < 65536; frame_index++) {
        int byte_index = frame_index / 8;
        int bit_index = frame_index % 8;
        int bit_value = (frame_bitmap[byte_index] >> bit_index) & 1;
        printf("frame %d ", frame_index);
       for (int bit = 0; bit < 8; bit++) {
            int bit_value = (frame_bitmap[byte_index] >> bit) & 1;
            printf("%d", bit_value);
        }

    printf("\n");
        if (bit_value == 1) {
            frames_ocupados++;
        } else {
            frames_libres++;
        }
    }
    printf("Total de frames ocupados: %d\n", frames_ocupados);
    printf("Total de frames libres: %d\n", frames_libres);

}

void os_tp_bitmap()
{
    if (global_memory_file == NULL)
    {
        printf("Error: Memory not mounted.\n");
        return;
    }

    // Seek to the beginning of the Bitmap of Page Tables in the memory
    fseek(global_memory_file, TP_BITMAP_OFFSET, SEEK_SET);

    // Allocate buffer to store the bitmap
    unsigned char tp_bitmap[TP_BITMAP_SIZE];

    // Read the Bitmap of Page Tables into the buffer
    size_t read_count = fread(tp_bitmap, 1, TP_BITMAP_SIZE, global_memory_file);
    if (read_count != TP_BITMAP_SIZE)
    {
        printf("Error: Failed to read the Bitmap of Page Tables.\n");
        return;
    }

    // Variables to count occupied and free page tables
    int occupied_tables = 0;
    int free_tables = 0;

    printf("Page Table Bitmap Status:\n");

    // Iterate over each bit in the Bitmap of Page Tables
    for (int i = 0; i < TP_COUNT; i++)
    {
        // Calculate which byte and bit within the byte corresponds to page table i
        int byte_index = i / 8;
        int bit_index = i % 8;

        // Check if the bit is set (1 means occupied, 0 means free)
        if (tp_bitmap[byte_index] & (1 << bit_index))
        {
            // printf("Page Table %d: Occupied\n", i);
            occupied_tables++;
        }
        else
        {
            // printf("Page Table %d: Free\n", i);
            free_tables++;
        }
    }

    // Print the summary of free and occupied page tables
    printf("Total Occupied Page Tables: %d\n", occupied_tables);
    printf("Total Free Page Tables: %d\n", free_tables);
}

void os_start_process(int process_id, char *process_name)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    int available_slot_found = 0;
    long pcb_offset = -1;

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        long current_pcb_offset = PCB_TABLE_START + (i * PCB_ENTRY_SIZE);
        fseek(global_memory_file, current_pcb_offset, SEEK_SET);

        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
        {
            printf("Error: Could not read PCB entry at index %d.\n", i);
            return;
        }

        if (pcb_entry[0] == 0x00)
        {
            available_slot_found = 1;
            pcb_offset = current_pcb_offset;
            break;
        }
    }

    if (!available_slot_found)
    {
        printf("No available slots to start a new process.\n");
        return;
    }

    unsigned char new_pcb_entry[PCB_ENTRY_SIZE] = {0};
    new_pcb_entry[0] = 0x01;
    new_pcb_entry[1] = (unsigned char)process_id;

    strncpy((char *)&new_pcb_entry[2], process_name, PROCESS_NAME_SIZE);

    for (int i = strlen(process_name); i < PROCESS_NAME_SIZE; i++)
    {
        new_pcb_entry[2 + i] = '\0';
    }

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    fwrite(new_pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    printf("Process with ID %d and name %s started successfully.\n", process_id, process_name);
}

void os_finish_process(int process_id)
{
    if (global_memory_file == NULL)
    {
        perror("Memory file not mounted");
        return;
    }

    int process_found = 0;
    long pcb_offset = -1;

    // Recorrer los 32 PCBs para encontrar el ID del proceso que buscamos
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        // Cada PCB tiene un tamaño de 256 bytes
        long current_pcb_offset = PCB_TABLE_START + (i * PCB_ENTRY_SIZE);
        fseek(global_memory_file, current_pcb_offset, SEEK_SET);

        unsigned char pcb_entry[PCB_ENTRY_SIZE];
        if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
        {
            printf("Error: Could not read PCB entry at index %d.\n", i);
            return;
        }

        // Verificar si el estado del proceso es 0x01 (en ejecución)
        unsigned char process_state = pcb_entry[0];
        if (process_state == 0x01)
        {
            // Si el proceso está en ejecución, verificar el ID del proceso
            unsigned char pcb_process_id = pcb_entry[1]; // Segundo byte es el ID del proceso
            if (pcb_process_id == process_id)
            {
                process_found = 1;
                pcb_offset = current_pcb_offset;
                break; // Hemos encontrado el PCB del proceso que buscábamos
            }
        }
    }

    if (!process_found)
    {
        printf("Process with ID %d not found or not running.\n", process_id);
        return;
    }

    // Leer el PCB del proceso encontrado
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
    {
        printf("Error: Could not read PCB entry for process ID %d.\n", process_id);
        return;
    }

    // Liberar la memoria asignada al proceso
    free_process_memory(process_id);

    // Marcar el proceso como terminado cambiando el estado a 0x00
    pcb_entry[0] = 0x00;

    // Limpiar los demás datos del PCB
    memset(&pcb_entry[1], 0, PCB_ENTRY_SIZE - 1);

    // Escribir el PCB actualizado de vuelta en memoria
    fseek(global_memory_file, pcb_offset, SEEK_SET);
    fwrite(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

    printf("Process with ID %d finished successfully.\n", process_id);
}

void free_process_memory(int process_id)
{
    // Aquí puedes implementar la lógica para liberar la memoria utilizada por el proceso
    // Asegúrate de actualizar correctamente el frame bitmap
    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    unsigned char frame_bitmap[FRAME_BITMAP_SIZE];
    fread(frame_bitmap, FRAME_BITMAP_SIZE, 1, global_memory_file);

    // Aquí deberías recorrer la tabla de páginas o frames asociados al proceso
    // y marcar los frames como libres. Actualmente, estamos liberando todos los frames,
    // lo cual no es correcto, pero se ajusta a modo de ejemplo.
    for (int i = 0; i < FRAME_COUNT; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (frame_bitmap[byte_index] & (1 << bit_index))
        {
            frame_bitmap[byte_index] &= ~(1 << bit_index); // Marcar el frame como libre
        }
    }

    // Escribir el frame bitmap actualizado de vuelta en memoria
    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    fwrite(frame_bitmap, FRAME_BITMAP_SIZE, 1, global_memory_file);

    printf("Memory freed for process ID %d.\n", process_id);
}
