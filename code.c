#include <stdlib.h>
#include <string.h>

#include "java.h"
#include "opcode.h"
#include "log.h"

#define T_BOOLEAN   4
#define T_CHAR      5
#define T_FLOAT     6
#define T_DOUBLE    7
#define T_BYTE      8
#define T_SHORT     9
#define T_INT       10
#define T_LONG      11

extern int
disassembleCode(u4 code_length, u1 *code)
{
    u4 j;
    int _defaultbyte, _lowbyte, _highbyte, _npairs, i;
    logInfo("\t\t// Human-readable =\r\n");
    for (j = 0u; j < code_length; j++)
    {
        logInfo("\t\t%8i: ", j);
        switch (code[j])
        {
            case OPCODE_nop:
                logInfo("nop\r\n");
                // do nothing
                break;
            case OPCODE_aconst_null:
                logInfo("aconst_null\r\n");
                // ... -> ..., null
                break;
            case OPCODE_iconst_ml:break;
            /* push value */
            case OPCODE_iconst_0:
                logInfo("iconst_0\r\n");
                break;
            case OPCODE_iconst_1:
                logInfo("iconst_1\r\n");
                break;
            case OPCODE_iconst_2:
                logInfo("iconst_2\r\n");
                break;
            case OPCODE_iconst_3:
                logInfo("iconst_3\r\n");
                break;
            case OPCODE_iconst_4:
                logInfo("iconst_4\r\n");
                break;
            case OPCODE_iconst_5:
                logInfo("iconst_5\r\n");
                break;
            case OPCODE_lconst_0:
                logInfo("lconst_0\r\n");
                break;
            case OPCODE_lconst_1:
                logInfo("lconst_1\r\n");
                break;
            case OPCODE_fconst_0:
                logInfo("fconst_0\r\n");
                break;
            case OPCODE_fconst_1:
                logInfo("fconst_1\r\n");
                break;
            case OPCODE_fconst_2:
                logInfo("fconst_2\r\n");
                break;
            case OPCODE_dconst_0:
                logInfo("dconst_0\r\n");
                break;
            case OPCODE_dconst_1:
                logInfo("dconst_1\r\n");
                break;
            case OPCODE_bipush:
                logInfo("bipush %i\r\n", code[++j]);
                // bipush 'byte'
                // ... -> ..., value
                // The immediate 'byte' is sign-extended to an 'int' value
                break;
            case OPCODE_sipush:
                logInfo("sipush %i\r\n", (code[++j] << 8) | code[++j]);
                // push short
                // ... -> ..., value
                break;
            case OPCODE_ldc:
                logInfo("ldc %i\r\n", code[++j]);
                // push item from runtime constant pool
                // ... -> ..., value
                break;
            case OPCODE_ldc_w:
                logInfo("ldc_w %i\r\n", (code[++j] << 8) | code[++j]);
                // push item from runtime constant pool (wide index)
                break;
            case OPCODE_ldc2_w:
                logInfo("ldc2_w %i\r\n", (code[++j] << 8) | code[++j]);
                // push long or double from runtime constant pool (wide index)
                break;
            /* load value from local variable */
            // ... -> ..., objectref
            case OPCODE_iload:
                logInfo("iload %i\r\n", code[++j]);
                break;
            case OPCODE_lload:
                logInfo("lload %i\r\n", code[++j]);
                break;
            case OPCODE_fload:
                logInfo("fload %i\r\n", code[++j]);
                break;
            case OPCODE_dload:
                logInfo("dload %i\r\n", code[++j]);
                break;
            case OPCODE_aload:
                logInfo("aload %i\r\n", code[++j]);
                break;
            case OPCODE_iload_0:
                logInfo("iload_0\r\n");
                break;
            case OPCODE_iload_1:
                logInfo("iload_1\r\n");
                break;
            case OPCODE_iload_2:
                logInfo("iload_2\r\n");
                break;
            case OPCODE_iload_3:
                logInfo("iload_3\r\n");
                break;
            case OPCODE_lload_0:
                logInfo("lload_0\r\n");
                break;
            case OPCODE_lload_1:
                logInfo("lload_1\r\n");
                break;
            case OPCODE_lload_2:
                logInfo("lload_2\r\n");
                break;
            case OPCODE_lload_3:
                logInfo("lload_3\r\n");
                break;
            case OPCODE_fload_0:
                logInfo("fload_0\r\n");
                break;
            case OPCODE_fload_1:
                logInfo("fload_1\r\n");
                break;
            case OPCODE_fload_2:
                logInfo("fload_2\r\n");
                break;
            case OPCODE_fload_3:
                logInfo("fload_3\r\n");
                break;
            case OPCODE_dload_0:
                logInfo("dload_0\r\n");
                break;
            case OPCODE_dload_1:
                logInfo("dload_1\r\n");
                break;
            case OPCODE_dload_2:
                logInfo("dload_2\r\n");
                break;
            case OPCODE_dload_3:
                logInfo("dload_3\r\n");
                break;
            case OPCODE_aload_0:
                logInfo("aload_0\r\n");
                break;
            case OPCODE_aload_1:
                logInfo("aload_1\r\n");
                break;
            case OPCODE_aload_2:
                logInfo("aload_2\r\n");
                break;
            case OPCODE_aload_3:
                logInfo("aload_3\r\n");
                break;
            /* load value from array with given index */
            // ..., arrayref, index -> ..., value
            case OPCODE_iaload:
                logInfo("iaload\r\n");
                break;
            case OPCODE_laload:
                logInfo("laload\r\n");
                break;
            case OPCODE_faload:
                logInfo("faload\r\n");
                break;
            case OPCODE_daload:
                logInfo("daload\r\n");
                break;
            case OPCODE_aaload:
                logInfo("aaload\r\n");
                break;
            case OPCODE_baload:
                logInfo("baload\r\n");
                break;
            case OPCODE_caload:
                logInfo("caload\r\n");
                break;
            case OPCODE_saload:
                logInfo("saload\r\n");
                break;
            /* store value into given index of an array */
            case OPCODE_istore:
                logInfo("istore %i\r\n", code[++j]);
                break;
            case OPCODE_lstore:
                logInfo("lstore %i\r\n", code[++j]);
                break;
            case OPCODE_fstore:
                logInfo("fstore %i\r\n", code[++j]);
                break;
            case OPCODE_dstore:
                logInfo("dstore %i\r\n", code[++j]);
                break;
            case OPCODE_astore:
                logInfo("astore %i\r\n", code[++j]);
                break;
            case OPCODE_istore_0:
                logInfo("istore_0\r\n");
                break;
            case OPCODE_istore_1:
                logInfo("istore_1\r\n");
                break;
            case OPCODE_istore_2:
                logInfo("istore_2\r\n");
                break;
            case OPCODE_istore_3:
                logInfo("istore_3\r\n");
                break;
            case OPCODE_lstore_0:
                logInfo("lstore_0\r\n");
                break;
            case OPCODE_lstore_1:
                logInfo("lstore_1\r\n");
                break;
            case OPCODE_lstore_2:
                logInfo("lstore_2\r\n");
                break;
            case OPCODE_lstore_3:
                logInfo("lstore_3\r\n");
                break;
            case OPCODE_fstore_0:
                logInfo("fstore_0\r\n");
                break;
            case OPCODE_fstore_1:
                logInfo("fstore_1\r\n");
                break;
            case OPCODE_fstore_2:
                logInfo("fstore_2\r\n");
                break;
            case OPCODE_fstore_3:
                logInfo("fstore_3\r\n");
                break;
            case OPCODE_dstore_0:
                logInfo("dstore_0\r\n");
                break;
            case OPCODE_dstore_1:
                logInfo("dstore_1\r\n");
                break;
            case OPCODE_dstore_2:
                logInfo("dstore_2\r\n");
                break;
            case OPCODE_dstore_3:
                logInfo("dstore_3\r\n");
                break;
            case OPCODE_astore_0:
                logInfo("astore_0\r\n");
                break;
            case OPCODE_astore_1:
                logInfo("astore_1\r\n");
                break;
            case OPCODE_astore_2:
                logInfo("astore_2\r\n");
                break;
            case OPCODE_astore_3:
                logInfo("astore_3\r\n");
                break;
            /* store into array */
            // ..., arrayref, index, value -> ...
            case OPCODE_iastore:
                logInfo("iastore\r\n");
                break;
            case OPCODE_lastore:
                logInfo("lastore\r\n");
                break;
            case OPCODE_fastore:
                logInfo("fastore\r\n");
                break;
            case OPCODE_dastore:
                logInfo("dastore\r\n");
                break;
            case OPCODE_aastore:
                logInfo("aastore\r\n");
                break;
            case OPCODE_bastore:
                logInfo("bastore\r\n");
                break;
            case OPCODE_castore:
                logInfo("castore\r\n");
                break;
            case OPCODE_sastore:
                logInfo("sastore\r\n");
                break;
            /* stack access */
            case OPCODE_pop:
                logInfo("pop\r\n");
                // pop the top operand stack value
                // ..., value -> ...
                break;
            case OPCODE_pop2:
                logInfo("pop2\r\n");
                // pop the top one or two operand stack values
                // (1) where each of value1 and value2 is a value of a category 1 computational type
                // ..., value2, value1 -> ...
                // (2) where value is a value of a category 2 computational type
                // ..., value -> ...
                break;
            case OPCODE_dup:
                logInfo("dup\r\n");
                // duplicate the top operand stack value
                // ..., value -> ..., value, value
                break;
            case OPCODE_dup_x1:
                logInfo("dup_x1\r\n");
                // duplicate the top operand stack value and insert two values down
                // ..., value2, value1 -> ..., value1, value2, value1
                break;
            case OPCODE_dup_x2:
                logInfo("dup_x2\r\n");
                // duplicate the top operand stack value and insert two or three values down
                // (1) when value1, value2, and value3 are all values of a category 1 computational type (§2.11.1)
                // ..., value3, value2, value1 -> ..., value1, value3, value2, value1
                // (2) when value1 is a value of a category 1 computational type and valu2 is a value of a category 2 computational type (§2.11.1)
                // ..., value2, value1 -> ..., value1, value2, value1
                break;
            case OPCODE_dup2:
                logInfo("dup2\r\n");
                // duplicate the top one or two operand stack values
                // (1) where both value1 and value2 are values of a category 1 computational type
                // ..., value2, value1 -> ..., value2, value1, value2, value1
                // (2) where value is a value of a category 2 computational type
                // ..., value -> value, value
                break;
            case OPCODE_dup2_x1:
                logInfo("dup2_x1\r\n");
                // duplicate the top one or two operand stack values and insert two or three values down
                // (1) where value1, value2 and value3 are all values of a category 1 computational type
                // ..., value3, value2, value1 -> ..., value2, value1, value3, value2, value1
                // (2) where value1 is a value of a category 2 computational type and value2 is a value of category 1 computational type
                // ..., value2, value1 -> ..., value1, value2, value1
                break;
            case OPCODE_dup2_x2:
                logInfo("dup2_x2\r\n");
                // (1) where value1, value2, value3, and value4 are all values of a category 1 computational type
                // ..., value4, value3, value2, value1 -> ..., value2, value1, value4, value2, value1
                // (2) where value 1 is a value of a category 2 computational type and value2 and value3 are both values of a category 1 computational type
                // ..., value3, value2, value1 -> ..., value1, value3, value2, value1
                break;
            case OPCODE_swap:
                logInfo("swap\r\n");
                // swap the top two operand stack values
                break;
            /* arithmetic calculation */
            case OPCODE_iadd:
                logInfo("iadd\r\n");
                break;
            case OPCODE_ladd:
                logInfo("ladd\r\n");
                break;
            case OPCODE_fadd:
                logInfo("fadd\r\n");
                break;
            case OPCODE_dadd:
                logInfo("dadd\r\n");
                break;
            case OPCODE_isub:
                logInfo("isub\r\n");
                break;
            case OPCODE_lsub:
                logInfo("lsub\r\n");
                break;
            case OPCODE_fsub:
                logInfo("fsub\r\n");
                break;
            case OPCODE_dsub:
                logInfo("dsub\r\n");
                break;
            case OPCODE_imul:
                logInfo("imul\r\n");
                break;
            case OPCODE_lmul:
                logInfo("lmul\r\n");
                break;
            case OPCODE_fmul:
                logInfo("fmul\r\n");
                break;
            case OPCODE_dmul:
                logInfo("dmul\r\n");
                break;
            case OPCODE_idiv:
                logInfo("idiv\r\n");
                break;
            case OPCODE_ldiv:
                logInfo("ldiv\r\n");
                break;
            case OPCODE_fdiv:
                logInfo("fdiv\r\n");
                break;
            case OPCODE_ddiv:
                logInfo("ddiv\r\n");
                break;
            /* remainder */
            case OPCODE_irem:
                logInfo("irem\r\n");
                break;
            case OPCODE_lrem:
                logInfo("lrem\r\n");
                break;
            case OPCODE_frem:
                logInfo("frem\r\n");
                break;
            case OPCODE_drem:
                logInfo("drem\r\n");
                break;
            /* negate value */
            case OPCODE_ineg:
                logInfo("ineg\r\n");
                break;
            case OPCODE_lneg:
                logInfo("lneg\r\n");
                break;
            case OPCODE_fneg:
                logInfo("fneg\r\n");
                break;
            case OPCODE_dneg:
                logInfo("dneg\r\n");
                break;
            /* shift */
            case OPCODE_ishl:
                logInfo("ishl\r\n");
                break;
            case OPCODE_lshl:
                logInfo("lshl\r\n");
                break;
            case OPCODE_ishr:
                logInfo("ishr\r\n");
                break;
            case OPCODE_lshr:
                logInfo("lshr\r\n");
                break;
            case OPCODE_iushr:
                logInfo("iushr\r\n");
                break;
            case OPCODE_lushr:
                logInfo("lushr\r\n");
                break;
            case OPCODE_iand:
                logInfo("iand\r\n");
                break;
            case OPCODE_land:
                logInfo("land\r\n");
                break;
            case OPCODE_ior:
                logInfo("ior\r\n");
                break;
            case OPCODE_lor:break;
            case OPCODE_ixor:
                logInfo("ixor\r\n");
                break;
            case OPCODE_lxor:
                logInfo("lxor\r\n");
                break;
            case OPCODE_iinc:
                logInfo("iinc %i %i\r\n", code[++j], code[++j]);
                // increment local variable by constant
                break;
            /* Type conversion */
            case OPCODE_i2l:
                logInfo("i2l\r\n");
                break;
            case OPCODE_i2f:
                logInfo("i2f\r\n");
                break;
            case OPCODE_i2d:
                logInfo("i2d\r\n");
                break;
            case OPCODE_l2i:
                logInfo("l2i\r\n");
                break;
            case OPCODE_l2f:
                logInfo("l2f\r\n");
                break;
            case OPCODE_l2d:
                logInfo("l2d\r\n");
                break;
            case OPCODE_f2i:
                logInfo("f2i\r\n");
                break;
            case OPCODE_f2l:
                logInfo("f2l\r\n");
                break;
            case OPCODE_f2d:
                logInfo("f2d\r\n");
                break;
            case OPCODE_d2i:
                logInfo("d2i\r\n");
                break;
            case OPCODE_d2l:
                logInfo("d2l\r\n");
                break;
            case OPCODE_d2f:
                logInfo("d2f\r\n");
                break;
            case OPCODE_i2b:
                logInfo("i2b\r\n");
                break;
            case OPCODE_i2c:
                logInfo("i2c\r\n");
                break;
            case OPCODE_i2s:
                logInfo("i2s\r\n");
                break;
            /* compare two values */
            // if 'value1' is greater than 'value2', then int value 1 is pushed
            // if equal, then int value 0 is pushed
            // if 'value1' is less than 'value2', int value -1 is pushed
            // otherwise, dcmpg pushes int value 1, while dcmpl pushes int value -1
            case OPCODE_lcmp:
                logInfo("lcmp\r\n");
                break;
            case OPCODE_fcmpl:
                logInfo("fcmpl\r\n");
                break;
            case OPCODE_fcmpg:
                logInfo("fcmpg\r\n");
                break;
            case OPCODE_dcmpl:
                logInfo("dcmpl\r\n");
                break;
            case OPCODE_dcmpg:
                logInfo("dcmpg\r\n");
                break;
            /* branch if int comparison with zero succeeds */
            case OPCODE_ifeq:
                logInfo("ifeq %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_ifne:
                logInfo("ifne %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_iflt:
                logInfo("iflt %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_ifge:
                logInfo("ifge %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_ifgt:
                logInfo("ifgt %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_ifle:
                logInfo("ifle %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            /* int value comparison */
            case OPCODE_if_icmpeq:
                logInfo("if_icmpeq %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_icmpne:
                logInfo("if_icmpne %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_icmplt:
                logInfo("if_icmplt %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_icmpge:
                logInfo("if_icmpge %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_icmpgt:
                logInfo("if_icmpgt %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_icmple:
                logInfo("if_icmple %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            /* branch if 'reference' comparison succeeds */
            case OPCODE_if_acmpeq:
                logInfo("if_acmpeq %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_if_acmpne:
                logInfo("if_acmpne %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_goto:
                logInfo("goto %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_jsr:
                logInfo("jsr %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_ret:
                logInfo("ret %i\r\n", code[++j]);
                // return from subroutine
                // no change on operand stack
                break;
            /* switch */
            case OPCODE_tableswitch:
                // make sure 'defaultbyte1' begins at an address that is a multiple of four bytes
                // move 'j' across padding part to 'defaultbyte1' location
                j += 4 - j % 4;
                _defaultbyte = (code[j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                _lowbyte = (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                _highbyte = (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                logInfo("tableswitch %i %i %i\r\n", _defaultbyte, _lowbyte, _highbyte);
                // both _lowbyte and _highbyte are inclusive
            case OPCODE_lookupswitch:
                // make sure 'defaultbyte1' begins at an address that is a multiple of four bytes
                // move 'j' across padding part to 'defaultbyte1' location
                j += 4 - j % 4;
                _defaultbyte = (code[j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                _npairs = (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                logInfo("lookupswitch %i %i {\r\n", _defaultbyte, _npairs);
                for (i = 0; i < _npairs; i++)
                {
                    logInfo("\t\t\t%i: %i\r\n",
                            (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j],
                            (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]);
                }
                logInfo("}\r\n");
                break;
            /* return value from method */
            case OPCODE_ireturn:
                logInfo("dreturn\r\n");
                break;
            case OPCODE_lreturn:
                logInfo("lreturn\r\n");
                break;
            case OPCODE_freturn:
                logInfo("freturn\r\n");
                break;
            case OPCODE_dreturn:
                logInfo("dreturn\r\n");
                break;
            case OPCODE_areturn:
                logInfo("areturn\r\n");
                break;
            case OPCODE_return:
                logInfo("return\r\n");
                // return void from method
                break;
            case OPCODE_getstatic:
                logInfo("getstatic %i\r\n", (code[++j] << 8) | code[++j]);
                // get static field from class
                break;
            case OPCODE_putstatic:
                logInfo("putstatic %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_getfield:
                logInfo("getfield %i\r\n", (code[++j] << 8) | code[++j]);
                // Fetch field from object
                break;
            case OPCODE_putfield:
                logInfo("putfield %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokevirtual:
                logInfo("invokevirtual %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokespecial:
                logInfo("invokespecial %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokestatic:
                logInfo("invokestatic %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokeinterface:
                logInfo("invokeinterface %i %i\r\n", (code[++j] << 8) | code[++j], code[++j]);
                ++j;
                // @see jvm7.pdf:p479
                break;
            case OPCODE_invokedynamic:
                logInfo("invokedynamic %i\r\n", (code[++j] << 8) | code[++j]);
                j += 2;
                // @see jvms7.pdf:p474
                break;
            case OPCODE_new:
                logInfo("new %i\r\n", (code[++j] << 8) | code[++j]);
                // create new object
                break;
            case OPCODE_newarray:
                switch (code[++j])
                {
                    case T_BOOLEAN:
                        logInfo("newarray T_BOOLEAN\r\n");
                        break;
                    case T_CHAR:
                        logInfo("newarray T_CHAR\r\n");
                        break;
                    case T_FLOAT:
                        logInfo("newarray T_FLOAT\r\n");
                        break;
                    case T_DOUBLE:
                        logInfo("newarray T_DOUBLE\r\n");
                        break;
                    case T_BYTE:
                        logInfo("newarray T_BYTE\r\n");
                        break;
                    case T_SHORT:
                        logInfo("newarray T_SHORT\r\n");
                        break;
                    case T_INT:
                        logInfo("newarray T_INT\r\n");
                        break;
                    case T_LONG:
                        logInfo("newarray T_LONG\r\n");
                        break;
                    default:
                        logError("Unknow atype[%i]!\r\n", code[j]);
                        return -1;
                }
                break;
            case OPCODE_anewarray:
                logInfo("anewarray %i %i\r\n", code[++j], code[++j]);
                break;
            case OPCODE_arraylength:
                logInfo("arraylength\r\n");
                break;
            case OPCODE_athrow:
                logInfo("athrow\r\n");
                break;
            case OPCODE_checkcast:
                logInfo("checkcast %i\r\n", (code[++j] << 8) | code[++j]);
                // checkcast 'indexbyte1' 'indexbyte2'
                // ..., objectref -> ..., objectref
                break;
            case OPCODE_instanceof:
                logInfo("instanceof %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_monitorenter:
                logInfo("monitorenter\r\n");
                // enter monitor for object
                break;
            case OPCODE_monitorexit:
                logInfo("monitorexit\r\n");
                // exit monitor for object
                break;
            case OPCODE_wide:
                // extend local variable index by additional bytes
                switch (code[++j])
                {
                    // (1) where <opcode> is one of <T>load or ret
                    case OPCODE_iload:
                        logInfo("wide iload %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_fload:
                        logInfo("wide fload %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_aload:
                        logInfo("wide aload %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_lload:
                        logInfo("wide lload %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_dload:
                        logInfo("wide dload %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_istore:
                        logInfo("wide istore %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_fstore:
                        logInfo("wide fstore %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_astore:
                        logInfo("wide astore %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_lstore:
                        logInfo("wide lstore %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_dstore:
                        logInfo("wide dstore %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    case OPCODE_ret:
                        logInfo("wide ret %i\r\n", (code[++j] << 8) | code[++j]);
                        break;
                    // (2) iinc
                    case OPCODE_iinc:
                        logInfo("wide iinc %i %i\r\n", (code[++j] << 8) | code[++j], (code[++j] << 8) | code[++j]);
                        break;
                    default:
                        logError("Unsupported wide opcode[%i]!\r\n", code[j]);
                        return -1;
                }
                break;
            case OPCODE_multianewarray:
                logInfo("multianewarray %i %i\r\n", (code[++j] << 8) | code[++j], code[++j]);
                // create new multi-dimensional array
                break;
            /* branch if reference null */
            case OPCODE_ifnull:
                logInfo("ifnull %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            /* branch if reference not null */
            case OPCODE_ifnonnull:
                logInfo("ifnonnull %i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_goto_w:
                logInfo("goto_w %lli\r\n", (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_jsr_w:
                logInfo("jsr_w %lli\r\n", (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]);
                break;
            /* reserved */
            case OPCODE_breakpoint:
                logInfo("breakpoint\r\n");
                break;
            case OPCODE_impdep1:
                logInfo("impdep1\r\n");
                break;
            case OPCODE_impdep2:
                logInfo("impdep2\r\n");
                break;
            default:
                logError("Unknow OPCODE %i!\r\n", code[j]);
                break;
        } // switch
    } // human-readable
    
    return 0;
}
