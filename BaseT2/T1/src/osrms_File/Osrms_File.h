#pragma once
#include <stdint.h>

typedef struct
{
    int process_id;
    char file_name[15];
    unsigned int size;
    unsigned int vaddr;
    char mode;
} osrmsFile;