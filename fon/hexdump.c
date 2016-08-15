/* http://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data */

#include <stdio.h>
#include "hexdump.h"

void
_hexdump(char *desc,
         char *file,
         int  line,
         const void *addr,
         int len)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;


    // Output description if given.
    if (desc != NULL)
        printf("%s(%s:%d):\n", desc, file, line);

    if(len == 0)
        return;

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
} // hexDump()

