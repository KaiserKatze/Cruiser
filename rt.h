#ifndef RT_H
#define	RT_H

#include "java.h"

#ifndef	__cplusplus
#error C++ TOOLCHAIN IS NEEDED TO COMPILE CRUISER!
#endif

typedef struct
{
    u1 tag;
    struct
    {
        u2 class_name_index;
    } * data;
}                                   rt_Class_info;
typedef struct
{
    u2 access_flags;
    u2 descriptor_index;
    u2 name_index;
}                                   rt_Parameter;
typedef struct
{
    u1 tag;
    struct
    {
        u2 class_name_index;
        u2 name_index;
        u2 descriptor_index;
        u2 parameters_count;
        u2 parameters_length;
        rt_Parameter * parameters;
    } * data;
}                                   rt_Fieldref_info,
                                    rt_Methodref_info,
                                    rt_InterfaceMethodref_info;
typedef CONSTANT_String_info        rt_String_info;
typedef CONSTANT_Integer_info       rt_Integer_info;
typedef CONSTANT_Float_info         rt_Float_info;
typedef CONSTANT_Long_info          rt_Long_info;
typedef CONSTANT_Double_info        rt_Double_info;
// I don't think NameAndType need to be resolved
// because it's a middle info
typedef CONSTANT_Utf8_info          rt_Utf8_info;
typedef struct
{
    u1 tag;
    struct
    {
        u1 reference_kind;
        union
        {
            u2 off_Fieldref;
            u2 off_Methodref;
            u2 off_InterfaceMethodref;
        };
    } * data;
}                                   rt_MethodHandle_info;
typedef struct
{
    u1 tag;
    struct
    {
        u2 descriptor_index;
    } * data;
}                                   rt_MethodType_info;
typedef struct
{
    u1 tag;
    struct
    {
        u2 bootstrap_method_attr_index;
        u2 name_index;
        u2 descriptor_index;
    } * data;
}                                   rt_InvokeDynamic_info;

struct rt_Attributes
{
    u2              attributes_count;
    attr_info *     attributes;
    u4              attributes_mark;
};

/*
 * Run-time structures and functions
 */
class rt_Class
{
public:
    u2              getAccessFlags();
    u2              getFieldsCount();
    rt_Class_info *
                    getConstant_Class(u2);
    rt_Fieldref_info *
                    getConstant_Fieldref(u2);
    rt_Methodref_info *
                    getConstant_Methodref(u2);
    rt_InterfaceMethodref_info *
                    getConstant_InterfaceMethodref(u2);
    rt_String_info *
                    getConstant_String(u2);
    rt_Integer_info *
                    getConstant_Integer(u2);
    rt_Float_info *
                    getConstant_Float(u2);
    rt_Long_info *
                    getConstant_Long(u2);
    rt_Double_info *
                    getConstant_Double(u2);
    rt_Utf8_info *
                    getConstant_Utf8(u2);
    rt_MethodHandle_info *
                    getConstant_MethodHandle(u2);
    rt_MethodType_info *
                    getConstant_MethodType(u2);
    rt_InvokeDynamic_info *
                    getConstant_InvokeDynamic(u2);
private:
    int             hash;
    u2              constant_pool_count;
    cp_info *       constant_pool;
    u2              access_flags;
    u2              this_class;
    u2              super_class;
    u2              interfaces_count;
    u2 *            interfaces;
    u2              fields_count;
    field_info *    fields;
    u2              methods_count;
    method_info *   methods;
    rt_Attributes * attributes;

#if VER_CMP(45, 3)
    u2 off_InnerClasses;
#endif
#if VER_CMP(49, 0)
    u2 off_EnclosingMethod;
    u2 off_RuntimeVisibleAnnotations;
    u2 off_RuntimeInvisibleAnnotations;
#endif
#if VER_CMP(51, 0)
    u2 off_BootstrapMethods;
#endif
#if VER_CMP(52, 0)
    u2 off_RuntimeVisibleTypeAnnotations;
    u2 off_RuntimeInvisibleTypeAnnotations;
#endif
public:
    bool            isInterface();
    bool            isArray();
    bool            isAnnotation();
    bool            isSynthetic();
    rt_Class_info * getThisClass();
    rt_Class_info * getSuperClass();
    u2              getInterfacesCount();
    rt_Class_info * getInterfaces(rt_Class_info *);
    rt_Class_info * getComponentType();
    u2              getModifiers();
};

class rt_Field
{
private:
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    u2              attributes_count;
    rt_Attributes * attributes;

#if VER_CMP(45, 3)
    u2              off_ConstantValue;
#endif
#if VER_CMP(49, 0)
    u2              off_RuntimeVisibleAnnotations;
    u2              off_RuntimeInvisibleAnnotations;
#endif
#if VER_CMP(52, 0)
    u2              off_RuntimeVisibleTypeAnnotations;
    u2              off_RuntimeInvisibleTypeAnnotations;
#endif
};

#endif	/* RT_H */

