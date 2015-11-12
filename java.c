#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "java.h"
#include "log.h"
#include "memory.h"

static int get_cp_size(u1);
static const char *get_cp_name(u1);
static int validateConstantPool(ClassFile *);

static char *convertAccessFlags_field(u2, u2);
static char *convertAccessFlags_method(u2, u2);

extern int
parseClassfile(struct BufferIO * input, ClassFile *cf)
{
    u2 i, j;
    //int cap;
    cp_info *info;
    CONSTANT_Class_info *cci;
    CONSTANT_Fieldref_info *cfi;
    CONSTANT_Integer_info *cii;
    CONSTANT_Long_info *cli;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_String_info *csi;
    CONSTANT_Utf8_info *cui;
    CONSTANT_MethodHandle_info *cmhi;
    CONSTANT_MethodType_info *cmti;
    CONSTANT_InvokeDynamic_info *cidi;
    char *buf;

    if (!input)
    {
        logError("Parameter 'input' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    if (!cf)
    {
        logError("Parameter 'cf' in function %s is NULL!\r\n", __func__);
        return -1;
    }
    
    // function 'allocMemory' includes bzero feature
    //bzero(cf, sizeof (ClassFile));
    //bzero(input->buffer, input->bufsize);
    //input->bufsrc = input->bufdst = 0;

    // validate file structure
    if (ru4(&(cf->magic), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (cf->magic != 0XCAFEBABE)
    {
        logError("File structure invalid, fail to decompile! [0x%X]\r\n", cf->magic);
        return -1;
    }
    logInfo("File structure is valid[0x%X], proceeding...\r\n", cf->magic);

    // retrieve version
    if (ru2(&(cf->minor_version), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (ru2(&(cf->major_version), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("Class version: %i.%i\r\n",
            cf->major_version, cf->minor_version);
#ifndef DEBUG
    // check compatibility
    if (compareVersion(cf->major_version, cf->minor_version) > 0)
    {
        logError("Class file version is higher than this implementation!\r\n");
        goto close;
    }
#endif

    // retrieve constant pool size
    if (ru2(&(cf->constant_pool_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("Constant pool size: %i\r\n", cf->constant_pool_count);
    if (cf->constant_pool_count > 0)
    {
        /*
        // calculate constant pool capacity
        cap = cf->constant_pool_count * sizeof (cp_info);
        // allocate memory for constant pool
        cf->constant_pool = (cp_info *) malloc(cap);
        if (!cf->constant_pool)
        {
            logError("Fail to allocate memory.\r\n");
            return -1;
        }
        logInfo("Constant pool allocated.\r\n");
        bzero(cf->constant_pool, cap);
        */
        cf->constant_pool = (cp_info *) allocMemory(cf->constant_pool_count, sizeof (cp_info));
        if (!cf->constant_pool) return -1;

        logInfo("Parsing constant pool...\r\n");
        // jvms7 says "The constant_pool table is indexed
        // from 1 to constant_pool_count - 1
        for (i = 1u; i < cf->constant_pool_count; i++)
        { // LOOP
            info = &(cf->constant_pool[i]);
            if (ru1(&(info->tag), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            logInfo("\t#%-5i = %-18s ",
                    i, get_cp_name(info->tag));
            /*
            cap = get_cp_size(info->tag);
            info->data = malloc(cap);
            if (!info->data)
            {
                logError("Fail to allocate memory!\r\n");
                return -1;
            }
            bzero(info->data, cap);
            */
            switch (info->tag)
            {
                case CONSTANT_Class:
                    info->data = allocMemory(1, sizeof *(cci->data));
                    if (!info->data) return -1;
                    cci = (CONSTANT_Class_info *) info;
                    if (ru2(&(cci->data->name_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i\r\n", cci->data->name_index);

                    cci = (CONSTANT_Class_info *) 0;
                    break;
                case CONSTANT_Fieldref:
                case CONSTANT_Methodref:
                case CONSTANT_InterfaceMethodref:
                    info->data = allocMemory(1, sizeof *(cfi->data));
                    if (!info->data) return -1;
                    cfi = (CONSTANT_Fieldref_info *) info;
                    if (ru2(&(cfi->data->class_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    if (ru2(&(cfi->data->name_and_type_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i.#%i\r\n", cfi->data->class_index, cfi->data->name_and_type_index);

                    cfi = (CONSTANT_Fieldref_info *) 0;
                    cci = (CONSTANT_Class_info *) 0;
                    buf = (char *) 0;
                    break;
                case CONSTANT_Integer:
                case CONSTANT_Float:
                    info->data = allocMemory(1, sizeof *(cii->data));
                    if (!info->data) return -1;
                    cii = (CONSTANT_Integer_info *) info;
                    if (ru4(&(cii->data->bytes), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    if (info->tag == CONSTANT_Integer)
                        logInfo("%i\r\n", cii->data->bytes);
                    else
                        logInfo("%fF\r\n", cii->data->float_value);

                    cii = (CONSTANT_Integer_info *) 0;
                    break;
                case CONSTANT_Long:
                case CONSTANT_Double:
                    info->data = allocMemory(1, sizeof *(cli->data));
                    if (!info->data) return -1;
                    cli = (CONSTANT_Long_info *) info;
                    if (ru4(&(cli->data->high_bytes), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    if (ru4(&(cli->data->low_bytes), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    // all 8-byte constants take up two entries in the constant_pool table of the class file
                    ++i;

                    if (info->tag == CONSTANT_Long)
                        logInfo("%lliL\r\n", cli->data->long_value);
                    else
                        logInfo("%dD\r\n", cli->data->double_value);

                    cli = (CONSTANT_Long_info *) 0;
                    break;
                case CONSTANT_NameAndType:
                    info->data = allocMemory(1, sizeof *(cni->data));
                    if (!info->data) return -1;
                    cni = (CONSTANT_NameAndType_info *) info;
                    if (ru2(&(cni->data->name_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    if (ru2(&(cni->data->descriptor_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i.#%i\r\n", cni->data->name_index, cni->data->descriptor_index);

                    cni = (CONSTANT_NameAndType_info *) 0;
                    break;
                case CONSTANT_String:
                    info->data = allocMemory(1, sizeof *(csi->data));
                    if (!info->data) return -1;
                    csi = (CONSTANT_String_info *) info;
                    if (ru2(&(csi->data->string_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i\r\n", csi->data->string_index);

                    csi = (CONSTANT_String_info *) 0;
                    break;
                case CONSTANT_Utf8:
                    info->data = allocMemory(1, sizeof *(cui->data));
                    if (!info->data) return -1;
                    cui = (CONSTANT_Utf8_info *) info;
                    if (ru2(&(cui->data->length), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    /*
                    cap = cui->data->length;
                    cui->data->bytes = (u1 *) malloc(cap);
                    if (!cui->data->bytes)
                    {
                        logError("Fail to allocate memory!\r\n");
                        return -1;
                    }
                    bzero(cui->data->bytes, cap);
                    */
                    cui->data->bytes = (u1 *) allocMemory(cui->data->length + 1, sizeof (u1));
                    if (!cui->data->bytes) return -1;

                    if (rbs(cui->data->bytes, input, cui->data->length) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    cui->data->bytes[cui->data->length] = '\0';

                    if (!cui->data->bytes)
                    {
                        logError("Runtime error!\r\n");
                        return -1;
                    }
                    logInfo("%.*s\r\n", cui->data->length, (char *) cui->data->bytes);

                    cui = (CONSTANT_Utf8_info *) 0;
                    //cap = 0;
                    break;
                case CONSTANT_MethodHandle:
                    info->data = allocMemory(1, sizeof *(cmhi->data));
                    if (!info->data) return -1;
                    cmhi = (CONSTANT_MethodHandle_info *) info;
                    if (ru1(&(cmhi->data->reference_kind), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    if (ru2(&(cmhi->data->reference_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("%i #%i\r\n", cmhi->data->reference_kind, cmhi->data->reference_index);

                    cmhi = (CONSTANT_MethodHandle_info *) 0;
                    break;
                case CONSTANT_MethodType:
                    info->data = allocMemory(1, sizeof *(cmti->data));
                    if (!info->data) return -1;
                    cmti = (CONSTANT_MethodType_info *) info;
                    if (ru2(&(cmti->data->descriptor_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i\r\n", cmti->data->descriptor_index);

                    cmti = (CONSTANT_MethodType_info *) 0;
                    break;
                case CONSTANT_InvokeDynamic:
                    info->data = allocMemory(1, sizeof *(cidi->data));
                    if (!info->data) return -1;
                    cidi = (CONSTANT_InvokeDynamic_info *) info;
                    if (ru2(&(cidi->data->bootstrap_method_attr_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }
                    if (ru2(&(cidi->data->name_and_type_index), input) < 0)
                    {
                        logError("IO exception in function %s!\r\n", __func__);
                        return -1;
                    }

                    logInfo("#%i.#%i\r\n",
                            cidi->data->bootstrap_method_attr_index,
                            cidi->data->name_and_type_index);

                    cidi = (CONSTANT_InvokeDynamic_info *) 0;
                    break;
                default:
                    logError("Unknown TAG:%X\r\n", info->tag);
                    return -1;
            }
        } // LOOP
    } // constant pool parsed

    // constant pool validation
    if (validateConstantPool(cf) < 0)
        return -1;

    if (ru2(&(cf->access_flags), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (ru2(&(cf->this_class), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (ru2(&(cf->super_class), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("\r\nAccess flags     : 0x%X\r\n"
            "Class this_class : %i\r\n"
            "Class super_class: %i\r\n",
            cf->access_flags,
            cf->this_class,
            cf->super_class);

    if (ru2(&(cf->interfaces_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("\r\nInterface count: %i\r\n", cf->interfaces_count);
    if (cf->interfaces_count > 0)
    {
        logInfo("Parsing interfaces...\r\n");
        /*
        cap = sizeof (u2) * cf->interfaces_count;
        cf->interfaces = (u2 *) malloc(cap);
        if (!cf->interfaces)
        {
            logError("Fail to allocate memory!\r\n");
            return -1;
        }
        bzero(cf->interfaces, cap);
        */
        cf->interfaces = (u2 *) allocMemory(cf->interfaces_count, sizeof (u2));
        if (!cf->interfaces) return -1;
        for (i = 0u; i < cf->interfaces_count; i++)
        {
            if (ru2(&(cf->interfaces[i]), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            cci = (CONSTANT_Class_info *) &(cf->constant_pool[cf->interfaces[i]]);
            if (!cci || cci->tag != CONSTANT_Class)
            {
                logError("Invalid constant pool index!\r\n");
                return -1;
            }
            cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cci->data->name_index]);
            if (!cui || cui->tag != CONSTANT_Utf8)
            {
                logError("Invalid constant pool index!\r\n");
                return -1;
            }
            logInfo("Interface[%i] #%i\t// %.*s\r\n",
                    i, cf->interfaces[i],
                    cui->data->length, cui->data->bytes);
        }
    }

    if (ru2(&(cf->fields_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("\r\nField count: %i\r\n", cf->fields_count);
    if (cf->fields_count > 0)
    {
        logInfo("Parsing fields...\r\n");
        /*
        cap = sizeof (field_info) * cf->fields_count;
        cf->fields = (field_info *) malloc(cap);
        if (!cf->fields)
        {
            logError("Fail to allocate memory!\r\n");
            return -1;
        }
        bzero(cf->fields, cap);
        */
        cf->fields = (field_info *) allocMemory(cf->fields_count, sizeof (field_info));
        if (!cf->fields) return -1;
        for (i = 0u; i < cf->fields_count; i++)
        {
            if (ru2(&(cf->fields[i].access_flags), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            if (ru2(&(cf->fields[i].name_index), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            if (ru2(&(cf->fields[i].descriptor_index), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            buf = convertAccessFlags_field(i, cf->fields[i].access_flags);
            logInfo("Field[%i]\r\n"
                    "\tAccess flag     : 0x%X\t// %s\r\n"
                    "\tName index      : #%i\t// %.*s\r\n"
                    "\tDescriptor index: #%i\t// %.*s\r\n",
                    i,
                    cf->fields[i].access_flags, buf ? buf : "",
                    cf->fields[i].name_index,
                    //getConstant_Utf8(cf, cf->fields[i].name_index)->data->length,
                    getConstant_Utf8Length(cf, cf->fields[i].name_index),
                    getConstant_Utf8String(cf, cf->fields[i].name_index),
                    cf->fields[i].descriptor_index,
                    //getConstant_Utf8(cf, cf->fields[i].descriptor_index)->data->length,
                    getConstant_Utf8Length(cf, cf->fields[i].descriptor_index),
                    getConstant_Utf8String(cf, cf->fields[i].descriptor_index));
            free(buf);
            buf = (char *) 0;
            loadAttributes_field(cf, input, &(cf->fields[i].attributes_count), &(cf->fields[i].attributes));
            logInfo("\tField Attribute count: %i\r\n",
                    cf->fields[i].attributes_count);
            // only list name and length of field attributes
            for (j = 0; j < cf->fields[i].attributes_count; j++)
            {
                logInfo("\tField Attribute [%i]\r\n"
                        "\t\tName index:\t#%i\t// %.*s\r\n"
                        "\t\tLength    :\t%i\r\n",
                        j, cf->fields[i].attributes[j].attribute_name_index,
                        getConstant_Utf8Length(cf,
                            cf->fields[i].attributes[j].attribute_name_index),
                        getConstant_Utf8String(cf,
                            cf->fields[i].attributes[j].attribute_name_index),
                        cf->fields[i].attributes[j].attribute_length);
            }
        } // fields listing end
    } // if (fields_count > 0)

    // retrieve method count
    if (ru2(&(cf->methods_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    logInfo("\r\nMethod count: %i\r\n", cf->methods_count);
    // load methods if possible
    if (cf->methods_count > 0)
    {
        logInfo("Parsing methods...\r\n");
        /*
        cap = sizeof (method_info) * cf->methods_count;
        cf->methods = (method_info *) malloc(cap);
        if (!cf->methods)
        {
            logError("Fail to allocate memory!\r\n");
            return -1;
        }
        bzero(cf->methods, cap);
        */
        cf->methods = (method_info *) allocMemory(cf->methods_count, sizeof (method_info));
        if (!cf->methods) return -1;
        for (i = 0u; i < cf->methods_count; i++)
        {
            if (ru2(&(cf->methods[i].access_flags), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            if (ru2(&(cf->methods[i].name_index), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            if (ru2(&(cf->methods[i].descriptor_index), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
            buf = convertAccessFlags_method(i, cf->methods[i].access_flags);
            logInfo("Method[%i]\r\n"
                    "\tAccess flag     : 0x%X\t// %s\r\n"
                    "\tName index      : #%i\t// %.*s\r\n"
                    "\tDescriptor index: #%i\t// %.*s\r\n", i,
                    cf->methods[i].access_flags,
                    buf ? buf : "",
                    cf->methods[i].name_index,
                    //getConstant_Utf8(cf, cf->methods[i].name_index)->data->length,
                    getConstant_Utf8Length(cf, cf->methods[i].name_index),
                    getConstant_Utf8String(cf, cf->methods[i].name_index),
                    cf->methods[i].descriptor_index,
                    //getConstant_Utf8(cf, cf->methods[i].descriptor_index)->data->length,
                    getConstant_Utf8Length(cf, cf->methods[i].descriptor_index),
                    getConstant_Utf8String(cf, cf->methods[i].descriptor_index));
            free(buf);
            buf = (char *) 0;
            loadAttributes_method(cf, input, &(cf->methods[i].attributes_count), &(cf->methods[i].attributes));
            logInfo("\tAttribute count : %i\r\n",
                    cf->methods[i].attributes_count);
            for (j = 0; j < cf->methods[i].attributes_count; j++)
            {
                logInfo("\tAttribute[%i]\r\n"
                        "\t\tName index:\t#%i\t// %.*s\r\n"
                        "\t\tLength    :\t%i\r\n",
                        j,
                        cf->methods[i].attributes[j].attribute_name_index,
                        //getConstant_Utf8(cf, cf->methods[i].attributes[j].attribute_name_index)->data->length,
                        getConstant_Utf8Length(cf,
                            cf->methods[i].attributes[j].attribute_name_index),
                        getConstant_Utf8String(cf,
                            cf->methods[i].attributes[j].attribute_name_index),
                        cf->methods[i].attributes[j].attribute_length);
            } // list method attributes
        }
    }

    loadAttributes_class(cf, input, &(cf->attributes_count), &(cf->attributes));
    logInfo("Class Attribute count: %i\r\n", cf->attributes_count);
    for (i = 0; i < cf->attributes_count; i++)
    {
        cui = getConstant_Utf8(cf, cf->attributes[i].attribute_name_index);
        buf = cui->data->bytes;
        logInfo("Class Attribute      : [%i]\r\n"
                "\tName index:\t#%i\t// %.*s\r\n"
                "\tLength    :\t%i\r\n",
                i,
                cf->attributes[i].attribute_name_index,
                cui->data->length,
                buf,
                cf->attributes[i].attribute_length);
        buf = (char *) 0;
    }

    return 0;
}

extern int
freeClassfile(ClassFile *cf)
{
    u2 i;
    cp_info *cp;
    CONSTANT_Utf8_info *cu;

    logInfo("Releasing ClassFile memory...\r\n");
    if (cf->interfaces)
    {
        free(cf->interfaces);
        cf->interfaces = (u2 *) 0;
    }
    if (cf->fields)
    {
        freeAttributes_field(cf, cf->fields->attributes_count, cf->fields->attributes);
        free(cf->fields);
        cf->fields = (field_info *) 0;
    }
    if (cf->methods)
    {
        freeAttributes_method(cf, cf->methods->attributes_count, cf->methods->attributes);
        free(cf->methods);
        cf->methods = (method_info *) 0;
    }
    freeAttributes_class(cf, cf->attributes_count, cf->attributes);
    // free constant pool at last
    if (cf->constant_pool)
    {
        for (i = 1u; i < cf->constant_pool_count; i++)
        {
            cp = &(cf->constant_pool[i]);
            if (cp->data)
            {
                if (cp->tag == CONSTANT_Utf8)
                {
                    cu = (CONSTANT_Utf8_info *) cp;
                    if (cu->data->bytes)
                    {
                        free(cu->data->bytes);
                        cu->data->bytes = (u1 *) 0;
                    }
                }
                free(cp->data);
                cp->data = (void *) 0;
            }
        }
        free(cf->constant_pool);
        cf->constant_pool = (cp_info *) 0;
    }

    return 0;
}

#if 0

static int
getTypeString(int len, char *ft, char *buffer)
{
    char *res, *name;
    int i, j, k;

    res = buffer;
    if (len <= 0)
        return 0;
    i = 0;
    if (ft[0] == '[')
        for (; i < len; i++)
            if (ft[i] != '[')
                break;
    if (ft[i] == 'L')
    {
        j = len - i - 1;
        name = ft + i + 1;
        memcpy(res, name, j);
        for (k = 0; k < j; k++)
            if (res[k] == '/')
                res[k] = '.';
    }
    else
    {
        switch (ft[i])
        {
            case 'B':
                j = 4;
                name = "byte";
                break;
            case 'C':
                j = 4;
                name = "char";
                break;
            case 'D':
                j = 6;
                name = "double";
                break;
            case 'F':
                j = 5;
                name = "float";
                break;
            case 'I':
                j = 3;
                name = "int";
                break;
            case 'J':
                j = 4;
                name = "long";
                break;
            case 'S':
                j = 5;
                name = "short";
                break;
            case 'Z':
                j = 7;
                name = "boolean";
                break;
            default:
                return -1;
        }
        memcpy(res, name, j);
    }
    for (k = 0; k < i; k++)
        memcpy(res + j + 2 * k, "[]", 2);
    j += i * 2;
    res[j] = 0;
    return j;
}

static char *
parse_method_descriptor(int desc_len, char * desc)
{
    char *buffer;
    int i, j, isParam, isObj;
    int cap, len;
    char c;

    // validate first character
    if (desc[0] != '(')
        return (char *) 0;
    cap = 1024;
    buffer = (char *) malloc(cap);
    if (!buffer)
        return (char *) 0;
    len = 0;
    j = 0;
    isParam = 1;
    isObj = 0;
    // get ')' index and caculate parameter list length
    for (i = 1; i < desc_len; i++)
    {
        c = desc[i];
        switch (c)
        {
            case ')':
                isParam = 0;
                break;
            case '[':
                if (!j) j = i;
                break;
            case 'L':
                if (!j) j = i;
                isObj = 1;
                break;
            case ';':
                getTypeString(i - j, desc + j, buffer + len);
                isObj = 0;
                j = 0;
                break;
            default:
                if (isObj) continue;
                getTypeString(i - j, desc + j, buffer + len);
                j = 0;
                break;
        }
    }
    // TODO bug-fix etc

    return buffer;
}
#endif

/*
 * Buggy function, deprecated
static int
get_cp_size(u1 tag)
{
    switch (tag)
    {
        case CONSTANT_Class:
            return sizeof (CONSTANT_Class_info);
        case CONSTANT_Fieldref:
            return sizeof (CONSTANT_Fieldref_info);
        case CONSTANT_Methodref:
            return sizeof (CONSTANT_Methodref_info);
        case CONSTANT_InterfaceMethodref:
            return sizeof (CONSTANT_InterfaceMethodref_info);
        case CONSTANT_String:
            return sizeof (CONSTANT_String_info);
        case CONSTANT_Integer:
            return sizeof (CONSTANT_Integer_info);
        case CONSTANT_Float:
            return sizeof (CONSTANT_Float_info);
        case CONSTANT_Long:
            return sizeof (CONSTANT_Long_info);
        case CONSTANT_Double:
            return sizeof (CONSTANT_Double_info);
        case CONSTANT_NameAndType:
            return sizeof (CONSTANT_NameAndType_info);
        case CONSTANT_Utf8:
            return sizeof (CONSTANT_Utf8_info);
        case CONSTANT_MethodHandle:
            return sizeof (CONSTANT_MethodHandle_info);
        case CONSTANT_MethodType:
            return sizeof (CONSTANT_MethodType_info);
        case CONSTANT_InvokeDynamic:
            return sizeof (CONSTANT_InvokeDynamic_info);
        default:
            return 0;
    }
}
*/

static const char *
get_cp_name(u1 tag)
{
    switch (tag)
    {
        case CONSTANT_Class:
            return "Class\0";
        case CONSTANT_Fieldref:
            return "Fieldref\0";
        case CONSTANT_Methodref:
            return "Methodref\0";
        case CONSTANT_InterfaceMethodref:
            return "InterfaceMethodref\0";
        case CONSTANT_String:
            return "String\0";
        case CONSTANT_Integer:
            return "Integer\0";
        case CONSTANT_Float:
            return "Float\0";
        case CONSTANT_Long:
            return "Long\0";
        case CONSTANT_Double:
            return "Double\0";
        case CONSTANT_NameAndType:
            return "NameAndType\0";
        case CONSTANT_Utf8:
            return "Utf8\0";
        case CONSTANT_MethodHandle:
            return "MethodHandle\0";
        case CONSTANT_MethodType:
            return "MethodType\0";
        case CONSTANT_InvokeDynamic:
            return "InvokeDynamic\0";
        default:
            return "Unknown\0";
    }
}

static char *
convertAccessFlags_field(u2 i, u2 af)
{
    const static int initBufSize = 0x100;
    char *buf, *out;
    int len;

    if (!af)
        return (char *) 0;
    if (af & ACC_SYNTHETIC)
        return (char *) 0;
    if (af & ACC_ENUM)
        return (char *) 0;
    if (af & ~ACC_FIELD)
    {
        logError("Unknown flags [0x%X] detected @ cf->fields[%i]:0x%X!\r\n",
                af & ~ACC_FIELD, i, af);
        return (char *) 0;
    }
    buf = (char *) malloc(initBufSize);
    if (!buf)
    {
        logError("Fail to allocate memory.\r\n");
        return (char *) 0;
    }
    bzero(buf, initBufSize);
    len = 0;

    // public, private, protected
    if (af & ACC_PUBLIC)
    {
        memcpy(buf + len, "public", 6);
        len += 6;
    }
    else if (af & ACC_PRIVATE)
    {
        memcpy(buf + len, "private", 7);
        len += 7;
    }
    else if (af & ACC_PROTECTED)
    {
        memcpy(buf + len, "protected", 9);
        len += 9;
    }

    // static
    if (af & ACC_STATIC)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "static", 6);
        len += 6;
    }

    // final, volatile
    if (af & ACC_FINAL)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "final", 5);
        len += 5;
    }
    else if (af & ACC_VOLATILE)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "volatile", 8);
        len += 8;
    }

    if (af & ACC_TRANSIENT)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "transient", 9);
        len += 9;
    }

end:
    out = realloc(buf, len + 1);
    if (!out)
    {
        free(buf);
        return buf = (char *) 0;
    }
    out[len] = 0;
    return out;
}

static char *
convertAccessFlags_method(u2 i, u2 af)
{
    const static int initBufSize = 0x100;
    char *buf, *out;
    int len;

    if (!af)
        return (char *) 0;
    if (af & ACC_BRIDGE)
        return (char *) 0;
    if (af & ACC_SYNTHETIC)
        return (char *) 0;
    if (af & ~ACC_METHOD)
    {
        logError("Unknown flags [%X] detected @ cf->methods[%i]!\r\n",
                af & ~ACC_METHOD, i);
        return (char *) 0;
    }
    buf = (char *) malloc(initBufSize);
    if (!buf)
    {
        logError("Fail to allocate memory.\r\n");
        return (char *) 0;
    }
    bzero(buf, initBufSize);
    len = 0;

    // public, private, protected
    if (af & ACC_PUBLIC)
    {
        memcpy(buf + len, "public", 6);
        len += 6;
    }
    else if (af & ACC_PRIVATE)
    {
        memcpy(buf + len, "private", 7);
        len += 7;
    }
    else if (af & ACC_PROTECTED)
    {
        memcpy(buf + len, "protected", 9);
        len += 9;
    }

    // final, native, abstract
    if (af & ACC_FINAL)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "final", 5);
        len += 5;
    }
    else if (af & ACC_NATIVE)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "native", 6);
        len += 6;
    }
    else if (af & ACC_ABSTRACT)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "abstract", 8);
        len += 8;
        goto end;
    }

    // following three tags won't appear with ACC_ABSTRACT
    // static
    if (af & ACC_STATIC)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "static", 6);
        len += 6;
    }
    // synchronized
    if (af & ACC_SYNCHRONIZED)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "synchronized", 12);
        len += 12;
    }
    // strictfp
    if (af & ACC_STRICT)
    {
        if (len > 0)
            memcpy(buf + len++, " ", 1);
        memcpy(buf + len, "strictfp", 8);
        len += 8;
    }

end:
    out = realloc(buf, len + 1);
    if (!out)
    {
        free(buf);
        return buf = (char *) 0;
    }
    out[len] = 0;
    return out;
}

extern CONSTANT_Utf8_info *
getConstant_Utf8(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *info;

    info = (CONSTANT_Utf8_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Utf8_info *) 0;
    }
    if (info->tag != CONSTANT_Utf8)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Utf8_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Utf8_info *) 0;
    }

    return info;
}

extern u2
getConstant_Utf8Length(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *cu;

    cu = getConstant_Utf8(cf, index);
    if (!cu || !cu->data)
        return -1;

    return cu->data->length;
}

extern char *
getConstant_Utf8String(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *cu;
    char *str;

    cu = getConstant_Utf8(cf, index);
    if (!cu)
        return (char *) 0;
    if (!cu->data)
        return (char *) 0;
    str = cu->data->bytes;

    return str;
}

extern char *
getConstant_ClassName(ClassFile *cf, u2 index)
{
    CONSTANT_Class_info *cc;
    
    cc = getConstant_Class(cf, index);
    if (!cc)
        return (char *) 0;
    if (!cc->data)
        return (char *) 0;

    return getConstant_Utf8String(cf, cc->data->name_index);
}

// index should be valid entry into constant pool and the entry must be a method reference
extern char *
getConstant_MethodName(ClassFile *cf, u2 index)
{
    CONSTANT_Methodref_info *cmi;
    CONSTANT_NameAndType_info *cni;
    char *name;
    
    cmi = getConstant_Methodref(cf, index);
    if (!cmi) return (char *) 0;
    cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
    if (!cni) return (char *) 0;
    name = getConstant_Utf8String(cf, cni->data->name_index);
    if (!name) return (char *) 0;
    if (strcmp(name, "<init>"))
        return name;
    return getConstant_ClassName(cf, cmi->data->class_index);
}

extern int
getConstant_MethodNParameters(ClassFile *cf, u2 index)
{
    CONSTANT_Methodref_info *cmi;
    
    cmi = getConstant_Methodref(cf, index);
    if (!cmi) return -1;
    
}

extern CONSTANT_Class_info *
getConstant_Class(ClassFile *cf, u2 index)
{
    CONSTANT_Class_info *info;

    info = (CONSTANT_Class_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Class_info *) 0;
    }
    if (info->tag != CONSTANT_Class)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Class_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Class_info *) 0;
    }

    return info;
}

extern CONSTANT_Fieldref_info *getConstant_Fieldref(ClassFile *cf, u2 index)
{
    CONSTANT_Fieldref_info *info;

    info = (CONSTANT_Fieldref_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Fieldref_info *) 0;
    }
    if (info->tag != CONSTANT_Fieldref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Fieldref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Fieldref_info *) 0;
    }

    return info;
}

extern CONSTANT_Methodref_info *getConstant_Methodref(ClassFile *cf, u2 index)
{
    CONSTANT_Methodref_info *info;

    info = (CONSTANT_Methodref_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Methodref_info *) 0;
    }
    if (info->tag != CONSTANT_Methodref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Methodref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Methodref_info *) 0;
    }

    return info;
}

extern CONSTANT_InterfaceMethodref_info *getConstant_InterfaceMethodref(ClassFile *cf, u2 index)
{
    CONSTANT_InterfaceMethodref_info *info;

    info = (CONSTANT_InterfaceMethodref_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_InterfaceMethodref_info *) 0;
    }
    if (info->tag != CONSTANT_InterfaceMethodref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_InterfaceMethodref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_InterfaceMethodref_info *) 0;
    }

    return info;
}

extern CONSTANT_String_info *getConstant_String(ClassFile *cf, u2 index)
{
    CONSTANT_String_info *info;

    info = (CONSTANT_String_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_String_info *) 0;
    }
    if (info->tag != CONSTANT_String)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_String_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_String_info *) 0;
    }

    return info;
}

extern CONSTANT_Integer_info *getConstant_Integer(ClassFile *cf, u2 index)
{
    CONSTANT_Integer_info *info;

    info = (CONSTANT_Integer_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Integer_info *) 0;
    }
    if (info->tag != CONSTANT_Integer)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Integer_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Integer_info *) 0;
    }

    return info;
}

extern CONSTANT_Float_info *getConstant_Float(ClassFile *cf, u2 index)
{
    CONSTANT_Float_info *info;

    info = (CONSTANT_Float_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Float_info *) 0;
    }
    if (info->tag != CONSTANT_Float)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Float_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Float_info *) 0;
    }

    return info;
}

extern CONSTANT_Long_info *getConstant_Long(ClassFile *cf, u2 index)
{
    CONSTANT_Long_info *info;

    info = (CONSTANT_Long_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Long_info *) 0;
    }
    if (info->tag != CONSTANT_Long)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Long_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Long_info *) 0;
    }

    return info;
}

extern CONSTANT_Double_info *getConstant_Double(ClassFile *cf, u2 index)
{
    CONSTANT_Double_info *info;

    info = (CONSTANT_Double_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Double_info *) 0;
    }
    if (info->tag != CONSTANT_Double)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Double_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Double_info *) 0;
    }

    return info;
}

extern CONSTANT_NameAndType_info *getConstant_NameAndType(ClassFile *cf, u2 index)
{
    CONSTANT_NameAndType_info *info;

    info = (CONSTANT_NameAndType_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_NameAndType_info *) 0;
    }
    if (info->tag != CONSTANT_NameAndType)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_NameAndType_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_NameAndType_info *) 0;
    }

    return info;
}

extern CONSTANT_MethodHandle_info *getConstant_MethodHandle(ClassFile *cf, u2 index)
{
    CONSTANT_MethodHandle_info *info;

    info = (CONSTANT_MethodHandle_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_MethodHandle_info *) 0;
    }
    if (info->tag != CONSTANT_MethodHandle)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_MethodHandle_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_MethodHandle_info *) 0;
    }

    return info;
}

extern CONSTANT_MethodType_info *getConstant_MethodType(ClassFile *cf, u2 index)
{
    CONSTANT_MethodType_info *info;

    info = (CONSTANT_MethodType_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_MethodType_info *) 0;
    }
    if (info->tag != CONSTANT_MethodType)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_MethodType_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_MethodType_info *) 0;
    }

    return info;
}

extern CONSTANT_InvokeDynamic_info *getConstant_InvokeDynamic(ClassFile *cf, u2 index)
{
    CONSTANT_InvokeDynamic_info *info;

    info = (CONSTANT_InvokeDynamic_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_InvokeDynamic_info *) 0;
    }
    if (info->tag != CONSTANT_InvokeDynamic)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_InvokeDynamic_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_InvokeDynamic_info *) 0;
    }

    return info;
}

extern int
compareVersion(u2 major_version, u2 minor_version)
{
    if (major_version > MAJOR_VERSION)
        return 1;
    else if (major_version == MAJOR_VERSION)
        if (minor_version > MINOR_VERSION)
            return 1;
        else if (minor_version == MINOR_VERSION)
            return 0;
    return -1;
}

static int
validateConstantPoolEntry(ClassFile *cf, u2 i, u1 *bul, u1 tag)
{
    cp_info *info;
    CONSTANT_Class_info *cci;
    CONSTANT_Fieldref_info *cfi;
    CONSTANT_String_info *csi;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_Utf8_info *cui;
    CONSTANT_MethodHandle_info *cmhi;
    CONSTANT_MethodType_info *cmti;
    CONSTANT_InvokeDynamic_info *cidi;
    u2 j;
#if VER_CMP(51, 0)
    struct attr_BootstrapMethods_info *dataBootstrapMethods;
#endif

    //logInfo("Examing constant pool[%i]...\r\n", i);
    info = &(cf->constant_pool[i]);
    if (i == 0)
    {
        logError("Pointing to null entry!\r\n");
        return -1;
    }
    if (tag != 0 && info->tag != tag)
    {
        logError("Assertion error: constant pool[%i] is not CONSTANT_%s_info!\r\n", info->tag, get_cp_name(info->tag));
        return -1;
    }
    if (bul[i])
        return 0;
    if (!info->data)
        return -1;
    switch (info->tag)
    {
        case CONSTANT_Class:
            cci = (CONSTANT_Class_info *) info;
            if (validateConstantPoolEntry(cf, cci->data->name_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            cfi = (CONSTANT_Fieldref_info *) info;
            if (validateConstantPoolEntry(cf, cfi->data->class_index, bul, CONSTANT_Class) < 0)
                return -1;
            if (validateConstantPoolEntry(cf, cfi->data->name_and_type_index, bul, CONSTANT_NameAndType) < 0)
                return -1;
            break;
        case CONSTANT_String:
            csi = (CONSTANT_String_info *) info;
            if (validateConstantPoolEntry(cf, csi->data->string_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
        case CONSTANT_NameAndType:
            cni = (CONSTANT_NameAndType_info *) info;
            if (validateConstantPoolEntry(cf, cni->data->name_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cni->data->name_index]);
            if (strncmp(cui->data->bytes, "<init>", 6)
                    && strncmp(cui->data->bytes, "<clinit>", 8))
            {
                for (j = 0; j < cui->data->length; j++)
                {
                    switch (cui->data->bytes[j])
                    {
                        case '.':case ';':case '[':case '/':case '<':case '>':
                            logError("Method name '%.*s' constains invalid character!\r\n",
                                    cui->data->length, cui->data->bytes);
                            return -1;
                    }
                }
            }
            if (validateConstantPoolEntry(cf, cni->data->descriptor_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
        case CONSTANT_Utf8:
            cui = (CONSTANT_Utf8_info *) info;
            if (!cui->data || !cui->data->bytes)
            {
                logError("Invalid CONSTANT_Utf8_info!\r\n");
                return -1;
            }
            break;
        case CONSTANT_MethodHandle:
            cmhi = (CONSTANT_MethodHandle_info *) info;
            switch (cmhi->data->reference_kind)
            {
                case 1: // REF_getField
                case 2: // REF_getStatic
                case 3: // REF_putField
                case 4: // REF_putStatic
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Fieldref) < 0)
                        return -1;
                    break;
                case 5: // REF_invokeVirtual
                case 6: // REF_invokeStatic
                case 7: // REF_inokeSpecial
                case 8: // REF_newInvokeSpecial
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Methodref) < 0)
                        return -1;
                    break;
                case 9: // REF_invokeInterface
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_InterfaceMethodref) < 0)
                        return -1;
                    break;
                default:
                    logError("Constant pool entry[%i] has invalid reference kind[%i] as CONSTANT_MethodHandle_info!\r\n",
                            i, cmhi->data->reference_kind);
                    return -1;
            }
            switch (cmhi->data->reference_kind)
            {
                case 5:case 6:case 7:case 9:
                    cfi = (CONSTANT_Fieldref_info *) &(cf->constant_pool[cmhi->data->reference_index]);
                    cni = (CONSTANT_NameAndType_info *) &(cf->constant_pool[cfi->data->name_and_type_index]);
                    cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cni->data->name_index]);
                    if (!strncmp(cui->data->bytes, "<init>", 6))
                    {
                        logError("Method name '<init>' is invalid because MethodHandle reference kind is %i!\r\n",
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    if (!strncmp(cui->data->bytes, "<clinit>", 8))
                    {
                        logError("Method name '<clinit>' is invalid because MethodHandle reference kind is %i!\r\n",
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    break;
                case 8:
                    cfi = (CONSTANT_Fieldref_info *) &(cf->constant_pool[cmhi->data->reference_index]);
                    cni = (CONSTANT_NameAndType_info *) &(cf->constant_pool[cfi->data->name_and_type_index]);
                    cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cni->data->name_index]);
                    if (strncmp(cui->data->bytes, "<init>", 6))
                    {
                        logError("Method name '%.*s' is invalid because MethodHandle reference kind is %i!\r\n",
                                cui->data->length, cui->data->bytes, cmhi->data->reference_kind);
                        return -1;
                    }
                    break;
            }
            break;
        case CONSTANT_MethodType:
            cmti = (CONSTANT_MethodType_info *) info;
            if (validateConstantPoolEntry(cf, cmti->data->descriptor_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
#if VER_CMP(51, 0)
        case CONSTANT_InvokeDynamic:
            cidi = (CONSTANT_InvokeDynamic_info *) info;
            if (validateConstantPoolEntry(cf, cidi->data->name_and_type_index, bul, CONSTANT_NameAndType) < 0)
                return -1;
            if (!cf->attributes)
            {
                logError("Class attributes ain't loaded!\r\n");
                return -1;
            }
            for (j = 0; j < cf->attributes_count; j++)
            {
                if (cf->attributes[j].tag != TAG_ATTR_BOOTSTRAPMETHODS)
                    continue;
                dataBootstrapMethods = cf->attributes[j].data;
                if (validateConstantPoolEntry(cf, dataBootstrapMethods->bootstrap_methods[cidi->data->bootstrap_method_attr_index].bootstrap_method_ref, bul, CONSTANT_MethodHandle) < 0)
                    return -1;
                // TODO validate 'bootstrap_arguments' P140
                break;
            }
            break;
#endif
        default:
            break;
    }

    bul[i] = 1;
    return 0;
}

static int
validateConstantPool(ClassFile *cf)
{
    u1 bul[cf->constant_pool_count];
    u2 i;

    bzero(bul, cf->constant_pool_count);
    for (i = 1u; i < cf->constant_pool_count; i++)
        if (validateConstantPoolEntry(cf, i, bul, 0) < 0)
        {
            logError("Constant pool is invalid[%i]!\r\n", i);
            return -1;
        }

    return 0;
}

extern int isFieldDescriptor(u2 len, u1 *str)
{
    u2 i;
    
    if (len == 1)
    {
        switch (str[0])
        {
            case 'B':case 'C':
            case 'D':case 'F':
            case 'I':case 'J':
            case 'S':case 'Z':
                return 1;
            default:
                return 0;
        }
    }
    if (str[0] == '[')
        return isFieldDescriptor(len - 1, str + 1);
    else if (str[0] == 'L' && str[len - 1] == ';')
    {
        for (i = 1; i < len - 1; i++)
            if (str[i] != '/' && str[i] != '$'
                    && (str[i] < 'a' || str[i] > 'z')
                    && (str[i] < 'A' || str[i] > 'Z')
                    && (str[i] < '0' || str[i] > '9'))
                return 0;
        return 1;
    }
    return 0;
}