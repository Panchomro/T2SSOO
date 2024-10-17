
typedef struct 
{
    int bit;
} FrameBit;

typedef struct
{
    FrameBit frames[65536];
} FrameBitMap;

char *getBytesFromFile(long possitionToStart, long numberOfBytes, char *fileName);
FrameBitMap getFrameBitmap(char *fileName);
void showFrameBitmap(FrameBitMap frameBitmap, int initialFrame, int finalFrame);