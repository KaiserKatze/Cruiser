#include <stdlib.h>
#include <string.h>

#include "java.h"
#include "opcode.h"
#include "log.h"

extern int
disassembleCode(u4 code_length, u1 *code)
{
    u4 i;

    for (i = 0; i < code_length; i++)
    {
        // check opcode
        switch (*code)
        {
            // TODO
            default:
                logError("Unknown OPCODE 0x%X!\r\n", *code);
                return -1;
        }
    }

    return i;
}
