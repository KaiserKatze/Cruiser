#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "java.h"
#include "memory.h"
#include "log.h"

#define SEPERATOR_FILTER        '|'

#define OPTION_CLASS_FILTER     "--class_filter"
#define OPTION_FIELD_FILTER     "--field_filter"
#define OPTION_METHOD_FILTER    "--method_filter"
#define OPTION_CODE_FILTER      "--code_filter"

#define OPTION_DISASSEMBLE      "-a"
#define MARK_DISASSEMBLE        0x0001
#define OPTION_DECOMPILE        "-c"
#define MARK_DECOMPILE          0x0002

static void generateFilter(struct AttributeFilter *, int, char *);
static void interpreteFilter(struct AttributeFilter *, int, char **);
static int interpreteFlags(int, char **);

/*
 * ./cruise [-a] [-c] [--class_filter=<filterA|filterB>] [--field_filter=<filterC>] [--method_filter=<filterD>] [--code_filter=<filterE>]
 */
int
main(int argc, char** argv)
{
    char *path;
    struct BufferIO input;
    struct AttributeFilter filter;
    time_t t;
    int flags;
    int result;

    if (argc < 2)
    {
        logError("Usage: %s <classfile_absolute_path>\r\n", argv[0]);
        return -1;
    }

    path = argv[argc - 1];
    memset((void *) &input, 0, sizeof (struct BufferIO));
    memset((void *) &filter, 0, sizeof (struct AttributeFilter));
    time(&t);

    flags = interpreteFlags(argc, argv);
    interpreteFilter(&filter, argc, argv);
    logInfo("Classfile '%s'...\r\n", path);

    if (initWithFile(&input, path) < 0)
        goto bad_end;

    result = parseClassfile(&input, &filter);

    free(input.buffer);
    fclose(input.file);

    if (result < 0) goto bad_end;
    else            goto good_end;

good_end:
    logInfo("Succeed! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return 0;
bad_end:
    logInfo("Fail! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return -1;
}

static int
interpreteFlags(int argc, char **argv)
{
    int i, res;

    res = 0;
    for (i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], OPTION_DISASSEMBLE) == 0)
        {
            res |= MARK_DISASSEMBLE;
        }
        else if (strcmp(argv[i], OPTION_DECOMPILE) == 0)
        {
            res |= MARK_DECOMPILE;
        }
    }

    return res;
}

static void
generateFilter(struct AttributeFilter *attr_filter,
        int index, char *str_filter)
{
    char *last, *mark;
    size_t len;
    u4 filter, tag;
    u4 *alt;

    last = (char *) str_filter;
    mark = strchr(str_filter, SEPERATOR_FILTER);
    filter = 0;
    while (mark > last)
    {
        len = mark - last;
        tag = getAttributeTag(len, last);
        filter |= tag;
        last = mark + 1;
        mark = strchr(last, SEPERATOR_FILTER);
    }
    if (filter)
    {
        alt = (u4 *) attr_filter;
        alt[index] = filter;
    }
}

// example:
// --method_filter="Code|Exceptions"
static void
interpreteFilter(struct AttributeFilter *attr_filter,
        int argc, char ** argv)
{
    int i, len;
    char *arg;

    i = 0;
    while (++i < argc)
    {
        arg = argv[i];
        len = strlen(arg);
        if (strncmp(arg, OPTION_CLASS_FILTER,
                    sizeof (OPTION_CLASS_FILTER)) == 0)
        {
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 0,
                    arg + sizeof (OPTION_CLASS_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_FIELD_FILTER,
                    sizeof (OPTION_FIELD_FILTER)) == 0)
        {
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 1,
                    arg + sizeof (OPTION_FIELD_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_METHOD_FILTER,
                    sizeof (OPTION_METHOD_FILTER)) == 0)
        {
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 2,
                    arg + sizeof (OPTION_METHOD_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_CODE_FILTER,
                    sizeof (OPTION_CODE_FILTER)) == 0)
        {
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 3,
                    arg + sizeof (OPTION_CODE_FILTER) + 2);
        }
        break;
    }
}
