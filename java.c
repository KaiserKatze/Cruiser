#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "java.h"
#include "log.h"
#include "memory.h"
#include "rt.h"

static int
freeClassfile(ClassFile *);

static const char *
get_cp_name(u1);

static int
loadConstantPool(struct BufferIO *, ClassFile *);

static int
loadInterfaces(struct BufferIO *, ClassFile *);

static int
loadFields(struct BufferIO *, ClassFile *);

static int
loadMethods(struct BufferIO *, ClassFile *);

static int
validateConstantPool(ClassFile *);

static int
validateFields(ClassFile *);

static int
validateMethods(ClassFile *);

static int
validateFieldDescriptor(u2, u1 *);

static int
validateMethodDescriptor(u2, u1 *);

static int
validateAttributes_class(ClassFile *, u2, attr_info *);

static int
validateAttributes_field(ClassFile *, field_info *);

static int
validateAttributes_method(ClassFile *, method_info *);

static int
validateAttributes_code(ClassFile *, attr_Code_info *);

static int
validateJavaTypeSignature(u2, u1 *);

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
    if (magic != 0XCAFEBABE)
    {
        logError("File structure invalid, fail to decompile! [0x%X]\r\n", magic);
        return -1;
    }
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
        goto close;
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

    /*
    loadAttributes_class(&cf, input, &(cf.attributes_count), &(cf.attributes));
#ifdef RT_H
    // constant pool validation
    if (validateConstantPool(&cf) < 0)
        return -1;
    if (validateFields(&cf) < 0)
        return -1;
    if (validateMethods(&cf) < 0)
        return -1;
#endif
    */
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
    CONSTANT_Utf8_info *cu;

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
            if (cp->data)
            {
                if (cp->tag == CONSTANT_Utf8)
                {
                    cu = (CONSTANT_Utf8_info *) cp;
                    if (cu->data->bytes)
                    {
                        freeMemory(cu->data->bytes);
                        cu->data->bytes = (u1 *) 0;
                    }
                }
                freeMemory(cp->data);
                cp->data = (void *) 0;
            }
        }
        freeMemory(cf->constant_pool);
        cf->constant_pool = (cp_info *) 0;
    }

    return 0;
}

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

static cp_info *
getConstant(ClassFile *cf, u2 index)
{
    if (index > 0 && index < cf->constant_pool_count)
        return &(cf->constant_pool[index]);
    return (cp_info *) 0;
}

extern CONSTANT_Utf8_info *
getConstant_Utf8(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *info;

    info = (CONSTANT_Utf8_info *) getConstant(cf, index);
    if (!info)
    {
        logError("Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Utf8_info *) 0;
    }
    if (info->tag != CONSTANT_Utf8)
    {
        logError("Constant pool entry #%i is not CONSTANT_Utf8_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
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

extern u1 *
getConstant_Utf8String(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *cu;
    u1 *str;

    cu = getConstant_Utf8(cf, index);
    if (!cu)
        return (u1 *) 0;
    if (!cu->data)
        return (u1 *) 0;
    str = cu->data->bytes;

    return str;
}

extern u1 *
getConstant_ClassName(ClassFile *cf, u2 index)
{
    CONSTANT_Class_info *cc;
    
    cc = getConstant_Class(cf, index);
    if (!cc)
        return (u1 *) 0;
    if (!cc->data)
        return (u1 *) 0;

    return getConstant_Utf8String(cf, cc->data->name_index);
}

// index should be valid entry into constant pool and the entry must be a method reference
extern u1 *
getConstant_MethodName(ClassFile *cf, u2 index)
{
    CONSTANT_Methodref_info *cmi;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_Utf8_info *cui;
    u1 *name;
    
    cmi = getConstant_Methodref(cf, index);
    cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
    cui = getConstant_Utf8(cf, cni->data->name_index);
    if (strncmp((char *) cui->data->bytes, "<init>", cui->data->length))
        return cui->data->bytes;
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

    info = (CONSTANT_Class_info *) getConstant(cf, index);
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

    info = (CONSTANT_Fieldref_info *) getConstant(cf, index);
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

    info = (CONSTANT_Methodref_info *) getConstant(cf, index);
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

    info = (CONSTANT_InterfaceMethodref_info *) getConstant(cf, index);
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

    info = (CONSTANT_String_info *) getConstant(cf, index);
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

    info = (CONSTANT_Integer_info *) getConstant(cf, index);
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

    info = (CONSTANT_Float_info *) getConstant(cf, index);
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

    info = (CONSTANT_Long_info *) getConstant(cf, index);
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

    info = (CONSTANT_Double_info *) getConstant(cf, index);
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

    info = (CONSTANT_NameAndType_info *) getConstant(cf, index);
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

    info = (CONSTANT_MethodHandle_info *) getConstant(cf, index);
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

    info = (CONSTANT_MethodType_info *) getConstant(cf, index);
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

    info = (CONSTANT_InvokeDynamic_info *) getConstant(cf, index);
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
    CONSTANT_Methodref_info *cmi;
    CONSTANT_String_info *csi;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_Utf8_info *cui;
    CONSTANT_MethodHandle_info *cmhi;
    CONSTANT_MethodType_info *cmti;
    CONSTANT_InvokeDynamic_info *cidi;
    u2 j;
#if VER_CMP(51, 0)
    struct attr_BootstrapMethods_info *dataBootstrapMethods;
    struct bootstrap_method *bm;
#endif

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
        case CONSTANT_Long:
        case CONSTANT_Double:
            bul[i + 1] = 1;
            break;
        case CONSTANT_NameAndType:
            cni = (CONSTANT_NameAndType_info *) info;
            if (validateConstantPoolEntry(cf, cni->data->name_index, bul, CONSTANT_Utf8) < 0)
                return -1;
            cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cni->data->name_index]);
            if (strncmp((char *) cui->data->bytes, "<init>", cui->data->length)
                    && strncmp((char *) cui->data->bytes, "<clinit>", cui->data->length))
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
                case REF_getField:
                case REF_getStatic:
                case REF_putField:
                case REF_putStatic:
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Fieldref) < 0)
                        return -1;
                    break;
                case REF_invokeVirtual:
                case REF_newInvokeSpecial:
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Methodref) < 0)
                        return -1;
                case REF_invokeStatic:
                case REF_invokeSpecial:
#if VER_CMP(52, 0)
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Methodref) < 0
                            && validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_InterfaceMethodref) < 0)
                        return -1;
#else
                    if (validateConstantPoolEntry(cf, cmhi->data->reference_index, bul, CONSTANT_Methodref) < 0)
                        return -1;
#endif
                    break;
                case REF_invokeInterface:
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
                    if (!strncmp((char *) cui->data->bytes, "<init>", cui->data->length))
                    {
                        logError("Method name '<init>' is invalid because MethodHandle reference kind is %i!\r\n",
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    else if (!strncmp((char *) cui->data->bytes, "<clinit>", cui->data->length))
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
                    if (strncmp((char *) cui->data->bytes, "<init>", cui->data->length))
                    {
                        logError("Method name '%.*s' is invalid because MethodHandle reference kind is %i!\r\n",
                                (int) cui->data->length, (char *) cui->data->bytes, cmhi->data->reference_kind);
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
            dataBootstrapMethods = (struct attr_BootstrapMethods_info *) 0;
#ifndef QUICK_REFERENCE
            for (j = 0; j < cf->attributes_count; j++)
            {
                if (cf->attributes[j].tag != TAG_ATTR_BOOTSTRAPMETHODS)
                    continue;
                dataBootstrapMethods = (struct attr_BootstrapMethods_info *)
                        cf->attributes[j].data;
                break;
            }
#else
            if (cf->off_BootstrapMethods >= 0)
            {
                dataBootstrapMethods = (struct attr_BootstrapMethods_info *)
                        cf->attributes[cf->off_BootstrapMethods].data;
            }
#endif
            if (!dataBootstrapMethods)
            {
                logError("Attribute BootstrapMethods is not found!\r\n");
                return -1;
            }
            bm = &(dataBootstrapMethods->bootstrap_methods[cidi->data->bootstrap_method_attr_index]);
            cmhi = (CONSTANT_MethodHandle_info *)
                    &(cf->constant_pool[bm->bootstrap_method_ref]);
            switch (cmhi->data->reference_kind)
            {
                case REF_invokeStatic:      // 6
                case REF_newInvokeSpecial:  // 8
                    break;
                default:
                    logError("Invalid reference_kind [%i]!\r\n",
                            cmhi->data->reference_kind);
                    return -1;
            }
            cmi = (CONSTANT_Methodref_info *)
                    &(cf->constant_pool[cmhi->data->reference_index]);
            cni = (CONSTANT_NameAndType_info *)
                    &(cf->constant_pool[cmi->data->name_and_type_index]);
            cui = (CONSTANT_Utf8_info *)
                    &(cf->constant_pool[cni->data->descriptor_index]);
            if (strncmp((char *) cui->data->bytes,
                    "(Ljava/lang/invoke/MethodHandles$Lookup;"
                    "Ljava/lang/String;"
                    "Ljava/lang/invoke/MethodType;)",
                    88))
            {
                logError("Invalid bootstrap method arguments [%.*s]!\r\n",
                        cui->data->length, cui->data->bytes);
                return -1;
            }
            for (j = 0; j < bm->num_bootstrap_arguments; j++)
            {
                if (validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_String) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_Class) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_Integer) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_Long) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_Float) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_Double) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_MethodHandle) < 0
                        && validateConstantPoolEntry(cf, bm->bootstrap_arguments[j], bul, CONSTANT_MethodType) < 0)
                {
                    logError("Invalid bootstrap method argument type!\r\n");
                    return -1;
                }
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

extern int getMethodParametersCount(ClassFile *cf, u2 descriptor_index)
{
    CONSTANT_Utf8_info *cui;
    u2 len, i;
    u1 *str;
    int count;
    
    cui = getConstant_Utf8(cf, descriptor_index);
    if (!cui)
        return -1;
    len = cui->data->length;
    str = cui->data->bytes;
    count = 0;
    for (i = 1; i < len; i++)
    {
        switch (str[i])
        {
            case 'B':
            case 'C':
            case 'D':
            case 'F':
            case 'I':
            case 'J':
            case 'S':
            case 'Z':
                ++count;
                break;
            case 'L':
                ++count;
                while (++i < len)
                    if (str[i] == ';')
                        break;
                break;
            case '[':
                break;
            case ')':
                return count;
            default:
                logError("Assertion error: Unknown token [%c]!\r\n", str[i]);
                return -1;
        }
    }
    logError("Assertion error: Method descriptor has no end ')'!\r\n");
    
    return -1;
}

extern u1 *getClassSimpleName(ClassFile *cf, u2 class_name_index)
{
    CONSTANT_Utf8_info *cui;
    u2 i, j, len;
    u1 *str;
    u1 *res;
    
    cui = getConstant_Utf8(cf, class_name_index);
    len = cui->data->length;
    str = cui->data->bytes;
    i = 0;
    // count array dimension: i
    switch (str[0])
    {
        case '[':
            for (i = 1; i < len; i++)
                if (str[i] != '[')
                    break;
            break;
    }
    switch (str[i])
    {
        case 'B':
            res = (u1 *) allocMemory(5 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "byte", 4);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 4 + 2 * j, "[]", 2);
            break;
        case 'C':
            res = (u1 *) allocMemory(5 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "u1", 4);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 4 + 2 * j, "[]", 2);
            break;
        case 'D':
            res = (u1 *) allocMemory(7 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "double", 6);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 6 + 2 * j, "[]", 2);
            break;
        case 'F':
            res = (u1 *) allocMemory(6 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "float", 5);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 5 + 2 * j, "[]", 2);
            break;
        case 'I':
            res = (u1 *) allocMemory(4 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "int", 3);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 3 + 2 * j, "[]", 2);
            break;
        case 'J':
            res = (u1 *) allocMemory(5 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "long", 4);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 4 + 2 * j, "[]", 2);
            break;
        case 'S':
            res = (u1 *) allocMemory(6 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "short", 5);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 5 + 2 * j, "[]", 2);
            break;
        case 'Z':
            res = (u1 *) allocMemory(8 + i * 2, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            memcpy(res, "boolean", 7);
            if (i)
                for (j = 0; j < i; j++)
                    memcpy(res + 7 + 2 * j, "[]", 2);
            break;
        case 'L':
            j = i;
            // calculate class name length
            for (; i < len;)
                if (str[++i] == ';')
                    break;
            len = i - (j + 1);
            res = (u1 *) allocMemory(len + j * 2 + 1, sizeof (u1));
            if (!res)
                return (u1 *) 0;
            for (i = 0; i < len; i++)
            {
                res[i] = *(str + j + 1 + i);
                if (res[i] == '/')
                    res[i] = '.';
            }
            for (i = 0; i < j; i++)
                memcpy(res + len + 2 * i, "[]", 2);
            break;
        default:
            logError("Assertion error: Unknown token [%c]!\r\n", str[i]);
            return (u1 *) 0;
    }
    
    return res;
}

static int
loadConstantPool(struct BufferIO *input, ClassFile *cf)
{
    u2 i;
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
            if (ru1(&(info->tag), input) < 0)
            {
                logError("IO exception in function %s!\r\n", __func__);
                return -1;
            }
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
                    cfi = (CONSTANT_Fieldref_info *) 0;
                    cci = (CONSTANT_Class_info *) 0;
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
                    cidi = (CONSTANT_InvokeDynamic_info *) 0;
                    break;
                default:
                    logError("Unknown TAG:%X\r\n", info->tag);
                    return -1;
            }
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
validateFields(ClassFile *cf)
{
    u2 i;
    field_info *field;
    u2 flags;
    u1 is_public, is_protected, is_private;
    u1 is_final, is_volatile;
    CONSTANT_Utf8_info *cui;
    
    for (i = 0; i < cf->fields_count; i++)
    {
        field = &(cf->fields[i]);
        
        // validate field access flags
        flags = field->access_flags;
        if (flags & ~ACC_FIELD)
        {
            logError("Unknown flags [0x%X] detected @ cf->fields[%i]!\r\n",
                    flags & ~ACC_FIELD, i);
            return -1;
        }
        
        is_public = (flags & ACC_PUBLIC) ? 1 : 0;
        is_protected = (flags & ACC_PROTECTED) ? 1 : 0;
        is_private = (flags & ACC_PRIVATE) ? 1 : 0;
        is_final = (flags & ACC_FINAL) ? 1 : 0;
        is_volatile = (flags & ACC_VOLATILE) ? 1 : 0;
        
        if (is_public && is_protected
                || is_public && is_private
                || is_protected && is_private)
        {
            logError("Fields can't be PUBLIC, PROTECTED and PRIVATE simultaneously!\r\n");
            return -1;
        }
        if (is_final && is_volatile)
        {
            logError("Fields can't be FINAL and VOLATILE simultaneously!\r\n");
            return -1;
        }
        if (cf->access_flags & ACC_INTERFACE)
        {
            if (!is_public || !(flags & ACC_STATIC) || !is_final)
            {
                logError("Fields should be PUBLIC STATIC FINAL!\r\n");
                return -1;
            }
            if (flags & ~(ACC_PUBLIC | ACC_STATIC | ACC_FINAL | ACC_SYNTHETIC))
            {
                logError("Interface field has invalid access flags!\r\n");
                return -1;
            }
        }
        else if (cf->access_flags & ACC_ENUM)
        {
            if (!is_public || !(flags & ACC_STATIC) || !is_final || !(flags & ACC_ENUM))
            {
                logError("Fields should be PUBLIC STATIC FINAL!\r\n");
                return -1;
            }
        }
        
        // validate field name
        cui = getConstant_Utf8(cf, field->name_index);
        if (!cui)
            return -1;
        
        // validate field descriptor
        cui = getConstant_Utf8(cf, field->descriptor_index);
        if (!cui)
            return -1;
        if (validateFieldDescriptor(cui->data->length, cui->data->bytes) < 0)
        {
            logError("Invalid name \"%.*s\" detected @ cf->fields[%i]!\r\n",
                    cui->data->length, cui->data->bytes, i);
            return -1;
        }
    }
    
    return 0;
}

static int
validateFieldDescriptor(u2 len, u1 *str)
{
    u2 i;
    
    logInfo("Validating FIELD descriptor \"%i: %.*s\"...\r\n",
            len, len, str);
    if (len == 1)
    {
        switch (str[0])
        {
            case 'B':case 'C':case 'D':case 'F':
            case 'I':case 'J':case 'S':case 'Z':
                return 0;
            default:
                logError("Invalid field descriptor \"%.*s\"!\r\n",
                        len, str);
                return -1;
        }
    }
    else if (len > 1)
    {
        switch (str[0])
        {
            case '[':
                for (i = 0; i < len;)
                    if (str[++i] != '[')
                        break;
                if (i > 255)
                {
                    logError("Invalid array dimension: %i!\r\n", i);
                    return -1;
                }
                return validateFieldDescriptor(len - i, str + i);
            default:
                if (str[0] != 'L')
                {
                    logError("Binary name doesn't begin with 'L'!\r\n");
                    return -1;
                }
                if (str[len - 1] != ';')
                {
                    logError("Binary name doesn't end with ';'!\r\n");
                    return -1;
                }
                return 0;
        }
    }
    return -1;
}

static int
validateMethods(ClassFile *cf)
{
    u2 i, flags;
    method_info *method;
    u1 is_public, is_protected, is_private;
    u1 is_final, is_abstract;
    CONSTANT_Utf8_info *cui;
    
    for (i = 0; i < cf->methods_count; i++)
    {
        method = &(cf->methods[i]);
        
        // validate field access flags
        flags = method->access_flags;
        if (flags & ~ACC_METHOD)
        {
            logError("Unknown flags [0x%X] detected @ cf->methods[%i]!\r\n",
                    flags & ~ACC_METHOD, i);
            return -1;
        }
        
        is_public = (flags & ACC_PUBLIC) ? 1 : 0;
        is_protected = (flags & ACC_PROTECTED) ? 1 : 0;
        is_private = (flags & ACC_PRIVATE) ? 1 : 0;
        is_final = (flags & ACC_FINAL) ? 1 : 0;
        is_abstract = (flags & ACC_ABSTRACT) ? 1 : 0;
        
        if (is_public && is_protected
                || is_public && is_private
                || is_protected && is_private)
        {
            logError("Methods can't be PUBLIC, PROTECTED and PRIVATE simultaneously!\r\n");
            return -1;
        }
        if (cf->access_flags & ACC_INTERFACE)
        {
            if (flags & (ACC_PROTECTED | ACC_FINAL | ACC_SYNCHRONIZED | ACC_NATIVE))
            {
                logError("Interface method has invalid access flags!\r\n");
                return -1;
            }
#if VER_CMP(52, 0)
            if (!is_public && !is_private)
            {
                logError("Interface method should either be PUBLIC or PRIVATE!\r\n");
                return -1;
            }
#else
            if (!is_public)
            {
                logError("Interface method is not public!\r\n");
                return -1;
            }
            if (!is_abstract)
            {
                logError("Interface method is not abstract!\r\n");
                return -1;
            }
#endif
        }
        if (is_abstract)
        {
            if (flags & (ACC_PRIVATE | ACC_STATIC | ACC_FINAL
                    | ACC_SYNCHRONIZED | ACC_NATIVE | ACC_STRICT))
            {
                logError("Abstract method can't be PRIVATE, STATIC, FINAL, SYNCHRONIZED, NATIE or STRICT!\r\n");
                return -1;
            }
        }
        
        // validate method name
        cui = getConstant_Utf8(cf, method->name_index);
        if (!cui)
            return -1;
        if (!strncmp((char *) cui->data->bytes, "<init>", cui->data->length))
        {
            if (flags & ~(ACC_PUBLIC | ACC_PROTECTED | ACC_PRIVATE | ACC_VARARGS | ACC_STRICT | ACC_SYNTHETIC))
            {
                logError("Initialization method has invalid access flags!\r\n");
                return -1;
            }
        }
        
        // validate method descriptor
        cui = getConstant_Utf8(cf, method->descriptor_index);
        if (!cui)
            return -1;
        if (validateMethodDescriptor(cui->data->length, cui->data->bytes) < 0)
        {
            logError("Invalid name \"%.*s\" detected @ cf->methods[%i]!\r\n",
                    cui->data->length, cui->data->bytes, i);
            return -1;
        }
    }
}

static int
validateMethodDescriptor(u2 len, u1 *str)
{
    u2 i, j, count;

    logInfo("Validating METHOD descriptor \"%i: %.*s\"...\r\n",
            len, len, str);
    if (str[0] != '(')
        return -1;
    i = j = 1;
    for (; i < len; i++)
    {
        if (str[i] == ';' || str[i] == ')')
        {
            if (i - j > 0 &&
                    validateFieldDescriptor(i - j + 1, str + j) < 0)
                return -1;
            j = i + 1;
        }
        if (str[i] == ')')
            break;
    }
ret:
    ++i;
    len -= i;
    str += i;
    logInfo("Validating RETURN descriptor \"%i: %.*s\"...\r\n",
            len, len, str);
    if (len == 1)
        switch (str[0])
        {
            case 'B':case 'C':case 'D':
            case 'F':case 'I':case 'J':
            case 'S':case 'Z':case 'V':
                return 0;
            default:
                return -1;
        }
    else if (len > 1)
        switch (str[0])
        {
            case '[':
                for (i = 0; i < len;)
                    if (str[++i] != '[')
                        break;
                if (i > 255)
                    return -1;
                return validateFieldDescriptor(len - i, str + i);
            default:
                if (str[0] != 'L')
                    return -1;
                if (str[len - 1] != ';')
                    return -1;
                return 0;
        }
    return -1;
}

static int
validateAttributes_class(ClassFile *cf, u2 len, attr_info *attributes)
{
    u2 i, j;
    attr_info *attribute;
    u4 attribute_length;
#if VER_CMP(45, 3)
    attr_SourceFile_info *asf;
    attr_InnerClasses_info *aic;
    attr_Synthetic_info *asyn;
    attr_Deprecated_info *ad;
#endif
#if VER_CMP(49, 0)
    attr_EnclosingMethod_info *aem;
    //attr_SourceDebugExtension_info *asde;
    attr_Signature_info *asig;
    attr_RuntimeVisibleAnnotations_info *arva;
    attr_RuntimeInvisibleAnnotations_info *aria;
#endif
#if VER_CMP(51, 0)
    attr_BootstrapMethods_info *abm;
#endif
#if VER_CMP(52, 0)
    attr_RuntimeVisibleTypeAnnotations_info *arvta;
    attr_RuntimeInvisibleTypeAnnotations_info *arita;
#endif
    CONSTANT_Utf8_info *descriptor;
    CONSTANT_Class_info *cci;
    CONSTANT_Utf8_info *cui;
    CONSTANT_NameAndType_info *cni;
    struct classes_entry *ce;
    u2 flags;

    for (i = 0; i < len; i++)
    {
        attribute = &(attributes[i]);
        switch (attribute->tag)
        {
#if VER_CMP(45, 3)
            case TAG_ATTR_SOURCEFILE:
                break;
            case TAG_ATTR_INNERCLASSES:
                aic = (attr_InnerClasses_info *) attribute->data;
                attribute_length = sizeof (aic->number_of_classes)
                    + sizeof (struct classes_entry)
                    * aic->number_of_classes;
                if (attribute_length != attribute->attribute_length)
                    return -1;
                for (j = 0; j < aic->number_of_classes; j++)
                {
                    ce = &(aic->classes[j]);
                    cci = getConstant_Class(cf, ce->inner_class_info_index);
                    if (!cci)
                        return -1;
                    // If C is not a member of a class or an interface
                    // (that is, if C is a top-level class or interface
                    // or a local class or an anonymous class),
                    // the value of the outer_class_info_index item
                    // must be zero
                    if (ce->outer_class_info_index != 0)
                    {
                        cci = getConstant_Class(cf, ce->outer_class_info_index);
                        if (!cci)
                            return -1;
                    }
                    // If C is anonymous, the value of the inner_name_index
                    // item must be zero
                    if (ce->inner_name_index != 0)
                    {
                        // original simple name of C given in the source
                        cui = getConstant_Utf8(cf, ce->inner_name_index);
                        if (!cui)
                            return -1;
                    }
                    flags = ce->inner_class_access_flags;
#if VER_CMP(51, 0)
                    if (ce->inner_name_index == 0
                            && ce->outer_class_info_index != 0)
                        return -1;
#endif
                    // Oracle's Java Virtual Machine implementation
                    // does not check the consistency of an InnerClasses
                    // attribute against a class file representing a class
                    // or interface referenced by the attribute
                }
                break;
            case TAG_ATTR_SYNTHETIC:
                if (attribute->attribute_length != 0)
                    return -1;
                break;
            case TAG_ATTR_DEPRECATED:
                break;
#endif
#if VER_CMP(49, 0)
            case TAG_ATTR_ENCLOSINGMETHOD:
                aem = (attr_EnclosingMethod_info *) attribute->data;
                attribute_length = sizeof (aem->class_index)
                    + sizeof (aem->method_index);
                if (attribute_length != attribute->attribute_length)
                    return -1;
                cci = getConstant_Class(cf, aem->class_index);
                if (!cci)
                    return -1;
                // If the current class is not immediately enclosed by
                // a method or a constructor, then the value of the
                // method_index item must be zero
                // In particular, method_index must be zero if the current
                // class was immediately enclosed in source code by an
                // instance initializer, static initializer, instance
                // variable initializer, or class variable initializer.
                // (The first two concern both local classes and anonymous
                // classes, while the last two concern anonymous classes
                // declared on the right hand side of a field assignment)
                if (aem->method_index != 0)
                {
                    cni = getConstant_NameAndType(cf, aem->method_index);
                    if (!cni)
                        return -1;
                }
                break;
            case TAG_ATTR_SOURCEDEBUGEXTENSION:
                break;
            case TAG_ATTR_SIGNATURE: // class
                asig = (attr_Signature_info *) attribute->data;
                attribute_length = sizeof (asig->signature_index);
                if (attribute_length != attribute->attribute_length)
                    return -1;
                cui = getConstant_Utf8(cf, asig->signature_index);
                if (!cui)
                    return -1;
                break;
            case TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS:
                break;
#endif
#if VER_CMP(51, 0)
            case TAG_ATTR_BOOTSTRAPMETHODS:
                break;
#endif
#if VER_CMP(52, 0)
            case TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS:
                break;
#endif
            default:
                return -1;
        }
    }
    
    return 0;
}

static int
validateAttributes_field(ClassFile *cf, field_info *field)
{
    u2 i;
    attr_info *attributes, *attribute;
    u4 attribute_length;
#if VER_CMP(45, 3)
    attr_ConstantValue_info *acv;
    attr_Synthetic_info *asyn;
    attr_Deprecated_info *ad;
#endif
#if VER_CMP(49, 0)
    attr_Signature_info *asig;
    attr_RuntimeVisibleAnnotations_info *arva;
    attr_RuntimeInvisibleAnnotations_info *aria;
#endif
#if VER_CMP(52, 0)
    attr_RuntimeVisibleTypeAnnotations_info *arvta;
    attr_RuntimeInvisibleTypeAnnotations_info *arita;
#endif
    CONSTANT_Utf8_info *descriptor;
    CONSTANT_Utf8_info *cui;

    attributes = field->attributes;
    for (i = 0; i < field->attributes_count; i++)
    {
        attribute = &(attributes[i]);
        switch (attribute->tag)
        {
#if VER_CMP(45, 3)
            case TAG_ATTR_CONSTANTVALUE:
                acv = (attr_ConstantValue_info *) attribute->data;
                if (attribute->attribute_length
                        != sizeof (acv->constantvalue_index))
                    return -1;
                descriptor = getConstant_Utf8(cf, field->descriptor_index);
                if (!descriptor)
                    return -1;
                switch (descriptor->data->bytes[0])
                {
                    case 'J':
                        if (!getConstant_Long(cf, acv->constantvalue_index))
                            return -1;
                        break;
                    case 'F':
                        if (!getConstant_Float(cf, acv->constantvalue_index))
                            return -1;
                        break;
                    case 'D':
                        if (!getConstant_Double(cf, acv->constantvalue_index))
                            return -1;
                        break;
                    case 'I':case 'S':case 'C':case 'B':case 'Z':
                        if (!getConstant_Integer(cf, acv->constantvalue_index))
                            return -1;
                        break;
                    case 'L':
                        if (!strncmp((char *) descriptor->data->bytes,
                                    "Ljava/lang/String;",
                                    descriptor->data->length))
                            if (!getConstant_String(cf, acv->constantvalue_index))
                                return -1;
                        break;
                    default:
                        return -1;
                }
                break;
            case TAG_ATTR_SYNTHETIC:
                if (attribute->attribute_length != 0)
                    return -1;
                break;
            case TAG_ATTR_DEPRECATED:
                break;
#endif
#if VER_CMP(49, 0)
            case TAG_ATTR_SIGNATURE: // field
                asig = (attr_Signature_info *) attribute->data;
                attribute_length = sizeof (asig->signature_index);
                if (attribute_length != attribute->attribute_length)
                    return -1;
                cui = getConstant_Utf8(cf, asig->signature_index);
                if (!cui)
                    return -1;
                break;
            case TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS:
                break;
#endif
#if VER_CMP(52, 0)
            case TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS:
                break;
#endif
            default:
                return -1;
        }
    }
    
    return 0;
}

static int
validateAttributes_method(ClassFile *cf, method_info *method)
{
    u2 i, j;
    attr_info *attributes, *attribute;
    u4 attribute_length;
#if VER_CMP(45, 3)
    attr_Code_info *ac;
    attr_Exceptions_info *ae;
    attr_Synthetic_info *asyn;
    attr_Deprecated_info *ad;
#endif
#if VER_CMP(49, 0)
    attr_RuntimeVisibleParameterAnnotations_info *arvpa;
    attr_RuntimeInvisibleParameterAnnotations_info *aripa;
    attr_AnnotationDefault_info *aad;
    attr_Signature_info *asig;
    attr_RuntimeVisibleAnnotations_info *arva;
    attr_RuntimeInvisibleAnnotations_info *aria;
#endif
#if VER_CMP(52, 0)
    attr_MethodParameters_info *amp;
    attr_RuntimeVisibleTypeAnnotations_info *arvta;
    attr_RuntimeInvisibleTypeAnnotations_info *arita;
#endif
    CONSTANT_Utf8_info *descriptor;
    CONSTANT_Utf8_info *cui;
    CONSTANT_Class_info *cci;
    struct exception_table_entry *ete;

    attributes = method->attributes;
    for (i = 0; i < method->attributes_count; i++)
    {
        attribute = &(attributes[i]);
        switch (attribute->tag)
        {
#if VER_CMP(45, 3)
            case TAG_ATTR_CODE:
                if (method->access_flags & (ACC_NATIVE | ACC_ABSTRACT))
                    return -1;
                ac = (attr_Code_info *) attribute->data;
                // valdiate attributes of Code attribute
                if (validateAttributes_code(cf, ac) < 0)
                    return -1;
                // sum up length of attributes of Code attribute
                // and compare it with attribute_length of Code attribute
                attribute_length = sizeof (ac->max_stack)
                    + sizeof (ac->max_locals)
                    + sizeof (ac->code_length)
                    + sizeof (*(ac->code)) * ac->code_length
                    + sizeof (ac->exception_table_length)
                    + sizeof (struct exception_table_entry) * ac->exception_table_length
                    + sizeof (ac->attributes_count);
                for (j = 0; j < ac->attributes_count; j++)
                {
                    attribute_length += sizeof (ac->attributes[j].attribute_length);
                    attribute_length += ac->attributes[j].attribute_length;
                }
                if (attribute_length != attribute->attribute_length)
                    return -1;
                // @see 4.9
                // The detailed constrains on the contents of code array

                // validate exception table
                for (j = 0; j < ac->exception_table_length; j++)
                {
                    ete = &(ac->exception_table[j]);
                    if (ete->start_pc < 0
                            || ete->start_pc >= ac->code_length)
                        return -1;
                    if (ete->end_pc <= ete->start_pc
                            || ete->end_pc > ac->code_length)
                        return -1;
                    if (ete->handler_pc < 0
                            || ete->handler_pc >= ac->code_length)
                        return -1;
                    if (ete->catch_type != 0)
                    {
                        cci = getConstant_Class(cf, ete->catch_type);
                        if (!cci)
                            return -1;
                        // TODO load rt.jar and check if the class
                        // represented by 'cci' extends any exception class
                        // HINT: might need algorithm quick union
                        // or quick find
                    }
                }
                break;
            case TAG_ATTR_EXCEPTIONS:
                ae = (attr_Exceptions_info *) attribute->data;
                attribute_length = sizeof (ae->number_of_exceptions)
                    + sizeof (*(ae->exception_index_table))
                    * ae->number_of_exceptions;
                if (attribute_length != attribute->attribute_length)
                    return -1;
                for (j = 0; j < ae->number_of_exceptions; j++)
                {
                    cci = getConstant_Class(cf, ae->exception_index_table[j]);
                    if (!cci)
                        return -1;
                    // TODO load rt.jar and check if the class
                    // represented by 'cci' extends any exception class
                }
                break;
            case TAG_ATTR_SYNTHETIC:
                if (attribute->attribute_length != 0)
                    return -1;
                break;
            case TAG_ATTR_DEPRECATED:
                break;
#endif
#if VER_CMP(49, 0)
            case TAG_ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS:
                break;
            case TAG_ATTR_ANNOTATIONDEFAULT:
                break;
            case TAG_ATTR_SIGNATURE: // method
                asig = (attr_Signature_info *) attribute->data;
                attribute_length = sizeof (asig->signature_index);
                if (attribute_length != attribute->attribute_length)
                    return -1;
                cui = getConstant_Utf8(cf, asig->signature_index);
                if (!cui)
                    return -1;
                break;
            case TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS:
                break;
#endif
#if VER_CMP(52, 0)
            case TAG_ATTR_METHODPARAMETERS:
                break;
            case TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS:
                break;
#endif
            default:
                return -1;
        }
    }
    
    return 0;
}

static int
validateAttributes_code(ClassFile *cf, attr_Code_info *code)
{
    u2 i, j;
    attr_info *attributes, *attribute;
    u4 attribute_length;
#if VER_CMP(45, 3)
    attr_LineNumberTable_info *alnt;
    attr_LocalVariableTable_info *alvt;
#endif
#if VER_CMP(49, 0)
    attr_LocalVariableTypeTable_info *alvtt;
#endif
#if VER_CMP(50, 0)
    attr_StackMapTable_info *asmt;
#endif
#if VER_CMP(52, 0)
    attr_RuntimeVisibleTypeAnnotations_info *arvta;
    attr_RuntimeInvisibleTypeAnnotations_info *arita;
#endif
    CONSTANT_Utf8_info *descriptor;
    union stack_map_frame *frame;
    u1 frame_type;

    attributes = code->attributes;
    for (i = 0; i < code->attributes_count; i++)
    {
        attribute = &(attributes[i]);
        switch (attribute->tag)
        {
#if VER_CMP(45, 3)
            case TAG_ATTR_LINENUMBERTABLE:
                break;
            case TAG_ATTR_LOCALVARIABLETABLE:
                break;
#endif
#if VER_CMP(49, 0)
            case TAG_ATTR_LOCALVARIABLETYPETABLE:
                break;
#endif
#if VER_CMP(50, 0)
            case TAG_ATTR_STACKMAPTABLE:
                asmt = (attr_StackMapTable_info *) attribute->data;
                attribute_length = sizeof (asmt->number_of_entries);
                for (j = 0; j < asmt->number_of_entries; j++)
                {
                    frame = &(asmt->entries[j]);
                    frame_type = frame->same_frame.frame_type;
                    // TODO when should I import StackMapTable validation?
                    // same frame
                    if (frame_type >= SMF_SAME_MIN
                            && frame_type <= SMF_SAME_MAX)
                    {
                        attribute_length += sizeof (frame->same_frame);
                    }
                    // same_locals_1_stack_item_frame
                    else if (frame_type >= SMF_SL1SI_MIN
                            && frame_type <= SMF_SL1SI_MAX)
                    {
                        attribute_length +=
                            sizeof (frame->same_locals_1_stack_item_frame);
                    }
                    else if (frame_type == SMF_SL1SIE)
                    {
                        attribute_length +=
                            sizeof (frame->same_locals_1_stack_item_frame_extended);
                    }
                    else if (frame_type >= SMF_CHOP_MIN
                            && frame_type <= SMF_CHOP_MAX)
                    {
                        attribute_length +=
                            sizeof (frame->chop_frame);
                    }
                    else if (frame_type == SMF_SAMEE)
                    {
                        attribute_length +=
                            sizeof (frame->same_frame_extended);
                    }
                    else if (frame_type >= SMF_APPEND_MIN
                            && frame_type <= SMF_APPEND_MAX)
                    {
                        attribute_length += (sizeof (frame->append_frame.frame_type)
                                + sizeof (frame->append_frame.offset_delta) 
                                + sizeof (union verification_type_info)
                                * (frame_type - 251));
                    }
                    else if (frame_type == SMF_FULL)
                    {
                        attribute_length += (sizeof (frame->full_frame.frame_type)
                                + sizeof (frame->full_frame.offset_delta)
                                + sizeof (frame->full_frame.number_of_locals)
                                + sizeof (union verification_type_info)
                                * frame->full_frame.number_of_locals
                                + sizeof (frame->full_frame.number_of_stack_items)
                                + sizeof (union verification_type_info)
                                * frame->full_frame.number_of_stack_items);
                    }
                }
                if (attribute_length != attribute->attribute_length)
                    return -1;
                break;
#endif
#if VER_CMP(52, 0)
            case TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS:
                break;
            case TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS:
                break;
#endif
            default:
                return -1;
        }
    }
    
    return 0;
}

// @see jvms8:p121
static int
validateJavaTypeSignature(u2 len, u1 *str)
{
    u2 i, j;

    if (len == 1)
    {
        switch (str[0])
        {
            case 'B':case 'C':case 'D':case 'F':
            case 'I':case 'J':case 'S':case 'Z':
                return 0;
            default:
                return -1;
        }
    }
    else if (len > 1)
    {
        switch (str[0])
        {
            case 'L':
                break;
        }
    }
    return -1;
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
        rt_Class_info *class_info)
{
    rt_Utf8_info *class_name;
    u2 class_name_index;

    class_name_index = class_info->data->name_index;
    class_name = rtc->getConstant_Utf8(class_name_index);

    return writeClassName0(out,
            class_name->data->length,
            class_name->data->bytes);
}

static int
logClassHeader(rt_Class *rtc)
{
#if (defined DEBUG && defined LOG_INFO)
    char buf[1024], *ptr;
    size_t n, m;
    u2 access_flags;
    rt_Class_info * this_class, * super_class;
    u2 interfaces_count;
    rt_Class_info ** interfaces;
    rt_Utf8_info *cui;
    u2 i;

    access_flags = rtc->getAccessFlags();
    this_class = rtc->getThisClass();
    super_class = rtc->getSuperClass();
    interfaces_count = rtc->getInterfacesCount();
    interfaces = (rt_Class_info **) NULL;
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
        cui = rtc->getConstant_Utf8(super_class->data->name_index);
        if (!cui) goto error;
        if (strncmp("java/lang/Object",
                    (char *) cui->data->bytes,
                    cui->data->length))
        {
            n = sprintf(ptr, "extends ");
            if (n < 0) goto error;
            ptr += n;
            n = writeClassName0(ptr,
                    cui->data->length,
                    cui->data->bytes);
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
    interfaces = (rt_Class_info **) 0;
    return 0;
error:
    freeMemory(interfaces);
    interfaces = (rt_Class_info **) 0;
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
    CONSTANT_Utf8_info *name, *descriptor;
    attr_ConstantValue_info *acv;
    u2 constantvalue_index;
    rt_Long_info *cv_long;
    rt_Float_info *cv_float;
    rt_Double_info *cv_double;
    rt_Integer_info *cv_integer;
    rt_String_info *cv_string;
    rt_Utf8_info *cui;

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
                    descriptor->data->length,
                    descriptor->data->bytes);
            if (n < 0) return -1;
            ptr += n;
        }

        n = sprintf(ptr, " %.*s",
                name->data->length,
                name->data->bytes);
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
                switch (descriptor->data->bytes[0])
                {
                    case 'J':
                        cv_long = rtc->getConstant_Long(
                                constantvalue_index);
                        n = sprintf(ptr, "%llil",
                                cv_long->data->long_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'F':
                        cv_float = rtc->getConstant_Float(
                                constantvalue_index);
                        n = sprintf(ptr, "%ff",
                                cv_float->data->float_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'D':
                        cv_double = rtc->getConstant_Double(
                                constantvalue_index);
                        n = sprintf(ptr, "%fd",
                                cv_double->data->double_value);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    case 'L':
                        cv_string = rtc->getConstant_String(
                                constantvalue_index);
                        cui = rtc->getConstant_Utf8(
                                cv_string->data->string_index);
                        n = writeConstantString(ptr,
                                cui->data->length,
                                cui->data->bytes);
                        if (n < 0) return -1;
                        ptr += n;
                        break;
                    default:
                        cv_integer = rtc->getConstant_Integer(
                                constantvalue_index);
                        n = sprintf(ptr, "%i",
                                cv_integer->data->bytes);
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

// FIXME
static int
writeParameterTable(char *out,
        rt_Method *method,
        u2 len, u1 *str,
        struct ParameterTable *pt)
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
    memset(pt, 0, sizeof (struct ParameterTable));

    // instance methods start with 'this' parameter
    access_flags = method->getAccessFlags();
    if (access_flags & ACC_STATIC)
        count = 0;
    else
        count = 1;

    pt->local_count = count;
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

        pt->local_count += parameter_type;
    }
    pt->local_names = (char **)
        allocMemory(pt->local_count, sizeof (char *));


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
writeCode(char *out, ClassFile *cf,
        method_info *method,
        attr_Code_info *info,
        struct ParameterTable *pt)
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
    rt_Utf8_info *class_name;
    rt_Utf8_info *name, *descriptor;
    attr_Code_info *code;
    attr_Exceptions_info *exceptions;
    u2 number_of_exceptions;
    u2 *exception_index_table;
    int has_method_body;
    rt_Class_info *cci;
    rt_Utf8_info *cui;

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
                    (char *) name->data->bytes,
                    name->data->length) != 0)
        {
            if (strncmp("<init>", (char *) name->data->bytes,
                        name->data->length) == 0)
            {
                // class name
                n = writeClassName0(ptr,
                        class_name->data->length,
                        class_name->data->bytes);
                if (n < 0) return -1;
                ptr += n;
            }
            else
            {
                // return type
                for (j = descriptor->data->length - 1;
                        j > 0; j--)
                {
                    if (descriptor->data->bytes[j] != ')')
                        continue;

                    ++j;
                    n = writeFieldDescriptor(ptr,
                            descriptor->data->length - j,
                            descriptor->data->bytes + j);
                    if (n < 0) return -1;
                    ptr += n;
                    n = sprintf(ptr, " ");
                    if (n < 0) return -1;
                    ptr += n;

                    break;
                }

                // method name
                n = sprintf(ptr, "%.*s",
                        name->data->length,
                        name->data->bytes);
                if (n < 0) return -1;
                ptr += n;
            }

            // parameter table
            // FIXME
            n = writeParameterTable(ptr,
                    method,
                    descriptor->data->length,
                    descriptor->data->bytes);
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

                // FIXME
                n = writeClassName(ptr, cf,
                        exception_index_table[j]);
                if (n < 0) return -1;
                ptr += n;
            }
        }

        if (code)
        {
            // FIXME
            n = writeCode(ptr, cf, method, code);
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
