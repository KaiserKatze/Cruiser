#include <string.h>

#include "java.h"
#include "opcode.h"
#include "rt.h"

typedef struct
{
    u1              len;
    u1              str     [255];
} dc_stack_entry;

typedef struct
{
    u4              depth;
    dc_stack_entry  entries [MAX_STACK_DEPTH];
} dc_stack;

typedef struct
{
    u2              off_p   [MAX_PARAMETERS_COUNT];
    u2              off_l   [MAX_LOCALS_COUNT];
} dc_frame;

static dc_stack_entry *     push_entry(dc_stack *);
static dc_stack_entry *     pop_entry(dc_stack *);
static int                  dc_printf(dc_stack_entry *, const char *, ...);

int decompile(
        u4          len_code,
        u1 *        str_code
)
{
    dc_stack        stack;
    dc_stack_entry *entry;
    u1 *            end_code;
    u1              opcode;
    u2              index;

    stack.depth = 1024;
    memset(&stack.entries, 0, sizeof (dc_stack));
    end_code = str_code + len_code;
    while (str_code < end_code)
    {
        opcode = *str_code;
        if (opcode == OPCODE_nop)
            continue;
        entry = push_entry(&stack);
        if (opcode == OPCODE_aconst_null)
            dc_printf(entry, "null");
        else if (opcode >= OPCODE_iconst_m1
                && opcode <= OPCODE_iconst_5)
            dc_printf(entry, "%i", opcode - OPCODE_iconst_0);
        else if (opcode == OPCODE_lconst_0
                && opcode == OPCODE_lconst_1)
            dc_printf(entry, "%llil", opcode - OPCODE_lconst_0);
        else if (opcode >= OPCODE_fconst_0
                && opcode <= OPCODE_fconst_2)
            dc_printf(entry, "%ff", opcode - OPCODE_fconst_0);
        else if (opcode == OPCODE_dconst_0
                && opcode == OPCODE_dconst_1)
            dc_printf(entry, "%fd", opcode - OPCODE_dconst_0);
        else if (opcode == OPCODE_bipush)
            dc_printf(entry, "%i", *++str_code);
        else if (opcode == OPCODE_sipush)
            dc_printf(entry, "%i", (*++str_code << 8) | *++str_code);
        else if (opcode >= OPCODE_ldc
                && opcode <= OPCODE_ldc2_w)
        {
            index = *++str_code;
            if (opcode != OPCODE_ldc)
                index = (index << 8) | *++str_code;
        }
    }
}

static dc_stack_entry *push_entry(dc_stack *stack)
{
    return &(stack->entries[--stack->depth]);
}

static dc_stack_entry *pop_entry(dc_stack *stack)
{
    return &(stack->entries[stack->depth++]);
}

static int dc_printf(dc_stack_entry *entry, const char *format, ...)
{
    va_list vl;
    size_t res;

    va_start(vl, format);
    res = vspirntf((char *) entry->str, format, vl);
    va_end(vl);
    // overflow
    if (res & 0xffffff00) return -1;
    entry->len = (u1) (res & 0xff);

    return res;
}
