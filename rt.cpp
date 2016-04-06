#include "java.h"
#include "rt.h"

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

rt_Class_info *
rt_Class::getConstant_Class(u2 index)
{
    return rt_getConstant<rt_Class_info, CONSTANT_Class>(index,
            constant_pool_count, constant_pool);
}

rt_Fieldref_info *
rt_Class::getConstant_Fieldref(u2 index)
{
    return rt_getConstant<rt_Fieldref_info, CONSTANT_Fieldref>(index,
            constant_pool_count, constant_pool);
}

rt_Methodref_info *
rt_Class::getConstant_Methodref(u2 index)
{
    return rt_getConstant<rt_Methodref_info,
           CONSTANT_Methodref>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_InterfaceMethodref_info *
rt_Class::getConstant_InterfaceMethodref(u2 index)
{
    return rt_getConstant<rt_InterfaceMethodref_info,
           CONSTANT_InterfaceMethodref>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_String_info *
rt_Class::getConstant_String(u2 index)
{
    return rt_getConstant<rt_String_info,
           CONSTANT_String>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_Integer_info *
rt_Class::getConstant_Integer(u2 index)
{
    return rt_getConstant<rt_Integer_info,
           CONSTANT_Integer>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_Float_info *
rt_Class::getConstant_Float(u2 index)
{
    return rt_getConstant<rt_Float_info,
           CONSTANT_Float>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_Long_info *
rt_Class::getConstant_Long(u2 index)
{
    return rt_getConstant<rt_Long_info,
           CONSTANT_Long>(index,
                   constant_pool_count,
                   constant_pool);
}

rt_Double_info *
rt_Class::getConstant_Double(u2 index)
{
    return rt_getConstant<rt_Double_info,
           CONSTANT_Double>(index,
                   constant_pool_count,
                   constant_pool);
}


