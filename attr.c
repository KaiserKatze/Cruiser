#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "java.h"
#include "log.h"
#include "opcode.h"
#include "memory.h"

static int
loadAttribute(struct BufferIO *input, attr_info *info)
{
    if (ru2(&(info->attribute_name_index), input) < 0)
        goto error;
    if (ru4(&(info->attribute_length), input) < 0)
        goto error;
    return 0;
error:
    logError("Vital error: fail to initialize attribute!\r\n");
    return -1;
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
    CONSTANT_Class_info *cc;
    u2 len;
    u1 *str;

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
    data->code = (u1 *) allocMemory(data->code_length, sizeof (u1));
    if (!data->code) return -1;
    if (rbs(data->code, input, data->code_length) < 0)
        return -1;
    logInfo("\t\t// Hex code =\r\n");
    logInfo("\t\t");
    for (j = 0u; j < data->code_length; j++)
    {
        if (j != 0)
        {
            if (j % 20 == 0)
                logInfo("\r\n\t\t");
            else if (j % 10 == 0)
                logInfo(" ");
        }
        logInfo("%02X ", data->code[j]);
    }
    logInfo("\r\n");
    
    disassembleCode(data->code_length, data->code);
    
    if (ru2(&(data->exception_table_length), input) < 0)
        return -1;
    logInfo("\t\t// Exception table length = %i.\r\n", data->exception_table_length);
    if (data->exception_table_length > 0)
    {
        data->exception_table = (struct exception_table_entry *)
            allocMemory(data->exception_table_length,
                sizeof (struct exception_table_entry));
        if (!data->exception_table) return -1;
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
            if (data->exception_table[i].catch_type != 0)
            {
                cc = getConstant_Class(cf, data->exception_table[i].catch_type);
                if (!cc) continue;
                len = getConstant_Utf8Length(cf, cc->data->name_index);
                str = getConstant_Utf8String(cf, cc->data->name_index);
                logInfo("\t\t%.*s\t\t"
                        "%i\t"
                        "%i\t"
                        "%i\r\n",
                        len, str,
                        data->exception_table[i].start_pc,
                        data->exception_table[i].end_pc,
                        data->exception_table[i].handler_pc);
                len = 0;
                str = (char *) 0;
            }
            else
            {
                logInfo("\t\tFINALLY\t\t"
                        "%i\t"
                        "%i\t"
                        "%i\r\n",
                        data->exception_table[i].start_pc,
                        data->exception_table[i].end_pc,
                        data->exception_table[i].handler_pc);
            }
            /*
             * If the value of the catch_type item is zero,
             * this exception handler is called for all exceptions.
             * This is used to implement finally (§3.13).
             */
        }
    }
    else if (data->exception_table_length == 0)
    {
        data->exception_table = (struct exception_table_entry *) 0;
    }
    else
    {
        logError("Assertion error: Exception table length is negative!\r\n");
        return -1;
    }
    loadAttributes_code(cf, input,
            &(data->attributes_count),
            &(data->attributes));

    info->data = data;
    return 0;
}

static int
freeAttribute_Code(ClassFile * cf, attr_info *info)
{
    struct attr_Code_info *data;

    if (info->tag != TAG_ATTR_CODE)
        return -1;
    data = (struct attr_Code_info *) info->data;
    free(data->code);
    data->code = (u1 *) 0;
    free(data->exception_table);
    data->exception_table = 0;
    freeAttributes_code(cf, data->attributes_count, data->attributes);
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
    CONSTANT_Utf8_info *utf8;
    u2 i, len;
    u1 *str;

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
    // Validate Exception attribute
    if ((data->number_of_exceptions + 2) * sizeof (u2) + sizeof (u4)
            != info->attribute_length)
    {
        logError("Exception attribute is not valid!\r\n");
    }
    data->exception_index_table = (u2 *)
        malloc(sizeof (u2) * data->number_of_exceptions);
    if (!data->exception_index_table)
    {
        logError("Fail to allocate memory!\r\n");
        return -1;
    }
    logInfo("\t\tExceptions:\r\n");
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
        if (!cc->data)
        {
            logError("Assertion error: constant_pool[%i] has no data!\r\n", data->exception_index_table[i]);
            return -1;
        }
        utf8 = getConstant_Utf8(cf, cc->data->name_index);
        if (!utf8->data)
        {
            logError("Assertion error: constant_pool[%i] has no data!\r\n", cc->data->name_index);
            return -1;
        }
        len = utf8->data->length;
        str = utf8->data->bytes;
        logInfo("\t\t\t%.*s\r\n", len, str);
        len = 0;
        str = (u1 *) 0;
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

// If the constant pool of a class or interface C
// contains a CONSTANT_Class_info entry which
// represents a class or interface that
// is not a member of a package, then C 's ClassFile structure
// must have exactly one InnerClasses attribute in its attributes table.
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
            allocMemory(data->number_of_classes,
                sizeof (struct classes_entry));
    if (!data->classes) return -1;
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
        /*
         * If C is not a member of a class or an interface
         * (that is, if C is a top-level class or interface (JLS §7.6)
         * or a local class (JLS §14.3) or an anonymous class (JLS §15.9.5)),
         * the value of the outer_class_info_index item must be zero.
         * Otherwise, the value of the outer_class_info_index item must be a
         * valid index into the constant_pool table, and the entry
         * at that index must be a CONSTANT_Class_info (§4.4.1)
         * structure representing the class or interface of which C is a member.
         */
        if (data->classes[i].outer_class_info_index != 0)
        {
            cc = getConstant_Class(cf, data->classes[i].outer_class_info_index);
            if (!cc)
            {
                logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->classes[i].outer_class_info_index);
                return -1;
            }
        }
        if (ru2(&(data->classes[i].inner_name_index), input) < 0)
            return -1;
        /*
         * If C is anonymous (JLS §15.9.5), the value of
         * the inner_name_index item must be zero.
         */
        if (data->classes[i].inner_name_index != 0) // not anonymous
        {
            cu = getConstant_Utf8(cf, data->classes[i].inner_name_index);
            if (!cu)
            {
                logError("Assertion error: constant_pool[%i] is not CONSTANT_Class_info instance!\r\n", data->classes[i].inner_name_index);
                return -1;
            }
        }
        if (ru2(&(data->classes[i].inner_class_access_flags), input) < 0)
            return -1;
        if (data->classes[i].inner_class_access_flags & ~ACC_NESTED_CLASS)
        {
            logError("Assertion error: data->classes[%i] has unknown inner_class_access_flags: 0x%X!\r\n",
                    i, data->classes[i].inner_class_access_flags & ~ACC_NESTED_CLASS);
            /*
             * All bits of the inner_class_access_flags item not assigned in Table 4.8
             * are reserved for future use. They should be set to zero in generated class
             * files and should be ignored by Java Virtual Machine implementations.
             */
            //return -1;
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
    if (info->tag != TAG_ATTR_SOURCEFILE)
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
    if (info->tag != TAG_ATTR_LOCALVARIABLETYPETABLE)
        return -1;
    free(info->data);
    info->data = (void *) 0;

    return 0;
}

static int
loadElementValue(ClassFile *, struct BufferIO *, struct element_value *);
static int
freeElementValue(ClassFile *, struct element_value *);

static int
loadAnnotation(ClassFile *cf, struct BufferIO *input,
        struct annotation *anno)
{
    CONSTANT_Utf8_info *utf8;
    u2 i;
    
    if (ru2(&(anno->type_index), input) < 0)
        return -1;
    utf8 = getConstant_Utf8(cf, anno->type_index);
    if (!utf8)
        return -1;
    if (!isFieldDescriptor(utf8->data->length, utf8->data->bytes))
        return -1;
    if (ru2(&(anno->num_element_value_pairs), input) < 0)
        return -1;
    
    if (anno->num_element_value_pairs < 0)
        return -1;
    if (anno->num_element_value_pairs == 0)
    {
        anno->element_value_pairs = (struct element_value_pair *) 0;
        return 0;
    }
    anno->element_value_pairs = (struct element_value_pair *)
            allocMemory(anno->num_element_value_pairs,
            sizeof (struct element_value_pair));
    for (i = 0; i < anno->num_element_value_pairs; i++)
    {
        if (ru2(&(anno->element_value_pairs[i].element_name_index), input) < 0)
            return -1;
        utf8 = getConstant_Utf8(cf, anno->element_value_pairs[i].element_name_index);
        if (!utf8)
            return -1;
        if (!isFieldDescriptor(utf8->data->length, utf8->data->bytes))
            return -1;
        anno->element_value_pairs[i].value = (struct element_value *)
                allocMemory(1, sizeof (struct element_value));
        if (loadElementValue(cf, input, anno->element_value_pairs[i].value) < 0)
            return -1;
    }
    return 0;
}

static int
freeAnnotation(ClassFile *cf, struct annotation *anno)
{
    u2 i;
    
    for (i = 0; i < anno->num_element_value_pairs; i++)
    {
        freeElementValue(cf, anno->element_value_pairs[i].value);
        free(anno->element_value_pairs[i].value);
        anno->element_value_pairs[i].value = (struct element_value *) 0;
    }
    free(anno->element_value_pairs);
    anno->element_value_pairs = (struct element_value_pair *) 0;
    return 0;
}

static int
loadElementValue(ClassFile *cf, struct BufferIO *input,
        struct element_value *value)
{
    CONSTANT_Fieldref_info *cfi;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_Utf8_info *cui;
    u2 i;
    
    if (ru1(&(value->tag), input) < 0)
        return -1;
    switch (value->tag)
    {
        // const_value_index
        case 'B':
        case 'C':
        case 'D':
        case 'F':
        case 'I':
        case 'J':
        case 'S':
        case 'Z':
        case 's':
            if (ru2(&(value->const_value_index), input) < 0)
                return -1;
            cfi = getConstant_Fieldref(cf, value->const_value_index);
            if (!cfi) return -1;
            cni = getConstant_NameAndType(cf, cfi->data->name_and_type_index);
            if (!cni) return -1;
            cui = getConstant_Utf8(cf, cni->data->descriptor_index);
            if (!cui) return -1;
            if (value->tag == 's')
            {
                if (strcmp(cui->data->bytes, "Ljava/lang/String;"))
                    return -1;
            }
            else
            {
                if (cui->data->length != 1
                        || cui->data->bytes[0] != value->tag)
                    return -1;
            }
            break;
        // enum
        case 'e':
            if (ru2(&(value->enum_const_value.type_name_index), input) < 0)
                return -1;
            cui = getConstant_Utf8(cf, value->enum_const_value.type_name_index);
            if (!cui) return -1;
            // representing a valid field descriptor (§4.3.2)
            // that denotes the internal form of the binary
            // name (§4.2.1) of the type of the enum constant represented by this
            // element_value structure.
            if (!isFieldDescriptor(cui->data->length, cui->data->bytes)) return -1;
            if (ru2(&(value->enum_const_value.const_name_index), input) < 0)
                return -1;
            cui = getConstant_Utf8(cf, value->enum_const_value.const_name_index);
            if (!cui) return -1;
            // representing the simple name of the enum constant
            // represented by this element_value structure
            break;
        case 'c':
            if (ru2(&(value->class_info_index), input) < 0)
                return -1;
            cui = getConstant_Utf8(cf, value->class_info_index);
            if (!cui) return -1;
            // representing the return descriptor (§4.3.3)
            // of the type that is reified by the class
            // represented by this element_value structure
        case '@':
            if (loadAnnotation(cf, input, &(value->annotation_value)) < 0)
                return -1;
            // The element_value structure represents a "nested" annotation
            break;
        case '[':
            if (ru2(&(value->array_value.num_values), input) < 0)
                return -1;
            if (value->array_value.num_values == 0)
            {
                value->array_value.values = (struct element_value *) 0;
            }
            else
            {
                value->array_value.values = (struct element_value *)
                        allocMemory(value->array_value.num_values,
                            sizeof (struct element_value));
                if (!value->array_value.values)
                    return -1;
                for (i = 0; i < value->array_value.num_values; i++)
                    loadElementValue(cf, input, &(value->array_value.values[i]));
            }
            break;
        default:
            logError("Assertion error: Invalid element_value tag!\r\n");
            return -1;
    }
    return 0;
}

static int
freeElementValue(ClassFile *cf, struct element_value *value)
{
    if (value->tag == '[' && value->array_value.values)
    {
        free(value->array_value.values);
        value->array_value.values = (struct element_value *) 0;
    }
    return 0;
}

static int
loadAttribute_RuntimeVisibleAnnotations(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_RuntimeVisibleAnnotations_info *data;
    u2 num_annotations, i;
    
    info->tag = TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS;
    if (ru2(&num_annotations, input) < 0)
        return -1;
    data = (struct attr_RuntimeVisibleAnnotations_info *)
            allocMemory(1, sizeof (struct attr_RuntimeVisibleAnnotations_info)
                + sizeof (struct annotation) * num_annotations);
    if (!data)
        return -1;
    data->num_annotations = num_annotations;
    for (i = 0; i < num_annotations; i++)
        if (loadAnnotation(cf, input, &(data->annotations[i])) < 0)
            return -1;
    
    info->data = data;
    return 0;
}

static int
freeAttribute_RuntimeVisibleAnnotations(ClassFile *cf, attr_info *info)
{
    struct attr_RuntimeVisibleAnnotations_info *data;
    u2 i;
    
    if (info->tag != TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS)
        return -1;
    data = info->data;
    for (i = 0; i < data->num_annotations; i++)
        freeAnnotation(cf, &(data->annotations[i]));
    
    free(info->data);
    info->data = (struct attr_RuntimeVisibleAnnotations_info *) 0;
    return 0;
}

static int
loadAttribute_RuntimeInvisibleAnnotations(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_RuntimeInvisibleAnnotations_info *data;
    u2 num_annotations, i;
    struct annotation *anno;
    
    info->tag = TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS;
    if (ru2(&num_annotations, input) < 0)
        return -1;
    data = (struct attr_RuntimeInvisibleAnnotations_info *)
            allocMemory(1, sizeof (struct attr_RuntimeInvisibleAnnotations_info)
                + sizeof (struct annotation) * num_annotations);
    if (!data)
        return -1;
    data->num_annotations = num_annotations;
    for (i = 0; i < num_annotations; i++)
        if (loadAnnotation(cf, input, &(data->annotations[i])) < 0)
            return -1;
    
    info->data = data;
    return 0;
}

static int
freeAttribute_RuntimeInvisibleAnnotations(ClassFile *cf, attr_info *info)
{
    struct attr_RuntimeInvisibleAnnotations_info *data;
    u2 i;
    
    if (info->tag != TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS)
        return -1;
    data = info->data;
    for (i = 0; i < data->num_annotations; i++)
        freeAnnotation(cf, &(data->annotations[i]));
    
    free(info->data);
    info->data = (struct attr_RuntimeInvisibleAnnotations_info *) 0;
    return 0;
}

static int
loadAttribute_RuntimeVisibleParameterAnnotations(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_RuntimeVisibleParameterAnnotations_info *data;
    u1 num_parameters, i;
    u2 j;
    
    info->tag = TAG_ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS;
    if (ru1(&num_parameters, input) < 0)
        return -1;
    data = (struct attr_RuntimeVisibleParameterAnnotations_info *)
            allocMemory(1, sizeof (struct attr_RuntimeVisibleParameterAnnotations_info)
            + num_parameters * sizeof (struct parameter_annotation));
    if (!data)
        return -1;
    data->num_parameters = num_parameters;
    for (i = 0; i < num_parameters; i++)
    {
        if (ru2(&(data->parameter_annotations[i].num_annotations), input) < 0)
            return -1;
        data->parameter_annotations[i].annotations = (struct annotation *)
                allocMemory(data->parameter_annotations[i].num_annotations,
                    sizeof (struct annotation));
        if (!data->parameter_annotations[i].annotations)
            return -1;
        for (j = 0; j < data->parameter_annotations[i].num_annotations; j++)
            if (loadAnnotation(cf, input,
                    &(data->parameter_annotations[i].annotations[j])) < 0)
                return -1;
    }
    
    info->data = data;
    return 0;
}

static int
freeAttribute_RuntimeVisibleParameterAnnotations(ClassFile *cf, attr_info *info)
{
    struct attr_RuntimeVisibleParameterAnnotations_info *data;
    u1 i;
    u2 j;
    
    if (info->tag != TAG_ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS)
        return -1;
    data = (struct attr_RuntimeVisibleParameterAnnotations_info *) info->data;
    for (i = 0; i < data->num_parameters; i++)
    {
        for (j = 0; j < data->parameter_annotations[i].num_annotations; j++)
            freeAnnotation(cf, &(data->parameter_annotations[i].annotations[j]));
        free(data->parameter_annotations[i].annotations);
    }
    free(info->data);
    info->data = (struct attr_RuntimeVisibleParameterAnnotations_info *) 0;
    
    return 0;
}

static int
loadAttribute_RuntimeInvisibleParameterAnnotations(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_RuntimeInvisibleParameterAnnotations_info *data;
    u1 num_parameters, i;
    u2 j;
    
    info->tag = TAG_ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS;
    if (ru1(&num_parameters, input) < 0)
        return -1;
    data = (struct attr_RuntimeInvisibleParameterAnnotations_info *)
            allocMemory(1, sizeof (struct attr_RuntimeInvisibleParameterAnnotations_info)
            + num_parameters * sizeof (struct parameter_annotation));
    if (!data)
        return -1;
    data->num_parameters = num_parameters;
    for (i = 0; i < num_parameters; i++)
    {
        if (ru2(&(data->parameter_annotations[i].num_annotations), input) < 0)
            return -1;
        data->parameter_annotations[i].annotations = (struct annotation *)
                allocMemory(data->parameter_annotations[i].num_annotations,
                    sizeof (struct annotation));
        if (!data->parameter_annotations[i].annotations)
            return -1;
        for (j = 0; j < data->parameter_annotations[i].num_annotations; j++)
            if (loadAnnotation(cf, input,
                    &(data->parameter_annotations[i].annotations[j])) < 0)
                return -1;
    }
    
    info->data = data;
    return 0;
}

static int
freeAttribute_RuntimeInvisibleParameterAnnotations(ClassFile *cf, attr_info *info)
{
    struct attr_RuntimeInvisibleParameterAnnotations_info *data;
    u1 i;
    u2 j;
    
    if (info->tag != TAG_ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS)
        return -1;
    data = (struct attr_RuntimeInvisibleParameterAnnotations_info *) info->data;
    for (i = 0; i < data->num_parameters; i++)
    {
        for (j = 0; j < data->parameter_annotations[i].num_annotations; j++)
            freeAnnotation(cf, &(data->parameter_annotations[i].annotations[j]));
        free(data->parameter_annotations[i].annotations);
    }
    free(info->data);
    info->data = (struct attr_RuntimeInvisibleParameterAnnotations_info *) 0;
    
    return 0;
}

static int
loadAttribute_AnnotationDefault(ClassFile *cf, struct BufferIO *input, attr_info *info)
{
    struct attr_AnnotationDefault_info *data;
    
    info->tag = TAG_ATTR_ANNOTATIONDEFAULT;
    data = (struct attr_AnnotationDefault_info *)
            allocMemory(1, sizeof (struct attr_AnnotationDefault_info));
    if (!data)
        return -1;
    if (loadElementValue(cf, input, &(data->default_value)) < 0)
        return -1;
    info->data = data;
    
    return 0;
}

static int
freeAttribute_AnnotationDefault(ClassFile *cf, attr_info *info)
{
    struct attr_AnnotationDefault_info *data;
    
    if (info->tag != TAG_ATTR_ANNOTATIONDEFAULT)
        return -1;
    data = (struct attr_AnnotationDefault_info *) info->data;
    freeElementValue(cf, &(data->default_value));
    free(info->data);
    info->data = (struct attr_AnnotationDefault_info *) 0;
    
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
    logError("Fail to load incompatible attribute: %s.\r\n", attribute_name);
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
    logError("Fail to load incompatible attribute: %s.\r\n", attribute_name);
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
    logError("Fail to load incompatible attribute: %s.\r\n", attribute_name);
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
    logError("Fail to load incompatible attribute: %s.\r\n", attribute_name);
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
        loadAttribute_class(cf, input, &((*attributes)[i]));
    return 0;
}

static int
freeAttribute_class(ClassFile * cf, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;
    
    //logError("Free class attribute tagged %i\r\n", info->tag);
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
    if (!freeAttribute_RuntimeVisibleAnnotations(cf, info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(cf, info))
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
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;
    logError("Fail to free incompatible attribute: %s.\r\n", attribute_name);
    return -1;
}

extern int
freeAttributes_class(ClassFile * cf, u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        freeAttribute_class(cf, &(attributes[i]));

    return 0;
}

static int
freeAttribute_field(ClassFile * cf, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;
    
#if VER_CMP(45, 3)
    if (!freeAttribute_ConstantValue(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_Signature(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleAnnotations(cf, info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(cf, info))
        return 0;
#endif
#if VER_CMP(52, 0)
    if (!freeAttribute_RuntimeVisibleTypeAnnotations(info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleTypeAnnotations(info))
        return 0;
#endif
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;
    logError("Fail to free incompatible attribute: %s.\r\n", attribute_name);
    return -1;
}

extern int
freeAttributes_field(ClassFile * cf, u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        freeAttribute_field(cf, &(attributes[i]));

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
    //*attributes = (attr_info *) malloc(*attributes_count * sizeof (attr_info));
    *attributes = (attr_info *) allocMemory(*attributes_count, sizeof (attr_info));
    if (!*attributes) return -1;
    for (i = 0u; i < *attributes_count; i++)
        loadAttribute_field(cf, input, &((*attributes)[i]));
    return 0;
}

static int
freeAttribute_method(ClassFile * cf, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;
    
#if VER_CMP(45, 3)
    if (!freeAttribute_Code(cf, info))
        return 0;
    if (!freeAttribute_Exceptions(info))
        return 0;
#endif
#if VER_CMP(49, 0)
    if (!freeAttribute_RuntimeVisibleParameterAnnotations(cf, info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleParameterAnnotations(cf, info))
        return 0;
    if (!freeAttribute_AnnotationDefault(cf, info))
        return 0;
    if (!freeAttribute_Signature(info))
        return 0;
    if (!freeAttribute_RuntimeVisibleAnnotations(cf, info))
        return 0;
    if (!freeAttribute_RuntimeInvisibleAnnotations(cf, info))
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
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;
    logError("Fail to free incompatible attribute: %s.\r\n", attribute_name);
    return -1;
}

extern int
freeAttributes_method(ClassFile * cf, u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        freeAttribute_method(cf, &(attributes[i]));

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
        loadAttribute_method(cf, input, &((*attributes)[i]));

    return 0;
}

static int
freeAttribute_code(ClassFile * cf, attr_info *info)
{
    CONSTANT_Utf8_info *utf8;
    char *attribute_name;
    
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
    utf8 = getConstant_Utf8(cf, info->attribute_name_index);
    if (!utf8) return -1;
    if (utf8->tag != CONSTANT_Utf8) return -1;
    attribute_name = utf8->data->bytes;
    logError("Fail to free incompatible attribute: %s.\r\n", attribute_name);
    return -1;
}

extern int
freeAttributes_code(ClassFile * cf, u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (!attributes)
        return 0;

    for (i = 0; i < attributes_count; i++)
        freeAttribute_code(cf, &(attributes[i]));

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
        loadAttribute_code(cf, input, &((*attributes)[i]));
    return 0;
}
