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
    return rt_getConstant<rt_Class_info, CONSTANT_Fieldref>(index,
            constant_pool_count, constant_pool);
}
