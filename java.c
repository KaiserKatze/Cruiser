#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "java.h"
#include "log.h"
#include "memory.h"
#include "rt.h"
#include "vrf.h"

static int
freeClassfile(ClassFile *);

static int
loadConstantPool(struct BufferIO *, ClassFile *);

static int
loadInterfaces(struct BufferIO *, ClassFile *);

static int
loadFields(struct BufferIO *, ClassFile *);

static int
loadMethods(struct BufferIO *, ClassFile *);

static u1 *
convertAccessFlags_field(u2, u2);

static u1 *
convertAccessFlags_method(u2, u2);

static int
logClassHeader(rt_Class *);

static int
logFields(rt_Class *);

static int
logMethods(rt_Class *);

extern int
parseClassfile(struct BufferIO * input,
        struct AttributeFilter *attr_filter)
{
    u4 magic;
    ClassFile cf;
    rt_Class rtc;

    if (!input)
    {
        logError("Parameter 'input' in function %s is NULL!\r\n", __func__);
        return -1;
    }

    // validate file structure
    if (ru4(&magic, input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (checkMagic(magic) < 0)
        return -1;
    // initialize ClassFile
    memset(&cf, 0, sizeof (ClassFile));
    // retrieve version
    if (ru2(&(cf.minor_version), input) < 0)
        return -1;
    if (ru2(&(cf.major_version), input) < 0)
        return -1;
#ifndef DEBUG
    // check compatibility
    if (compareVersion(cf.major_version, cf.minor_version) > 0)
    {
        logError("Class file version is higher than this implementation!\r\n");
        return -1;
    }
#endif

    if (loadConstantPool(input, &cf) < 0)
        return -1;

    if (ru2(&(cf.access_flags), input) < 0)
        return -1;
    if (ru2(&(cf.this_class), input) < 0)
        return -1;
    if (ru2(&(cf.super_class), input) < 0)
        return -1;
    if (loadInterfaces(input, &cf) < 0)
        return -1;
    if (loadFields(input, &cf) < 0)
        return -1;
    if (loadMethods(input, &cf) < 0)
        return -1;

    if (loadAttributes_class(&cf, input,
                &(cf.attributes_count), &(cf.attributes)) < 0)
        return -1;
    // constant pool validation
    if (validateConstantPool(&cf) < 0)
        return -1;
    if (validateFields(&cf) < 0)
        return -1;
    if (validateMethods(&cf) < 0)
        return -1;

    //if (linkClass(&cf, &rtc) < 0)                   return -1;
    //if (logClassHeader(&rtc) < 0)                   return -1;
    //if (logFields(&rtc) < 0)                        return -1;
    //if (logMethods(&rtc) < 0)                       return -1;
    //if (freeClassfile(&cf) < 0)                     return -1;

    return 0;
}

static int
freeClassfile(ClassFile *cf)
{
    u2 i;
    cp_info *cp;

    logInfo("Releasing ClassFile memory...\r\n");
    if (cf->interfaces)
    {
        freeMemory(cf->interfaces);
        cf->interfaces = (u2 *) 0;
    }
    if (cf->fields)
    {
        freeAttributes_field(cf, cf->fields->attributes_count, cf->fields->attributes);
        freeMemory(cf->fields);
        cf->fields = (field_info *) 0;
    }
    if (cf->methods)
    {
        freeAttributes_method(cf, cf->methods->attributes_count, cf->methods->attributes);
        freeMemory(cf->methods);
        cf->methods = (method_info *) 0;
    }
    freeAttributes_class(cf, cf->attributes_count, cf->attributes);
    // free constant pool at last
    if (cf->constant_pool)
    {
        for (i = 1u; i < cf->constant_pool_count; i++)
        {
            cp = &(cf->constant_pool[i]);
            if (cp->tag == CONSTANT_Utf8)
            {
                freeMemory(cp->info.cud.bytes);
                cp->info.cud.bytes = (u1 *) 0;
            }
        }
        freeMemory(cf->constant_pool);
        cf->constant_pool = (cp_info *) 0;
    }

    return 0;
}

static u1 *
convertAccessFlags_field(u2 i, u2 af)
{
    const static int initBufSize = 0x100;
    u1 *buf, *out;
    int len;

    if (!af)
        return (u1 *) 0;
    if (af & ACC_SYNTHETIC)
        return (u1 *) 0;
    if (af & ACC_ENUM)
        return (u1 *) 0;
    if (af & ~ACC_FIELD)
    {
        logError("Unknown flags [0x%X] detected @ cf->fields[%i]:0x%X!\r\n",
                af & ~ACC_FIELD, i, af);
        return (u1 *) 0;
    }
    buf = (u1 *) allocMemory(initBufSize, sizeof (char));
    if (!buf)
        return (u1 *) 0;
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
    out = (u1 *) realloc(buf, len + 1);
    if (!out)
    {
        freeMemory(buf);
        return buf = (u1 *) 0;
    }
    out[len] = 0;
    return out;
}

static u1 *
convertAccessFlags_method(u2 i, u2 af)
{
    const static int initBufSize = 0x100;
    u1 *buf, *out;
    int len;

    if (!af)
        return (u1 *) 0;
    if (af & ACC_BRIDGE)
        return (u1 *) 0;
    if (af & ACC_SYNTHETIC)
        return (u1 *) 0;
    if (af & ~ACC_METHOD)
    {
        logError("Unknown flags [%X] detected @ cf->methods[%i]!\r\n",
                af & ~ACC_METHOD, i);
        return (u1 *) 0;
    }
    buf = (u1 *) malloc(initBufSize);
    if (!buf)
    {
        logError("Fail to allocate memory.\r\n");
        return (u1 *) 0;
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
    out = (u1 *) realloc(buf, len + 1);
    if (!out)
    {
        free(buf);
        return buf = (u1 *) 0;
    }
    out[len] = 0;
    return out;
}

extern const char *
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

extern cp_info *
getConstant(ClassFile *cf, u2 index)
{
    if (index > 0 && index < cf->constant_pool_count)
        return &(cf->constant_pool[index]);
    return (cp_info *) 0;
}

extern const_Utf8_data *
getConstant_Utf8(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Utf8)
    {
        logError("Constant pool entry #%i is not CONSTANT_Utf8_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cud);
}

extern const_Class_data *
getConstant_Class(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Class)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Class_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.ccd);
}

extern const_Fieldref_data *
getConstant_Fieldref(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Fieldref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Fieldref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cfd);
}

extern const_Methodref_data *
getConstant_Methodref(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Methodref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Methodref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cfd);
}

extern const_InterfaceMethodref_data *
getConstant_InterfaceMethodref(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_InterfaceMethodref)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_InterfaceMethodref_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cfd);
}

extern const_String_data *
getConstant_String(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_String)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_String_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.csd);
}

extern const_Integer_data *
getConstant_Integer(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Integer)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Integer_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cid);
}

extern const_Float_data *
getConstant_Float(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Float)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Float_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cid);
}

extern const_Long_data *
getConstant_Long(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Long)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Long_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cld);
}

extern const_Double_data *
getConstant_Double(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_Double)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_Double_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cld);
}

extern const_NameAndType_data *
getConstant_NameAndType(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_NameAndType)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_NameAndType_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cnd);
}

extern const_MethodHandle_data *
getConstant_MethodHandle(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_MethodHandle)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_MethodHandle_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cmhd);
}

extern const_MethodType_data *
getConstant_MethodType(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_MethodType)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_MethodType_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cmtd);
}

extern const_InvokeDynamic_data *
getConstant_InvokeDynamic(ClassFile *cf, u2 index)
{
    cp_info *info;

    info = getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return NULL;
    }
    if (info->tag != CONSTANT_InvokeDynamic)
    {
        logInfo("Constant pool entry #%i is not CONSTANT_InvokeDynamic_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return NULL;
    }

    return &(info->info.cidd);
}

/*
 * Return:
 * * 1      When major.minor > major1.minor1
 * * 0      When major.minor == major1.minor1
 * * -1     When major.minor < major1.minor1
 */
extern int
compareVersion0(u2 major_version,
        u2 minor_version,
        u2 major_version1,
        u2 minor_version1)
{
    u4 version, version1;

    version = major_version << 8 | minor_version;
    version1 = major_version1 << 8 | minor_version;
    version -= version1;
    return (int) version;
}

extern int
compareVersion(u2 major_version, u2 minor_version)
{
    return compareVersion0(major_version, minor_version,
            MAJOR_VERSION, MINOR_VERSION);
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

static int
getConstantLength(u1 tag)
{
    switch (tag)
    {
        case CONSTANT_Class:
            return sizeof (const_Class_data);
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            return sizeof (const_Fieldref_data);
        case CONSTANT_String:
            return sizeof (const_String_data);
        case CONSTANT_Integer:
        case CONSTANT_Float:
            return sizeof (const_Integer_data);
        case CONSTANT_Long:
        case CONSTANT_Double:
            return sizeof (const_Long_data);
        case CONSTANT_NameAndType:
            return sizeof (const_NameAndType_data);
        case CONSTANT_Utf8:
            return sizeof (const_Utf8_data);
        case CONSTANT_MethodHandle:
            return sizeof (const_MethodHandle_data);
        case CONSTANT_MethodType:
            return sizeof (const_MethodType_data);
        case CONSTANT_InvokeDynamic:
            return sizeof (const_InvokeDynamic_data);
    }

    return 0;
}

static int
loadConstant(struct BufferIO *input, cp_info *info)
{
    u1      tag;
    int     length;
    u2      len;
    u1 *    str;

    if (ru1(&tag, input) < 0)                       return -1;

    if (tag == CONSTANT_Utf8)
    {
        if (ru2(&len, input) < 0)                   return -1;
        str = (u1 *) allocMemory(sizeof (u1), len);
        if (!str)                                   return -1;
        if (rbs(str, input, len) < 0)               return -1;
        info->info.cud.length = len;
        info->info.cud.bytes = str;
    }
    else
    {
        length = getConstantLength(tag);
        if (rbs((u1 *) &(info->info), input, length) < 0)  return -1;
    }

    info->tag = tag;

    return length;
}

static int
loadConstantPool(struct BufferIO *input, ClassFile *cf)
{
    u2 i;
    cp_info *                       info;
    
    // retrieve constant pool size
    if (ru2(&(cf->constant_pool_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (cf->constant_pool_count > 0)
    {
        cf->constant_pool = (cp_info *) allocMemory(cf->constant_pool_count, sizeof (cp_info));
        if (!cf->constant_pool) return -1;

        // jvms7 says "The constant_pool table is indexed
        // from 1 to constant_pool_count - 1
        for (i = 1u; i < cf->constant_pool_count; i++)
        { // LOOP
            info = &(cf->constant_pool[i]);
            if (loadConstant(input, info) < 0) return -1;
        } // LOOP
    }
    
    return 0;
}

static int
loadInterfaces(struct BufferIO *input, ClassFile *cf)
{
    u2 i;
    
    if (ru2(&(cf->interfaces_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (cf->interfaces_count > 0)
    {
        cf->interfaces = (u2 *) allocMemory(cf->interfaces_count, sizeof (u2));
        if (!cf->interfaces) return -1;
        for (i = 0u; i < cf->interfaces_count; i++)
        {
            if (ru2(&(cf->interfaces[i]), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
        }
    }
    
    return 0;
}

static int
loadFields(struct BufferIO *input, ClassFile *cf)
{
    u2 i;
    
    if (ru2(&(cf->fields_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (cf->fields_count > 0)
    {
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
            loadAttributes_field(cf, input, &(cf->fields[i]),
                    &(cf->fields[i].attributes_count), &(cf->fields[i].attributes));
        }
    }
    
    return 0;
}

static int
loadMethods(struct BufferIO *input, ClassFile *cf)
{
    u2 i;
    
    if (ru2(&(cf->methods_count), input) < 0)
    {
        logError("IO exception in function %s!\r\n", __func__);
        return -1;
    }
    if (cf->methods_count > 0)
    {
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
            loadAttributes_method(cf, input, &(cf->methods[i]),
                    &(cf->methods[i].attributes_count), &(cf->methods[i].attributes));
        }
    }
    
    return 0;
}

static int
writeClassName0(char *out,
        u2 len, u1 *str)
{
    size_t n;
    u2 i;

    n = sprintf(out, "%.*s", len, str);
    if (n < 0) return -1;

    for (i = 0; i < len; i++)
        if (out[i] == '/')
            out[i] = '.';

    return n;
}

static int
writeClassName(char *out,
        rt_Class *rtc,
        const_Class_data *class_info)
{
    const_Utf8_data *class_name;
    u2 class_name_index;

    class_name_index = class_info->name_index;
    class_name = rtc->getConstant_Utf8(class_name_index);

    return writeClassName0(out,
            class_name->length,
            class_name->bytes);
}

static int
logClassHeader(rt_Class *rtc)
{
#if (defined DEBUG && defined LOG_INFO)
    char buf[1024], *ptr;
    size_t n, m;
    u2 access_flags;
    const_Class_data * this_class, * super_class;
    u2 interfaces_count;
    const_Class_data ** interfaces;
    const_Utf8_data *cui;
    u2 i;

    access_flags = rtc->getAccessFlags();
    this_class = rtc->getThisClass();
    super_class = rtc->getSuperClass();
    interfaces_count = rtc->getInterfacesCount();
    interfaces = (const_Class_data **) NULL;
    if (!rtc->getInterfaces(interfaces))
        goto error;

    memset(buf, 0, sizeof (buf));
    ptr = (char *) buf;

    if (access_flags & ACC_PUBLIC)
    {
        n = sprintf(ptr, "public ");
        if (n < 0) goto error;
        ptr += n;
    }

    if (access_flags & ACC_ABSTRACT)
    {
        n = sprintf(ptr, "abstract ");
        if (n < 0) goto error;
        ptr += n;
    }
    else if (access_flags & ACC_FINAL)
    {
        n = sprintf(ptr, "final ");
        if (n < 0) goto error;
        ptr += n;
    }

    // type
    if (access_flags & ACC_ANNOTATION)
        n = sprintf(ptr, "@interface ");
    else if (access_flags & ACC_ENUM)
        n = sprintf(ptr, "enum ");
    else if (access_flags & ACC_INTERFACE)
        n = sprintf(ptr, "interface ");
    else
        n = sprintf(ptr, "class ");
    if (n < 0) goto error;
    ptr += n;

    // name
    n = writeClassName(ptr, rtc, this_class);
    if (n < 0) goto error;
    ptr += n;
    *ptr++ = ' ';

    // super class
    if (super_class)
    {
        cui = rtc->getConstant_Utf8(super_class->name_index);
        if (!cui) goto error;
        if (strncmp("java/lang/Object",
                    (char *) cui->bytes,
                    cui->length))
        {
            n = sprintf(ptr, "extends ");
            if (n < 0) goto error;
            ptr += n;
            n = writeClassName0(ptr,
                    cui->length,
                    cui->bytes);
            if (n < 0) goto error;
            ptr += n;
            *ptr++ = ' ';
        }
    }

    // interfaces
    if (interfaces_count > 0)
    {
        if (access_flags & ACC_INTERFACE)
            n = sprintf(ptr, "\r\n\textends ");
        else
            n = sprintf(ptr, "\r\n\timplements ");
        if (n < 0) goto error;
        ptr += n;

        for (i = 0u; i < interfaces_count; i++)
        {
            if (i > 0)
            {
                n = sprintf(ptr, ",\r\n\t\t");
                if (n < 0) goto error;
                ptr += n;
            }
            n = writeClassName(ptr, rtc, interfaces[i]);
            if (n < 0) goto error;
            ptr += n;
        }
    }

    logInfo("%s\r\n{\r\n", buf);
#endif
    freeMemory(interfaces);
    interfaces = (const_Class_data **) 0;
    return 0;
error:
    freeMemory(interfaces);
    interfaces = (const_Class_data **) 0;
    return -1;
}

static int
writeFieldDescriptor(char *out, u2 len, u1 *str)
{
    u2 i, j, m, n;

    if (len == 1)
    {
        switch (str[0])
        {
            case 'B':
                return sprintf(out, "byte");
            case 'C':
                return sprintf(out, "char");
            case 'D':
                return sprintf(out, "double");
            case 'F':
                return sprintf(out, "float");
            case 'I':
                return sprintf(out, "int");
            case 'J':
                return sprintf(out, "long");
            case 'S':
                return sprintf(out, "short");
            case 'Z':
                return sprintf(out, "boolean");
            // return type only
            case 'V':
                return sprintf(out, "void");
            default:
                return -1;
        }
    }
    else
    {
        if (str[0] == '[')
        {
            for (i = 1; i < len; i++)
                if (str[i] != '[')
                    break;
            m = writeFieldDescriptor(out, len - i, str + i);
            if (m < 0) return -1;
            for (j = 0; j < i; j++)
            {
                n = sprintf(out + m + j * 2, "[]");
                if (n != 2) return -1;
            }
            return m + i * 2;
        }
        else if (str[0] == 'L')
        {
            len -= 2;
            ++str;
            m = sprintf(out, "%.*s", len, str);
            if (m != len) return -1;
            for (i = 0; i < len; i++)
                if (out[i] == '/')
                    out[i] = '.';
            return m;
        }
    }
}

static int
writeConstantString(char *out, u2 len, u1 *str)
{
    int m;
    char *src, *dst;

    out[0] = '"';
    src = out++;
    dst = out + len;

    m = sprintf(out, "%.*s", len, str);
    if (m < 0) return -1;

    while (out < dst)
    {
        if (*out == '"')
        {
            // move
            memcpy(out + 1, out, dst - out);
            ++dst;

            // fill
            *out = '\\';
        }
        ++out;
    }

    return 0;
}

static int
logFields(rt_Class *rtc)
{
#if (defined DEBUG && defined LOG_INFO)
    char buf[1024], *ptr;
    u2 i, j;
    u2 fields_count;
    rt_Field * fields, * field;
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    attr_info *attributes;
    attr_info *attribute;
    size_t n;
    rt_Utf8_info *name, *descriptor;
    attr_ConstantValue_info *acv;
    u2 constantvalue_index;
    const_Long_data *cv_long;
    const_Float_data *cv_float;
    const_Double_data *cv_double;
    const_Integer_data *cv_integer;
    const_String_data *cv_string;
    const_Utf8_data *cui;

    memset(buf, 0, sizeof (buf));
    ptr = (char *) buf;
    fields_count = rtc->getFieldsCount();
    fields = rtc->getFields();

    for (i = 0; i < fields_count; i++)
    {
        field = &(fields[i]);

        access_flags = field->getAccessFlags();
        name = field->getName();
        descriptor = field->getDescriptor();

        attributes_count = field->getAttributes()->attributes_count;
        attributes = field->getAttributes()->attributes;

        n = sprintf(ptr, "\t");
        if (n < 0) return -1;
        ptr += n;

        // hide this field if it is synthetic
        if (access_flags & ACC_SYNTHETIC)
            continue;

        // display access flags and descriptor
        // only when the field is not an enum value
        if (!(access_flags & ACC_ENUM))
        {
            // flags
            if (access_flags & ACC_PUBLIC)
            {
                n = sprintf(ptr, "public ");
                if (n < 0) return -1;
                ptr += n;
            }
            else if (access_flags & ACC_PRIVATE)
            {
                n = sprintf(ptr, "private ");
                if (n < 0) return -1;
                ptr += n;
            }
            else if (access_flags & ACC_PROTECTED)
            {
                n = sprintf(ptr, "protected ");
                if (n < 0) return -1;
                ptr += n;
            }
            if (access_flags & ACC_STATIC)
            {
                n = sprintf(ptr, "static ");
                if (n < 0) return -1;
                ptr += n;
            }
            if (access_flags & ACC_FINAL)
            {
                n = sprintf(ptr, "final ");
                if (n < 0) return -1;
                ptr += n;
            }
            else if (access_flags & ACC_VOLATILE)
            {
                n = sprintf(ptr, "volatile ");
                if (n < 0) return -1;
                ptr += n;
            }
            if (access_flags & ACC_TRANSIENT)
            {
                n = sprintf(ptr, "transient ");
                if (n < 0) return -1;
                ptr += n;
            }

            // descriptor
            n = writeFieldDescriptor(ptr,
                    descriptor->length,
                    descriptor->bytes);
            if (n < 0) return -1;
            ptr += n;
        }

        n = sprintf(ptr, " %.*s",
                name->length,
                name->bytes);
        if (n < 0) return -1;
        ptr += n;

        if ((access_flags & ACC_FINAL) && !(access_flags & ACC_ENUM))
        {
            // retrieve constant value
            for (j = 0; j < attributes_count; j++)
            {
                attribute = &(attributes[j]);
                if (attribute->tag != TAG_ATTR_CONSTANTVALUE)
                    continue;

                n = sprintf(ptr, " = ");
                if (n < 0) return -1;
                ptr += n;

                acv = (attr_ConstantValue_info *) attribute->data;
                constantvalue_index = acv->constantvalue_index;
                switch (descriptor->bytes[0])
                {
                    case 'J':
                        cv_long = rtc->getConstant_Long(
                                constantvalue_index);
                        n = sprintf(ptr, "%llil",
                                cv_long->long_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'F':
                        cv_float = rtc->getConstant_Float(
                                constantvalue_index);
                        n = sprintf(ptr, "%ff",
                                cv_float->float_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'D':
                        cv_double = rtc->getConstant_Double(
                                constantvalue_index);
                        n = sprintf(ptr, "%fd",
                                cv_double->double_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'L':
                        cv_string = rtc->getConstant_String(
                                constantvalue_index);
                        cui = rtc->getConstant_Utf8(
                                cv_string->string_index);
                        n = writeConstantString(ptr,
                                cui->length,
                                cui->bytes);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    default:
                        cv_integer = rtc->getConstant_Integer(
                                constantvalue_index);
                        n = sprintf(ptr, "%i",
                                cv_integer->bytes);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                }

                // exit the loop coz there's
                // at most one ConstantValue attribute
                break;
            }
        }
        n = sprintf(ptr, ";\r\n");
        if (n < 0) return -1;
        ptr += n;
    }

    logInfo("%s\r\n", buf);
#endif
    return 0;
}

struct ParameterTable
{
    int local_count;
    char **local_names;
};

static int
writeParameterTable(char *out,
        rt_Method *method,
        u2 len, u1 *str)
{
    char *src;
    size_t n;
    u2 i, j;
    u2 m;
    u1 *p;
    u2 count;
    u2 access_flags;
    u1 parameter_type;

    src = out;
    *out++ = '(';
    --len;
    ++str;

    // instance methods start with 'this' parameter
    access_flags = method->getAccessFlags();
    if (access_flags & ACC_STATIC)
        count = 0;
    else
        count = 1;

    for (i = 0; i < len; i++)
    {
        if (str[i] == ')')
        {
            // remove " {\r\n" coz some methods
            // with ACC_NATIVE, ACC_ABSTRACT flags
            // end with ';'
            n = sprintf(out, ")");
            if (n < 0) return -1;
            out += n;
            break;
        }

        if (i > 0)
        {
            n = sprintf(out, ", ");
            if (n < 0) return -1;
            out += n;
        }

        j = i;
        parameter_type = 1;
        // Object parameter
        if (str[i] == 'L')
        {
            for (++j; j < len; j++)
                if (str[j] == ';')
                    break;
            m = j + 1 - i;
        }
        // Array parameter
        else if (str[i] == '[')
        {
            for (++j; j < len; j++)
                if (str[j] != '[')
                    break;
            if (str[j] == 'L')
                for (++j; j < len; j++)
                    if (str[j] == ';')
                        break;
            m = j + 1 - i;
        }
        // Primitive parameter
        else
        {
            m = 1;
            // detect long/double parameter
            if (str[i] == 'J'
                    || str[i] == 'D')
                parameter_type = 2;
        }
        p = str + i;
        i = j;

        n = writeFieldDescriptor(out, m, p);
        if (n < 0) return -1;
        out += n;

        // parameter name
        n = sprintf(out, " param%i", count++);
        if (n < 0) return -1;
        out += n;
    }

    //method->parameters_count = count;

    return out - src;
}

static void
insertTabIndent(size_t len, char *str)
{
    char *ptr, *dst;

    dst = str + len;
    do
    {
        // find the beginning of each line
        ptr = strstr(str, "\r\n");
        ptr += 2;
        // move each line rightwards
        memcpy(ptr + 1, ptr, len - (ptr - str));
        *ptr = '\t';
        // change length
        ++dst;
    }
    while (ptr && ptr < dst);
}

static int
writeCode(char *out, rt_Class *rtc,
        rt_Method *method,
        attr_Code_info *info)
{
    char *src;
    int tab, t;
    size_t n;
    u4 code_length;
    u1 *code;
    u2 exception_table_length;
    struct exception_table_entry *exceptions, *entry;
    u4 i;
    u2 j;

    src = out;
    tab = 2;

    code_length = info->code_length;
    code = info->code;
    exception_table_length = info->exception_table_length;
    exceptions = info->exception_table;

    i = 0;
    for (j = 0; j < exception_table_length; j++)
    {
        entry = &(exceptions[j]);

        while (i < code_length
                && i < entry->start_pc)
        {
        }
        while (i < code_length
                && i < entry->end_pc)
        {
        }
    }

    return out - src;
}

static int
logMethods(rt_Class *rtc)
{
#if (defined DEBUG && defined LOG_INFO)
    char buf[65536], *ptr;
    size_t n;
    u2 i, j;
    u2 methods_count;
    rt_Method *methods, *method;
    u2 access_flags, name_index, descriptor_index;
    u2 attributes_count;
    attr_info *attributes, *attribute;
    const_Utf8_data *class_name;
    const_Utf8_data *name, *descriptor;
    attr_Code_info *code;
    attr_Exceptions_info *exceptions;
    u2 number_of_exceptions;
    u2 *exception_index_table;
    int has_method_body;
    const_Class_data *cci;
    const_Utf8_data *cui;

    memset(buf, 0, sizeof (buf));
    methods_count = rtc->getMethodsCount();
    methods = rtc->getMethods();
    class_name = rtc->getClassName();

    logInfo("// Method count: %i.\r\n\r\n", methods_count);

    for (i = 0; i < methods_count; i++)
    {
        method = &(methods[i]);

        access_flags = method->getAccessFlags();
        name = method->getName();
        descriptor = method->getDescriptor();

        attributes_count = method->getAttributes()->attributes_count;
        attributes = method->getAttributes()->attributes;

        // note ACC_NATIVE, ACC_VARARGS
        // initialize Code attribute & Exceptions attribute
        code = (attr_Code_info *) 0;
        exceptions = (attr_Exceptions_info *) 0;

        if (ptr > buf)
            memset(buf, 0, ptr - buf);
        ptr = (char *) buf;

        // write meta info
        n = sprintf(ptr, "\t// Method #%i:\r\n", i);
        if (n < 0) return -1;
        ptr += n;

        if (access_flags & ACC_BRIDGE)
            continue;
        if (access_flags & ACC_SYNTHETIC)
            continue;

        n = sprintf(ptr, "\t");
        if (n < 0) return -1;
        ptr += n;

        if (access_flags & ACC_PUBLIC)
        {
            n = sprintf(ptr, "public ");
            if (n < 0) return -1;
            ptr += n;
        }
        else if (access_flags & ACC_PRIVATE)
        {
            n = sprintf(ptr, "private ");
            if (n < 0) return -1;
            ptr += n;
        }
        else if (access_flags & ACC_PROTECTED)
        {
            n = sprintf(ptr, "protected ");
            if (n < 0) return -1;
            ptr += n;
        }
        if (access_flags & ACC_STATIC)
        {
            n = sprintf(ptr, "static ");
            if (n < 0) return -1;
            ptr += n;
        }
        if (access_flags & ACC_SYNCHRONIZED)
        {
            n = sprintf(ptr, "synchronized ");
            if (n < 0) return -1;
            ptr += n;
        }
        if (access_flags & ACC_ABSTRACT)
        {
            n = sprintf(ptr, "final ");
            if (n < 0) return -1;
            ptr += n;
        }
        else
        {
            if (access_flags & ACC_NATIVE)
            {
                n = sprintf(ptr, "native ");
                if (n < 0) return -1;
                ptr += n;
            }
            if (access_flags & ACC_FINAL)
            {
                n = sprintf(ptr, "final ");
                if (n < 0) return -1;
                ptr += n;
            }
        }
        if (access_flags & ACC_STRICT)
        {
            n = sprintf(ptr, "strictfp ");
            if (n < 0) return -1;
            ptr += n;
        }

        // write method name
        // static initializer has no name
        if (strncmp("<clinit>",
                    (char *) name->bytes,
                    name->length) != 0)
        {
            if (strncmp("<init>", (char *) name->bytes,
                        name->length) == 0)
            {
                // class name
                n = writeClassName0(ptr,
                        class_name->length,
                        class_name->bytes);
                if (n < 0) return -1;
                ptr += n;
            }
            else
            {
                // return type
                for (j = descriptor->length - 1;
                        j > 0; j--)
                {
                    if (descriptor->bytes[j] != ')')
                        continue;

                    ++j;
                    n = writeFieldDescriptor(ptr,
                            descriptor->length - j,
                            descriptor->bytes + j);
                    if (n < 0) return -1;
                    ptr += n;
                    n = sprintf(ptr, " ");
                    if (n < 0) return -1;
                    ptr += n;

                    break;
                }

                // method name
                n = sprintf(ptr, "%.*s",
                        name->length,
                        name->bytes);
                if (n < 0) return -1;
                ptr += n;
            }

            // parameter table
            n = writeParameterTable(ptr,
                    method,
                    descriptor->length,
                    descriptor->bytes);
            if (n < 0) return -1;
            ptr += n;
        }

#if ! VER_CMP(45, 3)
    #error "Class version should be higher than 45.3"
#endif

        has_method_body = !(access_flags & (ACC_NATIVE | ACC_ABSTRACT));

        for (j = 0; j < attributes_count; j++)
        {
            attribute = &(attributes[j]);

            if (attribute->tag != TAG_ATTR_EXCEPTIONS)
                continue;
            exceptions = (attr_Exceptions_info *)
                attribute->data;
            break;
        }
        if (has_method_body)
            for (j = 0; j < attributes_count; j++)
            {
                attribute = &(attributes[j]);

                if (attribute->tag != TAG_ATTR_CODE)
                    continue;
                code = (attr_Code_info *)
                    attribute->data;
                break;
            }

        // analyze Exceptions attribute
        if (exceptions)
        {
            number_of_exceptions = exceptions->number_of_exceptions;
            exception_index_table = exceptions->exception_index_table;

            n = sprintf(ptr, "\r\n\t\tthrows ");
            if (n < 0) return -1;
            ptr += n;

            for (j = 0; j < number_of_exceptions; j++)
            {
                if (j > 0)
                {
                    n = sprintf(ptr, ",\r\n\t\t\t");
                    if (n < 0) return -1;
                    ptr += n;
                }

                n = writeClassName(ptr, rtc,
                        rtc->getConstant_Class(exception_index_table[j]));
                if (n < 0) return -1;
                ptr += n;
            }
        }

        if (code)
        {
            n = writeCode(ptr, rtc, method, code);
            if (n < 0) return -1;
            ptr += n;
        }

        // native methods and abstract methods
        // have no method body
        if (has_method_body)
            n = sprintf(ptr, " {\r\n");
        else
            n = sprintf(ptr, ";\r\n");
        if (n < 0) return -1;
        ptr += n;

        if (has_method_body)
        {
            n = sprintf(ptr, "\t}\r\n\r\n");
            if (n < 0) return -1;
            ptr += n;
        }
    
        logInfo("%s\r\n", buf);
    }

#endif
    return 0;
}
