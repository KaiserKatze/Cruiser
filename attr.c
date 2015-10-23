#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "java.h"
#include "log.h"
#include "opcode.h"

static int
loadAttribute(struct BufferIO *input, attr_info *info)
{
    if (ru2(&(info->attribute_name_index), input) < 0)
        return -1;
    if (ru4(&(info->attribute_length), input) < 0)
        return -1;
    return 0;
}

static int
skipAttribute(struct BufferIO *input, attr_info *info)
{
    logError("Skip attribute{name_index: 0x%02X, length: %i}!\r\n",
            info->attribute_name_index, info->attribute_length);
    return skp(input, info->attribute_length);
}

#if VER_CMP(45, 3)
static int
loadAttribute_ConstantValue(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_ConstantValue_info *data;

    info->tag = TAG_ATTR_CONSTANTVALUE;
    data = (struct attr_ConstantValue_info *)
            malloc(sizeof (struct attr_ConstantValue_info));
    if (!data)
        return -1;
    if (ru2(&(data->constantvalue_index), input) < 0)
        return -1;

    info->data = data;
    return 0;
}

static int
freeAttribute_ConstantValue(attr_info *info)
{
    if (info->tag != TAG_ATTR_CONSTANTVALUE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_Code(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_Code_info *data;
    u2 i;
    u4 j;

    info->tag = TAG_ATTR_CODE;
    data = (struct attr_Code_info *)
            malloc(sizeof (struct attr_Code_info));
    if (!data)
        return -1;
    logInfo("\tCode            :\r\n");
    if (ru2(&(data->max_stack), input) < 0)
        return -1;
    logInfo("\t\t// max_stack = %i\r\n", data->max_stack);
    if (ru2(&(data->max_locals), input) < 0)
        return -1;
    logInfo("\t\t// max_locals = %i\r\n", data->max_locals);
    if (ru4(&(data->code_length), input) < 0)
        return -1;
    logInfo("\t\t// code_length = %i\r\n", data->code_length);
    if (data->code_length <= 0)
    {
        logError("Assertion error: data->code_length <= 0!\r\n");
        return -1;
    }
    if (data->code_length >= 65536)
    {
        logError("Assertion error: data->code_length >= 65536!\r\n");
        return -1;
    }
    data->code = malloc(data->code_length);
    if (!data->code)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    if (rbs(data->code, input, data->code_length) < 0)
        return -1;
    logInfo("\t\t// Hex code =\r\n");
    logInfo("\t\t");
    for (j = 0u; j < data->code_length; j++)
    {
        if (j != 0u && j % 20 == 0)
            logInfo("\r\n\t\t");
        logInfo("%02X ", data->code[j]);
    }
    logInfo("\r\n");
    logInfo("\t\t// Human-readable =\r\n");
    for (j = 0u; j < data->code_length; j++)
    {
        logInfo("\t\t");
        switch (data->code[j])
        {
            case OPCODE_nop:break;
            case OPCODE_aconst_null:break;
            case OPCODE_iconst_ml:break;
            case OPCODE_iconst_0:break;
            case OPCODE_iconst_1:break;
            case OPCODE_iconst_2:break;
            case OPCODE_iconst_3:break;
            case OPCODE_iconst_4:break;
            case OPCODE_iconst_5:break;
            case OPCODE_lconst_0:break;
            case OPCODE_lconst_1:break;
            case OPCODE_fconst_0:break;
            case OPCODE_fconst_1:break;
            case OPCODE_fconst_2:break;
            case OPCODE_dconst_0:break;
            case OPCODE_dconst_1:break;
            case OPCODE_bipush:break;
            case OPCODE_sipush:break;
            case OPCODE_ldc:break;
            case OPCODE_ldc_w:break;
            case OPCODE_ldc2_w:break;
            case OPCODE_iload:break;
            case OPCODE_lload:break;
            case OPCODE_fload:break;
            case OPCODE_dload:break;
            case OPCODE_aload:break;
            case OPCODE_iload_0:break;
            case OPCODE_iload_1:break;
            case OPCODE_iload_2:break;
            case OPCODE_iload_3:break;
            case OPCODE_lload_0:break;
            case OPCODE_lload_1:break;
            case OPCODE_lload_2:break;
            case OPCODE_lload_3:break;
            case OPCODE_fload_0:break;
            case OPCODE_fload_1:break;
            case OPCODE_fload_2:break;
            case OPCODE_fload_3:break;
            case OPCODE_dload_0:break;
            case OPCODE_dload_1:break;
            case OPCODE_dload_2:break;
            case OPCODE_dload_3:break;
            case OPCODE_aload_0:break;
            case OPCODE_aload_1:break;
            case OPCODE_aload_2:break;
            case OPCODE_aload_3:break;
            case OPCODE_iaload:break;
            case OPCODE_laload:break;
            case OPCODE_faload:break;
            case OPCODE_daload:break;
            case OPCODE_aaload:break;
            case OPCODE_baload:break;
            case OPCODE_caload:break;
            case OPCODE_saload:break;
            case OPCODE_istore:break;
            case OPCODE_lstore:break;
            case OPCODE_fstore:break;
            case OPCODE_dstore:break;
            case OPCODE_astore:break;
            case OPCODE_istore_0:break;
            case OPCODE_istore_1:break;
            case OPCODE_istore_2:break;
            case OPCODE_istore_3:break;
            case OPCODE_lstore_0:break;
            case OPCODE_lstore_1:break;
            case OPCODE_lstore_2:break;
            case OPCODE_lstore_3:break;
            case OPCODE_fstore_0:break;
            case OPCODE_fstore_1:break;
            case OPCODE_fstore_2:break;
            case OPCODE_fstore_3:break;
            case OPCODE_dstore_0:break;
            case OPCODE_dstore_1:break;
            case OPCODE_dstore_2:break;
            case OPCODE_dstore_3:break;
            case OPCODE_astore_0:break;
            case OPCODE_astore_1:break;
            case OPCODE_astore_2:break;
            case OPCODE_astore_3:break;
            case OPCODE_iastore:break;
            case OPCODE_lastore:break;
            case OPCODE_fastore:break;
            case OPCODE_dastore:break;
            case OPCODE_aastore:break;
            case OPCODE_bastore:break;
            case OPCODE_castore:break;
            case OPCODE_sastore:break;
            case OPCODE_pop:break;
            case OPCODE_pop2:break;
            case OPCODE_dup:break;
            case OPCODE_dup_x1:break;
            case OPCODE_dup_x2:break;
            case OPCODE_dup2:break;
            case OPCODE_dup2_x1:break;
            case OPCODE_dup2_x2:break;
            case OPCODE_swap:break;
            case OPCODE_iadd:break;
            case OPCODE_ladd:break;
            case OPCODE_fadd:break;
            case OPCODE_dadd:break;
            case OPCODE_isub:break;
            case OPCODE_lsub:break;
            case OPCODE_fsub:break;
            case OPCODE_dsub:break;
            case OPCODE_imul:break;
            case OPCODE_lmul:break;
            case OPCODE_fmul:break;
            case OPCODE_dmul:break;
            case OPCODE_idiv:break;
            case OPCODE_ldiv:break;
            case OPCODE_fdiv:break;
            case OPCODE_ddiv:break;
            case OPCODE_irem:break;
            case OPCODE_lrem:break;
            case OPCODE_frem:break;
            case OPCODE_drem:break;
            case OPCODE_ineg:break;
            case OPCODE_lneg:break;
            case OPCODE_fneg:break;
            case OPCODE_dneg:break;
            case OPCODE_ishl:break;
            case OPCODE_lshl:break;
            case OPCODE_ishr:break;
            case OPCODE_lshr:break;
            case OPCODE_iushr:break;
            case OPCODE_lushr:break;
            case OPCODE_iand:break;
            case OPCODE_land:break;
            case OPCODE_ior:break;
            case OPCODE_lor:break;
            case OPCODE_ixor:break;
            case OPCODE_lxor:break;
            case OPCODE_iinc:break;
            case OPCODE_i2l:break;
            case OPCODE_i2f:break;
            case OPCODE_i2d:break;
            case OPCODE_l2i:break;
            case OPCODE_l2f:break;
            case OPCODE_l2d:break;
            case OPCODE_f2i:break;
            case OPCODE_f2l:break;
            case OPCODE_f2d:break;
            case OPCODE_d2i:break;
            case OPCODE_d2l:break;
            case OPCODE_d2f:break;
            case OPCODE_i2b:break;
            case OPCODE_i2c:break;
            case OPCODE_i2s:break;
            case OPCODE_lcmp:break;
            case OPCODE_fcmpl:break;
            case OPCODE_fcmpg:break;
            case OPCODE_dcmpl:break;
            case OPCODE_dcmpg:break;
            case OPCODE_ifeq:break;
            case OPCODE_ifne:break;
            case OPCODE_iflt:break;
            case OPCODE_ifge:break;
            case OPCODE_ifgt:break;
            case OPCODE_ifle:break;
            case OPCODE_if_icmpeq:break;
            case OPCODE_if_icmpne:break;
            case OPCODE_if_icmplt:break;
            case OPCODE_if_icmpge:break;
            case OPCODE_if_icmpgt:break;
            case OPCODE_if_icmple:break;
            case OPCODE_if_acmpeq:break;
            case OPCODE_if_acmpne:break;
            case OPCODE_goto:break;
            case OPCODE_jsr:break;
            case OPCODE_ret:break;
            case OPCODE_tableswitch:break;
            case OPCODE_lookupswitch:break;
            case OPCODE_ireturn:break;
            case OPCODE_lreturn:break;
            case OPCODE_freturn:break;
            case OPCODE_dreturn:break;
            case OPCODE_areturn:break;
            case OPCODE_return:break;
            case OPCODE_getstatic:break;
            case OPCODE_putstatic:break;
            case OPCODE_getfield:break;
            case OPCODE_putfield:break;
            case OPCODE_invokevirtual:break;
            case OPCODE_invokespecial:break;
            case OPCODE_invokestatic:break;
            case OPCODE_invokeinterface:break;
            case OPCODE_invokedynamic:break;
            case OPCODE_new:break;
            case OPCODE_newarray:break;
            case OPCODE_anewarray:break;
            case OPCODE_arraylength:break;
            case OPCODE_athrow:break;
            case OPCODE_checkcast:break;
            case OPCODE_instanceof:break;
            case OPCODE_monitorenter:break;
            case OPCODE_monitorexit:break;
            case OPCODE_wide:break;
            case OPCODE_multianewarray:break;
            case OPCODE_ifnull:break;
            case OPCODE_ifnonnull:break;
            case OPCODE_goto_w:break;
            case OPCODE_jsr_w:break;
            case OPCODE_breakpoint:break;
            case OPCODE_impdep1:break;
            case OPCODE_impdep2:break;
            default:
                logError("Unknow OPCODE %i!\r\n", data->code[j]);
                break;
        }
    }
    
    if (ru2(&(data->exception_table_length), input) < 0)
        return -1;
    data->exception_table = (struct exception_table_entry *)
        malloc(sizeof (struct exception_table_entry) * data->exception_table_length);
    if (!data->exception_table)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    for (i = 0u; i < data->exception_table_length; i++)
    {
        if (ru2(&(data->exception_table[i].start_pc), input) < 0)
            return -1;
        if (ru2(&(data->exception_table[i].end_pc), input) < 0)
            return -1;
        if (ru2(&(data->exception_table[i].handler_pc), input) < 0)
            return -1;
        if (ru2(&(data->exception_table[i].catch_type), input) < 0)
            return -1;
    }
    loadAttributes_code(cf, input, &(data->attributes_count), &(data->attributes));

    info->data = data;
    return 0;
}

static int
freeAttribute_Code(attr_info *info)
{
    struct attr_Code_info *data;

    if (info->tag != TAG_ATTR_CODE)
        return -1;
    data = (struct attr_Code_info *) info->data;
    free(data->code);
    data->code = (u1 *) 0;
    free(data->exception_table);
    data->exception_table = 0;
    freeAttributes_code(data->attributes_count, data->attributes);
    free(data->attributes);
    data->attributes = (attr_info *) 0;
    free(data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_Exceptions(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_Exceptions_info *data;
    CONSTANT_Class_info *cc;
    u2 i;

    info->tag = TAG_ATTR_EXCEPTIONS;
    data = (struct attr_Exceptions_info *)
            malloc(sizeof (struct attr_Exceptions_info));
    if (!data)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    bzero(data, sizeof (struct attr_Exceptions_info));
    if (ru2(&(data->number_of_exceptions), input) < 0)
        return -1;
    data->exception_index_table = (u2 *)
        malloc(sizeof (u2) * data->number_of_exceptions);
    if (!data->exception_index_table)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    for (i = 0u; i < data->number_of_exceptions; i++)
    {
        if (ru2(&(data->exception_index_table[i]), input) < 0)
            return -1;
        cc = getConstant_Class(cf, data->exception_index_table[i]);
        if (!cc)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->exception_index_table[i]);
            return -1;
        }
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_Exceptions(attr_info *info)
{
    struct attr_Exceptions_info *data;

    if (info->tag != TAG_ATTR_EXCEPTIONS)
        return -1;
    data = (struct attr_Exceptions_info *) info->data;
    free(data->exception_index_table);
    free(data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_InnerClasses(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_InnerClasses_info *data;
    CONSTANT_Class_info *cc;
    CONSTANT_Utf8_info *cu;
    u2 i;

    info->tag = TAG_ATTR_INNERCLASSES;
    data = (struct attr_InnerClasses_info *)
            malloc(sizeof (struct attr_InnerClasses_info));
    if (!data)
        return -1;
    if (ru2(&(data->number_of_classes), input) < 0)
        return -1;
    data->classes = (struct classes_entry *)
        malloc(sizeof (struct classes_entry) * data->number_of_classes);
    if (!data->classes)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    for (i = 0u; i < data->number_of_classes; i++)
    {
        if (ru2(&(data->classes[i].inner_class_info_index), input) < 0)
            return -1;
        cc = getConstant_Class(cf, data->classes[i].inner_class_info_index);
        if (!cc)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->classes[i].inner_class_info_index);
            return -1;
        }
        if (ru2(&(data->classes[i].outer_class_info_index), input) < 0)
            return -1;
        cc = getConstant_Class(cf, data->classes[i].outer_class_info_index);
        if (!cc)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->classes[i].outer_class_info_index);
            return -1;
        }
        if (ru2(&(data->classes[i].inner_name_index), input) < 0)
            return -1;
        cu = getConstant_Utf8(cf, data->classes[i].inner_name_index);
        if (!cu)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->classes[i].inner_name_index);
            return -1;
        }
        if (ru2(&(data->classes[i].inner_class_access_flags), input) < 0)
            return -1;
        if (data->classes[i].inner_class_access_flags & ACC_NESTED_CLASS)
        {
            logError("Assertion error: data->classes[%i].inner_class_access_flags != 0x%X!\r\n", i, ACC_NESTED_CLASS);
            return -1;
        }
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_InnerClasses(attr_info *info)
{
    struct attr_InnerClasses_info *data;

    if (info->tag != TAG_ATTR_INNERCLASSES)
        return -1;
    data = (struct attr_InnerClasses_info *) info->data;
    free(data->classes);
    data->classes = 0;
    free(data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_Synthetic(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    if (info->attribute_length != 0)
    {
        logError("Assertion error: info->attribute_length != 0!\r\n");
        return -1;
    }
    info->tag = TAG_ATTR_SYNTHETIC;
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_SourceFile(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_SourceFile_info *data;
    CONSTANT_Utf8_info *cu;

    info->tag = TAG_ATTR_SOURCEFILE;
    data = (struct attr_SourceFile_info *)
            malloc(sizeof (struct attr_SourceFile_info));
    if (!data)
        return -1;
    if (ru2(&(data->sourcefile_index), input) < 0)
        return -1;
    cu = getConstant_Utf8(cf, data->sourcefile_index);
    if (!cu)
    {
        logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n", data->sourcefile_index);
        return -1;
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_SourceFile(attr_info *info)
{
    if (info->tag == TAG_ATTR_SOURCEFILE)
        return -1;
    free(info->data);
    info->data = 0;

    return 0;
}

static int
loadAttribute_SourceDebugExtension(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    int cap;

    info->tag = TAG_ATTR_SOURCEDEBUGEXTENSION;
    cap = sizeof (u1) * info->attribute_length;
    info->data = (u1 *) malloc(info->attribute_length);
    if (!info->data)
        return -1;
    if (rbs((char *) info->data, input, cap) < 0)
        return -1;

    return 0;
}

static int
freeAttribute_SourceDebugExtension(attr_info *info)
{
    if (info->tag != TAG_ATTR_SOURCEDEBUGEXTENSION)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_LineNumberTable(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_LineNumberTable_info *data;
    u2 i, lntl;
    int cap;

    info->tag = TAG_ATTR_LINENUMBERTABLE;
    if (ru2(&lntl, input) < 0)
        return -1;
    cap = sizeof (struct attr_LineNumberTable_info)
        + sizeof (struct line_number_table_entry) * lntl;
    data = (struct attr_LineNumberTable_info *) malloc(cap);
    if (!data)
        return -1;
    data->line_number_table_length = lntl;
    for (i = 0; i < lntl; i++)
    {
        if (ru2(&(data->line_number_table[i].start_pc), input) < 0)
            return -1;
        if (ru2(&(data->line_number_table[i].line_number), input) < 0)
            return -1;
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_LineNumberTable(attr_info *info)
{
    if (info->tag != TAG_ATTR_LINENUMBERTABLE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_LocalVariableTable(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_LocalVariableTable_info *data;
    u2 i, lvtl;
    int cap;
    CONSTANT_Utf8_info *cu;

    info->tag = TAG_ATTR_LOCALVARIABLETABLE;
    if (ru2(&lvtl, input) < 0)
        return -1;
    cap = sizeof (struct attr_LocalVariableTable_info)
        + sizeof (struct local_variable_table_entry) * lvtl;
    data = (struct attr_LocalVariableTable_info *) malloc(cap);
    if (!data)
        return -1;
    data->local_variable_table_length = lvtl;
    for (i = 0u; i < lvtl; i++)
    {
        if (ru2(&(data->local_variable_table[i].start_pc), input) < 0)
            return -1;
        if (ru2(&(data->local_variable_table[i].length), input) < 0)
            return -1;
        if (ru2(&(data->local_variable_table[i].name_index), input) < 0)
            return -1;
        cu = getConstant_Utf8(cf, data->local_variable_table[i].name_index);
        if (!cu)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n");
            return -1;
        }
        if (ru2(&(data->local_variable_table[i].descriptor_index), input) < 0)
            return -1;
        cu = getConstant_Utf8(cf, data->local_variable_table[i].descriptor_index);
        if (!cu)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n");
            return -1;
        }
        if (ru2(&(data->local_variable_table[i].index), input) < 0)
            return -1;
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_LocalVariableTable(attr_info *info)
{
    if (info->tag != TAG_ATTR_LOCALVARIABLETABLE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_Deprecated(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    if (info->attribute_length != 0)
    {
        logError("Assertion error: info->attribute_length != 0!\r\n");
        return -1;
    }
    info->tag = TAG_ATTR_DEPRECATED;
    info->data = (void *) 0;

    return 0;
}
#endif /* VERSION 45.3 */
#if VER_CMP(49, 0)
static int
loadAttribute_EnclosingMethod(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_EnclosingMethod_info *data;
    CONSTANT_Class_info *cc;
    CONSTANT_NameAndType_info *cn;

    info->tag = TAG_ATTR_ENCLOSINGMETHOD;
    data = (struct attr_EnclosingMethod_info *)
            malloc(sizeof (struct attr_EnclosingMethod_info));
    if (!data)
        return -1;
    if (ru2(&(data->class_index), input) < 0)
        return -1;
    cc = getConstant_Class(cf, data->class_index);
    if (!cc)
    {
        logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->class_index);
        return -1;
    }
    if (ru2(&(data->method_index), input) < 0)
        return -1;
    if (data->method_index != 0)
    {
        cn = getConstant_NameAndType(cf, data->method_index);
        if (!cn)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_NameAndType_info!\r\n", data->method_index);
            return -1;
        }
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_EnclosingMethod(attr_info *info)
{
    if (info->tag != TAG_ATTR_ENCLOSINGMETHOD)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_Signature(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_Signature_info *data;
    CONSTANT_Utf8_info *cu;

    info->tag = TAG_ATTR_SIGNATURE;
    data = (struct attr_Signature_info *)
            malloc(sizeof (struct attr_Signature_info));
    if (!data)
        return -1;
    if (ru2(&(data->signature_index), input) < 0)
        return -1;
    cu = getConstant_Utf8(cf, data->signature_index);
    if (!cu)
    {
        logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n", data->signature_index);
        return -1;
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_Signature(attr_info *info)
{
    if (info->tag != TAG_ATTR_SIGNATURE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadAttribute_LocalVariableTypeTable(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_LocalVariableTypeTable_info *data;
    u2 i, lvttl;
    int cap;
    CONSTANT_Utf8_info *cu;

    info->tag = TAG_ATTR_LOCALVARIABLETYPETABLE;
    if (ru2(&lvttl, input) < 0)
        return -1;
    cap = sizeof (struct attr_LocalVariableTypeTable_info)
        + sizeof (struct local_variable_type_table_entry) * lvttl;
    data = (struct attr_LocalVariableTypeTable_info *) malloc(cap);
    data->local_variable_type_table_length = lvttl;
    if (!data)
        return -1;
    for (i = 0u; i < lvttl; i++)
    {
        if (ru2(&(data->local_variable_type_table[i].start_pc), input) < 0)
            return -1;
        if (ru2(&(data->local_variable_type_table[i].length), input) < 0)
            return -1;
        if (ru2(&(data->local_variable_type_table[i].name_index), input) < 0)
            return -1;
        cu = getConstant_Utf8(cf, data->local_variable_type_table[i].name_index);
        if (!cu)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n");
            return -1;
        }
        if (ru2(&(data->local_variable_type_table[i].signature_index), input) < 0)
            return -1;
        cu = getConstant_Utf8(cf, data->local_variable_type_table[i].signature_index);
        if (!cu)
        {
            logError("Assertion error: constant_pool[%i] is not CONSTANT_Utf8_info instance!\r\n");
            return -1;
        }
        if (ru2(&(data->local_variable_type_table[i].index), input) < 0)
            return -1;
    }

    info->data = data;
    return 0;
}

static int
freeAttribute_LocalVariableTypeTable(attr_info *info)
{
    if (info->data != TAG_ATTR_LOCALVARIABLETYPETABLE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}
#endif /* VERSION 49.0 */
#if VER_CMP(50, 0)
static int
loadAttribute_StackMapTable(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_StackMapTable_info *data;
    u2 i;

    info->tag = TAG_ATTR_STACKMAPTABLE;
    data = (struct attr_StackMapTable_info *)
            malloc(sizeof (struct attr_StackMapTable_info));
    if (!data)
        return -1;
    if (ru2(&(data->number_of_entries), input) < 0)
        return -1;
    for (i = 0u; i < data->number_of_entries; i++)
    {
        // TODO need implementation
    }

    info->data = data;
    return 0;
}
#endif /* VERSION 50.0 */

extern int
loadAttribute_class(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;

    if (loadAttribute(input, info) < 0)
        return -1;
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;

#if VER_CMP(45, 3)
    if (!strncmp(attribute_name, "SourceFile", 10))
        return loadAttribute_SourceFile(cf, input, info);
    if (!strncmp(attribute_name, "InnerClasses", 12))
        return loadAttribute_InnerClasses(cf, input, info);
    if (!strncmp(attribute_name, "Synthetic", 9))
        return loadAttribute_Synthetic(cf, input, info);
    if (!strncmp(attribute_name, "Deprecated", 10))
        return loadAttribute_Deprecated(cf, input, info);
#endif
#if VER_CMP(49, 0)
    if (!strncmp(attribute_name, "EnclosingMethod", 15))
        return loadAttribute_EnclosingMethod(cf, input, info);
    if (!strncmp(attribute_name, "SourceDebugExtension", 20))
        return loadAttribute_SourceDebugExtension(cf, input, info);
    if (!strncmp(attribute_name, "Signature", 9))
        return loadAttribute_Signature(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeVisibleAnnotations", 25))
        return loadAttribute_RuntimeVisibleAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleAnnotations", 27))
        return loadAttribute_RuntimeInvisibleAnnotations(cf, input, info);
#endif
#if VER_CMP(51, 0)
    if (!strncmp(attribute_name, "BootstrapMethods", 16))
        return loadAttribute_BootstrapMethods(cf, input, info);
#endif
#if VER_CMP(52, 0)
    if (!strncmp(attribute_name, "RuntimeVisibleTypeAnnotations", 29))
        return loadAttribute_RuntimeVisibleTypeAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleTypeAnnotations", 31))
        return loadAttribute_RuntimeInvisibleTypeAnnotations(cf, input, info);
#endif
    return skipAttribute(input, info);
}

extern int
loadAttribute_field(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;

    if (loadAttribute(input, info) < 0)
        return -1;
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;

#if VER_CMP(45, 3)
    if (!strncmp(attribute_name, "ConstantValue", 13))
        return loadAttribute_ConstantValue(cf, input, info);
    if (!strncmp(attribute_name, "Synthetic", 9))
        return loadAttribute_Synthetic(cf, input, info);
    if (!strncmp(attribute_name, "Deprecated", 10))
        return loadAttribute_Deprecated(cf, input, info);
#endif
#if VER_CMP(49, 0)
    if (!strncmp(attribute_name, "Signature", 9))
        return loadAttribute_Signature(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeVisibleAnnotations", 25))
        return loadAttribute_RuntimeVisibleAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleAnnotations", 27))
        return loadAttribute_RuntimeInvisibleAnnotations(cf, input, info);
#endif
#if VER_CMP(52, 0)
    if (!strncmp(attribute_name, "RuntimeVisibleTypeAnnotations", 29))
        return loadAttribute_RuntimeVisibleTypeAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleTypeAnnotations", 31))
        return loadAttribute_RuntimeInvisibleTypeAnnotations(cf, input, info);
#endif

    return skipAttribute(input, info);
}

extern int
loadAttribute_method(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;

    if (loadAttribute(input, info) < 0)
        return -1;
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;

#if VER_CMP(45, 3)
    if (!strncmp(attribute_name, "Code", 4))
        return loadAttribute_Code(cf, input, info);
    if (!strncmp(attribute_name, "Exceptions", 10))
        return loadAttribute_Exceptions(cf, input, info);
    if (!strncmp(attribute_name, "Synthetic", 9))
        return loadAttribute_Synthetic(cf, input, info);
    if (!strncmp(attribute_name, "Deprecated", 10))
        return loadAttribute_Deprecated(cf, input, info);
#endif
#if VER_CMP(49, 0)
    if (!strncmp(attribute_name, "RuntimeVisibleParameterAnnotations", 34))
        return loadAttribute_RuntimeVisibleParameterAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleParameterAnnotations", 36))
        return loadAttribute_RuntimeInvisibleParameterAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "AnnotationDefault", 17))
        return loadAttribute_AnnotationDefault(cf, input, info);
    if (!strncmp(attribute_name, "Signature", 9))
        return loadAttribute_Signature(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeVisibleAnnotations", 25))
        return loadAttribute_RuntimeVisibleAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleAnnotations", 27))
        return loadAttribute_RuntimeInvisibleAnnotations(cf, input, info);
#endif
#if VER_CMP(52, 0)
    if (!strncmp(attribute_name, "MethodParameters", 16))
        return loadAttribute_MethodParameters(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeVisibleTypeAnnotations", 29))
        return loadAttribute_RuntimeVisibleTypeAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleTypeAnnotations", 31))
        return loadAttribute_RuntimeInvisibleTypeAnnotations(cf, input, info);
#endif

    return skipAttribute(input, info);
}

extern int
loadAttribute_code(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;

    if (loadAttribute(input, info) < 0)
        return -1;
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;

#if VER_CMP(45, 3)
    if (!strncmp(attribute_name, "LineNumberTable", 15))
        return loadAttribute_LineNumberTable(cf, input, info);
    if (!strncmp(attribute_name, "LocalVariableTable", 18))
        return loadAttribute_LocalVariableTable(cf, input, info);
#endif
#if VER_CMP(49, 0)
    if (!strncmp(attribute_name, "LocalVariableTypeTable", 22))
        return loadAttribute_LocalVariableTypeTable(cf, input, info);
#endif
#if VER_CMP(50, 0)
    if (!strncmp(attribute_name, "StackMapTable", 13))
        return loadAttribute_StackMapTable(cf, input, info);
#endif
#if VER_CMP(52, 0)
    if (!strncmp(attribute_name, "RuntimeVisibleTypeAnnotations", 29))
        return loadAttribute_RuntimeVisibleTypeAnnotations(cf, input, info);
    if (!strncmp(attribute_name, "RuntimeInvisibleTypeAnnotations", 31))
        return loadAttribute_RuntimeInvisibleTypeAnnotations(cf, input, info);
#endif

    return skipAttribute(input, info);
}

extern int
loadAttributes_class(ClassFile *cf, struct BufferIO *input, u2 *attributes_count, attr_info **attributes)
{
    u2 i;

    if (!cf)
        return -1;
    if (checkInput(input) < 0)
        return -1;
    if (ru2(attributes_count, input) < 0)
        return -1;
    *attributes = (attr_info *) malloc(*attributes_count * sizeof (attr_info));
    for (i = 0u; i < *attributes_count; i++)
        if (loadAttribute_class(cf, input, &((*attributes)[i])) < 0)
            return -1;
    return 0;
}

static int
freeAttribute_class(attr_info *info)
{
    logError("Free class attribute tagged %i\r\n", info->tag);
#if VER_CMP(45, 3)
    if (!freeAttribute_SourceFile(info))
        return 0;
    if (!freeAttribute_InnerClasses(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_InnerClasses(info))
        return 0;
    if (!freeAttribute_EnclosingMethod(info))
        return 0;
    if (!freeAttribute_Signature(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(info))
        return 0;
#endif
#if VER_CMP(51, 0)
    if (!freeAttribute_BootstrapMethods(info))
        return 0;
#endif
#if VER_CMP(52, 0)
    if (!freeAttribute_RuntimeVisibleTypeAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleTypeAnnotations(info))
        return 0;
#endif

    logError("Fail to free attriute %p!\r\n", info);
    return -1;
}

extern int
freeAttributes_class(u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        if (freeAttribute_class(&(attributes[i])) < 0)
            return -1;

    return 0;
}

static int
freeAttribute_field(attr_info *info)
{
#if VER_CMP(45, 3)
    if (!freeAttribute_ConstantValue(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_Signature(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(info))
        return 0;
#endif
#if VER_CMP(52, 0)
    if (!freeAttribute_RuntimeVisibleTypeAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleTypeAnnotations(info))
        return 0;
#endif

    return -1;
}

extern int
freeAttributes_field(u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        if (freeAttribute_field(&(attributes[i])) < 0)
            return -1;

    return 0;
}


extern int
loadAttributes_field(ClassFile *cf, struct BufferIO *input, u2 *attributes_count, attr_info **attributes)
{
    u2 i;

    if (!cf)
        return -1;
    if (checkInput(input) < 0)
        return -1;
    if (ru2(attributes_count, input) < 0)
        return -1;
    *attributes = (attr_info *) malloc(*attributes_count * sizeof (attr_info));
    for (i = 0u; i < *attributes_count; i++)
        if (loadAttribute_field(cf, input, &((*attributes)[i])) < 0)
            return -1;
    return 0;
}

static int
freeAttribute_method(attr_info *info)
{
#if VER_CMP(45, 3)
    if (!freeAttribute_Code(info))
        return 0;
    if (!freeAttribute_Exceptions(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_RuntimeVisibleParameterAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleParameterAnnotations(info))
        return 0;
    if (!freeAttribute_AnnotationDefault(info))
        return 0;
    if (!freeAttribute_Signature(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(info))
        return 0;
#endif
#if VER_CMP(52, 0)
    if (!freeAttribute_MethodParameters(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleTypeAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleTypeAnnotations(info))
        return 0;
#endif

    return -1;
}

extern int
freeAttributes_method(u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        if (freeAttribute_method(&(attributes[i])) < 0)
            return -1;

    return 0;
}

extern int
loadAttributes_method(ClassFile *cf, struct BufferIO *input, u2 *attributes_count, attr_info **attributes)
{
    u2 i;

    if (!cf)
        return -1;
    if (checkInput(input) < 0)
        return -1;
    if (ru2(attributes_count, input) < 0)
        return -1;
    *attributes = (attr_info *) malloc(*attributes_count * sizeof (attr_info));
    for (i = 0u; i < *attributes_count; i++)
        if (loadAttribute_method(cf, input, &((*attributes)[i])) < 0)
        {
            logError("Fail to load method attributes!\r\n");
            return -1;
        }

    return 0;
}

static int
freeAttribute_code(attr_info *info)
{
#if VER_CMP(45, 3)
    if (!freeAttribute_LineNumberTable(info))
        return 0;
    if (!freeAttribute_LocalVariableTable(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_LocalVariableTypeTable(info))
        return 0;
#endif
#if VER_CMP(50, 0)
    if (!freeAttribute_StackMapTable(info))
        return 0;
#endif
#if VER_CMP(52, 0)
    if (!freeAttribute_RuntimeVisibleTypeAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleTypeAnnotations(info))
        return 0;
#endif

    return -1;
}

extern int
freeAttributes_code(u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        if (freeAttribute_code(&(attributes[i])) < 0)
            return -1;

    return 0;
}

extern int
loadAttributes_code(ClassFile *cf, struct BufferIO *input, u2 *attributes_count, attr_info **attributes)
{
    u2 i;

    if (!cf)
        return -1;
    if (checkInput(input) < 0)
        return -1;
    if (ru2(attributes_count, input) < 0)
        return -1;
    *attributes = (attr_info *) malloc(*attributes_count * sizeof (attr_info));
    for (i = 0u; i < *attributes_count; i++)
        if (loadAttribute_code(cf, input, &((*attributes)[i])) < 0)
            return -1;
    return 0;
}
