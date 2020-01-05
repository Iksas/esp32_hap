#include "hk_logging.h"

#include <stdio.h>

void hk_log_print_bytewise(const char *title, char *data, size_t size)
{
    printf("-------------- %s (size: %d) --------------", title, size);
    for (size_t i = 0; i < size; i++)
    {
        if (i % 8 == 0 && i != 0)
        {
            printf("\t");
        }

        if (i % 16 == 0)
        {
            printf("\n");
        }

        printf("0x%02x, ", data[i]);
    }
    printf("\n------------------------------------------\n");
}

void hk_log_print_as_string(const char *title, char *data, size_t size)
{
    printf("-------------- %s (size: %d) --------------\n", title, size);
    if (data != NULL)
    {
        for (size_t i = 0; i < size; i++)
        {
            printf("%c", data[i]);
        }
        if (data[size - 1] == 0)
        {
            if (data[size - 2] != '\n')
            {
                printf("\n");
            }
        }
        else if (data[size - 1] != '\n')
        {
            printf("\n");
        }
    }
    else
    {
        printf("Memory not initialized.\n");
    }
    printf("------------------------------------------\n");
}