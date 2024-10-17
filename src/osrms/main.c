#include "../osrms_API/osrms_API.h"
#include "../osrms_File/Osrms_File.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{

    // montar la memoria
    os_mount((char *)argv[1]);

    // resto de instrucciones
    int exist = os_exists(117, "facebook.png");
    if (exist == 1)
    // {
    //     printf("El archivo existe\n");
    // }
    // else
    // {
    //     printf("El archivo no existe\n");
    // }

    os_start_process(117, "process1");
    os_start_process(118, "process2");

    // Step 3: List running processes
    printf("\nListing all running processes:\n");
    os_ls_processes();

    // Step 4: Open a file for writing
    printf("\nOpening a file 'file1.txt' for process 1 in write mode...\n");
    osrmsFile *file1 = os_open(117, "prueba.jpg", 'w');
    if (file1 != NULL)
    {
        printf("\nFile Information:\n");
        printf("File Name: %s\n", file1->file_name);
        printf("Process ID: %d\n", file1->process_id);
        printf("File Size: %u bytes\n", file1->size);
        printf("Virtual Address: 0x%x\n", file1->vaddr); // Imprime la dirección en hexadecimal
        printf("Mode: %c\n", file1->mode == 'r' ? 'r' : 'w');
    }
    else
    {
        printf("Failed to open the file.\n");
    }

    // // Step 5: Write to the file
    // printf("\nWriting contents from 'input.txt' to 'file1.txt' in memory...\n");
    // int bytes_written = os_write_file(file1, "../../input.txt");
    // printf("Bytes written: %d\n", bytes_written);

    // // // Step 6: Close the file
    // os_close(file1);

    printf("\nListing all files for process ID 117:\n");
    os_ls_files(117);

    // printf("\nOpening 'file1.txt' for process 1 in read mode...\n");
    // file1 = os_open(117, "../../file1.txt", 'r');
    // if (file1 == NULL)
    // {
    //     printf("Error: Could not open 'file1.txt' for reading.\n");
    //     return 1;
    // }

    // printf("\nReading contents from 'file1.txt' in memory to 'output.txt'...\n");
    // int bytes_read = os_read_file(file1, "../../output.txt");
    // printf("Bytes read: %d\n", bytes_read);

    // // Step 10: Close the file again
    // os_close(file1);

    os_frame_bitmap();

    printf("\nPrinting the page table bitmap:\n");
    
    os_tp_bitmap();

    // printf("\nFinishing process with ID 117...\n");
    // os_finish_process(117);
    // os_finish_process(118);

    // printf("\nListing all running processes:\n");
    // os_ls_processes();

    printf("\nProgram finished successfully. All changes saved to '%s'.\n", argv[1]);

    return 0;
}