#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "java.h"
#include "opcode.h"
#include "rt.h"

typedef struct
{
    u1              str     [255];
    u1              len;
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

static inline u2            nextIndex(u1 &, u1 *);
static inline u1            next_u1(u1 *);
static inline u2            next_u2(u1 *);
static inline u4            next_u4(u1 *);

static dc_stack_entry *     push_entry(dc_stack *);
static dc_stack_entry *     pop_entry(dc_stack *);
static dc_stack_entry *     push_entry_w(dc_stack *);
static dc_stack_entry *     pop_entry_w(dc_stack *);

static int                  dc_printf(dc_stack_entry *, const char *, ...);
static int                  dc_printf(dc_stack_entry *, rt_Class *, u2);
static int                  dc_initFrame(dc_frame *, rt_Class *, rt_Method *);
static int                  dc_printf(dc_stack_entry *, dc_frame *, u2);
static int                  dc_calculate(dc_stack *, u1, const char *);

int decompile(
        char *      output,
        rt_Class *  rtc,
        rt_Method * method,
        u4          len_code,
        u1 *        str_code
)
{
    dc_stack                stack;
    dc_frame 		        frame;
    dc_stack_entry *        entry;
    dc_stack_entry *        entry1;
    dc_stack_entry *        entry2;
    dc_stack_entry *        entry3;
    u1 *                    end_code;
    u1                      is_wide;
    u1                      opcode;
    const char *            oporater;
    u2                      index;
    u2                      cbyte;

    stack.depth = 1024;
    if (dc_initFrame(&frame, rtc, method) < 0)
        return -1;
    memset(&stack.entries, 0, sizeof (dc_stack));
    end_code = str_code + len_code;
    is_wide = 0;
    while (str_code < end_code)
    {
        opcode = *str_code;
        if (opcode == OPCODE_nop)
            continue;

        switch (opcode)
        {
            // FIXME push_entry function should push two entries
            // if it needs to push long/double value
            case OPCODE_aconst_null:
                entry = push_entry(&stack);
                if (!entry)
                    return -1;
            	if (dc_printf(entry, "null") < 0)
                    return -1;
                break;
            case OPCODE_iconst_m1:
            case OPCODE_iconst_0:
            case OPCODE_iconst_1:
            case OPCODE_iconst_2:
            case OPCODE_iconst_3:
            case OPCODE_iconst_4:
            case OPCODE_iconst_5:
                entry = push_entry(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%i", opcode - OPCODE_iconst_0) < 0)
                    return -1;
                break;
            case OPCODE_lconst_0:
            case OPCODE_lconst_1:
                entry = push_entry_w(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%llil", opcode - OPCODE_lconst_0) < 0)
                    return -1;
                break;
            case OPCODE_fconst_0:
            case OPCODE_fconst_1:
            case OPCODE_fconst_2:
                entry = push_entry(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%ff", opcode - OPCODE_fconst_0) < 0)
                    return -1;
                break;
            case OPCODE_dconst_0:
            case OPCODE_dconst_1:
                entry = push_entry_w(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%fd", opcode - OPCODE_dconst_0) < 0)
                    return -1;
                break;

            case OPCODE_bipush:
                entry = push_entry(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%i", next_u1(str_code)) < 0)
                    return -1;
                break;
            case OPCODE_sipush:
                entry = push_entry(&stack);
                if (!entry)
                    return -1;
                if (dc_printf(entry, "%i", next_u2(str_code) < 0)
                    return -1;

            case OPCODE_ldc:
            case OPCODE_ldc_w:
                entry = push_entry(&stack);
ldc:
                if (!entry)
                    return -1;
                if (opcode == OPCODE_ldc)
                    index = next_u1(str_code);
                else
                    index = next_u2(str_code);
                if (dc_printf(entry, rtc, index) < 0)
                    return -1;
                break;
            case OPCODE_ldc2_w:
                entry = push_entry_w(&stack);
                goto ldc;

            // iload, fload, aload; lload, dload
            // can be prefixed by wide
            case OPCODE_iload:
            case OPCODE_fload:
            case OPCODE_aload:
                index = nextIndex(is_wide, str_code);
load:
                entry = push_entry(&stack);
load_w:
                if (!entry)
                    return -1;
                if (dc_printf(entry, &frame, index) < 0)
                    return -1;
                break;
            case OPCODE_lload:
            case OPCODE_dload:
                index = nextIndex(is_wide, str_code);
                entry = push_entry_w(&stack);
                goto load_w;
            case OPCODE_iload_0:
            case OPCODE_iload_1:
            case OPCODE_iload_2:
            case OPCODE_iload_3:
                index = opcode - OPCODE_iload_0;
                goto load;
            case OPCODE_lload_0:
            case OPCODE_lload_1:
            case OPCODE_lload_2:
            case OPCODE_lload_3:
                index = opcode - OPCODE_lload_0;
                entry = push_entry_w(&stack);
                goto load_w;
            case OPCODE_fload_0:
            case OPCODE_fload_1:
            case OPCODE_fload_2:
            case OPCODE_fload_3:
                index = opcode - OPCODE_fload_0;
                goto load;
            case OPCODE_dload_0:
            case OPCODE_dload_1:
            case OPCODE_dload_2:
            case OPCODE_dload_3:
                index = opcode - OPCODE_dload_0;
                entry = push_entry_w(&stack);
                goto load_w;
            case OPCODE_aload_0:
            case OPCODE_aload_1:
            case OPCODE_aload_2:
            case OPCODE_aload_3:
                index = opcode - OPCODE_aload_0;
                goto load;

            case OPCODE_iaload:
            case OPCODE_faload:
            case OPCODE_aaload:
            case OPCODE_baload:
            case OPCODE_caload:
            case OPCODE_saload:
                // pop index
                entry1 = pop_entry(&stack);
                if (!entry1)
                    return -1;
                // pop arrayref
                entry2 = pop_entry(&stack);
                if (!entry2)
                    return -1;
                // push value ( = arrayref[index] )
                entry = push_entry(&stack);
xaload:
                if (dc_printf(entry, "%.*s[%.*s]",
                            entry2->len, entry2->str,
                            entry1->len, entry1->str) < 0)
                    return -1;
                break;
            case OPCODE_laload:
            case OPCODE_daload:
                entry = push_entry_w(&stack);
                goto xaload;

            // istore, fstore, astore; lstore, dstore
            // can be prefixed by wide
            case OPCODE_istore:
            case OPCODE_fstore:
            case OPCODE_astore:
                index = nextIndex(is_wide, str_code);
store:
                entry1 = pop_entry(&stack);
store_w:
                if (!entry1)
                    return -1;
                entry = push_entry(&stack);
                if (sprintf(output, "%s = %.*s;\r\n",
                        (char *) frame.locals[index],
                        entry1->len, entry1->str) < 0)
                    return -1;
                break;
            case OPCODE_lstore:
            case OPCODE_dstore:
                index = nextIndex(is_wide, str_code);
                entry1 = pop_entry_w(&stack);
                goto store_w;
            case OPCODE_istore_0:
            case OPCODE_istore_1:
            case OPCODE_istore_2:
            case OPCODE_istore_3:
                index = opcode - OPCODE_istore_0;
                goto store;
            case OPCODE_lstore_0:
            case OPCODE_lstore_1:
            case OPCODE_lstore_2:
            case OPCODE_lstore_3:
                index = opcode - OPCODE_lstore_0;
                entry1 = pop_entry_w(&stack);
                goto store_w;
            case OPCODE_fstore_0:
            case OPCODE_fstore_1:
            case OPCODE_fstore_2:
            case OPCODE_fstore_3:
                index = opcode - OPCODE_fstore_0;
                goto store;
            case OPCODE_dstore_0:
            case OPCODE_dstore_1:
            case OPCODE_dstore_2:
            case OPCODE_dstore_3:
                index = opcode - OPCODE_dstore_0;
                entry1 = pop_entry_w(&stack);
                goto store_w;
            case OPCODE_astore_0:
            case OPCODE_astore_1:
            case OPCODE_astore_2:
            case OPCODE_astore_3:
                index = opcode - OPCODE_astore_0;
                goto store;

            case OPCODE_iastore:
            case OPCODE_fastore:
            case OPCODE_aastore:
            case OPCODE_bastore:
            case OPCODE_castore:
            case OPCODE_sastore:
                // pop value
                entry1 = pop_entry(&stack);
xastore:
                if (!entry1)
                    return -1;
                // pop index
                entry2 = pop_entry(&stack);
                if (!entry2)
                    return -1;
                // pop arrayref
                entry3 = pop_entry(&stack);
                if (!entry3)
                    return -1;
                if (sprintf(output, "%.*s[%.*s] = %.*s;\r\n",
                            entry3->len, entry3->str,
                            entry2->len, entry2->str,
                            entry1->len, entry1->str) < 0)
                    return -1;
                break;
            case OPCODE_lastore:
            case OPCODE_dastore:
                entry1 = pop_entry_w(&stack);
                goto xastore;

            case OPCODE_iadd:
            case OPCODE_fadd:
                if (dc_calculate(&stack, 0, "+") < 0)
                    return -1;
                break;
            case OPCODE_ladd:
            case OPCODE_dadd:
                if (dc_calculate(&stack, 1, "+") < 0)
                    return -1;
                break;
            case OPCODE_isub:
            case OPCODE_fsub:
                if (dc_calculate(&stack, 0, "-") < 0)
                    return -1;
                break;
            case OPCODE_lsub:
            case OPCODE_dsub:
                if (dc_calculate(&stack, 1, "-") < 0)
                    return -1;
                break;
            case OPCODE_imul:
            case OPCODE_fmul:
                if (dc_calculate(&stack, 0, "*") < 0)
                    return -1;
                break;
            case OPCODE_lmul:
            case OPCODE_dmul:
                if (dc_calculate(&stack, 1, "*") < 0)
                    return -1;
                break;
            case OPCODE_idiv:
            case OPCODE_fdiv:
                if (dc_calculate(&stack, 0, "/") < 0)
                    return -1;
                break;
            case OPCODE_ldiv:
            case OPCODE_ddiv:
                if (dc_calculate(&stack, 1, "/") < 0)
                    return -1;
                break;
            case OPCODE_irem:
            case OPCODE_frem:
                if (dc_calculate(&stack, 0, "%") < 0)
                    return -1;
                break;
            case OPCODE_lrem:
            case OPCODE_drem:
                if (dc_calculate(&stack, 1, "%") < 0)
                    return -1;
                break;
            case OPCODE_ineg:
            case OPCODE_fneg:
                entry1 = pop_entry(&stack);
                if (!entry1)
                    return -1;
                entry = push_entry(&stack);
                if (dc_printf(entry, "- %.*s", entry1->len, entry1->str) < 0)
                    return -1;
                break;
            case OPCODE_lneg:
            case OPCODE_dneg:
                entry1 = pop_entry_w(&stack);
                if (!entry1)
                    return -1;
                entry = push_entry_w(&stack);
                if (dc_printf(entry, "- %.*s", entry1->len, entry1->str) < 0)
                    return -1;
                break;
            case OPCODE_ishl:
                if (dc_calculate(&stack, 0, "<<") < 0)
                    return -1;
                break;
            case OPCODE_lshl:
                if (dc_calculate(&stack, 1, "<<") < 0)
                    return -1;
                break;
            case OPCODE_ishr:
                if (dc_calculate(&stack, 0, ">>") < 0)
                    return -1;
                break;
            case OPCODE_lshr:
                if (dc_calculate(&stack, 1, ">>") < 0)
                    return -1;
                break;
            case OPCODE_iushr:
                if (dc_calculate(&stack, 0, ">>>") < 0)
                    return -1;
                break;
            case OPCODE_lushr:
                if (dc_calculate(&stack, 1, ">>>") < 0)
                    return -1;
                break;
            case OPCODE_iand:
                if (dc_calculate(&stack, 0, "&") < 0)
                    return -1;
                break;
            case OPCODE_land:
                if (dc_calculate(&stack, 1, "&") < 0)
                    return -1;
                break;
            case OPCODE_ior:
                if (dc_calculate(&stack, 0, "|") < 0)
                    return -1;
                break;
            case OPCODE_lor:
                if (dc_calculate(&stack, 1, "|") < 0)
                    return -1;
                break;
            case OPCODE_ixor:
                if (dc_calculate(&stack, 0, "^") < 0)
                    return -1;
                break;
            case OPCODE_lxor:
                if (dc_calculate(&stack, 1, "^") < 0)
                    return -1;
                break;
            case OPCODE_iinc:
                index = *++str_code;
                cbyte = *++str_code;
                if (is_wide)
                {
                    index = (index << 8) | cbyte;
                    cbyte = (*++str_code << 8) | *++str_code;
                    is_wide = 0;
                }
                // TODO
                break;

            case OPCODE_wide:
                is_wide = 1;
                continue;
	        default:
            	// TODO more opcodes to be processed
		        return -1;
        }

        // wide is followed by invalid opcode
        if (is_wide)
            return -1;
    }
}

static inline u2 nextIndex(u1 &is_wide, u1 *str_code)
{
    u2 index;

    if (is_wide)
    {
        index = next_u2(str_code);
        is_wide = 0;
    }
    else
    {
        index = next_u1(str_code);
    }

    return index;
}

static inline u1 next_u1(u1 *str_code)
{
    return *++str_code;
}

static inline u2 next_u2(u1 *str_code)
{
    return (next_u1(str_code) << 8) | next_u1(str_code);
}

static inline u4 next_u4(u1 *str_code)
{
    return (next_u2(str_code) << 16) | next_u2(str_code);
}

static dc_stack_entry *push_entry(dc_stack *stack)
{
    u4 index;

    index = stack->depth;
    if (--index < 0)
        return (dc_stack_entry *) 0;
    stack->depth = index;

    return &(stack->entries[index]);
}

static dc_stack_entry *push_entry_w(dc_stack *stack)
{
    u4 index;

    index = stack->depth;
    index -= 2;
    if (index < 0)
        return (dc_stack_entry *) 0;
    stack->depth = index;

    return &(stack->entries[index]);
}

static dc_stack_entry *pop_entry(dc_stack *stack)
{
    u4 index;

    index = stack->depth;
    if (index >= MAX_STACK_DEPTH)
        return (dc_stack_entry *) 0;

    return &(stack->entries[stack->depth++]);
}

static dc_stack_entry *pop_entry_w(dc_stack *stack)
{
    u4 index;
    dc_stack_entry *entry;

    index = stack->depth;
    index += 2;
    entry = (dc_stack_entry *) 0;
    if (index < MAX_STACK_DEPTH)
    {
        entry = &(stack->entries[stack->depth]);
        stack->depth = index;
    }
        
    return entry;
}

static int dc_printf(dc_stack_entry *entry, const char *format, ...)
{
    va_list vl;
    size_t res;

    memset(entry, 0, 256);
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
            // TODO MethodType/MethodHandle is unsupported yet 
            return -1;
    }
}

static int dc_printf(dc_stack_entry *entry, dc_frame *frame, u2 index)
{
    return dc_printf(entry, "%s", frame->locals[index]);
}

static int dc_calculate(dc_stack *stack, u1 is_wide, const char *op)
{
    dc_stack_entry *to, *p1, *p2;

    p1 = is_wide ? pop_entry_w(stack) : pop_entry(stack);
    if (!p1)
        return -1;
    p2 = is_wide ? pop_entry_w(stack) : pop_entry(stack);
    if (!p2)
        return -1;
    to = is_wide ? push_entry_w(stack) : push_entry(stack);

    return dc_printf(to, "%.*s %s %.*s",
            p2->len, p2->str, op, p1->len, p1->str);
}

static int dc_initFrame(dc_frame *frame,
        rt_Class *rtc, rt_Method *rm)
{
    u1                                  not_static;
    u1                                  count, state;
    struct parameter_entry *            pe;
    u2                                  i, j;
    u1                                  k;
    u2                                  len;
    u1 *                                str;
    const_Utf8_data *                   cud;
    attr_info *                         attribute;
#if VER_CMP(52, 0)
    attr_MethodParameters_info *        ampi;
#endif
#if VER_CMP(45, 3)
    attr_LocalVariableTable_info *      alvti;
#endif
#if VER_CMP(49, 0)
    attr_LocalVariableTypeTable_info *  alvtti;
#endif

    not_static = rm->isStatic() ? 0 : 1;
    // static methods' valid parameter entry starts from 0;
    // instance methods' valid parameter entry starts from 1,
    // with `this` object as default parameter at index 0.
    if (not_static)
    {
        memset(frame->locals[0], 0, MAX_NAME_LENGTH);
        if (sprintf((char *) frame->locals[0], "this") < 0)
            return -1;
    }

    // retrieve method description
    cud = rm->getDescriptor();
    if (!cud)
        return -1;
    len = cud->length;
    str = cud->bytes;
    cud = (const_Utf8_data *) 0;
    // method descriptor finite automata
    state = 0;
    // parameter name entry
    j = 0;
    // stack entry
    k = not_static;
#if VER_CMP(52, 0)
    // retrieve MethodParameters attribute
    ampi = rm->getAttribute_MethodParameters();
    count = 0;
    if (ampi)
        count = ampi->parameters_count;
#endif
    // | state | description          |
    // | ----- | -------------------- |
    // |     0 | terminal             |
    // |     1 | computational type 1 |
    // |     2 | computational type 2 |
    // |     4 | array type ( ct 1 )  |
    for (i = 0; i < len; i++)
    {
        if (str[i] == ')')
            break;
        // inteprete parameter descriptor
        // instance/array parameters belongs to computational category 1
        // primitive parameters except long/double belongs to
        //      computational category 1
        // long/double parameters belongs to computational category 2
        switch (str[i])
        {
            case '(':
                continue;
            case 'L':
                while (i < len && str[i++] != ';');
                state |= 1;
                break;
            case 'C':case 'B':case 'I':case 'F':case 'Z':case 'S':
                state |= 1;
                break;
            case 'J':case 'D':
                state |= 2;
                break;
            case '[':
                state = 4;
                while (++i < len && str[i] == '[');
                continue;
            default:
                return -1;
        }

        switch (state)
        {
            case 1:case 5:case 6:
                // computational category 1
                state = 1;
                break;
            case 2:
                // computational category 2
                state = 2;
                break;
            default:
                // non-accepting state
                return -1;
        }

        // assert j is never greater than count
        if (j >= count)
            return -1;
        // retrieve or create parameter names
        memset(frame->locals[k], 0, MAX_NAME_LENGTH);
#if VER_CMP(52, 0)
        if (ampi)
        {
            // MethodParameters is available
            // retrieve j-th parameter entry
            pe = &(ampi->parameters[j]);
            cud = rtc->getConstant_Utf8(pe->name_index);
            if (!cud)
                return -1;
            // write j-th parameter's name
            if (sprintf((char *) frame->locals[k],
                        "%.*s", cud->length, cud->bytes) < 0)
                return -1;
        }
        else
        {
#endif
            // MethodParameters is not available
            if (sprintf((char *) frame->locals[k],
                        "param%i", j) < 0)
                return -1;
#if VER_CMP(52, 0)
        }
#endif
        // point to next parameter name entry
        // and next parameter stack entry
        ++j;
        k += state;

        state = 0;
    }

    return 0;
}



