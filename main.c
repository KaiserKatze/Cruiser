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

static void generateFilter(struct AttributeFilter *, int, char *);
static int interpreteFilter(struct AttributeFilter *, int, char **);

int
main(int argc, char** argv)
{
    ClassFile cf;
    char *path;
    struct BufferIO input;
    struct AttributeFilter filter;
    time_t t;
    int pos_path;
    int result;

    path = (char *) 0;
    memset((void *) &input, 0, sizeof (struct BufferIO));
    memset((void *) &filter, 0, sizeof (struct AttributeFilter));
    time(&t);

    // <exec> <classfile_absolute_path>
    if (argc == 2)
    {
        pos_path = interpreteFilter(&filter, argc, argv);
        path = argv[pos_path];
        logInfo("Parsing ClassFile '%s'...\r\n", path);

        if (initWithFile(&input, path) < 0)
            goto bad_end;

        memset(&cf, 0, sizeof (ClassFile));
        result = parseClassfile(&input, &cf, &filter);

        freeClassfile(&cf);
        free(input.buffer);
        fclose(input.file);

        if (result < 0) goto bad_end;
        else            goto good_end;
    }
    else
    {
        logError("Usage: %s <classfile_absolute_path>\r\n", argv[0]);
        return 0;
    }
good_end:
    logInfo("Succeed! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return 0;
bad_end:
    logInfo("Fail! Time used: %.2f seconds.\r\n",
            difftime(time(0), t));
    return -1;
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
static int
interpreteFilter(struct AttributeFilter *attr_filter,
        int argc, char ** argv)
{
    int i, len;
    char *arg;

    i = 0;
    while (++i < argc)
    {
        arg = argv[i];
        logInfo("Interpreting argument '%s'...\r\n", arg);
        len = strlen(arg);
        if (strncmp(arg, OPTION_CLASS_FILTER,
                    sizeof (OPTION_CLASS_FILTER)) == 0)
        {
            logInfo("Found %s!\r\n", OPTION_CLASS_FILTER);
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 0,
                    arg + sizeof (OPTION_CLASS_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_FIELD_FILTER,
                    sizeof (OPTION_FIELD_FILTER)) == 0)
        {
            logInfo("Found %s!\r\n", OPTION_FIELD_FILTER);
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 1,
                    arg + sizeof (OPTION_FIELD_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_METHOD_FILTER,
                    sizeof (OPTION_METHOD_FILTER)) == 0)
        {
            logInfo("Found %s!\r\n", OPTION_METHOD_FILTER);
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 2,
                    arg + sizeof (OPTION_METHOD_FILTER) + 2);
        }
        else if (strncmp(arg, OPTION_CODE_FILTER,
                    sizeof (OPTION_CODE_FILTER)) == 0)
        {
            logInfo("Found %s!\r\n", OPTION_CODE_FILTER);
            arg[len - 1] = '\0';
            generateFilter(attr_filter, 3,
                    arg + sizeof (OPTION_CODE_FILTER) + 2);
        }
        break;
    }

    logInfo("Return %i\r\n", i);

    return i;
}
