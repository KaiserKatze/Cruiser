#ifndef OPCODE_H
#define OPCODE_H

// Constants

#define OPCODE_nop              0x00
#define OPCODE_aconst_null      0x01
#define OPCODE_iconst_ml        0x02
#define OPCODE_iconst_0         0x03
#define OPCODE_iconst_1         0x04
#define OPCODE_iconst_2         0x05
#define OPCODE_iconst_3         0x06
#define OPCODE_iconst_4         0x07
#define OPCODE_iconst_5         0x08
#define OPCODE_lconst_0         0x09
#define OPCODE_lconst_1         0x0a
#define OPCODE_fconst_0         0x0b
#define OPCODE_fconst_1         0x0c
#define OPCODE_fconst_2         0x0d
#define OPCODE_dconst_0         0x0e
#define OPCODE_dconst_1         0x0f
#define OPCODE_bipush           0x10
#define OPCODE_sipush           0x11
#define OPCODE_ldc              0x12
#define OPCODE_ldc_w            0x13
#define OPCODE_ldc2_w           0x14

// Loads
#define OPCODE_iload            0x15
#define OPCODE_lload            0x16
#define OPCODE_fload            0x17
#define OPCODE_dload            0x18
#define OPCODE_aload            0x19
#define OPCODE_iload_0          0x1a
#define OPCODE_iload_1          0x1b
#define OPCODE_iload_2          0x1c
#define OPCODE_iload_3          0x1d
#define OPCODE_lload_0          0x1e
#define OPCODE_lload_1          0x1f
#define OPCODE_lload_2          0x20
#define OPCODE_lload_3          0x21
#define OPCODE_fload_0          0x22
#define OPCODE_fload_1          0x23
#define OPCODE_fload_2          0x24
#define OPCODE_fload_3          0x25
#define OPCODE_dload_0          0x26
#define OPCODE_dload_1          0x27
#define OPCODE_dload_2          0x28
#define OPCODE_dload_3          0x29
#define OPCODE_aload_0          0x2a
#define OPCODE_aload_1          0x2b
#define OPCODE_aload_2          0x2c
#define OPCODE_aload_3          0x2d
#define OPCODE_iaload           0x2e
#define OPCODE_laload           0x2f
#define OPCODE_faload           0x30
#define OPCODE_daload           0x31
#define OPCODE_aaload           0x32
#define OPCODE_baload           0x33
#define OPCODE_caload           0x34
#define OPCODE_saload           0x35

// Stores
#define OPCODE_istore           0x36
#define OPCODE_lstore           0x37
#define OPCODE_fstore           0x38
#define OPCODE_dstore           0x39
#define OPCODE_astore           0x3a
#define OPCODE_istore_0         0x3b
#define OPCODE_istore_1         0x3c
#define OPCODE_istore_2         0x3d
#define OPCODE_istore_3         0x3e
#define OPCODE_lstore_0         0x3f
#define OPCODE_lstore_1         0x40
#define OPCODE_lstore_2         0x41
#define OPCODE_lstore_3         0x42
#define OPCODE_fstore_0         0x43
#define OPCODE_fstore_1         0x44
#define OPCODE_fstore_2         0x45
#define OPCODE_fstore_3         0x46
#define OPCODE_dstore_0         0x47
#define OPCODE_dstore_1         0x48
#define OPCODE_dstore_2         0x49
#define OPCODE_dstore_3         0x4a
#define OPCODE_astore_0         0x4b
#define OPCODE_astore_1         0x4c
#define OPCODE_astore_2         0x4d
#define OPCODE_astore_3         0x4e
#define OPCODE_iastore          0x4f
#define OPCODE_lastore          0x50
#define OPCODE_fastore          0x51
#define OPCODE_dastore          0x52
#define OPCODE_aastore          0x53
#define OPCODE_bastore          0x54
#define OPCODE_castore          0x55
#define OPCODE_sastore          0x56

// Stack
#define OPCODE_pop              0x57
#define OPCODE_pop2             0x58
#define OPCODE_dup              0x59
#define OPCODE_dup_x1           0x5a
#define OPCODE_dup_x2           0x5b
#define OPCODE_dup2             0x5c
#define OPCODE_dup2_x1          0x5d
#define OPCODE_dup2_x2          0x5e
#define OPCODE_swap             0x5f

// Math
#define OPCODE_iadd             0x60
#define OPCODE_ladd             0x61
#define OPCODE_fadd             0x62
#define OPCODE_dadd             0x63
#define OPCODE_isub             0x64
#define OPCODE_lsub             0x65
#define OPCODE_fsub             0x66
#define OPCODE_dsub             0x67
#define OPCODE_imul             0x68
#define OPCODE_lmul             0x69
#define OPCODE_fmul             0x6a
#define OPCODE_dmul             0x6b
#define OPCODE_idiv             0x6c
#define OPCODE_ldiv             0x6d
#define OPCODE_fdiv             0x6e
#define OPCODE_ddiv             0x6f
#define OPCODE_irem             0x70
#define OPCODE_lrem             0x71
#define OPCODE_frem             0x72
#define OPCODE_drem             0x73
#define OPCODE_ineg             0x74
#define OPCODE_lneg             0x75
#define OPCODE_fneg             0x76
#define OPCODE_dneg             0x77
#define OPCODE_ishl             0x78
#define OPCODE_lshl             0x79
#define OPCODE_ishr             0x7a
#define OPCODE_lshr             0x7b
#define OPCODE_iushr            0x7c
#define OPCODE_lushr            0x7d
#define OPCODE_iand             0x7e
#define OPCODE_land             0x7f
#define OPCODE_ior              0x80
#define OPCODE_lor              0x81
#define OPCODE_ixor             0x82
#define OPCODE_lxor             0x83
#define OPCODE_iinc             0x84

//Conversions
#define OPCODE_i2l              0x85
#define OPCODE_i2f              0x86
#define OPCODE_i2d              0x87
#define OPCODE_l2i              0x88
#define OPCODE_l2f              0x89
#define OPCODE_l2d              0x8a
#define OPCODE_f2i              0x8b
#define OPCODE_f2l              0x8c
#define OPCODE_f2d              0x8d
#define OPCODE_d2i              0x8e
#define OPCODE_d2l              0x8f
#define OPCODE_d2f              0x90
#define OPCODE_i2b              0x91
#define OPCODE_i2c              0x92
#define OPCODE_i2s              0x93

// Comparisons
#define OPCODE_lcmp             0x94
#define OPCODE_fcmpl            0x95
#define OPCODE_fcmpg            0x96
#define OPCODE_dcmpl            0x97
#define OPCODE_dcmpg            0x98
#define OPCODE_ifeq             0x99
#define OPCODE_ifne             0x9a
#define OPCODE_iflt             0x9b
#define OPCODE_ifge             0x9c
#define OPCODE_ifgt             0x9d
#define OPCODE_ifle             0x9e
#define OPCODE_if_icmpeq        0x9f
#define OPCODE_if_icmpne        0xa0
#define OPCODE_if_icmplt        0xa1
#define OPCODE_if_icmpge        0xa2
#define OPCODE_if_icmpgt        0xa3
#define OPCODE_if_icmple        0xa4
#define OPCODE_if_acmpeq        0xa5
#define OPCODE_if_acmpne        0xa6

// Control
#define OPCODE_goto             0xa7
#define OPCODE_jsr              0xa8
#define OPCODE_ret              0xa9
#define OPCODE_tableswitch      0xaa
#define OPCODE_lookupswitch     0xab
#define OPCODE_ireturn          0xac
#define OPCODE_lreturn          0xad
#define OPCODE_freturn          0xae
#define OPCODE_dreturn          0xaf
#define OPCODE_areturn          0xb0
#define OPCODE_return           0xb1

// References
#define OPCODE_getstatic        0xb2
#define OPCODE_putstatic        0xb3
#define OPCODE_getfield         0xb4
#define OPCODE_putfield         0xb5
#define OPCODE_invokevirtual    0xb6
#define OPCODE_invokespecial    0xb7
#define OPCODE_invokestatic     0xb8
#define OPCODE_invokeinterface  0xb9
#define OPCODE_invokedynamic    0xba
#define OPCODE_new              0xbb
#define OPCODE_newarray         0xbc
#define OPCODE_anewarray        0xbd
#define OPCODE_arraylength      0xbe
#define OPCODE_athrow           0xbf
#define OPCODE_checkcast        0xc0
#define OPCODE_instanceof       0xc1
#define OPCODE_monitorenter     0xc2
#define OPCODE_monitorexit      0xc3

// Extended
#define OPCODE_wide             0xc4
#define OPCODE_multianewarray   0xc5
#define OPCODE_ifnull           0xc6
#define OPCODE_ifnonnull        0xc7
#define OPCODE_goto_w           0xc8
#define OPCODE_jsr_w            0xc9

// Reserved
#define OPCODE_breakpoint       0xca
#define OPCODE_impdep1          0xfe
#define OPCODE_impdep2          0xff

#endif /* OPCODE_H */
