/* 
 * File:   rt.h
 * Author: donizyo
 *
 * Created on November 12, 2015, 5:33 PM
 */

#ifndef RT_H
#define	RT_H

#include "java.h"

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct
    {
        u1 tag;
        struct
        {
            u2 class_name_index;
        } * data;
    } rt_Class_info;
    typedef struct
    {
        u1 tag;
        struct
        {
            u2 class_name_index;
            u2 name_index;
            u2 descriptor_index;
            u2 parameters_count;
        } * data;
    } rt_Fieldref_info,
            rt_Methodref_info,
            rt_InterfaceMethodref_info;
    typedef struct
    {
        u1 tag;
        struct
        {
            u2 string_index;
        } * data;
    } rt_String_info;
    typedef CONSTANT_Integer_info rt_Integer_info;
    typedef CONSTANT_Float_info rt_Float_info;
    typedef CONSTANT_Long_info rt_Long_info;
    typedef CONSTANT_Double_info rt_Double_info;
    // I don't think NameAndType need to be resolved
    // because it's a middle info
    typedef CONSTANT_Utf8_info rt_Utf8_info;
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
    } rt_MethodHandle_info;
    typedef struct
    {
        u1 tag;
        struct
        {
            u2 descriptor_index;
        } * data;
    } rt_MethodType_info;
    typedef struct
    {
        u1 tag;
        struct
        {
            u2 bootstrap_method_attr_index;
            u2 name_index;
            u2 descriptor_index;
        } * data;
    } rt_InvokeDynamic_info;
    
    /*
     * Run-time structures and functions
     */
    struct rt_Class
    {
        u2 constant_pool_count;
        cp_info *constant_pool;
        u2 access_flags;
        u2 this_class;
        u2 super_class;
        u2 interfaces_count;
        struct rt_Class *interfaces;
        u2 fields_count;
        field_info *fields;
        u2 methods_count;
        method_info *methods;
        u2 attributes_count;
        attr_info *attributes;
        
        int _hash;
        struct rt_Class *_super;
        struct rt_Class *_component;
        
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
    };
    
    struct rt_ClassLoader
    {
        
    };
    
    u2 rt_Class_getModifiers(struct rt_Class *);
    struct rt_Class *rt_Class_getSuperclass(struct rt_Class *);
    struct rt_Class *rt_Class_getComponentType(struct rt_Class *);
    u1 *rt_Class_getSimpleName(struct rt_Class *);
    
    u2 rt_Class_getInterfacesCount(struct rt_Class *);
    struct rt_Class *rt_Class_getInterfaces(struct rt_Class *);

#ifdef	__cplusplus
}
#endif

#endif	/* RT_H */

