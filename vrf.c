#include <string.h>

#include "java.h"
#include "vrf.h"
#include "log.h"

static int validateConstantPoolEntry(ClassFile *, u2, u1 *, u1);
static int validateFieldDescriptor(u2, u1 *);
static int validateMethodDescriptor(u2, u1 *);
static int validateAttributes_class(ClassFile *, u2, attr_info *);
static int validateAttributes_field(ClassFile *, field_info *);
static int validateAttributes_method(ClassFile *, method_info *);
static int validateAttributes_code(ClassFile *, attr_Code_info *);
static int validateJavaTypeSignature(u2, u1 *);

extern int
checkMagic(u4 magic)
{
    if (magic == MAGIC_ORACLE)
        return 0;

    logError("File structure invalid,"
            " fail to decompile! [0x%X]\r\n",
            magic);
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
            if (validateConstantPoolEntry(cf,
                        cci->data->name_index,
                        bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
        case CONSTANT_Fieldref:
        case CONSTANT_Methodref:
        case CONSTANT_InterfaceMethodref:
            cfi = (CONSTANT_Fieldref_info *) info;
            if (validateConstantPoolEntry(cf,
                        cfi->data->class_index, bul, CONSTANT_Class) < 0)
                return -1;
            if (validateConstantPoolEntry(cf,
                        cfi->data->name_and_type_index,
                        bul, CONSTANT_NameAndType) < 0)
                return -1;
            cni = (CONSTANT_NameAndType_info *)
                getConstant(cf, cfi->data->name_and_type_index);
            cui = (CONSTANT_Utf8_info *)
                getConstant(cf, cni->data->descriptor_index);
            if (info->tag == CONSTANT_Fieldref)
            {
                if (validateFieldDescriptor(cui->data->length,
                            cui->data->bytes) < 0)
                    return -1;
            }
            else
            {
                if (info->tag == CONSTANT_Methodref
                        && cui->data->bytes[0] == '<')
                {
                    // return type must be void
                    for (j = 0; j < cui->data->length;)
                        if (cui->data->bytes[j++] == ')')
                            break;
                    if (cui->data->bytes[j++] != 'V'
                            || cui->data->length != j)
                        return -1;
                    // special method name for constructors
                    cui = (CONSTANT_Utf8_info *)
                        getConstant(cf, cni->data->name_index);
                    if (strncmp((char *) cui->data->bytes,
                                "<init>", cui->data->length)
                            && strncmp((char *) cui->data->bytes,
                                "<clinit>", cui->data->length))
                        return -1;
                    break;
                }
                if (validateMethodDescriptor(cui->data->length,
                        cui->data->bytes) < 0)
                    return -1;
            }
            break;
        case CONSTANT_String:
            csi = (CONSTANT_String_info *) info;
            if (validateConstantPoolEntry(cf,
                        csi->data->string_index,
                        bul, CONSTANT_Utf8) < 0)
                return -1;
            break;
        case CONSTANT_Long:
        case CONSTANT_Double:
            bul[i + 1] = 1;
            break;
        case CONSTANT_NameAndType:
            cni = (CONSTANT_NameAndType_info *) info;
            if (validateConstantPoolEntry(cf,
                        cni->data->name_index,
                        bul, CONSTANT_Utf8) < 0)
                return -1;
            if (validateConstantPoolEntry(cf,
                        cni->data->descriptor_index,
                        bul, CONSTANT_Utf8) < 0)
                return -1;
            // `CONSTANT_NameAndType_info` is never used directly
            // which means Cruiser validates its items in:
            //      `CONSTANT_Fieldref_info`,
            //      `CONSTANT_Methodref_info`,
            //      and `CONSTANT_InterfaceMethodref_info`
            break;
        case CONSTANT_Utf8:
            cui = (CONSTANT_Utf8_info *) info;
            if (!cui->data || !cui->data->bytes)
            {
                logError("Invalid CONSTANT_Utf8_info!\r\n");
                return -1;
            }
            for (j = 0; j < cui->data->length; j++)
                if (cui->data->bytes[j] == 0
                        || cui->data->bytes[j] >= 0xf0
                        && cui->data->bytes[j] <= 0xff)
                    return -1;
            break;
        case CONSTANT_MethodHandle:
            cmhi = (CONSTANT_MethodHandle_info *) info;
            switch (cmhi->data->reference_kind)
            {
                case REF_getField:
                case REF_getStatic:
                case REF_putField:
                case REF_putStatic:
                    if (validateConstantPoolEntry(cf,
                                cmhi->data->reference_index,
                                bul, CONSTANT_Fieldref) < 0)
                        return -1;
                    break;
                case REF_invokeVirtual:
                case REF_newInvokeSpecial:
                    if (validateConstantPoolEntry(cf,
                                cmhi->data->reference_index,
                                bul, CONSTANT_Methodref) < 0)
                        return -1;
                case REF_invokeStatic:
                case REF_invokeSpecial:
                    if (compareVersion0(cf->major_version,
                                cf->minor_version,
                                52, 0) >= 0)
                    {
                        if (validateConstantPoolEntry(cf,
                                    cmhi->data->reference_index,
                                    bul, CONSTANT_Methodref) < 0
                                && validateConstantPoolEntry(cf,
                                    cmhi->data->reference_index,
                                    bul, CONSTANT_InterfaceMethodref) < 0)
                        return -1;
                    }
                    else if (validateConstantPoolEntry(cf,
                                cmhi->data->reference_index,
                                bul, CONSTANT_Methodref) < 0)
                        return -1;
                    break;
                case REF_invokeInterface:
                    if (validateConstantPoolEntry(cf,
                                cmhi->data->reference_index,
                                bul, CONSTANT_InterfaceMethodref) < 0)
                        return -1;
                    break;
                default:
                    logError("Constant pool entry[%i] has invalid reference kind[%i] as CONSTANT_MethodHandle_info!\r\n",
                            i, cmhi->data->reference_kind);
                    return -1;
            }
            cfi = (CONSTANT_Fieldref_info *)
                &(cf->constant_pool[cmhi->data->reference_index]);
            cni = (CONSTANT_NameAndType_info *)
                &(cf->constant_pool[cfi->data->name_and_type_index]);
            cui = (CONSTANT_Utf8_info *)
                &(cf->constant_pool[cni->data->name_index]);
            switch (cmhi->data->reference_kind)
            {
                case REF_invokeVirtual:
                case REF_invokeStatic:
                case REF_invokeSpecial:
                case REF_invokeInterface:
                    if (!strncmp((char *) cui->data->bytes,
                                "<init>", cui->data->length))
                    {
                        logError("Method name '<init>' is invalid "
                                "because MethodHandle reference kind "
                                "is %i!\r\n",
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    else if (!strncmp((char *) cui->data->bytes,
                                "<clinit>", cui->data->length))
                    {
                        logError("Method name '<clinit>' is invalid "
                                "because MethodHandle reference kind "
                                "is %i!\r\n",
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    break;
                case REF_newInvokeSpecial:
                    if (strncmp((char *) cui->data->bytes,
                                "<init>", cui->data->length))
                    {
                        logError("Method name '%.*s' is invalid "
                                "because MethodHandle reference kind "
                                "is %i!\r\n",
                                cui->data->length,
                                (char *) cui->data->bytes,
                                cmhi->data->reference_kind);
                        return -1;
                    }
                    break;
            }
            break;
        case CONSTANT_MethodType:
            cmti = (CONSTANT_MethodType_info *) info;
            if (validateConstantPoolEntry(cf,
                        cmti->data->descriptor_index,
                        bul, CONSTANT_Utf8) < 0)
                return -1;
            cui = (CONSTANT_Utf8_info *)
                getConstant(cf, cmti->data->descriptor_index);
            if (validateMethodDescriptor(cui->data->length,
                        cui->data->bytes) < 0)
                return -1;
            break;
#if VER_CMP(51, 0)
        case CONSTANT_InvokeDynamic:
            cidi = (CONSTANT_InvokeDynamic_info *) info;
            if (validateConstantPoolEntry(cf,
                        cidi->data->name_and_type_index,
                        bul, CONSTANT_NameAndType) < 0)
                return -1;
            cni = (CONSTANT_NameAndType_info *)
                getConstant(cf, cidi->data->name_and_type_index);
            cui = (CONSTANT_Utf8_info *)
                getConstant(cf, cni->data->descriptor_index);
            if (validateMethodDescriptor(cui->data->length,
                        cui->data->bytes) < 0)
                return -1;
            if (!cf->attributes)
            {
                logError("Class attributes ain't loaded!\r\n");
                return -1;
            }
            dataBootstrapMethods =
                (struct attr_BootstrapMethods_info *) 0;
            for (j = 0; j < cf->attributes_count; j++)
            {
                if (cf->attributes[j].tag !=
                        TAG_ATTR_BOOTSTRAPMETHODS)
                    continue;
                dataBootstrapMethods =
                    (struct attr_BootstrapMethods_info *)
                        cf->attributes[j].data;
                break;
            }
            if (!dataBootstrapMethods)
            {
                logError("Attribute BootstrapMethods is not found!\r\n");
                return -1;
            }
            bm = &(dataBootstrapMethods->bootstrap_methods[
                    cidi->data->bootstrap_method_attr_index]);
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
                    &(cf->constant_pool[
                            cmhi->data->reference_index]);
            cni = (CONSTANT_NameAndType_info *)
                    &(cf->constant_pool[
                            cmi->data->name_and_type_index]);
            cui = (CONSTANT_Utf8_info *)
                    &(cf->constant_pool[
                            cni->data->descriptor_index]);
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
                if (validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_String) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_Class) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_Integer) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_Long) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_Float) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_Double) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_MethodHandle) < 0
                        && validateConstantPoolEntry(cf,
                            bm->bootstrap_arguments[j],
                            bul, CONSTANT_MethodType) < 0)
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

extern int
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

extern int
validateFields(ClassFile *cf)
{
    u2 i, j, fields_count;
    field_info *field, *field1;
    u2 flags;
    u1 is_public, is_protected, is_private;
    u1 is_final, is_volatile;
    CONSTANT_Utf8_info *cui, *cui1;
    
    fields_count = cf->fields_count;
    // No two fields in one `class` file may have
    // the same name and descriptor
    for (i = 0; i < fields_count;)
    {
        field = &(cf->fields[i]);
        cui = getConstant_Utf8(cf, field->name_index);
        if (!cui) return -1;
        for (j = ++i; j < fields_count; j++)
        {
            field1 = &(cf->fields[j]);
            cui1 = getConstant_Utf8(cf, field1->name_index);
            if (!cui1) return -1;
            if (cui->data->length != cui1->data->length)
                continue;
            if (strncmp((char *) cui->data->bytes,
                        (char *) cui1->data->bytes,
                        cui->data->length) == 0)
                return -1;
        }
    }

    for (i = 0; i < fields_count; i++)
    {
        field = &(cf->fields[i]);
        
        // validate field access flags
        flags = field->access_flags;
        j = flags & ~ACC_FIELD;
        if (j)
        {
            logError("Unknown flags [0x%X] detected @ "
                    "cf->fields[%i]!\r\n",
                    j, i);
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
            logError("Fields can't be PUBLIC, PROTECTED and "
                    "PRIVATE simultaneously!\r\n");
            return -1;
        }
        if (is_final && is_volatile)
        {
            logError("Fields can't be FINAL and "
                    "VOLATILE simultaneously!\r\n");
            return -1;
        }
        if (cf->access_flags & ACC_INTERFACE)
        {
            if (!is_public || !(flags & ACC_STATIC) || !is_final)
            {
                logError("Fields should be PUBLIC STATIC FINAL!\r\n");
                return -1;
            }
            if (flags & ~(ACC_PUBLIC | ACC_STATIC
                        | ACC_FINAL | ACC_SYNTHETIC))
            {
                logError("Interface field has invalid access flags!\r\n");
                return -1;
            }
        }
        else if (cf->access_flags & ACC_ENUM)
        {
            if (!is_public || !(flags & ACC_STATIC)
                    || !is_final || !(flags & ACC_ENUM))
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
        if (validateFieldDescriptor(cui->data->length,
                    cui->data->bytes) < 0)
        {
            logError("Invalid name \"%.*s\" detected @ "
                    "cf->fields[%i]!\r\n",
                    cui->data->length, cui->data->bytes, i);
            return -1;
        }
    }
    
    return 0;
}

extern int
validateMethods(ClassFile *cf)
{
    u2 i, j, flags;
    method_info *method, *method1;
    u1 is_public, is_protected, is_private;
    u1 is_final, is_abstract;
    CONSTANT_Utf8_info *cui, *cui1, *cui2, *cui3;

    for (i = 0; i < cf->methods_count;)
    {
        method = &(cf->methods[i]);
        cui = getConstant_Utf8(cf, method->name_index);
        if (!cui) return -1;
        cui1 = getConstant_Utf8(cf, method_descriptor_index);
        if (!cui1) return -1;
        for (j = ++i; j < cf->methods_count; j++)
        {
            method1 = &(cf->methods[j]);
            cui2 = getConstant_Utf8(cf, method1->name_index);
            if (!cui2) return -1;
            cui3 = getConstant_Utf8(cf, method1->descriptor_index);
            if (!cui3) return -1;
            if (cui != cui2)
                continue;
            if (strncmp((char *) cui->data->bytes,
                        (char *) cui2->data->bytes,
                        cui->data->length) != 0)
                continue;
            if (strncmp((char *) cui1->data->bytes,
                        (char *) cui3->data->bytes,
                        cui1->data->length) == 0)
                return -1;
        }
    }

    for (i = 0; i < cf->methods_count; i++)
    {
        method = &(cf->methods[i]);
        
        // validate field access flags
        flags = method->access_flags;
        if (flags & ~ACC_METHOD)
        {
            logError("Unknown flags [0x%X] detected @ "
                    "cf->methods[%i]!\r\n",
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
            logError("Methods can't be PUBLIC, PROTECTED and PRIVATE"
                    " simultaneously!\r\n");
            return -1;
        }
        if (cf->access_flags & ACC_INTERFACE)
        {
            if (flags & (ACC_PROTECTED | ACC_FINAL
                        | ACC_SYNCHRONIZED | ACC_NATIVE))
            {
                logError("Interface method has invalid access flags!\r\n");
                return -1;
            }
#if VER_CMP(52, 0)
            if (!is_public && !is_private)
            {
                logError("Interface method should either be "
                        "PUBLIC or PRIVATE!\r\n");
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
                logError("Abstract method can't be PRIVATE, "
                        "STATIC, FINAL, SYNCHRONIZED, "
                        "NATIE or STRICT!\r\n");
                return -1;
            }
        }
        
        // validate method name
        cui = getConstant_Utf8(cf, method->name_index);
        if (!cui)
            return -1;
        if (!strncmp((char *) cui->data->bytes,
                    "<init>", cui->data->length))
        {
            if (flags & ~(ACC_PUBLIC | ACC_PROTECTED
                        | ACC_PRIVATE | ACC_VARARGS
                        | ACC_STRICT | ACC_SYNTHETIC))
            {
                logError("Initialization method has "
                        "invalid access flags!\r\n");
                return -1;
            }
        }
        
        // validate method descriptor
        cui = getConstant_Utf8(cf, method->descriptor_index);
        if (!cui)
            return -1;
        if (validateMethodDescriptor(cui->data->length,
                    cui->data->bytes) < 0)
        {
            logError("Invalid name \"%.*s\" detected @ "
                    "cf->methods[%i]!\r\n",
                    cui->data->length, cui->data->bytes, i);
            return -1;
        }
    }
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


