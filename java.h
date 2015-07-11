#ifndef JAVA_H
#define	JAVA_H

#include <zip.h>

#include "input.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define MINOR_VERSION                   3
#define MAJOR_VERSION                   45
    
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

// Cruise-specific constants
#define TAG_ATTR_CONSTANTVALUE                          1
#define TAG_ATTR_CODE                                   2
#define TAG_ATTR_STACKMAPTABLE                          3
#define TAG_ATTR_EXCEPTIONS                             4
#define TAG_ATTR_INNERCLASSES                           5
#define TAG_ATTR_ENCLOSINGMETHOD                        6
#define TAG_ATTR_SYNTHETIC                              7
#define TAG_ATTR_SIGNATURE                              8
#define TAG_ATTR_SOURCEFILE                             9
#define TAG_ATTR_SOURCEDEBUGEXTENSION                   10
#define TAG_ATTR_LINENUMBERTABLE                        11
#define TAG_ATTR_LOCALVARIABLETABLE                     12
#define TAG_ATTR_LOCALVARIABLETYPETABLE                 13
#define TAG_ATTR_DEPRECATED                             14
#define TAG_ATTR_RUNTIMEVISIBLEANNOTATIONS              15
#define TAG_ATTR_RUNTIMEINVISIBLEANNOTATIONS            16
#define TAG_ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS     17
#define TAG_ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS   18
#define TAG_ATTR_ANNOTATIONDEFAULT                      19
#define TAG_ATTR_BOOTSTRAPMETHODS                       20
#define TAG_ATTR_RUNTIMEVISIBLETYPEANNOTATIONS          21
#define TAG_ATTR_RUNTIMEINVISIBLETYPEANNOTATIONS        22

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
        u2 tag;
        void *data;
    } attr_info;

#if VER_CMP(45, 3)
    struct attr_SourceFile_info
    {
        u2 sourcefile_index;
    };
    struct attr_InnerClasses_info
    {
        u2 number_of_classes;
        struct classes_entry
        {
            u2 inner_class_info_index;
            u2 outer_class_info_index;
            u2 inner_name_index;
            u2 inner_class_access_flags;
        } *classes;
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

    struct attr_Code_info
    {
        u2 max_stack;
        u2 max_locals;
        u4 code_length;
        u1 *code;
        u2 exception_table_length;
        struct exception_table_entry
        {
            u2 start_pc;
            u2 end_pc;
            u2 handler_pc;
            u2 catch_type;
        } *exception_table;
        u2 attributes_count;
        attr_info *attributes;

#ifdef QUICK_REFERENCE
        attr_info *attrStackMapTable;

        attr_info *attrRuntimeVisibleTypeAnnotations;
        attr_info *attrRuntimeInvisibleTypeAnnotations;
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
    struct attr_RuntimeVisibleParameterAnnotations;
    struct attr_RuntimeInvisibleParameterAnnotations;
    struct attr_AnnotationDefault_info;
    struct attr_Signature_info
    {
        u2 signature_index;
    };
    struct attr_RuntimeVisibleAnnotations;
    struct attr_RuntimeInvisibleAnnotations;
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
    struct attr_StackMapTable_info
    {
        u2 number_of_entries;
        struct stack_map_frame
        {
        } *entries;
    };
#endif /* VERSION 50.0 */
#if VER_CMP(51, 0)
    struct attr_BootstrapMethods_info
    {
        u2 num_bootstrap_methods;
        strcut bootstrap_method
        {
            u2 bootstrap_method_ref;
            u2 num_bootstrap_arguments;
            u2 *bootstrap_arguments;
        } bootstrap_methods[];
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

#if QUICK_REFERENCE
        attr_info *attrConstantValue;

        attr_info *attrRuntimeVisibleAnnotations;
        attr_info *attrRuntimeInvisibleAnnotations;

        attr_info *attrRuntimeVisibleTypeAnnotations;
        attr_info *attrRuntimeInvisibleTypeAnnotations;
#endif
    } field_info;

    typedef struct {
        u2 access_flags;
        u2 name_index;
        u2 descriptor_index;
        u2 attributes_count;
        attr_info *attributes;

#ifdef QUICK_REFERENCE
        attr_info *attrCode;
        attr_info *attrExceptions;

        attr_info *attrRuntimeVisibleAnnotations;
        attr_info *attrRuntimeInvisibleAnnotations;

        attr_info *attrRuntimeVisibleParameterAnnotations;
        attr_info *attrRuntimeInvisibleParameterAnnotations;

        attr_info *attrRuntimeVisibleTypeAnnotations;
        attr_info *attrRuntimeInvisibleTypeAnnotations;

        attr_info *attrAnnotationDefault;
        attr_info *attrMethodParameters;
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
        attr_info *attrEnclosingMethod;
        attr_info *attrSourceFile;
        attr_info *attrSourceDebugExtension;

        attr_info *attrRuntimeVisibleAnnotations;
        attr_info *attrRuntimeInvisibleAnnotations;

        attr_info *attrRuntimeVisibleTypeAnnotations;
        attr_info *attrRuntimeInvisibleTypeAnnotations;

        attr_info *attrBootstrapMethods;
#endif
    } ClassFile;

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
    extern char *getConstant_Utf8String(ClassFile *, u2);

    extern int loadAttributes_class(ClassFile *, struct BufferIO *, u2 *, attr_info **);
    extern int loadAttributes_field(ClassFile *, struct BufferIO *, u2 *, attr_info **);
    extern int loadAttributes_method(ClassFile *, struct BufferIO *, u2 *, attr_info **);
    extern int loadAttributes_code(ClassFile *, struct BufferIO *, u2 *, attr_info **);
    extern int freeAttributes_class(u2, attr_info *);
    extern int freeAttributes_field(u2, attr_info *);
    extern int freeAttributes_method(u2, attr_info *);
    extern int freeAttributes_code(u2, attr_info *);

    extern int parseClassfile(struct BufferIO *, ClassFile *);
    extern int freeClassfile(ClassFile *);

    extern int compareVersion(u2, u2);
#ifdef	__cplusplus
}
#endif
#endif	/* JAVA_H */
