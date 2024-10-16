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

    printf("\nStarting process with ID 1 and name 'processA'...\n");
    //os_start_process(1, "processA");

    // Step 3: List running processes
    printf("\nListing all running processes:\n");
    os_ls_processes();

    // Step 4: Open a file for writing
    // printf("\nOpening a file 'file1.txt' for process 1 in write mode...\n");
    // osrmsFile *file1 = os_open(1, "file1.txt", 'w');
    // if (file1 == NULL)
    // {
    //     printf("Error: Could not open 'file1.txt' for writing.\n");
    //     return 1;
    // }

    // // Step 5: Write to the file
    // printf("\nWriting contents from 'input.txt' to 'file1.txt' in memory...\n");
    // int bytes_written = os_write_file(file1, "input.txt");
    // printf("Bytes written: %d\n", bytes_written);

    // // Step 6: Close the file
    // os_close(file1);

    // Step 7: List files of the process
    printf("\nListing all files for process ID 1:\n");
    os_ls_files(1);

    // Step 8: Open the file for reading
    printf("\nOpening 'file1.txt' for process 1 in read mode...\n");
    // file1 = os_open(1, "file1.txt", 'r');
    // if (file1 == NULL)
    // {
    //     printf("Error: Could not open 'file1.txt' for reading.\n");
    //     return 1;
    // }

    // Step 9: Read the file contents back to a new file
    // printf("\nReading contents from 'file1.txt' in memory to 'output.txt'...\n");
    // int bytes_read = os_read_file(file1, "output.txt");
    // printf("Bytes read: %d\n", bytes_read);

    // // Step 10: Close the file again
    // os_close(file1);

    // Step 11: Print the frame bitmap
    printf("\nPrinting the frame bitmap:\n");
    os_frame_bitmap();

    // Step 12: Print the page table bitmap
    printf("\nPrinting the page table bitmap:\n");
    //os_tp_bitmap();

    // Step 13: Finish the process and free its resources
    printf("\nFinishing process with ID 1...\n");
    os_finish_process(1);

    // Step 14: List processes again (to verify process termination)
    printf("\nListing all running processes (after finishing process 1):\n");
    os_ls_processes();

    printf("\nProgram finished successfully. All changes saved to '%s'.\n", argv[1]);

    return 0;
}