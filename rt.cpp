#include "java.h"
#include "rt.h"
#include "memory.h"

u2
rt_Accessible::getAccessFlags()
{
    return access_flags;
}

rt_Attributes *
rt_Accessible::getAttributes()
{
    return &attributes;
}

attr_info *
rt_Accessible::getAttribute(u2 index)
{
    return getAttribute(index, 0);
}

attr_info *
rt_Accessible::getAttribute(u2 index, u4 tag)
{
    attr_info *info;

    if (index < 0 || index > attributes.attributes_count)
        return (attr_info *) 0;
    info = &(attributes.attributes[index]);
    if (tag == 0)
        return info;
    if (info->tag == tag)
        return info;
    return (attr_info *) 0;
}

u2
rt_Class::getFieldsCount()
{
    return fields_count;
}

rt_Field *
rt_Class::getFields()
{
    return fields;
}

u2
rt_Class::getMethodsCount()
{
    return methods_count;
}

rt_Method *
rt_Class::getMethods()
{
    return methods;
}

u1
rt_Class::getConstantTag(u2 index)
{
    if (index < 1 ||
            index > constant_pool_count)
        return 0xff;
    return constant_pool[index].tag;
}

template <typename rt_info, u1 tag>
rt_info *
rt_getConstant(u2 index, u2 cp_count, cp_info *cp)
{
    cp_info * info;

    if (index < 1 ||
            index > cp_count)
        return (rt_info *) NULL;
    info = &(cp[index]);
    if (info->tag != tag)
        return (rt_info *) NULL;
    return (rt_info *) info;
}

const_Class_data *
rt_Class::getConstant_Class(u2 index)
{
    return rt_getConstant<const_Class_data, CONSTANT_Class>(index,
            constant_pool_count, constant_pool);
}

const_Fieldref_data *
rt_Class::getConstant_Fieldref(u2 index)
{
    return rt_getConstant<const_Fieldref_data, CONSTANT_Fieldref>(index,
            constant_pool_count, constant_pool);
}

const_Methodref_data *
rt_Class::getConstant_Methodref(u2 index)
{
    return rt_getConstant<const_Methodref_data,
           CONSTANT_Methodref>(index,
                   constant_pool_count,
                   constant_pool);
}

const_InterfaceMethodref_data *
rt_Class::getConstant_InterfaceMethodref(u2 index)
{
    return rt_getConstant<const_InterfaceMethodref_data,
           CONSTANT_InterfaceMethodref>(index,
                   constant_pool_count,
                   constant_pool);
}

const_String_data *
rt_Class::getConstant_String(u2 index)
{
    return rt_getConstant<const_String_data,
           CONSTANT_String>(index,
                   constant_pool_count,
                   constant_pool);
}

const_Integer_data *
rt_Class::getConstant_Integer(u2 index)
{
    return rt_getConstant<const_Integer_data,
           CONSTANT_Integer>(index,
                   constant_pool_count,
                   constant_pool);
}

const_Float_data *
rt_Class::getConstant_Float(u2 index)
{
    return rt_getConstant<const_Float_data,
           CONSTANT_Float>(index,
                   constant_pool_count,
                   constant_pool);
}

const_Long_data *
rt_Class::getConstant_Long(u2 index)
{
    return rt_getConstant<const_Long_data,
           CONSTANT_Long>(index,
                   constant_pool_count,
                   constant_pool);
}

const_Double_data *
rt_Class::getConstant_Double(u2 index)
{
    return rt_getConstant<const_Double_data,
           CONSTANT_Double>(index,
                   constant_pool_count,
                   constant_pool);
}

const_Utf8_data *
rt_Class::getConstant_Utf8(u2 index)
{
    return rt_getConstant<const_Utf8_data,
           CONSTANT_Utf8>(index,
                   constant_pool_count,
                   constant_pool);
}

const_MethodHandle_data *
rt_Class::getConstant_MethodHandle(u2 index)
{
    return rt_getConstant<const_MethodHandle_data,
           CONSTANT_MethodHandle>(index,
                   constant_pool_count,
                   constant_pool);
}

const_MethodType_data *
rt_Class::getConstant_MethodType(u2 index)
{
    return rt_getConstant<const_MethodType_data,
           CONSTANT_MethodType>(index,
                   constant_pool_count,
                   constant_pool);
}

const_InvokeDynamic_data *
rt_Class::getConstant_InvokeDynamic(u2 index)
{
    return rt_getConstant<const_InvokeDynamic_data,
           CONSTANT_InvokeDynamic>(index,
                   constant_pool_count,
                   constant_pool);
}

bool
rt_Class::isInterface()
{
    return getAccessFlags() & ACC_INTERFACE;
}

bool
rt_Class::isAnnotation()
{
    return getAccessFlags() & ACC_ANNOTATION;
}

bool
rt_Class::isSynthetic()
{
    return getAccessFlags() & ACC_SYNTHETIC;
}

const_Class_data *
rt_Class::getThisClass()
{
    return getConstant_Class(this_class);
}

const_Class_data *
rt_Class::getSuperClass()
{
    return getConstant_Class(super_class);
}

const_Utf8_data *
rt_Class::getClassName()
{
    const_Class_data * info;
    u2              name_index;

    info = getThisClass();
    name_index = info->name_index;

    return getConstant_Utf8(name_index);
}

u2
rt_Class::getInterfacesCount()
{
    return interfaces_count;
}

const_Class_data **
rt_Class::getInterfaces(const_Class_data ** out)
{
    u2 count;
    u2 i;

    count = getInterfacesCount();
    if (count == 0)
        return (const_Class_data **) NULL;
    if (!out)
    {
        out = (const_Class_data **)
            allocMemory(count, sizeof (const_Class_data *));
        if (!out)
            return out;
    }

    for (i = 0; i < count; i++)
        out[i] = getConstant_Class(interfaces[i]);

    return out;
}

rt_Class *
rt_Member::getDefClass()
{
    return def_class;
}

const_Utf8_data *
rt_Member::getName()
{
    rt_Class *cls;

    cls = getDefClass();
    return cls->getConstant_Utf8(name_index);
}

const_Utf8_data *
rt_Member::getDescriptor()
{
    rt_Class *cls;

    cls = getDefClass();
    return cls->getConstant_Utf8(descriptor_index);
}

attr_Code_info *
rt_Method::getAttribute_Code(u2 index)
{
    attr_info *info;

    info = getAttribute(index, TAG_ATTR_CODE);
    if (!info)
        return (attr_Code_info *) 0;
    return (attr_Code_info *) info->data;
}

rt_Member::rt_Member(rt_Class *rtc)
{
    this->def_class = rtc;
}

rt_Accessible::rt_Accessible(ClassFile *classfile)
{
    rt_Attributes *attrsp;

    access_flags = classfile->access_flags;
    attrsp->attributes_count = classfile->attributes_count;
    attrsp->attributes = classfile->attributes;
    attrsp->attributes_mark = 0;
}

rt_Accessible::rt_Accessible(field_info *info)
{
    rt_Attributes *attrsp;

    access_flags = info->access_flags;

    attrsp = &attributes;
    attrsp->attributes_count = info->attributes_count;
    attrsp->attributes = info->attributes;
    attrsp->attributes_mark = 0;
}
