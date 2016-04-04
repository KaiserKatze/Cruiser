#ifndef RT_H
#define	RT_H

#include "java.h"

#ifndef	__cplusplus
#error C++ TOOLCHAIN IS NEEDED TO COMPILE CRUISER!
#endif

typedef struct
{
    u2              off_parameter_descriptor;
    u2              len_parameter_descirptor;
    // TODO
    // If the value of the name_index item is zero,
    // then this parameters element
    // indicates a formal parameter with no name.
    // WTF???
    u2              name_index;
    u2              name_length;
    u1 *            name_bytes;
    // The value of the access_flags item is as follows:
    // 0x0010 (ACC_FINAL)
    // 0x1000 (ACC_SYNTHETIC)
    // 0x8000 (ACC_MANDATED)
    u2              access_flags;
}                                   rt_Parameter;

typedef struct
{
    u2              off_return_descriptor;
    u2              len_return_descriptor;
    u1              parameters_count;
    u1              parameters_length;
    rt_Parameter *  parameters;
}                                   rt_Descriptor;

typedef struct
{
    u2              class_name_index;
}                                   rt_Class_data;
typedef struct CONSTANT_Class_info  rt_Class_info;

typedef struct
{
    u2              class_name_index;
    u2              name_index;
    u2              descriptor_index;
    rt_Descriptor   descriptor;
}                                   rt_Fieldref_data,
                                    rt_Methodref_data,
                                    rt_InterfaceMethodref_data;

typedef CONSTANT_Fieldref_info      rt_Fieldref_info;
typedef CONSTANT_Methodref_info     rt_Methodref_info;
typedef CONSTANT_InterfaceMethodref_info
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
    u1              reference_kind;
    union
    {
        u2          off_Fieldref;
        u2          off_Methodref;
        u2          off_InterfaceMethodref;
    };
}                                   rt_MethodHandle_data;
typedef struct
{
    u2              descriptor_index;
}                                   rt_MethodType_data;
typedef struct
{
    u2              bootstrap_method_attr_index;
    u2              name_index;
    u2              descriptor_index;
}                                   rt_InvokeDynamic_data;

typedef struct
{
    u2              attributes_count;
    attr_info *     attributes;
    u4              attributes_mark;
}                                   rt_Attributes;

/*
 * Run-time structures and functions
 */
class rt_Class
{
public:
    u2              getAccessFlags();
    u2              getFieldsCount();
    rt_Class_data *
                    getConstant_Class(u2);
    rt_Fieldref_data *
                    getConstant_Fieldref(u2);
    rt_Methodref_data *
                    getConstant_Methodref(u2);
    rt_InterfaceMethodref_data *
                    getConstant_InterfaceMethodref(u2);
    rt_String_data *
                    getConstant_String(u2);
    rt_Integer_data *
                    getConstant_Integer(u2);
    rt_Float_data *
                    getConstant_Float(u2);
    rt_Long_data *
                    getConstant_Long(u2);
    rt_Double_data *
                    getConstant_Double(u2);
    rt_Utf8_data *
                    getConstant_Utf8(u2);
    rt_MethodHandle_data *
                    getConstant_MethodHandle(u2);
    rt_MethodType_data *
                    getConstant_MethodType(u2);
    rt_InvokeDynamic_data *
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
    rt_Attributes   attributes;

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
    rt_Class_data * getThisClass();
    rt_Class_data * getSuperClass();
    u2              getInterfacesCount();
    rt_Class_data * getInterfaces(rt_Class_data *);
    rt_Class_data * getComponentType();
    u2              getModifiers();
}; // rt_Class

class rt_Field
{
private:
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    rt_Attributes   attributes;

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
}; // rt_Field

class rt_Method
{
private:
    u2              access_flags;
    u2              name_index;
    u2              descriptor_index;
    rt_Attributes   attributes;
    rt_Descriptor   descriptor;

#if VER_CMP(45, 3)
    u2              off_Code;
    u2              off_Exceptions;
#endif
#if VER_CMP(49, 0)
    u2              off_RuntimeVisibleParameterAnnotations;
    u2              off_RuntimeInvisibleParameterAnnotations;
    u2              off_AnnotationDefault;
    u2              off_RuntimeVisibleAnnotations;
    u2              off_RuntimeInvisibleAnnotations;
#endif
#if VER_CMP(52, 0)
    u2              off_RuntimeVisibleTypeAnnotations;
    u2              off_RuntimeInvisibleTypeAnnotations;
#endif
}; // rt_Method

#endif	/* RT_H */

