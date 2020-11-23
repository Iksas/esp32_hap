#include "hk_logging.h"

#include <stdio.h>

void hk_log_print_bytewise(const char *title, char *data, size_t size, bool formatted)
{
    printf("-------------- %s (size: %d) --------------", title, size);
    for (size_t i = 0; i < size; i++)
    {
        if (i % 8 == 0 && i != 0 && formatted)
        {
            printf("\t");
        }

        if ((i % 16 == 0 && formatted) || i == 0)
        {
            printf("\n");
        }
        if (formatted)
        {
            printf("0x%02x, ", data[i]);
        }
        else
        {
            printf("%02x", data[i]);
        }
    }
    printf("\n------------------------------------------\n");
}