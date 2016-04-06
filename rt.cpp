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
