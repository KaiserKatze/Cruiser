#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
    // map local index with its name
    // trade off time with space
    u2              locals  [MAX_LOCALS_COUNT]
                            [MAX_NAME_LENGTH];
} dc_frame;

static dc_stack_entry *     push_entry(dc_stack *);
static dc_stack_entry *     pop_entry(dc_stack *);
static int                  dc_printf(dc_stack_entry *, const char *, ...);
static int                  dc_printf(dc_stack_entry *, rt_Class *, u2);
static int                  dc_initFrame(dc_frame *, rt_Class *, rt_Method *);

int decompile(
        rt_Class *  rtc,
        rt_Method * method,
        u4          len_code,
        u1 *        str_code
)
{
    dc_stack                stack;
    dc_stack_entry *        entry;
    u1 *                    end_code;
    u1                      opcode;
    u2                      index;
    // TODO

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
        {
            if (dc_printf(entry, "null") < 0)
                return -1;
        }
        else if (opcode >= OPCODE_iconst_m1
                && opcode <= OPCODE_iconst_5)
        {
            if (dc_printf(entry, "%i", opcode - OPCODE_iconst_0) < 0)
                return -1;
        }
        else if (opcode == OPCODE_lconst_0
                && opcode == OPCODE_lconst_1)
        {
            if (dc_printf(entry, "%llil", opcode - OPCODE_lconst_0) < 0)
                return -1;
        }
        else if (opcode >= OPCODE_fconst_0
                && opcode <= OPCODE_fconst_2)
        {
            if (dc_printf(entry, "%ff", opcode - OPCODE_fconst_0) < 0)
                return -1;
        }
        else if (opcode == OPCODE_dconst_0
                && opcode == OPCODE_dconst_1)
        {
            if (dc_printf(entry, "%fd", opcode - OPCODE_dconst_0) < 0)
                return -1;
        }
        else if (opcode == OPCODE_bipush)
        {
            if (dc_printf(entry, "%i", *++str_code) < 0)
                return -1;
        }
        else if (opcode == OPCODE_sipush)
        {
            if (dc_printf(entry, "%i",
                        (*++str_code << 8) | *++str_code) < 0)
                return -1;
        }
        else if (opcode >= OPCODE_ldc
                && opcode <= OPCODE_ldc2_w)
        {
            index = *++str_code;
            if (opcode != OPCODE_ldc)
                index = (index << 8) | *++str_code;
            if (dc_printf(entry, rtc, index) < 0)
                return -1;
        }
            // TODO
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
    res = vsprintf((char *) entry->str, format, vl);
    va_end(vl);
    // overflow
    if (res & 0xffffff00) return -1;
    entry->len = (u1) (res & 0xff);

    return res;
}

static int dc_printf(dc_stack_entry *entry, rt_Class *rtc, u2 index)
{
    int                     i, len;
    u1                      cp_tag;
    const_Integer_data *    cid;
    const_Long_data *       cld;
    const_String_data *     csd;
    const_Utf8_data *       cud;
    const_Class_data *      ccd;

    cp_tag = rtc->getConstantTag(index);
    switch (cp_tag)
    {
        case CONSTANT_Utf8:
            cud = rtc->getConstant_Utf8(index);
            return dc_printf(entry, "%.*s", cud->length, cud->bytes);
        case CONSTANT_Integer:
            cid = rtc->getConstant_Integer(index);
            return dc_printf(entry, "%i", cid->bytes);
        case CONSTANT_Float:
            cid = rtc->getConstant_Float(index);
            return dc_printf(entry, "%ff", cid->float_value);
        case CONSTANT_String:
            csd = rtc->getConstant_String(index);
            return dc_printf(entry, rtc, csd->string_index);
        case CONSTANT_Class:
            ccd = rtc->getConstant_Class(index);
            len = dc_printf(entry, rtc, ccd->name_index);
            if (len < 0)
                return -1;
            for (i = 0; i < len; i++)
                if (entry->str[i] == '/')
                    entry->str[i] = '.';
            return len;
        case CONSTANT_Long:
            cld = rtc->getConstant_Long(index);
            return dc_printf(entry, "%llil", cld->long_value);
        case CONSTANT_Double:
            cld = rtc->getConstant_Double(index);
            return dc_printf(entry, "%fd", cld->double_value);
        case CONSTANT_MethodType:
        case CONSTANT_MethodHandle:
            // TODO unsupported yet
            return -1;
    }
}

static int dc_initFrame(dc_frame *frame,
        rt_Class *rtc, rt_Method *rm)
{
    bool                    is_static;
    u2                      i, j;
    u2                      len;
    u1 *                    str;
    const_Utf8_data *       cud;
    attr_info *             attribute;
#if VER_CMP(52, 0)
    attr_MethodParameters_info *
                            ampi;
#endif
#if VER_CMP(45, 3)
    attr_LocalVariableTable_info *
                            alvti;
#endif
#if VER_CMP(49, 0)
    attr_LocalVariableTypeTable_info *
                            alvtti;
#endif

    cud = rm->getDescriptor();
    len = cud->length;
    str = cud->bytes;
    if (!cud)
        return -1;
    is_static = rm->isStatic();
    j = 0;
    if (!is_static)
    {
        memset(frame->locals[0], 0, MAX_NAME_LENGTH);
        if (sprintf((char *) frame->locals[0], "this") < 0)
            return -1;
        j = 1;
    }
#if VER_CMP(52, 0)
    // retrieve MethodParameters attribute
    // retrieve parameter names
    attribute = rm->getAttribute(off_MethodParameters,
            TAG_ATTR_METHODPARAMETERS);
    if (attribute)
    {
        // TODO
        goto mpi;
    }
#endif
    // if MethodParameters is not provided
    for (i = 0; i < len; i++)
    {
    }
mpi:        // MethodParameters is detected

    return 0;
}


