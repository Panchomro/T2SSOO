#pragma once
#ifndef OSRMS_FILE_H
#define OSRMS_FILE_H

#include <stdint.h>
#include <stdio.h>
#include "../osrms_API/osrms_API.h"

// Constants related to frame size and memory layout
#define MEMORY_DATA_OFFSET 139392 // Offset where data begins in the memory

// Function declarations

typedef struct
{
    int process_id;
    char file_name[15];
    unsigned int size;
    unsigned int vaddr;
    char mode;
} osrmsFile;

// Reads the content of a file described by file_desc and writes it to the destination path
int os_read_file(osrmsFile *file_desc, char *dest);

// Finds the first free frame in the frame bitmap
int find_free_frame(unsigned char *frame_bitmap);

// Writes the content of the source file to the memory
int os_write_file(osrmsFile *file_desc, char *src);

// Closes the file and frees associated resources
void os_close(osrmsFile *file_desc);

osrmsFile *os_open(int process_id, char *file_name, char mode);

#endif // OSRMS_FILE_H