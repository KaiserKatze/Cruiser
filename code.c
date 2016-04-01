#if 0
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "java.h"
#include "opcode.h"
#include "log.h"
#include "memory.h"

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
    u4 j, k;
    int _defaultbyte, _lowbyte, _highbyte, _npairs, i;
    logInfo("\t\t// Human-readable =\r\n");
    for (j = 0u; j < code_length; j++)
    {
        k = j;
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
            /* push value */
            case OPCODE_iconst_m1:
                logInfo("iconst_m1\r\n");
                break;
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
                logInfo("ldc #%i\r\n", code[++j]);
                // push item from runtime constant pool
                // ... -> ..., value
                break;
            case OPCODE_ldc_w:
                logInfo("ldc_w #%i\r\n", (code[++j] << 8) | code[++j]);
                // push item from runtime constant pool (wide index)
                break;
            case OPCODE_ldc2_w:
                logInfo("ldc2_w #%i\r\n", (code[++j] << 8) | code[++j]);
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
            case OPCODE_lor:
                logInfo("lor\r\n");
                break;
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
                logInfo("ifeq %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_ifne:
                logInfo("ifne %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_iflt:
                logInfo("iflt %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_ifge:
                logInfo("ifge %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_ifgt:
                logInfo("ifgt %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_ifle:
                logInfo("ifle %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            /* int value comparison */
            case OPCODE_if_icmpeq:
                logInfo("if_icmpeq %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_icmpne:
                logInfo("if_icmpne %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_icmplt:
                logInfo("if_icmplt %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_icmpge:
                logInfo("if_icmpge %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_icmpgt:
                logInfo("if_icmpgt %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_icmple:
                logInfo("if_icmple %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            /* branch if 'reference' comparison succeeds */
            case OPCODE_if_acmpeq:
                logInfo("if_acmpeq %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_if_acmpne:
                logInfo("if_acmpne %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_goto:
                logInfo("goto %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_jsr:
                logInfo("jsr %i\r\n", k + ((code[++j] << 8) | code[++j]));
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
                for (i = _lowbyte; i <= _highbyte; i++)
                {
                    logInfo("\t\t\t\t%i: %i\r\n", i,
                            k + (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]);
                }
                // both _lowbyte and _highbyte are inclusive
            case OPCODE_lookupswitch:
                // make sure 'defaultbyte1' begins at an address that is a multiple of four bytes
                // move 'j' across padding part to 'defaultbyte1' location
                j += 4 - j % 4;
                _defaultbyte = (code[j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                _npairs = (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j];
                if (_npairs < 0)
                {
                    logError("Assertion Error: npairs < 0!\r\n");
                    return -1;
                }
                logInfo("lookupswitch %i %i {\r\n", _defaultbyte + k, _npairs);
                for (i = 0; i < _npairs; i++)
                {
                    logInfo("\t\t\t\t%i: %i\r\n",
                            k + ((code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]),
                            (code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]);
                }
                logInfo("\t\t}\r\n");
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
                logInfo("invokevirtual #%i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokespecial:
                logInfo("invokespecial #%i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokestatic:
                logInfo("invokestatic #%i\r\n", (code[++j] << 8) | code[++j]);
                break;
            case OPCODE_invokeinterface:
                logInfo("invokeinterface #%i %i\r\n", (code[++j] << 8) | code[++j], code[++j]);
                ++j;
                // @see jvm7.pdf:p479
                break;
            case OPCODE_invokedynamic:
                logInfo("invokedynamic #%i\r\n", (code[++j] << 8) | code[++j]);
                j += 2;
                // @see jvms7.pdf:p474
                break;
            case OPCODE_new:
                logInfo("new #%i\r\n", (code[++j] << 8) | code[++j]);
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
                logInfo("anewarray %i\r\n", (code[++j] << 8) | code[++j]);
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
                logInfo("ifnull %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            /* branch if reference not null */
            case OPCODE_ifnonnull:
                logInfo("ifnonnull %i\r\n", k + ((code[++j] << 8) | code[++j]));
                break;
            case OPCODE_goto_w:
                logInfo("goto_w %lli\r\n", k + ((code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]));
                break;
            case OPCODE_jsr_w:
                logInfo("jsr_w %lli\r\n", k + ((code[++j] << 24) | (code[++j] << 16) | (code[++j] << 8) | code[++j]));
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

#if 1
struct _StackOutputEntry
{
    u2 is_const;
    u2 category;
    u2 count;
    u2 length;
    u1 *bytes;
};

struct _StackOutput
{
    u2 max;
    u2 off;
    struct _StackOutputEntry entries[];
};

static struct _StackOutput *sop_newStackOutput(u2 max_stack)
{
    struct _StackOutput *output;

    output = (struct _StackOutput *) allocMemory(1, sizeof (struct _StackOutput)
            + max_stack * sizeof (struct _StackOutputEntry));
    if (!output) return (struct _StackOutput *) 0;
    output->max = max_stack;
    output->off = max_stack;

    return output;
}

static int sop_push(struct _StackOutput *output, u2 is_const, u2 category,
        u2 count, u2 length, u1 *bytes)
{
    struct _StackOutputEntry *entry;
    
    if (!output || !bytes)
        return -1;
    if (output->off - 1 < 0
            || output->off - 1 >= output->max)
        return -1;
    entry = &(output->entries[--output->off]);
    entry->is_const = is_const;
    entry->category = category;
    entry->count = count;
    entry->length = length;
    entry->bytes = bytes;
    
    return 0;
}

static int sop_push_c(struct _StackOutput *output, u2 category, u2 count, u1 *bytes)
{
    return sop_push(output, 1, category, count, strlen((char *) bytes), bytes);
}

static int sop_push_e(struct _StackOutput *output, struct _StackOutputEntry *newentry)
{
    struct _StackOutputEntry *entry;
    
    if (!output)
        return -1;
    if (output->off - 1 < 0
            || output->off - 1 >= output->max)
        return -1;
    entry = &(output->entries[--output->off]);
    entry->is_const = newentry->is_const;
    entry->count = newentry->count;
    entry->length = newentry->length;
    entry->bytes = newentry->bytes;
    
    return 0;
}

/* index 0 represents for top entry in the stack */
static struct _StackOutputEntry *sop_peek(struct _StackOutput *output, u2 index)
{
    if (!output)
        return (struct _StackOutputEntry *) 0;
    return &(output->entries[output->off + index]);
}

static inline struct _StackOutputEntry *sop_pop(struct _StackOutput *output)
{
    if (output->off < output->max)
        return &(output->entries[output->off++]);
    return (struct _StackOutputEntry *) 0;
}

static inline void sop_freeEntry(struct _StackOutputEntry *entry)
{
    if (!entry)
        return;
    if (!entry->is_const)
        free(entry->bytes);
}

static int sop_flush(struct _StackOutput *output)
{
    struct _StackOutputEntry *entry;
    char buff[0xffff];
    char *str, **split;
    int count, nbits, i, j, k;

    if (!output)
        return -1;
    // create a copy of output
    while (output->max > output->off)
    {
        entry = sop_pop(output);
        count = entry->count;
        split = (char **) allocMemory(count, sizeof (char *));
        for (i = j = k = 0; i < entry->length; i++)
        {
            // move i until encounters single '%' mark
            if (entry->bytes[i] == '%' && entry->bytes[++i] != '%')
            {
                // calculate new split string length
                entry->length = i - k + 1;
                // assign new split string to j-th entry
                split[j] = (char *) allocMemory(entry->length, sizeof (char));
                // copy 
                memcpy(split[j], &(entry->bytes[k]), entry->length);
                // move k pointer to new search start point
                k = i + 1;
            }
        }
        for (i = count; i > 0;)
        {
            entry = sop_pop(output);
            if (snprintf((char *) buff, entry->length + 1, split[--i], (char *) entry->bytes) < 0)
                return -1;
        }
    }
    return 0;
}

static u1 *
generateParameterFormat(int parameters_count)
{
    u1 *res, *ptr;
    int i;
    
    if (parameters_count <= 0)
        return (u1 *) 0;
    res = (u1 *) allocMemory(parameters_count * 4 - 2, sizeof (u1));
    if (!res)
        return (u1 *) 0;
    ptr = res;
    memcpy(ptr, "%s", 2);
    ptr += 2;
    for (i = 1; i < parameters_count; i++)
    {
        memcpy(ptr, ", %s", 4);
        ptr += 4;
    }
    
    return res;
}

static inline int
getComputationalType0(u1 token)
{
    switch (token)
    {
        // category 1 computational type
        case 'L':case '[':
        case 'B':case 'C':
        case 'F':case 'I':
        case 'S':case 'Z':
            return 1;
        // category 2 computational type
        case 'D':case 'J':
            return 2;
        default:
            logError("Unknown method return type descriptor token [%c]!\r\n", token);
            return -1;
    }
}

static int
getComputationalType_field(CONSTANT_Utf8_info *cui)
{
    return getComputationalType0(*(cui->data->bytes));
}

static int
getComputationalType_method(CONSTANT_Utf8_info *cui)
{
    u1 *puf;
    
    for (puf = cui->data->bytes;
            puf < (u1 *) (cui->data->bytes + cui->data->length);
            puf++)
    {
        if (*puf != ')')
            continue;
        ++puf;
        return getComputationalType0(*puf);
    }
    logError("Fail to determine computational type[%.*s]!\r\n",
        cui->data->length, cui->data->bytes);
    
    return -1;
}

extern int
decompileCode(ClassFile *cf, method_info *method,
        struct attr_Code_info *code_info)
{
    struct _StackOutput *output;
    struct _StackOutputEntry entry1, entry2, entry3, entry4;
    u4 i, j, k, l, p;
    u1 inCatch, *code;
    u1 **localNames, *buf, *puf;
    cp_info *info;
    CONSTANT_Fieldref_info *cfi;
    CONSTANT_Methodref_info *cmi;
    CONSTANT_InterfaceMethodref_info *cimi;
    CONSTANT_NameAndType_info *cni;
    CONSTANT_Utf8_info *cui;
    CONSTANT_Class_info *cci;
    CONSTANT_Integer_info *cii;
    CONSTANT_Long_info *cli;
    CONSTANT_InvokeDynamic_info *cidi;
    CONSTANT_MethodHandle_info *cmhi;
    struct attr_BootstrapMethods_info *aBMi;
    struct bootstrap_method *bm;
    int _defaultbyte, _lowbyte, _highbyte, _npairs;
    int methodParametersCount;

    output = sop_newStackOutput(code_info->max_stack);
    if (!output)
        return -1;
    localNames = (u1 **) allocMemory(code_info->max_locals, sizeof (u1 *));
    if (!localNames)
        return -1;
    code = code_info->code;
    for (i = j = k = inCatch = 0; i < code_info->code_length; i++)
    {
        if (k < code_info->exception_table_length)
        {
            if (i == code_info->exception_table[j].start_pc)
            {
                logInfo("try {\r\n");
                inCatch = 1;
            }
            else if (i == code_info->exception_table[j].handler_pc)
            {
                if (code_info->exception_table[j].catch_type != 0)
                    logInfo("} catch (%s ex%i) {\r\n",
                            getConstant_ClassName(cf, code_info->exception_table[j].catch_type), j);
                else
                    logInfo("} finally {");
                --j;
                if (j == 0)
                    j = ++k;
            }
            else if (j + 1 < code_info->exception_table_length
                    && i == code_info->exception_table[j + 1].start_pc)
            {
                ++j;
                if (j > k)
                    k = j;
                logInfo("try {\r\n");
            }
        }
        // store opcode offset
        p = i;
        switch (code_info->code[i])
        {
            case OPCODE_nop:
                break;
            case OPCODE_aconst_null:
                sop_push_c(output, 1, 0, (u1 *) "null");
                // ... -> ..., null
                break;
            case OPCODE_iconst_m1:
                sop_push_c(output, 1, 0, (u1 *) "-1");
                break;
            /* push value */
            case OPCODE_iconst_0:
                sop_push_c(output, 1, 0, (u1 *) "0");
                break;
            case OPCODE_iconst_1:
                sop_push_c(output, 1, 0, (u1 *) "1");
                break;
            case OPCODE_iconst_2:
                sop_push_c(output, 1, 0, (u1 *) "2");
                break;
            case OPCODE_iconst_3:
                sop_push_c(output, 1, 0, (u1 *) "3");
                break;
            case OPCODE_iconst_4:
                sop_push_c(output, 1, 0, (u1 *) "4");
                break;
            case OPCODE_iconst_5:
                sop_push_c(output, 1, 0, (u1 *) "5");
                break;
            case OPCODE_lconst_0:
                sop_push_c(output, 2, 0, (u1 *) "0L");
                break;
            case OPCODE_lconst_1:
                sop_push_c(output, 2, 0, (u1 *) "1L");
                break;
            case OPCODE_fconst_0:
                sop_push_c(output, 1, 0, (u1 *) "0.0F");
                break;
            case OPCODE_fconst_1:
                sop_push_c(output, 1, 0, (u1 *) "1.0F");
                break;
            case OPCODE_fconst_2:
                sop_push_c(output, 1, 0, (u1 *) "2.0F");
                break;
            case OPCODE_dconst_0:
                sop_push_c(output, 2, 0, (u1 *) "0.0");
                break;
            case OPCODE_dconst_1:
                sop_push_c(output, 2, 0, (u1 *) "1.0");
                break;
            case OPCODE_bipush:
                buf = (u1 *) allocMemory(4, sizeof (u1));
                if (sprintf((char *) buf, "%i", code[++i]) < 0)
                    return -1;
                sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_sipush:
                buf = (u1 *) allocMemory(6, sizeof (u1));
                if (sprintf((char *) buf, "%i", (code[++i] << 8) | code[++i]) < 0)
                    return -1;
                sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_ldc:
                info = &(cf->constant_pool[code[++i]]);
                if (!info->data)
                    return -1;
                switch (info->tag)
                {
                    case CONSTANT_Integer:
                        cii = (CONSTANT_Integer_info *) info;
                        buf = (u1 *) allocMemory(11, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "%i", cii->data->bytes);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_Float:
                        cii = (CONSTANT_Integer_info *) info;
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "%fF", cii->data->float_value);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_String:
                        cui = (CONSTANT_Utf8_info *) info;
                        buf = (u1 *) allocMemory(cui->data->length, sizeof (u1));
                        if (!buf)
                            return -1;
                        memcpy(buf, cui->data->bytes, cui->data->length);
                        sop_push(output, 0, 1, 0, cui->data->length, buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_Class:
                        cci = (CONSTANT_Class_info *) info;
                        buf = getClassSimpleName(cf, cci->data->name_index);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    default:
                        logError("Unsupported ldc target with [%i] tag!\r\n", info->tag);
                        return -1;
                }
                break;
            case OPCODE_ldc_w:
                info = &(cf->constant_pool[(code[++i] << 8) | code[++i]]);
                if (!info->data)
                    return -1;
                switch (info->tag)
                {
                    case CONSTANT_Integer:
                        cii = (CONSTANT_Integer_info *) info;
                        buf = (u1 *) allocMemory(11, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "%i", cii->data->bytes);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_Float:
                        cii = (CONSTANT_Integer_info *) info;
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "%fF", cii->data->float_value);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_String:
                        cui = (CONSTANT_Utf8_info *) info;
                        buf = (u1 *) allocMemory(cui->data->length, sizeof (u1));
                        if (!buf)
                            return -1;
                        memcpy(buf, cui->data->bytes, cui->data->length);
                        sop_push(output, 0, 1, 0, cui->data->length, buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_Class:
                        cci = (CONSTANT_Class_info *) info;
                        buf = getClassSimpleName(cf, cci->data->name_index);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    default:
                        logError("Unsupported ldc_w target with [%i] tag!\r\n", info->tag);
                        return -1;
                }
                break;
            case OPCODE_ldc2_w:
                info = &(cf->constant_pool[(code[++i] << 8) | code[++i]]);
                if (!info->data)
                    return -1;
                switch (info->tag)
                {
                    case CONSTANT_Long:
                        cli = (CONSTANT_Long_info *) info;
                        buf = (u1 *) allocMemory(20, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "0x%llxL", cli->data->long_value);
                        sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case CONSTANT_Double:
                        cli = (CONSTANT_Long_info *) info;
                        buf = (u1 *) allocMemory(32, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "%fD", cli->data->double_value);
                        sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    default:
                        logError("Unsupported ldc2_w target with [%i] tag!\r\n", info->tag);
                        return -1;
                }
                break;
            case OPCODE_iload:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "iV%i", code[++i]);
                sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_lload:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "lV%i", code[++i]);
                sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_fload:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "fV%i", code[++i]);
                sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_dload:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "dV%i", code[++i]);
                sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_aload:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "aV%i", code[++i]);
                sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_iload_0:
                sop_push_c(output, 1, 0, (u1 *) "iV0");
                break;
            case OPCODE_iload_1:
                sop_push_c(output, 1, 0, (u1 *) "iV1");
                break;
            case OPCODE_iload_2:
                sop_push_c(output, 1, 0, (u1 *) "iV2");
                break;
            case OPCODE_iload_3:
                sop_push_c(output, 1, 0, (u1 *) "iV3");
                break;
            case OPCODE_lload_0:
                sop_push_c(output, 1, 0, (u1 *) "lV0");
                break;
            case OPCODE_lload_1:
                sop_push_c(output, 2, 0, (u1 *) "lV1");
                break;
            case OPCODE_lload_2:
                sop_push_c(output, 2, 0, (u1 *) "lV2");
                break;
            case OPCODE_lload_3:
                sop_push_c(output, 2, 0, (u1 *) "lV3");
                break;
            case OPCODE_fload_0:
                sop_push_c(output, 1, 0, (u1 *) "fV0");
                break;
            case OPCODE_fload_1:
                sop_push_c(output, 1, 0, (u1 *) "fV1");
                break;
            case OPCODE_fload_2:
                sop_push_c(output, 1, 0, (u1 *) "fV2");
                break;
            case OPCODE_fload_3:
                sop_push_c(output, 1, 0, (u1 *) "fV3");
                break;
            case OPCODE_dload_0:
                sop_push_c(output, 2, 0, (u1 *) "dV0");
                break;
            case OPCODE_dload_1:
                sop_push_c(output, 2, 0, (u1 *) "dV1");
                break;
            case OPCODE_dload_2:
                sop_push_c(output, 2, 0, (u1 *) "dV2");
                break;
            case OPCODE_dload_3:
                sop_push_c(output, 2, 0, (u1 *) "dV3");
                break;
            case OPCODE_aload_0:
                sop_push_c(output, 1, 0, (u1 *) "aV0");
                break;
            case OPCODE_aload_1:
                sop_push_c(output, 1, 0, (u1 *) "aV1");
                break;
            case OPCODE_aload_2:
                sop_push_c(output, 1, 0, (u1 *) "aV2");
                break;
            case OPCODE_aload_3:
                sop_push_c(output, 1, 0, (u1 *) "aV3");
                break;
            case OPCODE_iaload:
            case OPCODE_faload:
            case OPCODE_aaload:
            case OPCODE_baload:
            case OPCODE_caload:
            case OPCODE_saload:
                sop_push_c(output, 1, 2, (u1 *) "%s[%s]");
                break;
            case OPCODE_laload:
            case OPCODE_daload:
                sop_push_c(output, 2, 2, (u1 *) "%s[%s]");
                break;
            /* store value into given index of an array */
            case OPCODE_istore:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "iV%i = %%s", code[++i]);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_lstore:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "lV%i = %%s", code[++i]);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_fstore:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "fV%i = %%s", code[++i]);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_dstore:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "dV%i = %%s", code[++i]);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_astore:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "aV%i = %%s", code[++i]);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_istore_0:
                sop_push_c(output, 0, 1, (u1 *) "iV0 = %s");
                break;
            case OPCODE_istore_1:
                sop_push_c(output, 0, 1, (u1 *) "iV1 = %s");
                break;
            case OPCODE_istore_2:
                sop_push_c(output, 0, 1, (u1 *) "iV2 = %s");
                break;
            case OPCODE_istore_3:
                sop_push_c(output, 0, 1, (u1 *) "iV3 = %s");
                break;
            case OPCODE_lstore_0:
                sop_push_c(output, 0, 1, (u1 *) "lV0 = %s");
                break;
            case OPCODE_lstore_1:
                sop_push_c(output, 0, 1, (u1 *) "lV1 = %s");
                break;
            case OPCODE_lstore_2:
                sop_push_c(output, 0, 1, (u1 *) "lV2 = %s");
                break;
            case OPCODE_lstore_3:
                sop_push_c(output, 0, 1, (u1 *) "lV3 = %s");
                break;
            case OPCODE_fstore_0:
                sop_push_c(output, 0, 1, (u1 *) "fV0 = %s");
                break;
            case OPCODE_fstore_1:
                sop_push_c(output, 0, 1, (u1 *) "fV1 = %s");
                break;
            case OPCODE_fstore_2:
                sop_push_c(output, 0, 1, (u1 *) "fV2 = %s");
                break;
            case OPCODE_fstore_3:
                sop_push_c(output, 0, 1, (u1 *) "fV3 = %s");
                break;
            case OPCODE_dstore_0:
                sop_push_c(output, 0, 1, (u1 *) "dV0 = %s");
                break;
            case OPCODE_dstore_1:
                sop_push_c(output, 0, 1, (u1 *) "dV1 = %s");
                break;
            case OPCODE_dstore_2:
                sop_push_c(output, 0, 1, (u1 *) "dV2 = %s");
                break;
            case OPCODE_dstore_3:
                sop_push_c(output, 0, 1, (u1 *) "dV3 = %s");
                break;
            case OPCODE_astore_0:
                sop_push_c(output, 0, 1, (u1 *) "aV0 = %s");
                break;
            case OPCODE_astore_1:
                sop_push_c(output, 0, 1, (u1 *) "aV1 = %s");
                break;
            case OPCODE_astore_2:
                sop_push_c(output, 0, 1, (u1 *) "aV2 = %s");
                break;
            case OPCODE_astore_3:
                sop_push_c(output, 0, 1, (u1 *) "aV3 = %s");
                break;
            /* store into array */
            // ..., arrayref, index, value -> ...
            case OPCODE_iastore:
            case OPCODE_lastore:
            case OPCODE_fastore:
            case OPCODE_dastore:
            case OPCODE_aastore:
            case OPCODE_bastore:
            case OPCODE_castore:
            case OPCODE_sastore:
                sop_push_c(output, 0, 3, (u1 *) "%s[%s] = %s");
                break;
            /* stack access */
            case OPCODE_pop:
                sop_freeEntry(sop_pop(output));
                // pop the top operand stack value
                // ..., value -> ...
                break;
            case OPCODE_pop2:
                // pop the top one or two operand stack values
                // (1) where each of value1 and value2 is a value of a category 1 computational type
                // ..., value2, value1 -> ...
                // (2) where value is a value of a category 2 computational type
                // ..., value -> ...
                sop_freeEntry(sop_pop(output));
                sop_freeEntry(sop_pop(output));
                break;
            case OPCODE_dup:
                // duplicate the top operand stack value
                // ..., value -> ..., value, value
                memcpy(&entry1, sop_peek(output, 0), sizeof (struct _StackOutputEntry));
                if (entry1.category != 1)
                {
                    logError("Assertion error: Opcode 'dup' encounters wrong category computational type!\r\n");
                    return -1;
                }
                sop_push_e(output, &entry1);
                break;
            case OPCODE_dup_x1:
                // duplicate the top operand stack value and insert two values down
                // ..., value2, value1 -> ..., value1, value2, value1
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category != 1 || entry2.category != 1)
                {
                    logError("Assertion error: Opcode 'dup_x1' encounters wrong category computational type!\r\n");
                    return -1;
                }
                sop_push_e(output, &entry1);
                sop_push_e(output, &entry2);
                sop_push_e(output, &entry1);
                break;
            case OPCODE_dup_x2:
                // duplicate the top operand stack value and insert two or three values down
                // (1) when value1, value2, and value3 are all values of a category 1 computational type (§2.11.1)
                // ..., value3, value2, value1 -> ..., value1, value3, value2, value1
                // (2) when value1 is a value of a category 1 computational type and valu2 is a value of a category 2 computational type (§2.11.1)
                // ..., value2, value1 -> ..., value1, value2, value1
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category == 1)
                {
                    if (entry2.category == 1 && entry3.category == 1)
                    {
                        memcpy(&entry3, sop_pop(output), sizeof (struct _StackOutputEntry));
                        sop_push_e(output, &entry1);
                        sop_push_e(output, &entry3);
                        sop_push_e(output, &entry2);
                        sop_push_e(output, &entry1);
                        break;
                    }
                    else if (entry2.category == 2)
                    {
                        sop_push_e(output, &entry1);
                        sop_push_e(output, &entry2);
                        sop_push_e(output, &entry1);
                        break;
                    }
                }
                logError("Assertion error: Opcode 'dup_x2' encounters wrong category computational type!\r\n");
                return -1;
            case OPCODE_dup2:
                // duplicate the top one or two operand stack values
                // (1) where both value1 and value2 are values of a category 1 computational type
                // ..., value2, value1 -> ..., value2, value1, value2, value1
                // (2) where value is a value of a category 2 computational type
                // ..., value -> value, value
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category == 2)
                {
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry1);
                    break;
                }
                else if (entry1.category == 1 && entry2.category == 1)
                {
                    memcpy(&entry2, sop_peek(output, 1), sizeof (struct _StackOutputEntry));
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    break;
                }
                logError("Assertion error: Opcode 'dup2' encounters wrong category computational type!\r\n");
                return -1;
            case OPCODE_dup2_x1:
                // duplicate the top one or two operand stack values and insert two or three values down
                // (1) where value1, value2 and value3 are all values of a category 1 computational type
                // ..., value3, value2, value1 -> ..., value2, value1, value3, value2, value1
                // (2) where value1 is a value of a category 2 computational type and value2 is a value of category 1 computational type
                // ..., value2, value1 -> ..., value1, value2, value1
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category == 2)
                {
                    memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry2.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x1' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    break;
                }
                else if (entry1.category == 1)
                {
                    memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry2.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x1' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    memcpy(&entry3, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry3.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x1' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry3);
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    break;
                }
                logError("Assertion error: Opcode 'dup2_x1' encounters wrong category computational type!\r\n");
                return -1;
            case OPCODE_dup2_x2:
                // (1) where value1, value2, value3, and value4 are all values of a category 1 computational type
                // ..., value4, value3, value2, value1 -> ..., value2, value1, value4, value3, value2, value1
                // (2) where value 1 is a value of a category 2 computational type and value2 and value3 are both values of a category 1 computational type
                // ..., value3, value2, value1 -> ..., value1, value3, value2, value1
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category == 1)
                {
                    memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry2.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    memcpy(&entry3, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry3.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    memcpy(&entry4, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry4.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry4);
                    sop_push_e(output, &entry3);
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    break;
                }
                else if (entry1.category == 2)
                {
                    memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry2.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    memcpy(&entry3, sop_pop(output), sizeof (struct _StackOutputEntry));
                    if (entry3.category != 1)
                    {
                        logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                        return -1;
                    }
                    sop_push_e(output, &entry1);
                    sop_push_e(output, &entry3);
                    sop_push_e(output, &entry2);
                    sop_push_e(output, &entry1);
                    break;
                }
                logError("Assertion error: Opcode 'dup2_x2' encounters wrong category computational type!\r\n");
                return -1;
            case OPCODE_swap:
                memcpy(&entry1, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry1.category != 1)
                {
                    logError("Assertion error: Opcode 'swap' encounters wrong category computational type!\r\n");
                    return -1;
                }
                memcpy(&entry2, sop_pop(output), sizeof (struct _StackOutputEntry));
                if (entry2.category != 1)
                {
                    logError("Assertion error: Opcode 'swap' encounters wrong category computational type!\r\n");
                    return -1;
                }
                sop_push_e(output, &entry1);
                sop_push_e(output, &entry2);
                // swap the top two operand stack values
                break;
            /* arithmetic calculation */
            case OPCODE_iadd:
            case OPCODE_fadd:
                sop_push_c(output, 1, 2, (u1 *) "(%s + %s)");
                break;
            case OPCODE_ladd:
            case OPCODE_dadd:
                sop_push_c(output, 2, 2, (u1 *) "(%s + %s)");
                break;
            case OPCODE_isub:
            case OPCODE_fsub:
                sop_push_c(output, 1, 2, (u1 *) "(%s - %s)");
                break;
            case OPCODE_lsub:
            case OPCODE_dsub:
                sop_push_c(output, 2, 2, (u1 *) "(%s - %s)");
                break;
            case OPCODE_imul:
            case OPCODE_fmul:
                sop_push_c(output, 1, 2, (u1 *) "%s * %s");
                break;
            case OPCODE_lmul:
            case OPCODE_dmul:
                sop_push_c(output, 2, 2, (u1 *) "%s * %s");
                break;
            case OPCODE_idiv:
            case OPCODE_fdiv:
                sop_push_c(output, 1, 2, (u1 *) "%s / %s");
                break;
            case OPCODE_ldiv:
            case OPCODE_ddiv:
                sop_push_c(output, 2, 2, (u1 *) "%s / %s");
                break;
            /* remainder */
            case OPCODE_irem:
            case OPCODE_frem:
                sop_push_c(output, 1, 2, (u1 *) "%s %% %s");
                break;
            case OPCODE_lrem:
            case OPCODE_drem:
                sop_push_c(output, 2, 2, (u1 *) "%s %% %s");
                break;
            /* negate value */
            case OPCODE_ineg:
            case OPCODE_fneg:
                sop_push_c(output, 1, 1, (u1 *) "(-%s)");
                break;
            case OPCODE_lneg:
            case OPCODE_dneg:
                sop_push_c(output, 2, 1, (u1 *) "(-%s)");
                break;
            /* shift */
            case OPCODE_ishl:
                sop_push_c(output, 1, 2, (u1 *) "(%s << %s)");
                break;
            case OPCODE_ishr:
                sop_push_c(output, 1, 2, (u1 *) "(%s >> %s)");
                break;
            case OPCODE_lshl:
                sop_push_c(output, 2, 2, (u1 *) "(%s << %s)");
                break;
            case OPCODE_lshr:
                sop_push_c(output, 2, 2, (u1 *) "(%s >> %s)");
                break;
            case OPCODE_iushr:
                sop_push_c(output, 1, 2, (u1 *) "(%s >>> %s)");
                break;
            case OPCODE_lushr:
                sop_push_c(output, 2, 2, (u1 *) "(%s >>> %s)");
                break;
            case OPCODE_iand:
                sop_push_c(output, 1, 2, (u1 *) "(%s & %s)");
                break;
            case OPCODE_land:
                sop_push_c(output, 2, 2, (u1 *) "(%s & %s)");
                break;
            case OPCODE_ior:
                sop_push_c(output, 1, 2, (u1 *) "(%s | %s)");
                break;
            case OPCODE_lor:
                sop_push_c(output, 2, 2, (u1 *) "(%s | %s)");
                break;
            case OPCODE_ixor:
                sop_push_c(output, 1, 2, (u1 *) "(%s ^ %s)");
                break;
            case OPCODE_lxor:
                sop_push_c(output, 2, 2, (u1 *) "(%s ^ %s)");
                break;
            case OPCODE_iinc:
                // increment local variable by constant
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "iV%i += %i", code[++i], code[++i]);
                sop_push_c(output, 0, 0, buf);
                buf = (u1 *) 0;
                break;
            /* Type conversion */
            case OPCODE_i2l:
                sop_push_c(output, 2, 0, (u1 *) "(long) ");
                break;
            case OPCODE_i2f:
                sop_push_c(output, 1, 0, (u1 *) "(float) ");
                break;
            case OPCODE_i2d:
                sop_push_c(output, 2, 0, (u1 *) "(double) ");
                break;
            case OPCODE_l2i:
                sop_push_c(output, 1, 0, (u1 *) "(int) ");
                break;
            case OPCODE_l2f:
                sop_push_c(output, 1, 0, (u1 *) "(float) ");
                break;
            case OPCODE_l2d:
                sop_push_c(output, 2, 0, (u1 *) "(double) ");
                break;
            case OPCODE_f2i:
                sop_push_c(output, 1, 0, (u1 *) "(int) ");
                break;
            case OPCODE_f2l:
                sop_push_c(output, 2, 0, (u1 *) "(long) ");
                break;
            case OPCODE_f2d:
                sop_push_c(output, 2, 0, (u1 *) "(double) ");
                break;
            case OPCODE_d2i:
                sop_push_c(output, 1, 0, (u1 *) "(int) ");
                break;
            case OPCODE_d2l:
                sop_push_c(output, 2, 0, (u1 *) "(long) ");
                break;
            case OPCODE_d2f:
                sop_push_c(output, 1, 0, (u1 *) "(float) ");
                break;
            case OPCODE_i2b:
                sop_push_c(output, 1, 0, (u1 *) "(byte) ");
                break;
            case OPCODE_i2c:
                sop_push_c(output, 1, 0, (u1 *) "(u1) ");
                break;
            case OPCODE_i2s:
                sop_push_c(output, 1, 0, (u1 *) "(short) ");
                break;
            /* compare two values */
            // if 'value1' is greater than 'value2', then int value 1 is pushed
            // if equal, then int value 0 is pushed
            // if 'value1' is less than 'value2', int value -1 is pushed
            // otherwise, dcmpg pushes int value 1, while dcmpl pushes int value -1
            case OPCODE_lcmp:
            case OPCODE_fcmpl:
            case OPCODE_fcmpg:
            case OPCODE_dcmpl:
            case OPCODE_dcmpg:
                // TODO unsupported
                logError("Unsupported decompiling!\r\n");
                return -1;
            /* branch if int comparison with zero succeeds */
            case OPCODE_ifeq:
                // TODO unsupported, i'm driven insane!
                // how should i work around multiple blocks?
                // remember to replace 'k' with 'p'
                logInfo("ifeq %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_ifne:
                logInfo("ifne %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_iflt:
                logInfo("iflt %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_ifge:
                logInfo("ifge %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_ifgt:
                logInfo("ifgt %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_ifle:
                logInfo("ifle %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            /* int value comparison */
            case OPCODE_if_icmpeq:
                logInfo("if_icmpeq %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_icmpne:
                logInfo("if_icmpne %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_icmplt:
                logInfo("if_icmplt %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_icmpge:
                logInfo("if_icmpge %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_icmpgt:
                logInfo("if_icmpgt %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_icmple:
                logInfo("if_icmple %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            /* branch if 'reference' comparison succeeds */
            case OPCODE_if_acmpeq:
                logInfo("if_acmpeq %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_if_acmpne:
                logInfo("if_acmpne %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_goto:
                logInfo("goto %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_jsr:
                logInfo("jsr %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_ret:
                // return from subroutine
                // no change on operand stack
                ++i;
                logError("Unsupported opcode 'ret' appeared!\r\n");
                break;
            /* switch */
            case OPCODE_tableswitch:
                // make sure 'defaultbyte1' begins at an address that is a multiple of four bytes
                // move 'j' across padding part to 'defaultbyte1' location
                // TODO tableswitch
                i += 4 - i % 4;
                _defaultbyte = (code[i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i];
                _lowbyte = (code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i];
                _highbyte = (code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i];
                logInfo("tableswitch %i %i %i\r\n", _defaultbyte, _lowbyte, _highbyte);
                for (l = _lowbyte; l <= _highbyte; l++)
                {
                    logInfo("\t\t\t\t%i: %i\r\n", l,
                            p + (code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i]);
                }
                // both _lowbyte and _highbyte are inclusive
            case OPCODE_lookupswitch:
                // make sure 'defaultbyte1' begins at an address that is a multiple of four bytes
                // move 'j' across padding part to 'defaultbyte1' location
                // TODO lookupswitch
                i += 4 - i % 4;
                _defaultbyte = (code[i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i];
                _npairs = (code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i];
                if (_npairs < 0)
                {
                    logError("Assertion Error: npairs < 0!\r\n");
                    return -1;
                }
                logInfo("lookupswitch %i %i {\r\n", _defaultbyte + k, _npairs);
                for (l = 0; l < _npairs; l++)
                {
                    logInfo("\t\t\t\t%i: %i\r\n",
                            p + ((code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i]),
                            (code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i]);
                }
                logInfo("\t\t}\r\n");
                break;
            /* return value from method */
            case OPCODE_ireturn:
            case OPCODE_lreturn:
            case OPCODE_freturn:
            case OPCODE_dreturn:
            case OPCODE_areturn:
                sop_push_c(output, 0, 1, (u1 *) "return %s");
                break;
            case OPCODE_return:
                sop_push_c(output, 0, 0, (u1 *) "return");
                // return void from method
                break;
            case OPCODE_getstatic:
                // get static field from class
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                cfi = getConstant_Fieldref(cf, (code[++i] << 8) | code[++i]);
                if (!cfi)
                    return -1;
                puf = getClassSimpleName(cf, cfi->data->class_index);
                if (!puf)
                    return -1;
                cni = getConstant_NameAndType(cf, cfi->data->name_and_type_index);
                cui = getConstant_Utf8(cf, cni->data->name_index);
                sprintf((char *) buf, "%s.%.*s", puf, cui->data->length, cui->data->bytes);
                freeMemory(puf);
                puf = (u1 *) 0;
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_field(cui),
                        0, strlen((char *) buf), buf);
                break;
            case OPCODE_putstatic:
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                cfi = getConstant_Fieldref(cf, (code[++i] << 8) | code[++i]);
                if (!cfi)
                    return -1;
                puf = getClassSimpleName(cf, cfi->data->class_index);
                if (!puf)
                    return -1;
                cni = getConstant_NameAndType(cf, cfi->data->name_and_type_index);
                cui = getConstant_Utf8(cf, cni->data->name_index);
                sprintf((char *) buf, "%s.%.*s = %%s", puf, cui->data->length, cui->data->bytes);
                freeMemory(puf);
                puf = (u1 *) 0;
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                break;
            case OPCODE_getfield:
                // Fetch field from object
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                cfi = getConstant_Fieldref(cf, (code[++i] << 8) | code[++i]);
                if (!cfi)
                    return -1;
                cni = getConstant_NameAndType(cf, cfi->data->name_and_type_index);
                cui = getConstant_Utf8(cf, cni->data->name_index);
                sprintf((char *) buf, "%%s.%.*s", cui->data->length, cui->data->bytes);
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_field(cui),
                        1, strlen((char *) buf), buf);
                break;
            case OPCODE_putfield:
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                cfi = getConstant_Fieldref(cf, (code[++i] << 8) | code[++i]);
                if (!cfi)
                    return -1;
                cni = getConstant_NameAndType(cf, cfi->data->name_and_type_index);
                cui = getConstant_Utf8(cf, cni->data->name_index);
                sprintf((char *) buf, "%%s.%.*s = %%s", cui->data->length, cui->data->bytes);
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, 0, 2, strlen((char *) buf), buf);
                break;
            // Invoke instance method; dispatch based on class
            case OPCODE_invokevirtual:
                cmi = getConstant_Methodref(cf, (code[++i] << 8) | code[++i]);
                if (!cmi)
                    return -1;
                cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
                methodParametersCount = getMethodParametersCount(cf, cni->data->descriptor_index);
                puf = generateParameterFormat(methodParametersCount);
                if (!puf)
                    return -1;
                cui = getConstant_Utf8(cf, cni->data->name_index);
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "%%s.%.*s(%s)", cui->data->length, cui->data->bytes, puf);
                freeMemory(puf);
                // determine return type category
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_method(cui),
                        1 + methodParametersCount, strlen((char *) buf), buf);
                puf = (u1 *) 0;
                buf = (u1 *) 0;
                break;
            // Invoke instance method; special handling for superclass, private,
            // and instance initialization method invocations
            case OPCODE_invokespecial:
                // FIXME how to distinguish 'super.method()' and 'this.method()'?
                cmi = getConstant_Methodref(cf, (code[++i] << 8) | code[++i]);
                if (!cmi)
                    return -1;
                cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
                methodParametersCount = getMethodParametersCount(cf, cni->data->descriptor_index);
                puf = generateParameterFormat(methodParametersCount);
                if (!puf)
                    return -1;
                cui = getConstant_Utf8(cf, cni->data->name_index);
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "%%s.%.*s(%s)", cui->data->length, cui->data->bytes, puf);
                freeMemory(puf);
                // determine return type category
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_method(cui),
                        1 + methodParametersCount, strlen((char *) buf), buf);
                puf = (u1 *) 0;
                buf = (u1 *) 0;
                break;
            case OPCODE_invokestatic:
                cmi = getConstant_Methodref(cf, (code[++i] << 8) | code[++i]);
                if (!cmi)
                    return -1;
                cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
                methodParametersCount = getMethodParametersCount(cf, cni->data->descriptor_index);
                puf = generateParameterFormat(methodParametersCount);
                if (!puf)
                    return -1;
                cui = getConstant_Utf8(cf, cni->data->name_index);
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "%.*s(%s)", cui->data->length, cui->data->bytes, puf);
                freeMemory(puf);
                // determine return type category
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_method(cui),
                        methodParametersCount, strlen((char *) buf), buf);
                puf = (u1 *) 0;
                buf = (u1 *) 0;
                break;
            case OPCODE_invokeinterface:
                cimi = getConstant_InterfaceMethodref(cf, (code[++i] << 8) | code[++i]);
                if (!cimi)
                    return -1;
                cni = getConstant_NameAndType(cf, cimi->data->name_and_type_index);
                methodParametersCount = getMethodParametersCount(cf, cni->data->descriptor_index);
                puf = generateParameterFormat(methodParametersCount);
                if (!puf)
                    return -1;
                cui = getConstant_Utf8(cf, cni->data->name_index);
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                sprintf((char *) buf, "%%s.%.*s(%s)", cui->data->length, cui->data->bytes, puf);
                freeMemory(puf);
                // determine return type category
                cui = getConstant_Utf8(cf, cni->data->descriptor_index);
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, getComputationalType_method(cui),
                        1 + methodParametersCount, strlen((char *) buf), buf);
                puf = (u1 *) 0;
                buf = (u1 *) 0;
                // get over zero
                ++i;
                break;
            case OPCODE_invokedynamic:
                // The run-time constant pool item at that index must be a symbolic
                // reference to a call site specifier
                // TODO invoke dynamic call site
                
                // retrieve call site specifier
                cidi = getConstant_InvokeDynamic(cf, (code[++i] << 8) | code[++i]);
                if (!cidi)
                    return -1;
                // go over zeros
                i += 2;
#ifndef QUICK_REFERENCE
#error QUICK_REFERENCE IS NOT ACTIVATIED!
#else
                if (cf->off_BootstrapMethods >= 0)
                {
                    cni = getConstant_NameAndType(cf, cidi->data->name_and_type_index);
                    
                    // retrieve bootstrap method
                    aBMi = (struct attr_BootstrapMethods_info *)
                            cf->attributes[cf->off_BootstrapMethods].data;
                    bm = &(aBMi->bootstrap_methods[cidi->data->bootstrap_method_attr_index]);
                    /*
                    cmhi = getConstant_MethodHandle(cf, bm->bootstrap_method_ref);
                    for (l = 0; l < bm->num_bootstrap_arguments; l++)
                    {
                        info = cf->constant_pool[bm->bootstrap_arguments[l]];
                        switch (info->tag)
                        {
                            case CONSTANT_MethodHandle:
                                break;
                            case CONSTANT_String:
                            case CONSTANT_Class:
                            case CONSTANT_Integer:
                            case CONSTANT_Long:
                            case CONSTANT_Float:
                            case CONSTANT_Double:
                            case CONSTANT_MethodType:
                                break;
                            default:
                                logError("Constant pool entry has invalid tag [%i]!\r\n",
                                        info->tag);
                                return -1;
                        }
                    }
                    */
                    info = &(cf->constant_pool[bm->bootstrap_arguments[1]]);
                    if (info->tag != CONSTANT_MethodHandle)
                    {
                        logError("Unexpected constant pool entry tag [%i]!\r\n", info->tag);
                        return -1;
                    }
                    // retrieve implementation code location
                    cmhi = (CONSTANT_MethodHandle_info *) info;
                    info = &(cf->constant_pool[cmhi->data->reference_index]);
                    switch (info->tag)
                    {
                        case CONSTANT_Methodref:
                        case CONSTANT_InterfaceMethodref:
                            cmi = (CONSTANT_Methodref_info *) info;
                            cci = getConstant_Class(cf, cmi->data->class_index);
                            cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
                            // search implementation method with 'cci' and 'cni'
                            /*
                            // determine if the method is in the scope of the current class loader
                            // if not, output a method invocation
                            // otherwise, output in-line code of the method
                            // check if this class is the outer class
                            if (cf->this_class == cmi->data->class_index)
                            {
                                buf = getClassSimpleName(cf,
                                        getConstant_Class(cf, cf->this_class)->data->name_index);
                                puf = getClassSimpleName(cf, cci->data->name_index);
                                // this class is the outer class
                                if (!strcmp(buf, puf))
                                {
                                    // search the specified method in this class
                                    for (l = 0; l < cf->methods_count; l++)
                                    {
                                    }
                                }
                                freeMemory(buf);
                                buf = (u1 *) 0;
                                freeMemory(puf);
                                puf = (u1 *) 0;
                            }
                            */
                            break;
                        default:
                            logError("Unexpected constant pool entry tag [%i]!\r\n", info->tag);
                            return -1;
                    }
                }
#endif
                break;
            case OPCODE_new:
                // create new object
                if (code[++i] != OPCODE_dup)
                {
                    logError("Assertion error: Opcode following opcode 'new' "
                            "is not 'dup'! [%i]\r\n", code[i]);
                    return -1;
                }
                if (code[++i] != OPCODE_invokespecial)
                {
                    logError("Assertion error: Opcode following opcode 'new' and 'dup' "
                            "is not 'invokespecial'! [%i]\r\n", code[i]);
                    return -1;
                }
                cmi = getConstant_Methodref(cf, (code[++i] << 8) | code[++i]);
                if (!cmi)
                    return -1;
                cni = getConstant_NameAndType(cf, cmi->data->name_and_type_index);
                methodParametersCount = getMethodParametersCount(cf, cni->data->descriptor_index);
                puf = generateParameterFormat(methodParametersCount);
                if (!puf)
                    return -1;
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                cci = getConstant_Class(cf, cmi->data->class_index);
                cui = getConstant_Utf8(cf, cci->data->name_index);
                sprintf((char *) buf, "new %.*s(%s)", cui->data->length, cui->data->bytes, puf);
                freeMemory(puf);
                puf = (u1 *) 0;
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, 1, methodParametersCount, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_newarray:
                buf = (u1 *) allocMemory(16, sizeof (u1));
                if (!buf)
                    return -1;
                memcpy(buf, "new ", 4);
                switch (code[++i])
                {
                    case T_BOOLEAN:
                        puf = (u1*) "boolean[%s]";
                        break;
                    case T_CHAR:
                        puf = (u1*) "char[%s]";
                        break;
                    case T_FLOAT:
                        puf = (u1*) "float[%s]";
                        break;
                    case T_DOUBLE:
                        puf = (u1*) "double[%s]";
                        break;
                    case T_BYTE:
                        puf = (u1*) "byte[%s]";
                        break;
                    case T_SHORT:
                        puf = (u1*) "short[%s]";
                        break;
                    case T_INT:
                        puf = (u1*) "int[%s]";
                        break;
                    case T_LONG:
                        puf = (u1*) "long[%s]";
                        break;
                    default:
                        logError("Unknow atype[%i]!\r\n", code[i]);
                        return -1;
                }
                memcpy(buf, puf, strlen((char *) puf));
                puf = (u1 *) 0;
                sop_push(output, 0, 1, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_anewarray:
                cci = getConstant_Class(cf, (code[++i] << 8) | code[++i]);
                if (!cci)
                    return -1;
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                puf = getClassSimpleName(cf, cci->data->name_index);
                sprintf((char *) buf, "new %s[%%s]", puf);
                freeMemory(puf);
                puf = (u1 *) 0;
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, 1, 1, strlen((char *) buf), buf);
                buf = (u1 *) 0;
                break;
            case OPCODE_arraylength:
                sop_push_c(output, 1, 1, (u1*) "%s.length");
                break;
            case OPCODE_athrow:
                sop_push_c(output, 1, 1, (u1*) "throw %s");
                break;
            case OPCODE_checkcast:
                // checkcast 'indexbyte1' 'indexbyte2'
                // ..., objectref -> ..., objectref
                i += 2;
                logError("Unsupported opcode 'checkcast' appeared!\r\n");
                break;
            case OPCODE_instanceof:
                cci = getConstant_Class(cf, (code[++i] << 8) | code[++i]);
                buf = getClassSimpleName(cf, cci->data->name_index);
                if (!buf)
                    return -1;
                sop_push(output, 0, 1, 1, strlen((char *) buf), buf);
                sop_push_c(output, 1, 2, (u1*) "%s instanceof %s");
                buf = (u1 *) 0;
                break;
            case OPCODE_monitorenter:
                sop_push_c(output, 0, 1, (u1*) "synchronized (%s) {");
                // enter monitor for object
                break;
            case OPCODE_monitorexit:
                sop_push_c(output, 0, 0, (u1*) "}");
                // exit monitor for object
                break;
            case OPCODE_wide:
                // extend local variable index by additional bytes
                switch (code[++i])
                {
                    // (1) where <opcode> is one of <T>load or ret
                    case OPCODE_iload:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "iV%i", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_fload:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "fV%i", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_aload:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "aV%i", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 1, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_lload:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "lV%i", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_dload:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "dV%i", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 2, 0, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_istore:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "iV%i = %%s", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_fstore:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "fV%i = %%s", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_astore:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "aV%i = %%s", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_lstore:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "lV%i = %%s", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_dstore:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "dV%i = %%s", (code[++i] << 8) | code[++i]);
                        sop_push(output, 0, 0, 1, strlen((char *) buf), buf);
                        buf = (u1 *) 0;
                        break;
                    case OPCODE_ret:
                        // In Oracle's implementation of a compiler for the Java
                        // programming language prior to Java SE 6, the ret instruction
                        // was used with the jsr and jsr_w instructions (§jsr, §jsr_w) in the
                        // implementation of the finally clause (§3.13, §4.10.2.5).
                        ++i;
                        logError("Unsupported opcode 'wide ret' appeared!\r\n");
                        break;
                    // (2) iinc
                    case OPCODE_iinc:
                        buf = (u1 *) allocMemory(16, sizeof (u1));
                        if (!buf)
                            return -1;
                        sprintf((char *) buf, "iV%i += %i",
                                (code[++i] << 8) | code[++i],
                                (code[++i] << 8) | code[++i]);
                        sop_push_c(output, 0, 0, buf);
                        buf = (u1 *) 0;
                        break;
                    default:
                        logError("Unsupported wide opcode[%i]!\r\n", code[i]);
                        return -1;
                }
                break;
            case OPCODE_multianewarray:
                // create new multi-dimensional array
                cci = getConstant_Class(cf, (code[++i] << 8) | code[++i]);
                if (!cci)
                    return -1;
                puf = getClassSimpleName(cf, cci->data->name_index);
                if (!puf)
                    return -1;
                buf = (u1 *) allocMemory(1024, sizeof (u1));
                if (!buf)
                    return -1;
                memcpy(buf, "new ", 4);
                l = strlen((char *) puf);
                memcpy(buf + 4, puf, l);
                freeMemory(puf);
                puf = buf + 4 + l;
                for (l = 0; l < code[++i]; l++)
                {
                    memcpy(puf, "[%s]", 3);
                    puf += 3;
                }
                buf = (u1 *) trimMemory(buf);
                sop_push(output, 0, 1, code[i], strlen((char *) buf), buf);
                break;
            /* branch if reference null */
            case OPCODE_ifnull:
                logInfo("ifnull %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            /* branch if reference not null */
            case OPCODE_ifnonnull:
                logInfo("ifnonnull %i\r\n", k + ((code[++i] << 8) | code[++i]));
                break;
            case OPCODE_goto_w:
                logInfo("goto_w %lli\r\n", k + ((code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i]));
                break;
            case OPCODE_jsr_w:
                logInfo("jsr_w %lli\r\n", k + ((code[++i] << 24) | (code[++i] << 16) | (code[++i] << 8) | code[++i]));
                break;
            /* reserved */
            case OPCODE_breakpoint:
            case OPCODE_impdep1:
            case OPCODE_impdep2:
                break;
            default:
                logError("Unknow OPCODE %i!\r\n", code[i]);
                return -1;
        }
    }
    return 0;
}
#endif
#endif
