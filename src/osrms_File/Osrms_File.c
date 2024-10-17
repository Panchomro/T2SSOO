#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Osrms_File.h"
#include "../osrms_API/osrms_API.h"

extern FILE *global_memory_file;

int os_read_file(osrmsFile *file_desc, char *dest)
{
    if (global_memory_file == NULL)
    {
        return -1;
    }

    if (file_desc == NULL || dest == NULL)
    {
        printf("Error: Invalid file descriptor or destination path.\n");
        return -1;
    }

    FILE *dest_file = fopen(dest, "wb");
    if (dest_file == NULL)
    {
        printf("Error: Could not open destination file.\n");
        return -1;
    }

    unsigned int file_size = file_desc->size;
    unsigned int bytes_read = 0;
    unsigned int bytes_to_read = file_size;

    unsigned int vaddr = file_desc->vaddr;
    unsigned int frame_number = vaddr / FRAME_SIZE;
    unsigned int offset_within_frame = vaddr % FRAME_SIZE;

    while (bytes_to_read > 0)
    {
        unsigned int bytes_from_frame = FRAME_SIZE - offset_within_frame;
        if (bytes_from_frame > bytes_to_read)
        {
            bytes_from_frame = bytes_to_read;
        }

        long frame_offset = MEMORY_DATA_OFFSET + (frame_number * FRAME_SIZE);
        fseek(global_memory_file, frame_offset + offset_within_frame, SEEK_SET);

        unsigned char *buffer = (unsigned char *)malloc(bytes_from_frame);

        // Check if memory allocation was successful
        if (buffer == NULL)
        {
            printf("Error: Memory allocation failed.\n");
            fclose(dest_file);
            return -1;
        }

        if (fread(buffer, 1, bytes_from_frame, global_memory_file) != bytes_from_frame)
        {
            printf("Error: Failed to read memory file.\n");
            free(buffer);
            fclose(dest_file);
            return -1;
        }

        if (fwrite(buffer, 1, bytes_from_frame, dest_file) != bytes_from_frame)
        {
            printf("Error: Failed to write to destination file.\n");
            free(buffer);
            fclose(dest_file);
            return -1;
        }
        free(buffer);

        bytes_read += bytes_from_frame;
        bytes_to_read -= bytes_from_frame;

        frame_number++;
        offset_within_frame = 0;
    }

    fclose(dest_file);
    return bytes_read;
}

int find_free_frame(unsigned char *frame_bitmap)
{
    if (global_memory_file == NULL)
    {
        return -1;
    }

    for (int i = 0; i < FRAME_COUNT; i++)
    {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(frame_bitmap[byte_index] & (1 << bit_index)))
        {
            frame_bitmap[byte_index] |= (1 << bit_index);
            return i;
        }
    }
    return -1;
}

int os_write_file(osrmsFile *file_desc, char *src)
{
    if (global_memory_file == NULL)
    {
        return -1;
    }

    if (file_desc == NULL || src == NULL)
    {
        printf("Error: Invalid file descriptor or source path.\n");
        return -1;
    }

    FILE *src_file = fopen(src, "rb");
    if (src_file == NULL)
    {
        printf("Error: Could not open source file.\n");
        return -1;
    }

    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    unsigned char frame_bitmap[FRAME_COUNT / 8];
    fread(frame_bitmap, 1, sizeof(frame_bitmap), global_memory_file);

    unsigned int bytes_written = 0;
    unsigned int free_space_in_frame = FRAME_SIZE;
    unsigned int frame_number = find_free_frame(frame_bitmap);
    unsigned int offset_within_frame = 0;

    if (frame_number == -1)
    {
        printf("Error: No free frames available.\n");
        fclose(src_file);
        return bytes_written;
    }

    unsigned char buffer[FRAME_SIZE];
    size_t bytes_to_write;

    while ((bytes_to_write = fread(buffer, 1, FRAME_SIZE, src_file)) > 0)
    {
        while (bytes_to_write > 0)
        {
            long frame_offset = MEMORY_DATA_OFFSET + (frame_number * FRAME_SIZE) + offset_within_frame;
            unsigned int bytes_in_this_frame = (bytes_to_write < free_space_in_frame) ? bytes_to_write : free_space_in_frame;

            fseek(global_memory_file, frame_offset, SEEK_SET);
            fwrite(buffer, 1, bytes_in_this_frame, global_memory_file);

            bytes_written += bytes_in_this_frame;
            bytes_to_write -= bytes_in_this_frame;
            offset_within_frame += bytes_in_this_frame;
            free_space_in_frame -= bytes_in_this_frame;

            if (free_space_in_frame == 0)
            {
                frame_number = find_free_frame(frame_bitmap);
                if (frame_number == -1)
                {
                    printf("Error: No free frames available.\n");
                    fclose(src_file);
                    return bytes_written;
                }

                offset_within_frame = 0;
                free_space_in_frame = FRAME_SIZE;
            }
        }
    }

    fseek(global_memory_file, FRAME_BITMAP_OFFSET, SEEK_SET);
    fwrite(frame_bitmap, sizeof(frame_bitmap), 1, global_memory_file);

    file_desc->size = bytes_written;
    fclose(src_file);

    return bytes_written;
}

void os_close(osrmsFile *file_desc)
{
    if (file_desc == NULL)
    {
        printf("Error: Invalid file descriptor.\n");
        return;
    }

    free(file_desc);

    printf("File closed successfully.\n");
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
            return NULL;
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
        return NULL;
    }

    fseek(global_memory_file, pcb_offset, SEEK_SET);
    unsigned char pcb_entry[PCB_ENTRY_SIZE];
    if (fread(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file) != 1)
    {
        printf("Error: Could not read PCB entry for process ID %d.\n", process_id);
        return NULL;
    }

    int file_count = 0;

    for (int i = 0; i < FILE_TABLE_SIZE; i++)
    {
        unsigned char *file_entry = &pcb_entry[FILE_TABLE_OFFSET + (i * FILE_ENTRY_SIZE)];
        unsigned char file_validity = file_entry[0];
        char entry_file_name[15];
        strncpy(entry_file_name, (char *)&file_entry[1], 14);
        entry_file_name[14] = '\0';

        if (file_validity == 0x01)
        {
            file_count++;

            if (mode == 'r' && strcmp(entry_file_name, file_name) == 0)
            {
                osrmsFile *file = (osrmsFile *)malloc(sizeof(osrmsFile));
                strncpy(file->file_name, entry_file_name, 15);
                file->process_id = process_id;
                file->size = *(unsigned int *)&file_entry[15];
                file->vaddr = *(unsigned int *)&file_entry[19];
                file->mode = mode;
                return file;
            }

            if (mode == 'w' && strcmp(entry_file_name, file_name) == 0)
            {
                return NULL;
            }
        }

        if (mode == 'w' && file_validity == 0x00)
        {
            file_entry[0] = 0x01;
            strncpy((char *)&file_entry[1], file_name, 14);

            unsigned int initial_size = 0;
            unsigned int initial_vaddr = 0x0;

            memcpy(&file_entry[15], &initial_size, 4);
            memcpy(&file_entry[19], &initial_vaddr, 4);

            fseek(global_memory_file, pcb_offset, SEEK_SET);
            fwrite(pcb_entry, PCB_ENTRY_SIZE, 1, global_memory_file);

            osrmsFile *file = (osrmsFile *)malloc(sizeof(osrmsFile));
            strncpy(file->file_name, file_name, 15);
            file->process_id = process_id;
            file->size = initial_size;
            file->vaddr = initial_vaddr;
            file->mode = mode;
            return file;
        }
    }

    if (mode == 'w' && file_count >= FILE_TABLE_SIZE)
    {
        printf("No more space to create new files for process ID %d.\n", process_id);
        return NULL;
    }

    return NULL;
}