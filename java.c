#include <stdlib.h>
#include <stdio.h>
#include <endian.h>
#include <string.h>

#include "java.h"

static u1 ru1(struct BufferInput *);
static u2 ru2(struct BufferInput *);
static u4 ru4(struct BufferInput *);
static int rbs(char *, struct BufferInput *, int);

static int get_cp_size(u1);
static const char *get_cp_name(u1);

static char *convertAccessFlags_field(u2, u2);
static char *convertAccessFlags_method(u2, u2);

static int loadAttributes(struct BufferInput * input, u2 *, attr_info **);
static int releaseAttributes(u2, attr_info *);

static CONSTANT_Utf8_info *getConstant_Utf8(ClassFile *, u2);
static CONSTANT_Class_info *getConstant_Class(ClassFile *, u2);

static int checkAttribute_field(ClassFile *, field_info *, int);
static int checkAttribute_method(ClassFile *, method_info *, int);

extern int
parseClassfile(struct BufferInput * input, ClassFile *cf)
{
    u2 i, j;
    int cap;
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

    if (!cf)
    {
        fprintf(stderr, "Parameter 'cf' in function %s is NULL!\r\n", __func__);
        return -1;
    }

    bzero(input->buffer, input->bufsize);
    input->bufsrc = input->bufdst = 0;

    // validate file structure
    cf->magic = ru4(input);
    if (cf->magic != 0XCAFEBABE)
    {
        fprintf(stderr, "File structure invalid, fail to decompile! [0x%X]\r\n", cf->magic);
        goto close;
    }
    printf("File structure is valid[0x%X], proceeding...\r\n", cf->magic);

    // retrieve version
    cf->minor_version =
        ru2(input);
    cf->major_version =
        ru2(input);
    printf("Class version: %i.%i\r\n",
            cf->major_version, cf->minor_version);

    // retrieve constant pool size
    cf->constant_pool_count =
        ru2(input);
    printf("Constant pool size: %i\r\n", cf->constant_pool_count);
    if (cf->constant_pool_count > 0)
    {
        cap = cf->constant_pool_count * sizeof (cp_info);
        cf->constant_pool = (cp_info *) malloc(cap);
        if (!cf->constant_pool)
        {
            fprintf(stderr, "Fail to allocate memory.\r\n");
            goto close;
        }
        printf("Constant pool allocated.\r\n");
        bzero(cf->constant_pool, cap);

        printf("Parsing constant pool...\r\n");
        // jvms7 says "The constant_pool table is indexed
        // from 1 to constant_pool_count - 1
        for (i = 1u; i < cf->constant_pool_count; i++)
        { // LOOP
            info = &(cf->constant_pool[i]);
            info->tag = ru1(input);
            printf("\t#%-5i = %-18s ",
                    i, get_cp_name(info->tag));
            cap = get_cp_size(info->tag);
            info->data = malloc(cap);
            if (!info->data)
            {
                fprintf(stderr, "Fail to allocate memory!\r\n");
                goto close;
            }
            bzero(info->data, cap);
            switch (info->tag)
            {
                case CONSTANT_Class:
                    cci = (CONSTANT_Class_info *) info;
                    cci->data->name_index =
                        ru2(input);

                    printf("#%i\r\n", cci->data->name_index);

                    cci = (CONSTANT_Class_info *) 0;
                    break;
                case CONSTANT_Fieldref:
                case CONSTANT_Methodref:
                case CONSTANT_InterfaceMethodref:
                    cfi = (CONSTANT_Fieldref_info *) info;
                    cfi->data->class_index =
                        ru2(input);
                    cfi->data->name_and_type_index =
                        ru2(input);

                    printf("#%i.#%i\r\n", cfi->data->class_index, cfi->data->name_and_type_index);

                    cfi = (CONSTANT_Fieldref_info *) 0;
                    cci = (CONSTANT_Class_info *) 0;
                    buf = (char *) 0;
                    break;
                case CONSTANT_Integer:
                case CONSTANT_Float:
                    cii = (CONSTANT_Integer_info *) info;
                    cii->data->bytes =
                        ru4(input);

                    printf("%i\r\n", cii->data->bytes);

                    cii = (CONSTANT_Integer_info *) 0;
                    break;
                case CONSTANT_Long:
                case CONSTANT_Double:
                    cli = (CONSTANT_Long_info *) info;
                    cli->data->high_bytes =
                        ru4(input);
                    cli->data->low_bytes =
                        ru4(input);
                    // all 8-byte constants take up two entries in the constant_pool table of the class file
                    ++i;

                    printf("%i %i\r\n", cli->data->high_bytes, cli->data->low_bytes);

                    cli = (CONSTANT_Long_info *) 0;
                    break;
                case CONSTANT_NameAndType:
                    cni = (CONSTANT_NameAndType_info *) info;
                    cni->data->name_index =
                        ru2(input);
                    cni->data->descriptor_index =
                        ru2(input);

                    printf("#%i.#%i\r\n", cni->data->name_index, cni->data->descriptor_index);

                    cni = (CONSTANT_NameAndType_info *) 0;
                    break;
                case CONSTANT_String:
                    csi = (CONSTANT_String_info *) info;
                    csi->data->string_index =
                        ru2(input);

                    printf("#%i\r\n", csi->data->string_index);

                    csi = (CONSTANT_String_info *) 0;
                    break;
                case CONSTANT_Utf8:
                    cui = (CONSTANT_Utf8_info *) info;
                    cui->data->length =
                        ru2(input);
                    cap = (cui->data->length + 1) * sizeof (u1);
                    cui->data->bytes = (u1 *) malloc(cap);
                    if (!cui->data->bytes)
                    {
                        fprintf(stderr, "Fail to allocate memory!\r\n");
                        goto close;
                    }
                    bzero(cui->data->bytes, cap);

                    if (rbs(cui->data->bytes, input, cui->data->length) < 0)
                    {
                        fprintf(stderr, "IO exception in function %s!\r\n", __func__);
                        goto close;
                    }

                    if (!cui->data->bytes)
                    {
                        fprintf(stderr, "Runtime error!\r\n");
                        goto close;
                    }
                    printf("%s\r\n", (char *) cui->data->bytes);

                    cui = (CONSTANT_Utf8_info *) 0;
                    cap = 0;
                    break;
                case CONSTANT_MethodHandle:
                    cmhi = (CONSTANT_MethodHandle_info *) info;
                    cmhi->data->reference_kind =
                        ru1(input);
                    cmhi->data->reference_index =
                        ru2(input);

                    printf("%i #%i\r\n", cmhi->data->reference_kind, cmhi->data->reference_index);

                    cmhi = (CONSTANT_MethodHandle_info *) 0;
                    break;
                case CONSTANT_MethodType:
                    cmti = (CONSTANT_MethodType_info *) info;
                    cmti->data->descriptor_index =
                        ru2(input);

                    printf("#%i\r\n", cmti->data->descriptor_index);

                    cmti = (CONSTANT_MethodType_info *) 0;
                    break;
                case CONSTANT_InvokeDynamic:
                    cidi = (CONSTANT_InvokeDynamic_info *) info;
                    cidi->data->bootstrap_method_attr_index =
                        ru2(input);
                    cidi->data->name_and_type_index =
                        ru2(input);

                    printf("#%i.#%i\r\n",
                            cidi->data->bootstrap_method_attr_index,
                            cidi->data->name_and_type_index);

                    cidi = (CONSTANT_InvokeDynamic_info *) 0;
                    break;
                default:
                    printf("TAG:%X\r\n", info->tag);
                    break;
            }
        } // LOOP
    } // constant pool parsed

    cf->access_flags =
        ru2(input);
    cf->this_class =
        ru2(input);
    cf->super_class =
        ru2(input);
    printf("\r\nAccess flags     : 0x%X\r\n"
            "Class this_class : 0x%X\r\n"
            "Class super_class: 0x%X\r\n",
            cf->access_flags, cf->this_class, cf->super_class);

    cf->interfaces_count =
        ru2(input);
    printf("\r\nInterface count: %i\r\n", cf->interfaces_count);
    if (cf->interfaces_count > 0)
    {
        printf("Parsing interfaces...\r\n");
        cap = sizeof (u2) * cf->interfaces_count;
        cf->interfaces = (u2 *) malloc(cap);
        if (!cf->interfaces)
        {
            fprintf(stderr, "Fail to allocate memory!\r\n");
            goto close;
        }
        bzero(cf->interfaces, cap);
        for (i = 0u; i < cf->interfaces_count; i++)
        {
            cf->interfaces[i] =
                ru2(input);
            cci = (CONSTANT_Class_info *) &(cf->constant_pool[cf->interfaces[i]]);
            if (!cci || cci->tag != CONSTANT_Class)
            {
                fprintf(stderr, "Invalid constant pool index!\r\n");
                goto close;
            }
            cui = (CONSTANT_Utf8_info *) &(cf->constant_pool[cci->data->name_index]);
            if (!cui || cui->tag != CONSTANT_Utf8)
            {
                fprintf(stderr, "Invalid constant pool index!\r\n");
                goto close;
            }
            buf = cui->data->bytes;
            if (!buf) buf = "";
            printf("Interface[%i] #%i\t// %s\r\n",
                    i, cf->interfaces[i], buf);
            buf = (char *) 0;
        }
    }

    cf->fields_count =
        ru2(input);
    printf("\r\nField count: %i\r\n", cf->fields_count);
    if (cf->fields_count > 0)
    {
        printf("Parsing fields...\r\n");
        cap = sizeof (field_info) * cf->fields_count;
        cf->fields = (field_info *) malloc(cap);
        if (!cf->fields)
        {
            fprintf(stderr, "Fail to allocate memory!\r\n");
            goto close;
        }
        bzero(cf->fields, cap);
        for (i = 0u; i < cf->fields_count; i++)
        {
            cf->fields[i].access_flags =
                ru2(input);
            cf->fields[i].name_index =
                ru2(input);
            cf->fields[i].descriptor_index =
                ru2(input);
            buf = convertAccessFlags_field(i, cf->fields[i].access_flags);
            printf("Field[%i]\r\n"
                    "\tAccess flag:      0x%X\t// %s\r\n"
                    "\tName index:       #%i\t// %s\r\n"
                    "\tDescriptor index: #%i\t// %s\r\n", i,
                    cf->fields[i].access_flags, buf ? buf : "",
                    cf->fields[i].name_index,
                    getConstant_Utf8(cf, cf->fields[i].name_index)->data->bytes,
                    cf->fields[i].descriptor_index,
                    getConstant_Utf8(cf, cf->fields[i].descriptor_index)->data->bytes
                    );
            free(buf);
            buf = (char *) 0;
            loadAttributes(input,
                    &(cf->fields[i].attributes_count), &(cf->fields[i].attributes));
            printf("\tField Attribute count: %i\r\n",
                    cf->fields[i].attributes_count);
            for (j = 0; j < cf->fields[i].attributes_count; j++)
            {
                printf("\tField Attribute [%i]\r\n"
                    "\t\tName index:\t#%i\t// %s\r\n"
                    "\t\tLength    :\t%i\r\n"
                    "\t\tInfo      :\t%s\r\n", j,
                    cf->fields[i].attributes[j].attribute_name_index,
                    getConstant_Utf8(cf, cf->fields[i].attributes[j].attribute_name_index)->data->bytes,
                    cf->fields[i].attributes[j].attribute_length,
                    cf->fields[i].attributes[j].info);
            }
        }
    }

    cf->methods_count =
        ru2(input);
    printf("\r\nMethod count: %i\r\n", cf->methods_count);
    if (cf->methods_count > 0)
    {
        printf("Parsing methods...\r\n");
        cap = sizeof (method_info) * cf->methods_count;
        cf->methods = (method_info *) malloc(cap);
        if (!cf->methods)
        {
            fprintf(stderr, "Fail to allocate memory!\r\n");
            goto close;
        }
        bzero(cf->methods, cap);
        for (i = 0u; i < cf->methods_count; i++)
        {
            cf->methods[i].access_flags =
                ru2(input);
            cf->methods[i].name_index =
                ru2(input);
            cf->methods[i].descriptor_index =
                ru2(input);
            buf = convertAccessFlags_method(i, cf->methods[i].access_flags);
            printf("Method[%i]\r\n"
                    "\tAccess flag:      0x%X\t// %s\r\n"
                    "\tName index:       #%i\t// %s\r\n"
                    "\tDescriptor index: #%i\t// %s\r\n", i,
                    cf->methods[i].access_flags,
                    buf ? buf : "",
                    cf->methods[i].name_index,
                    getConstant_Utf8(cf, cf->methods[i].name_index)->data->bytes,
                    cf->methods[i].descriptor_index,
                    getConstant_Utf8(cf, cf->methods[i].descriptor_index)->data->bytes);
            free(buf);
            buf = (char *) 0;
            loadAttributes(input,
                    &(cf->methods[i].attributes_count), &(cf->methods[i].attributes));
            printf("\tMethod Attribute count: %i\r\n",
                    cf->methods[i].attributes_count);
            for (j = 0; j < cf->methods[i].attributes_count; j++)
            {
                printf("\tMethod Attribute [%i]\r\n"
                    "\t\tName index:\t#%i\t// %s\r\n"
                    "\t\tLength    :\t%i\r\n"
                    "\t\tInfo      :\t%s\r\n", j,
                    cf->methods[i].attributes[j].attribute_name_index,
                    getConstant_Utf8(cf, cf->methods[i].attributes[j].attribute_name_index)->data->bytes,
                    cf->methods[i].attributes[j].attribute_length,
                    cf->methods[i].attributes[j].info);
            }
        }
    }

    printf("\r\nParsing attributes...\r\n");
    loadAttributes(input,
            &(cf->attributes_count), &(cf->attributes));
    for (i = 0; i < cf->attributes_count; i++)
    {
        printf("Class Attribute [%i]\r\n"
                "\tName index:\t#%i\t// %s\r\n"
                "\tLength    :\t%i\r\n"
                "\tInfo      :\t%s\r\n", i,
                cf->attributes[i].attribute_name_index,
                getConstant_Utf8(cf, cf->attributes[i].attribute_name_index)->data->bytes,
                cf->attributes[i].attribute_length,
                cf->attributes[i].info);
    }

close:

    return 0;
}

extern int
freeClassfile(ClassFile *cf)
{
    u2 i;

    printf("Releasing memory...\r\n");
    if (cf->constant_pool)
    {
        for (i = 1u; i < cf->constant_pool_count; i++)
        {
            if (cf->constant_pool[i].tag == CONSTANT_Utf8)
            {
                free(((CONSTANT_Utf8_info *) &(cf->constant_pool[i]))->data->bytes);
                ((CONSTANT_Utf8_info *) &(cf->constant_pool[i]))->data->bytes = (u1 *) 0;
            }
            free(cf->constant_pool[i].data);
            cf->constant_pool[i].data = (void *) 0;
        }
        free(cf->constant_pool);
        cf->constant_pool = (cp_info *) 0;
    }
    if (cf->interfaces)
    {
        free(cf->interfaces);
        cf->interfaces = (u2 *) 0;
    }
    if (cf->fields)
    {
        releaseAttributes(cf->fields->attributes_count, cf->fields->attributes);
        free(cf->fields);
        cf->fields = (field_info *) 0;
    }
    if (cf->methods)
    {
        releaseAttributes(cf->methods->attributes_count, cf->methods->attributes);
        free(cf->methods);
        cf->methods = (method_info *) 0;
    }
    releaseAttributes(cf->attributes_count, cf->attributes);

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

static int
checkInput(struct BufferInput * input)
{
    if (!input)
    {
        fprintf(stderr, "Member 'input' is NULL!\r\n");
        return -1;
    }
    if (!input->buffer)
    {
        fprintf(stderr, "Member 'buffer' is NULL!\r\n");
        return -1;
    }
    if (input->bufsrc > input->bufdst)
    {
        fprintf(stderr, "Assertion error in function %s: "
                "bufsrc[%i] > bufdst[%i]\r\n",
                __func__, input->bufsrc, input->bufdst);
        return -1;
    }

    return 0;
}

extern char *
fillBuffer_f(struct BufferInput * input, int nbits)
{
    FILE *file;
    int bufsize, buflen, cap, rbit;

    if (checkInput(input))
        return (char *) 0;
    file = input->file;
    bufsize = input->bufsize;
    if (!file)
    {
        fprintf(stderr, "Member 'file' is NULL!\r\n");
    }
    if (nbits < 0)
    {
        fprintf(stderr, "Parameter 'nbits' in function %s is negative!\r\n", __func__);
        return (char *) 0;
    }

    // calculate the length of remaining data
    buflen = input->bufdst - input->bufsrc;
    // the remaining data is not enough
    if (buflen < nbits)
    {
        // move memory
        if (input->bufsrc != 0)
        {
            memmove(input->buffer, &(input->buffer[input->bufsrc]), buflen);
            input->bufsrc = 0;
            input->bufdst = buflen;
        }
        // fill in more data if possible
        if (input->more)
        {
            cap = bufsize - buflen;
            rbit = fread(&(input->buffer[buflen]), sizeof (u1), cap, file);
            if (rbit < 0)
            {
                fprintf(stderr, "IO exception in function %s!\r\n", __func__);
                return (char *) 0;
            }
            else if (rbit < cap)
                input->more = 0;

            input->bufdst += rbit;
            if (input->bufdst < bufsize)
                bzero(&(input->buffer[input->bufdst]), bufsize - input->bufdst);
        }
    }

    return &(input->buffer[input->bufsrc]);
}

extern char *
fillBuffer_z(struct BufferInput * input, int nbits)
{
    struct zip_file *zf;
    int bufsize, buflen, cap, rbit;

    if (checkInput(input))
        return (char *) 0;
    zf = input->entry;
    bufsize = input->bufsize;
    if (!zf)
    {
        fprintf(stderr, "Member 'zf' is NULL!\r\n");
        return (char *) 0;
    }
    if (nbits < 0)
    {
        fprintf(stderr, "Parameter 'nbits' in function %s is negative!\r\n", __func__);
        return (char *) 0;
    }

    // calculate the length of remaining data
    buflen = input->bufdst - input->bufsrc;
    // the remaining data is not enough
    if (buflen < nbits)
    {
        // move memory
        if (input->bufsrc != 0)
        {
            memmove(input->buffer, &(input->buffer[input->bufsrc]), buflen);
            input->bufsrc = 0;
            input->bufdst = buflen;
        }
        // fill in more data if possible
        if (input->more)
        {
            cap = bufsize - buflen;
            rbit = zip_fread(zf, &(input->buffer[buflen]), cap);
            if (rbit < 0)
            {
                fprintf(stderr, "IO exception in function %s!\r\n", __func__);
                return (char *) 0;
            }
            else if (rbit < cap)
                input->more = 0;

            input->bufdst += rbit;
            if (input->bufdst < bufsize)
                bzero(&(input->buffer[input->bufdst]), bufsize - input->bufdst);
        }
    }

    return &(input->buffer[input->bufsrc]);
}

static int rbs(char *out, struct BufferInput * input, int nbits)
{
    char *buf;

    if (!out)
    {
        fprintf(stderr, "Parameter 'out' in function %s is NULL!\r\n", __func__);
        return -1;
    }

    buf = (*input->fp)(input, nbits);
    if (buf < 0)
        return -1;
    memcpy(out, buf, nbits);
    input->bufsrc += nbits;

    return nbits;
}

static u1
ru1(struct BufferInput * input)
{
    u1 res;
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u1));
    res = *(u1 *) ptr;
    input->bufsrc += sizeof (u1);

    return res;
}

static u2
ru2(struct BufferInput * input)
{
    u2 res;
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u2));
    res = *(u2 *) ptr;
    res = htobe16(res);
    input->bufsrc += sizeof (u2);

    return res;
}

static u4
ru4(struct BufferInput * input)
{
    u4 res;
    char *ptr;

    ptr = (*input->fp)(input, sizeof (u4));
    res = *(u4 *) ptr;
    res = htobe32(res);
    input->bufsrc += sizeof (u4);

    return res;
}

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
        fprintf(stderr,
                "Unknown flags [%X] detected @ cf->fields[%i]!\r\n",
                af & ~ACC_FIELD, i);
        return (char *) 0;
    }
    buf = (char *) malloc(initBufSize);
    if (!buf)
    {
        fprintf(stderr, "Fail to allocate memory.\r\n");
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
        fprintf(stderr,
                "Unknown flags [%X] detected @ cf->methods[%i]!\r\n",
                af & ~ACC_METHOD, i);
        return (char *) 0;
    }
    buf = (char *) malloc(initBufSize);
    if (!buf)
    {
        fprintf(stderr, "Fail to allocate memory.\r\n");
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

static int
loadAttributes(struct BufferInput * input,
        u2 *attr_count_p, attr_info **attributes_p)
{
    u2 i;
    int cap;

    *attr_count_p =
        ru2(input);
    if (*attr_count_p > 0)
    {
        cap = sizeof (attr_info) * *attr_count_p;
        *attributes_p = (attr_info *) malloc(cap);
        if (!*attributes_p)
        {
            fprintf(stderr, "Fail to allocate memory!\r\n");
            return -1;
        }
        bzero(*attributes_p, cap);
        for (i = 0u; i < *attr_count_p; i++)
        {
            (*attributes_p)[i].attribute_name_index =
                ru2(input);
            (*attributes_p)[i].attribute_length =
                ru4(input);
            cap = (*attributes_p)[i].attribute_length * sizeof (u1);
            (*attributes_p)[i].info = (u1 *) malloc(cap);
            if (!(*attributes_p)[i].info)
            {
                fprintf(stderr, "Fail to allocate memory!\r\n");
                return -1;
            }
            bzero((*attributes_p)[i].info, cap);
            if (rbs((*attributes_p)[i].info, input,
                    (*attributes_p)[i].attribute_length) < 0)
            {
                fprintf(stderr, "IO exception in function %s!\r\n", __func__);
                return -1;
            }
        }
    }

    return 0;
}

static int
releaseAttributes(u2 attributes_count, attr_info *attributes)
{
    u2 i;

    if (attributes)
    {
        for (i = 0; i < attributes_count; i++)
        {
            free(attributes[i].info);
            attributes[i].info = (u1 *) 0;
        }
        free(attributes);
    }

    return 0;
}

static CONSTANT_Utf8_info *
getConstant_Utf8(ClassFile *cf, u2 index)
{
    CONSTANT_Utf8_info *info;

    info = (CONSTANT_Utf8_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        fprintf(stderr, "Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Utf8_info *) 0;
    }
    if (info->tag != CONSTANT_Utf8)
    {
        fprintf(stdout, "Constant pool entry #%i is not CONSTANT_Utf8_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Utf8_info *) 0;
    }

    return info;
}

static CONSTANT_Class_info *
getConstant_Class(ClassFile *cf, u2 index)
{
    CONSTANT_Class_info *info;

    info = (CONSTANT_Class_info *) &(cf->constant_pool[index]);
    if (!info)
    {
        fprintf(stderr, "Constant pool entry #%i is NULL!\r\n", index);
        return (CONSTANT_Class_info *) 0;
    }
    if (info->tag != CONSTANT_Class)
    {
        fprintf(stdout, "Constant pool entry #%i is not CONSTANT_Class_info entry, but CONSTANT_%s_info entry!\r\n", index, get_cp_name(info->tag));
        return (CONSTANT_Class_info *) 0;
    }

    return info;
}

static int
checkAttribute_field(ClassFile *cf, field_info *field_info, int index)
{
    attr_info info;
    CONSTANT_Utf8_info *utf8;
    char *name, *str;
    u2 len;

    info = field_info->attributes[index];
    if (!info.attribute_name_index)
        return -1;
    utf8 = getConstant_Utf8(cf, info.attribute_name_index);
    if (!utf8) goto fail;
    name = (char *) utf8->data->bytes;
    if (!name) goto fail;
    len = utf8->data->length;
    // ConstantValue, Synthetic, Signature, Deprecated
    // RuntimeVisibleAnnotations, RuntimeInvisibleAnnotations
    str = "ConstantValue\0";
    if (!strncmp(name, str, len))
    {
        return 0;
    }
    if (cf->major_version > 49)
    {
        str = "Signature\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeVisibleAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeInvisibleAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
    }
    str = "Synthetic\0";
    if (!strncmp(name, str, len))
    {
        return 0;
    }

fail:
    info.attribute_name_index = 0;
    info.attribute_length = 0;
    free(info.info);
    info.info = (u1 *) 0;
    return -1;
}

static int
checkAttribute_method(ClassFile *cf, method_info *method_info, int index)
{
    attr_info info;
    CONSTANT_Utf8_info *utf8;
    char *name, *str;
    u2 len;

    info = method_info->attributes[index];
    if (!info.attribute_name_index)
        return -1;
    utf8 = getConstant_Utf8(cf, info.attribute_name_index);
    if (!utf8) goto fail;
    name = (char *) utf8->data->bytes;
    if (!name) goto fail;
    len = utf8->data->length;
    // Code, Exceptions, Synthetic, Signature, Deprecated
    // RuntimeVisibleAnnotations, RuntimeInvisibleAnnotations
    // RuntimeVisibleParameterAnnotations
    // RuntimeInvisibleParameterAnnotations
    // AnnotationDefault
    str = "Code\0";
    if (!strncmp(name, str, len))
    {
        return 0;
    }
    str = "Exceptions\0";
    if (!strncmp(name, str, len))
    {
        return 0;
    }
    if (cf->major_version > 49)
    {
        str = "Signature\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeVisibleAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeInvisibleAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeVisibleParameterAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "RuntimeInvisibleParameterAnnotations\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
        str = "AnnotationDefault\0";
        if (!strncmp(name, str, len))
        {
            return 0;
        }
    }

fail:
    info.attribute_name_index = 0;
    info.attribute_length = 0;
    free(info.info);
    info.info = (u1 *) 0;
    return -1;
}
