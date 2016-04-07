#include "java.h"
#include "vrf.h"
#include "log.h"

extern int checkMagic(u4 magic)
{
    if (magic == MAGIC_ORACLE)
        return 0;

    logError("File structure invalid,"
            " fail to decompile! [0x%X]\r\n",
            magic);
    return -1;
}
