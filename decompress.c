#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
    if (!argv[1])
    {
        prtinf("Usage: decompress <file>\n");
        exit(1);
    }
    FILE* in = fopen(argv[1], "r");
    if (!in)
    {
        printf("Couldn not open file for reading.\n");
        exit(1);
    }

    int32_t text_length = 0;
    int16_t tree_length = 0;

    if (!fread(&text_length, sizeof(int32_t), 1, in))
    {
        printf("Could not read from file.\n");
        exit(1);
    }

}
