#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "Osrms_File.h"
#include "../osrms_API/osrms_API.h"

extern FILE *global_memory_file;

#define FRAME_SIZE 32768 // 32KB
#define MEMORY_DATA_OFFSET 139392
#define FRAME_BITMAP_OFFSET 8192 // 8KB
#define FRAME_COUNT 65536

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

        fread(buffer, 1, bytes_from_frame, global_memory_file);
        fwrite(buffer, 1, bytes_from_frame, dest_file);

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
