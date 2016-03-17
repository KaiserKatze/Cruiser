#ifndef JAVA_H
#define	JAVA_H

#include <stdio.h>

#include "input.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define MINOR_VERSION                   0
#define MAJOR_VERSION                   51
    
#if (defined MAJOR_VERSION && defined MINOR_VERSION)
#define VER_CMP(major, minor)  (MAJOR_VERSION > major || MAJOR_VERSION == major && MINOR_VERSION >= minor)
#else
#error "Macro 'MAJOR_VERSION' and 'MINOR_VERSION' is missing!"
#endif

#define CONSTANT_Class                  7
#define CONSTANT_Fieldref               9
#define CONSTANT_Methodref              10
#define CONSTANT_InterfaceMethodref     11
#define CONSTANT_String                 8
#define CONSTANT_Integer                3
#define CONSTANT_Float                  4
#define CONSTANT_Long                   5
#define CONSTANT_Double                 6
#define CONSTANT_NameAndType            12
#define CONSTANT_Utf8                   1
#define CONSTANT_MethodHandle           15
#define CONSTANT_MethodType             16
#define CONSTANT_InvokeDynamic          18

// Class access
#define ACC_PUBLIC                      0x0001 // Declared public
#define ACC_FINAL                       0x0010 // Declared final
#define ACC_SUPER                       0x0020 // Tread superclass methods specially when invoked by the invokespecial instruction
#define ACC_INTERFACE                   0x0200 // Is an interface, not a class
#define ACC_ABSTRACT                    0x0400 // Declared abstract; must not be instantiated
#define ACC_SYNTHETIC                   0x1000 // Declared synthetic; not present in the source code
#define ACC_ANNOTATION                  0x2000 // Declared as an annotation type
#define ACC_ENUM                        0x4000 // Declared as an enum type
    
// Field access (Additional)
#define ACC_PRIVATE                     0x0002 // Declared private
#define ACC_PROTECTED                   0x0004 // Declared protected
#define ACC_STATIC                      0x0008 // Declared static
#define ACC_VOLATILE                    0x0040 // Declared volatile; cannot be cached
#define ACC_TRANSIENT                   0x0080 // Declared transient

// Method access (additional)
#define ACC_SYNCHRONIZED                0x0020 // Declared synchronized; invocation is wrapped by a monitor use
#define ACC_BRIDGE                      0x0040 // A bridge method, generated by the compiler
#define ACC_VARARGS                     0x0080 // Declared with variable number of arguments
#define ACC_NATIVE                      0x0100 // Declared native; implemented in a language other than Java
#define ACC_STRICT                      0x0800 // Declared strictfp; floating-point mode is FP-strict

#define ACC_CLASS                       (ACC_PUBLIC | ACC_FINAL | ACC_SUPER | ACC_INTERFACE | ACC_ABSTRACT | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM)
#define ACC_NESTED_CLASS                (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM)
#define ACC_FIELD                       (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM)
#define ACC_METHOD                      (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC)

#define REF_getField            1
#define REF_getStatic           2
#define REF_putField            3
#define REF_putStatic           4
#define REF_invokeVirtual       5
#define REF_invokeStatic        6
#define REF_invokeSpecial       7
#define REF_newInvokeSpecial    8
#define REF_invokeInterface     9

// Cruise-specific constants
#define TAG_ATTR_CONSTANTVALUE                          0x1
#define TAG_ATTR_CODE                                   0x2
#define TAG_ATTR_STACKMAPTABLE                          0x4
#define TAG_ATTR_EXCEPTIONS                             0x8
#define TAG_ATTR_INNERCLASSES                           0x10
#define TAG_ATTR_ENCLOSINGMETHOD                        0x20
#define TAG_ATTR_SYNTHETIC                              0x40
#define TAG_ATTR_SIGNATURE                              0x80
#define TAG_ATTR_SOURCEFILE                             0x100
#define TAG_ATTR_SOURCEDEBUGEXTENSION                   0x200
#define TAG_ATTR_LINENUMBERTABLE                        0x400
#define TAG_ATTR_LOCALVARIABLETABLE                     0x800
#define TAG_ATTR_LOCALVARIABLETYPETABLE                 0x1000
#define TAG_ATTR_DEPRECATED                             0x2000
#define TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS              0x4000
#define TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS            0x8000
#define TAG_ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS     0x10000
#define TAG_ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS   0x20000
#define TAG_ATTR_ANNOTATIONDEFAULT                      0x40000
#define TAG_ATTR_BOOTSTRAPMETHODS                       0x80000
#define TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS          0x100000
#define TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS        0x200000
#define TAG_ATTR_METHODPARAMETERS                       0x400000

    typedef struct
    {
        u1 tag;
        void * data;
    } cp_info;

    typedef struct
    {
        u2 attribute_name_index;
        u4 attribute_length;
        // Cruise-specific members
        u4 tag;
        void *data;
    } attr_info;

#if VER_CMP(45, 3)
    struct attr_SourceFile_info
    {
        u2 sourcefile_index;
    };
    struct classes_entry
    {
        u2 inner_class_info_index;
        u2 outer_class_info_index;
        u2 inner_name_index;
        u2 inner_class_access_flags;
    };
    struct attr_InnerClasses_info
    {
        u2 number_of_classes;
        struct classes_entry *classes;
    };
    struct attr_Synthetic_info {};
    struct attr_Deprecated_info {};
    struct line_number_table_entry
    {
        u2 start_pc;
        u2 line_number;
    };
    struct attr_LineNumberTable_info
    {
        u2 line_number_table_length;
        struct line_number_table_entry line_number_table[];
    };
    struct local_variable_table_entry
    {
        u2 start_pc;
        u2 length;
        u2 name_index;
        u2 descriptor_index;
        u2 index;
    };
    struct attr_LocalVariableTable_info
    {
        u2 local_variable_table_length;
        struct local_variable_table_entry local_variable_table[];
    };

    struct attr_ConstantValue_info
    {
        u2 constantvalue_index;
    };

    struct exception_table_entry
    {
        u2 start_pc;
        u2 end_pc;
        u2 handler_pc;
        u2 catch_type;
    };
    struct attr_Code_info
    {
        u2 max_stack;
        u2 max_locals;
        u4 code_length;
        u1 *code;
        u2 exception_table_length;
        struct exception_table_entry *exception_table;
        u2 attributes_count;
        attr_info *attributes;

#ifdef QUICK_REFERENCE
#if VER_CMP(50, 0)
        u2 off_StackMapTable;
#endif
#if VER_CMP(52, 0)
        u2 off_RuntimeVisibleTypeAnnotations;
        u2 off_RuntimeInvisibleTypeAnnotations;
#endif
#endif
    };

    struct attr_Exceptions_info
    {
        u2 number_of_exceptions;
        u2 *exception_index_table;
    };
#endif /* VERSION 45.3 */
#if VER_CMP(49, 0)
    struct attr_EnclosingMethod_info
    {
        u2 class_index;
        u2 method_index;
    };
    struct element_value;
    struct element_value_pair
    {
        u2 element_name_index;
        struct element_value *value;
    };
    struct annotation
    {
        u2 type_index;
        u2 num_element_value_pairs;
        struct element_value_pair *element_value_pairs;
    };
    struct element_value
    {
        u1 tag;
        union
        {
            u2 const_value_index;
            
            struct
            {
                u2 type_name_index;
                u2 const_name_index;
            } enum_const_value;
            
            u2 class_info_index;
            
            struct annotation annotation_value;
            
            struct
            {
                u2 num_values;
                struct element_value *values;
            } array_value;
        };
    };
    struct parameter_annotation
    {
        u2 num_annotations;
        struct annotation *annotations;
    };
    struct attr_RuntimeVisibleParameterAnnotations_info
    {
        u1 num_parameters;
        struct parameter_annotation parameter_annotations[];
    };
    struct attr_RuntimeInvisibleParameterAnnotations_info
    {
        u1 num_parameters;
        struct parameter_annotation parameter_annotations[];
    };
    struct attr_AnnotationDefault_info
    {
        struct element_value default_value;
    };
    struct attr_Signature_info
    {
        u2 signature_index;
    };
    struct attr_RuntimeVisibleAnnotations_info
    {
        u2 num_annotations;
        struct annotation annotations[];
    };
    struct attr_RuntimeInvisibleAnnotations_info
    {
        u2 num_annotations;
        struct annotation annotations[];
    };
    struct local_variable_type_table_entry
    {
        u2 start_pc;
        u2 length;
        u2 name_index;
        u2 signature_index;
        u2 index;
    };
    struct attr_LocalVariableTypeTable_info
    {
        u2 local_variable_type_table_length;
        struct local_variable_type_table_entry local_variable_type_table[];
    };
#endif /* VERSION 49.0 */
#if VER_CMP(50, 0)
#define SMF_SAME_MIN        0
#define SMF_SAME_MAX        63
#define SMF_SL1SI_MIN       64
#define SMF_SL1SI_MAX       127
#define SMF_SL1SIE          247
#define SMF_CHOP_MIN        248
#define SMF_CHOP_MAX        250
#define SMF_SAMEE           251
#define SMF_APPEND_MIN      252
#define SMF_APPEND_MAX      254
#define SMF_FULL            255
    
#define ITEM_Top                0
#define ITEM_Integer            1
#define ITEM_Float              2
#define ITEM_Long               4
#define ITEM_Double             3
#define ITEM_Null               5
#define ITEM_UninitializedThis  6
#define ITEM_Object             7
#define ITEM_Uninitialized      8
    union verification_type_info
    {
        struct
        {
            u1 tag;
        } Top_variable_info;
        struct
        {
            u1 tag;
        } Integer_variable_info;
        struct
        {
            u1 tag;
        } Float_variable_info;
        struct
        {
            u1 tag;
        } Long_variable_info;
        struct
        {
            u1 tag;
        } Double_variable_info;
        struct
        {
            u1 tag;
        } Null_variable_info;
        struct
        {
            u1 tag;
        } UninitializedThis_variable_info;
        struct
        {
            u1 tag;
            u2 cpool_index;
        } Object_variable_info;
        struct
        {
            u1 tag;
            u2 offset;
        } Uninitialized_variable_info;
    };
    union stack_map_frame
    {
        struct
        {
            u1 frame_type;
        } same_frame;
        struct
        {
            u1 frame_type;
            union verification_type_info stack;
        } same_locals_1_stack_item_frame;
        struct
        {
            u1 frame_type;
            u2 offset_delta;
            union verification_type_info stack;
        } same_locals_1_stack_item_frame_extended;
        struct
        {
            u1 frame_type;
            u2 offset_delta;
        } chop_frame;
        struct
        {
            u1 frame_type;
            u2 offset_delta;
        } same_frame_extended;
        struct
        {
            u1 frame_type;
            u2 offset_delta;
            union verification_type_info *stack;
        } append_frame;
        struct
        {
            u1 frame_type;
            u2 offset_delta;
            u2 number_of_locals;
            union verification_type_info *locals;
            u2 number_of_stack_items;
            union verification_type_info *stack;
        } full_frame;
    };
    struct attr_StackMapTable_info
    {
        u2 number_of_entries;
        union stack_map_frame entries[];
    };
#endif /* VERSION 50.0 */
#if VER_CMP(51, 0)
    struct bootstrap_method
    {
        u2 bootstrap_method_ref;
        u2 num_bootstrap_arguments;
        u2 *bootstrap_arguments;
    };
    struct attr_BootstrapMethods_info
    {
        u2 num_bootstrap_methods;
        struct bootstrap_method bootstrap_methods[];
    };
#endif /* VERSION 51.0 */
#if VER_CMP(52, 0)
    struct attr_MethodParameters_info;
    struct attr_RuntimeVisibleTypeAnnotations_info;
    struct attr_RuntimeInvisibleTypeAnnotations_info;
#endif /* VERSION 52.0 */

    typedef struct {
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        u2 attributes_count;
        attr_info *attributes;

#ifdef QUICK_REFERENCE
#if VER_CMP(45, 3)
        u2 off_ConstantValue;
#endif
#if VER_CMP(49, 0)
        u2 off_RuntimeVisibleAnnotations;
        u2 off_RuntimeInvisibleAnnotations;
#endif
#if VER_CMP(52, 0)
        u2 off_RuntimeVisibleTypeAnnotations;
        u2 off_RuntimeInvisibleTypeAnnotations;
#endif
#endif
    } field_info;

    typedef struct {
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        u2 attributes_count;
        attr_info *attributes;

#ifdef QUICK_REFERENCE
#if VER_CMP(45, 3)
        u2 off_Code;
        u2 off_Exceptions;
#endif
#if VER_CMP(49, 0)
        u2 off_RuntimeVisibleParameterAnnotations;
        u2 off_RuntimeInvisibleParameterAnnotations;
        u2 off_AnnotationDefault;
        u2 off_RuntimeVisibleAnnotations;
        u2 off_RuntimeInvisibleAnnotations;
#endif
#if VER_CMP(52, 0)
        u2 off_RuntimeVisibleTypeAnnotations;
        u2 off_RuntimeInvisibleTypeAnnotations;
#endif
#endif
    } method_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 name_index;
        } * data;
    } CONSTANT_Class_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } * data;
    } CONSTANT_Fieldref_info,
        CONSTANT_Methodref_info,
        CONSTANT_InterfaceMethodref_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 string_index;
        } * data;
    } CONSTANT_String_info;
    
    typedef struct
    {
        u1 tag;
        union
        {
            u4 bytes;
            float float_value;
        } * data;
    } CONSTANT_Integer_info,
        CONSTANT_Float_info;
    
    typedef struct
    {
        u1 tag;
        union
        {
            struct
            {
                u4 low_bytes;
                u4 high_bytes;
            };
            u8 long_value;
            double double_value;
        } * data;
    } CONSTANT_Long_info,
        CONSTANT_Double_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 name_index;
            u2 descriptor_index;
        } * data;
    } CONSTANT_NameAndType_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 length;
            u1 * bytes;
        } * data;
    } CONSTANT_Utf8_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u1 reference_kind;
            u2 reference_index;
        } * data;
    } CONSTANT_MethodHandle_info;
    
    typedef struct
    {
        u1 tag;
        struct {
            u2 descriptor_index;
        } * data;
    } CONSTANT_MethodType_info;

    typedef struct
    {
        u1 tag;
        struct {
            u2 bootstrap_method_attr_index;
            u2 name_and_type_index;
        } * data;
    } CONSTANT_InvokeDynamic_info;

    typedef struct
    {
        u4 magic;
        u2 minor_version, major_version;
        u2 constant_pool_count;
        cp_info * constant_pool;
        u2 access_flags;
        u2 this_class;
        u2 super_class;
        u2 interfaces_count;
        u2 * interfaces;
        u2 fields_count;
        field_info * fields;
        u2 methods_count;
        method_info * methods;
        u2 attributes_count;
        attr_info * attributes;

#ifdef QUICK_REFERENCE
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
#endif
    } ClassFile;

    struct AttributeFilter
    {
        u4 class_attribute_filter;
        u4 field_attribute_filter; 
        u4 method_attribute_filter;
        u4 code_attribute_filter;
    };

    extern CONSTANT_Class_info *getConstant_Class(ClassFile *, u2);
    extern CONSTANT_Fieldref_info *getConstant_Fieldref(ClassFile *, u2);
    extern CONSTANT_Methodref_info *getConstant_Methodref(ClassFile *, u2);
    extern CONSTANT_InterfaceMethodref_info *getConstant_InterfaceMethodref(ClassFile *, u2);
    extern CONSTANT_String_info *getConstant_String(ClassFile *, u2);
    extern CONSTANT_Integer_info *getConstant_Integer(ClassFile *, u2);
    extern CONSTANT_Float_info *getConstant_Float(ClassFile *, u2);
    extern CONSTANT_Long_info *getConstant_Long(ClassFile *, u2);
    extern CONSTANT_Double_info *getConstant_Double(ClassFile *, u2);
    extern CONSTANT_NameAndType_info *getConstant_NameAndType(ClassFile *, u2);
    extern CONSTANT_Utf8_info *getConstant_Utf8(ClassFile *, u2);
    extern CONSTANT_MethodHandle_info *getConstant_MethodHandle(ClassFile *, u2);
    extern CONSTANT_MethodType_info *getConstant_MethodType(ClassFile *, u2);
    extern CONSTANT_InvokeDynamic_info *getConstant_InvokeDynamic(ClassFile *, u2);

    extern u2 getConstant_Utf8Length(ClassFile *, u2);
    extern u1 *getConstant_Utf8String(ClassFile *, u2);
    extern u1 *getConstant_ClassName(ClassFile *, u2);

    extern int loadAttributes_class(ClassFile *, struct BufferIO *, u2 *, attr_info **);
    extern int loadAttributes_field(ClassFile *, struct BufferIO *, field_info *, u2 *, attr_info **);
    extern int loadAttributes_method(ClassFile *, struct BufferIO *, method_info *, u2 *, attr_info **);
    extern int loadAttributes_code(ClassFile *, struct BufferIO *, u2 *, attr_info **);

    extern int freeAttributes_class(ClassFile *, u2, attr_info *);
    extern int freeAttributes_field(ClassFile *, u2, attr_info *);
    extern int freeAttributes_method(ClassFile *, u2, attr_info *);
    extern int freeAttributes_code(ClassFile *, u2, attr_info *);
    
    extern int disassembleCode(u4, u1 *);

    extern int parseClassfile(struct BufferIO *, ClassFile *);
    extern int freeClassfile(ClassFile *);

    extern int compareVersion(u2, u2);
    
    extern int isFieldDescriptor(u2, u1 *);
    extern int getMethodParametersCount(ClassFile *, u2);
    extern u1 *getClassSimpleName(ClassFile *, u2);
#ifdef	__cplusplus
}
#endif
#endif	/* JAVA_H */
