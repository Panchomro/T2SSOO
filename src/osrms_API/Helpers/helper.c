#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  
#include <stdbool.h> 
#include "./helper.h"



char *getBytesFromFile(long possitionToStart, long numberOfBytes, char *fileName)
{
    FILE *file;
    file = fopen(fileName, "rb");
    if (file == NULL || fseek(file, possitionToStart, SEEK_SET) != 0)
    {
        fclose(file);
        exit(1);
    }

    char *buffer = (char *)malloc(numberOfBytes * sizeof(char));
    if (buffer == NULL || fread(buffer, sizeof(char), numberOfBytes, file) != numberOfBytes)
    {
        perror("Error al asignar memoria");
        free(buffer);
        fclose(file);
        exit(1);
    }

    fclose(file);
    return buffer;
}

FrameBitMap getFrameBitmap(char *fileName)
{
    long startPosition = 204800;
    char *buffer = getBytesFromFile(startPosition, 8192, fileName);

    FrameBitMap frameBitmap;
    for (int i = 0; i < 8192; i++)
    {
        unsigned char byte = buffer[i];
        for (int j = 0; j < 8; j++)
        {
            frameBitmap.frames[i * 8 + j].bit = (byte >> j) & 0x1;
        }
    }


    free(buffer);
    return frameBitmap;
}   
void showFrameBitmap(FrameBitMap frameBitmap, int initialFrame, int finalFrame)
{
    int usedFrames = 0;
    int freeFrames = 0;
    for (; initialFrame < finalFrame; initialFrame++)
    {
        printf("%d \n", frameBitmap.frames[initialFrame].bit);
        if (frameBitmap.frames[initialFrame].bit == 1)
        {
            usedFrames++;
        }
        else
        {
            freeFrames++;
        }
    }
    printf("\n");
    printf("USADOS %d\n", usedFrames);
    printf("LIBRES %d\n", freeFrames);
}