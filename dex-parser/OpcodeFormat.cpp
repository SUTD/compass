#include "OpcodeFormat.h"

OpcodeFormat::OpcodeFormat(CodeHeader* codeheader, DexOpcode opcode)
{
	_codeheader = codeheader;
	_opcode = opcode;
}

OpcodeFormat::~OpcodeFormat()
{
}

//default apiLevel is 13
Format OpcodeFormat::getFormat()
{
	switch (_opcode) 
	{
       // case 0x00F0:// OP_INVOKE_DIRECT_EMPTY,OP_INVOKE_OBJECT_INIT_RANGE
       //         return F35c;
        case OP_GOTO:
            return F10t;
        case OP_NOP:
        case OP_RETURN_VOID:
        //case OP_RETURN_VOID_BARRIER:
            return F10x;
        case OP_CONST_4:
            return F11n;
        case OP_MOVE_RESULT:
        case OP_MOVE_RESULT_WIDE:
        case OP_MOVE_RESULT_OBJECT:
        case OP_MOVE_EXCEPTION:
        case OP_RETURN:
        case OP_RETURN_WIDE:
        case OP_RETURN_OBJECT:
        case OP_MONITOR_ENTER:
        case OP_MONITOR_EXIT:
        case OP_THROW:
            return F11x;
        case OP_MOVE:
        case OP_MOVE_WIDE:
        case OP_MOVE_OBJECT:
        case OP_ARRAY_LENGTH:
        case OP_NEG_INT:
        case OP_NOT_INT:
        case OP_NEG_LONG:
        case OP_NOT_LONG:
        case OP_NEG_FLOAT:
        case OP_NEG_DOUBLE:
        case OP_INT_TO_LONG:
        case OP_INT_TO_FLOAT:
        case OP_INT_TO_DOUBLE:
        case OP_LONG_TO_INT:
        case OP_LONG_TO_FLOAT:
        case OP_LONG_TO_DOUBLE:
        case OP_FLOAT_TO_INT:
        case OP_FLOAT_TO_LONG:
        case OP_FLOAT_TO_DOUBLE:
        case OP_DOUBLE_TO_INT:
        case OP_DOUBLE_TO_LONG:
        case OP_DOUBLE_TO_FLOAT:
        case OP_INT_TO_BYTE:
        case OP_INT_TO_CHAR:
        case OP_INT_TO_SHORT:
        case OP_ADD_INT_2ADDR:
        case OP_SUB_INT_2ADDR:
        case OP_MUL_INT_2ADDR:
        case OP_DIV_INT_2ADDR:
        case OP_REM_INT_2ADDR:
        case OP_AND_INT_2ADDR:
        case OP_OR_INT_2ADDR:
        case OP_XOR_INT_2ADDR:
        case OP_SHL_INT_2ADDR:
        case OP_SHR_INT_2ADDR:
        case OP_USHR_INT_2ADDR:
        case OP_ADD_LONG_2ADDR:
        case OP_SUB_LONG_2ADDR:
        case OP_MUL_LONG_2ADDR:
        case OP_DIV_LONG_2ADDR:
        case OP_REM_LONG_2ADDR:
        case OP_AND_LONG_2ADDR:
        case OP_OR_LONG_2ADDR:
        case OP_XOR_LONG_2ADDR:
        case OP_SHL_LONG_2ADDR:
        case OP_SHR_LONG_2ADDR:
        case OP_USHR_LONG_2ADDR:
        case OP_ADD_FLOAT_2ADDR:
        case OP_SUB_FLOAT_2ADDR:
        case OP_MUL_FLOAT_2ADDR:
        case OP_DIV_FLOAT_2ADDR:
        case OP_REM_FLOAT_2ADDR:
        case OP_ADD_DOUBLE_2ADDR:
        case OP_SUB_DOUBLE_2ADDR:
        case OP_MUL_DOUBLE_2ADDR:
        case OP_DIV_DOUBLE_2ADDR:
        case OP_REM_DOUBLE_2ADDR:
            return F12x;
        //case OP_THROW_VERIFICATION_ERROR:
        //    return F20bc;
        case OP_GOTO_16:
            return F20t;
        case OP_CONST_STRING:
        case OP_CONST_CLASS:
        case OP_CHECK_CAST:
        case OP_NEW_INSTANCE:
        case OP_SGET:
        case OP_SGET_WIDE:
        case OP_SGET_OBJECT:
        case OP_SGET_BOOLEAN:
        case OP_SGET_BYTE:
        case OP_SGET_CHAR:
        case OP_SGET_SHORT:
        case OP_SPUT:
        case OP_SPUT_WIDE:
        case OP_SPUT_OBJECT:
        case OP_SPUT_BOOLEAN:
        case OP_SPUT_BYTE:
        case OP_SPUT_CHAR:
        case OP_SPUT_SHORT:
        //case OP_SGET_VOLATILE:
        //case OP_SPUT_VOLATILE:
       // case OP_SGET_WIDE_VOLATILE:
       // case OP_SPUT_WIDE_VOLATILE:
       // case OP_SGET_OBJECT_VOLATILE:
       // case OP_SPUT_OBJECT_VOLATILE:
            return F21c;
        case OP_CONST_HIGH16:
        case OP_CONST_WIDE_HIGH16:
            return F21h;
        case OP_CONST_16:
        case OP_CONST_WIDE_16:
            return F21s;
        case OP_IF_EQZ:
        case OP_IF_NEZ:
        case OP_IF_LTZ:
        case OP_IF_GEZ:
        case OP_IF_GTZ:
        case OP_IF_LEZ:
            return F21t;
        case OP_ADD_INT_LIT8:
        case OP_RSUB_INT_LIT8:
        case OP_MUL_INT_LIT8:
        case OP_DIV_INT_LIT8:
        case OP_REM_INT_LIT8:
        case OP_AND_INT_LIT8:
        case OP_OR_INT_LIT8:
        case OP_XOR_INT_LIT8:
        case OP_SHL_INT_LIT8:
        case OP_SHR_INT_LIT8:
        case OP_USHR_INT_LIT8:
            return F22b;
        case OP_INSTANCE_OF:
        case OP_NEW_ARRAY:
        case OP_IGET:
        case OP_IGET_WIDE:
        case OP_IGET_OBJECT:
        case OP_IGET_BOOLEAN:
        case OP_IGET_BYTE:
        case OP_IGET_CHAR:
        case OP_IGET_SHORT:
        case OP_IPUT:
        case OP_IPUT_WIDE:
        case OP_IPUT_OBJECT:
        case OP_IPUT_BOOLEAN:
        case OP_IPUT_BYTE:
        case OP_IPUT_CHAR:
        case OP_IPUT_SHORT:
       // case OP_IGET_VOLATILE:
       // case OP_IPUT_VOLATILE:
       // case OP_IGET_OBJECT_VOLATILE:
       // case OP_IGET_WIDE_VOLATILE:
       // case OP_IPUT_WIDE_VOLATILE:
       // case OP_IPUT_OBJECT_VOLATILE:
            return F22c;
        //case OP_IGET_QUICK:
        //case OP_IGET_WIDE_QUICK:
        //case OP_IGET_OBJECT_QUICK:
        //case OP_IPUT_QUICK:
        //case OP_IPUT_WIDE_QUICK:
        // case OP_IPUT_OBJECT_QUICK:
        //    return F22cs;
        case OP_ADD_INT_LIT16:
        case OP_RSUB_INT:
        case OP_MUL_INT_LIT16:
        case OP_DIV_INT_LIT16:
        case OP_REM_INT_LIT16:
        case OP_AND_INT_LIT16:
        case OP_OR_INT_LIT16:
        case OP_XOR_INT_LIT16:
            return F22s;
        case OP_IF_EQ:
        case OP_IF_NE:
        case OP_IF_LT:
        case OP_IF_GE:
        case OP_IF_GT:
        case OP_IF_LE:
            return F22t;
        case OP_MOVE_FROM16:
        case OP_MOVE_WIDE_FROM16:
        case OP_MOVE_OBJECT_FROM16:
            return F22x;
        case OP_CMPL_FLOAT:
        case OP_CMPG_FLOAT:
        case OP_CMPL_DOUBLE:
        case OP_CMPG_DOUBLE:
        case OP_CMP_LONG:
        case OP_AGET:
        case OP_AGET_WIDE:
        case OP_AGET_OBJECT:
        case OP_AGET_BOOLEAN:
        case OP_AGET_BYTE:
        case OP_AGET_CHAR:
        case OP_AGET_SHORT:
        case OP_APUT:
        case OP_APUT_WIDE:
        case OP_APUT_OBJECT:
        case OP_APUT_BOOLEAN:
        case OP_APUT_BYTE:
        case OP_APUT_CHAR:
        case OP_APUT_SHORT:
        case OP_ADD_INT:
        case OP_SUB_INT:
        case OP_MUL_INT:
        case OP_DIV_INT:
        case OP_REM_INT:
        case OP_AND_INT:
        case OP_OR_INT:
        case OP_XOR_INT:
        case OP_SHL_INT:
        case OP_SHR_INT:
        case OP_USHR_INT:
        case OP_ADD_LONG:
        case OP_SUB_LONG:
        case OP_MUL_LONG:
        case OP_DIV_LONG:
        case OP_REM_LONG:
        case OP_AND_LONG:
        case OP_OR_LONG:
        case OP_XOR_LONG:
        case OP_SHL_LONG:
        case OP_SHR_LONG:
        case OP_USHR_LONG:
        case OP_ADD_FLOAT:
        case OP_SUB_FLOAT:
        case OP_MUL_FLOAT:
        case OP_DIV_FLOAT:
        case OP_REM_FLOAT:
        case OP_ADD_DOUBLE:
        case OP_SUB_DOUBLE:
        case OP_MUL_DOUBLE:
        case OP_DIV_DOUBLE:
        case OP_REM_DOUBLE:
            return F23x;
        case OP_GOTO_32:
            return F30t;
        case OP_CONST_STRING_JUMBO:
            return F31c;
        case OP_CONST:
        case OP_CONST_WIDE_32:
            return F31i;
        case OP_FILL_ARRAY_DATA:
        case OP_PACKED_SWITCH:
        case OP_SPARSE_SWITCH:
            return F31t;
        case OP_MOVE_16:
        case OP_MOVE_WIDE_16:
        case OP_MOVE_OBJECT_16:
            return F32x;
        case OP_FILLED_NEW_ARRAY:
        case OP_INVOKE_VIRTUAL:
        case OP_INVOKE_SUPER:
        case OP_INVOKE_DIRECT:
        case OP_INVOKE_STATIC:
        case OP_INVOKE_INTERFACE:
            return F35c;
        //case OP_EXECUTE_INLINE:
        //    return F35mi;
        //case OP_INVOKE_VIRTUAL_QUICK:
       // case OP_INVOKE_SUPER_QUICK:
        //    return F35ms;
        case OP_FILLED_NEW_ARRAY_RANGE:
        case OP_INVOKE_VIRTUAL_RANGE:
        case OP_INVOKE_SUPER_RANGE:
        case OP_INVOKE_DIRECT_RANGE:
        case OP_INVOKE_STATIC_RANGE:
        case OP_INVOKE_INTERFACE_RANGE:
            return F3rc;
       // case OP_EXECUTE_INLINE_RANGE:
       //     return F3rmi;
       // case OP_INVOKE_VIRTUAL_QUICK_RANGE:
       // case OP_INVOKE_SUPER_QUICK_RANGE:
       //     return F3rms;
       // case OP_CONST_CLASS_JUMBO:
       // case OP_CHECK_CAST_JUMBO:
       // case OP_NEW_INSTANCE_JUMBO:
       // case OP_SGET_JUMBO:
       // case OP_SGET_WIDE_JUMBO:
       // case OP_SGET_OBJECT_JUMBO:
       // case OP_SGET_BOOLEAN_JUMBO:
       // case OP_SGET_BYTE_JUMBO:
       // case OP_SGET_CHAR_JUMBO:
       // case OP_SGET_SHORT_JUMBO:
       // case OP_SPUT_JUMBO:
       // case OP_SPUT_WIDE_JUMBO:
       // case OP_SPUT_OBJECT_JUMBO:
       // case OP_SPUT_BOOLEAN_JUMBO:
       // case OP_SPUT_BYTE_JUMBO:
       // case OP_SPUT_CHAR_JUMBO:
       // case OP_SPUT_SHORT_JUMBO:
       // case OP_SGET_VOLATILE_JUMBO:
       // case OP_SGET_WIDE_VOLATILE_JUMBO:
       // case OP_SGET_OBJECT_VOLATILE_JUMBO:
       // case OP_SPUT_VOLATILE_JUMBO:
       // case OP_SPUT_WIDE_VOLATILE_JUMBO:
       // case OP_SPUT_OBJECT_VOLATILE_JUMBO:
       //     return F41c;
        case OP_CONST_WIDE:
            return F51l;
        //case OP_INSTANCE_OF_JUMBO:
        //case OP_NEW_ARRAY_JUMBO:
        //case OP_IGET_JUMBO:
        //case OP_IGET_WIDE_JUMBO:
       // case OP_IGET_OBJECT_JUMBO:
       // case OP_IGET_BOOLEAN_JUMBO:
       // case OP_IGET_BYTE_JUMBO:
       // case OP_IGET_CHAR_JUMBO:
       // case OP_IGET_SHORT_JUMBO:
       // case OP_IPUT_JUMBO:
       // case OP_IPUT_WIDE_JUMBO:
       // case OP_IPUT_OBJECT_JUMBO:
       // case OP_IPUT_BOOLEAN_JUMBO:
       // case OP_IPUT_BYTE_JUMBO:
       // case OP_IPUT_CHAR_JUMBO:
       // case OP_IPUT_SHORT_JUMBO:
       // case OP_IGET_VOLATILE_JUMBO:
       // case OP_IGET_WIDE_VOLATILE_JUMBO:
       // case OP_IGET_OBJECT_VOLATILE_JUMBO:
       // case OP_IPUT_VOLATILE_JUMBO:
       // case OP_IPUT_WIDE_VOLATILE_JUMBO:
       // case OP_IPUT_OBJECT_VOLATILE_JUMBO:
        //    return F52c;
        //case OP_FILLED_NEW_ARRAY_JUMBO:
        //case OP_INVOKE_VIRTUAL_JUMBO:
        //case OP_INVOKE_SUPER_JUMBO:
        //case OP_INVOKE_DIRECT_JUMBO:
        //case OP_INVOKE_STATIC_JUMBO:
        //case OP_INVOKE_INTERFACE_JUMBO:
        //case OP_INVOKE_OBJECT_INIT_JUMBO:
        //    return F5rc;
	default:
		cout << "Do not has this opcode, exit!!!!" <<endl;
		exit(1);
        }
}


int OpcodeFormat::getSize(Format format)
{
	if(( F10t <= format) && ( format <= F12x ))
	{
		_size = 1;
	}
	if((F20t <= format)&&(format <= F23x ))
	{
		_size = 2;
	}
	if((F30t <= format) && (format <= F3rc ))
	{
		_size = 3;
	}
	if(format == F40sc)
	{
		_size = 4;
	}
	if(format == F51l)
	{
		_size = 5;
	}
	
	return _size;
}


void OpcodeFormat::printFormat(Format f)
{
	switch(f)
	{
		case F10t: cout << "F10t" << endl; break;
		case F10x: cout << "F10x" <<endl;break;
		case F11n: cout << "F11n" <<endl;break;
		case F11x: cout <<"F11x" <<endl;break;
		case F12x: cout <<"F12x" <<endl;break;
		case F20t: cout <<"F20t" <<endl;break;
		case F21c: cout << "F21c"<<endl;break;
		case F21h: cout <<"F21h" <<endl;break;
		case F21s: cout << "F21s" <<endl;break;
		case F21t: cout << "F21t" <<endl;break;
		case F22b: cout <<"F22b" <<endl;break;
		case F22c: cout << "F22c" <<endl;break;
//		case F22cs: cout <<"F22cs" <<endl;break;
		case F22s : cout << "F22s" <<endl;break;
		case F22t : cout <<"F22t" <<endl;break;
		case F22x : cout << "F22x" <<endl;break;
		case F23x : cout << "F23x" <<endl;break;
		case F30t: cout << "F30t" <<endl;break;
		case F31c: cout << "F31c" <<endl;break;
		case F31i: cout << "F31i" <<endl;break;
		case F31t: cout << "F31t" <<endl;break;
//		case F32s: cout << "F32s" <<endl;break;
		case F32x: cout << "F32x" <<endl;break;
		//case F33x: cout << "F33x" <<endl;break;
		case F35c: cout << "F35c" <<endl;break;
		case F3rc: cout << "F3rc" <<endl;break;
		//case F3rmi: cout << "F3rmi" <<endl;break;
		//case F3rms: cout <<"F2rms" <<endl;break;
		case F40sc: cout << "F40sc" <<endl;break;
		case F51l: cout << "F51l" <<endl;
		/*
		F10t,
                F10x,
                F11n,
                F11x,
                F12x, // 5
                // 2 shorts
                F20bc, //6
                F20t,
                F21c,
                F21h,
                F21s, //10
                F21t,
                F22b,
                F22c,
                F22cs,
                F22s,
                F22t,
                F22x,
                F23x,//18
                // 3 shorts
                F30t, //19
                F31c,
                F31i,
                F31t,
                F32s,
                F32x,
                F33x,
                F35c,
               //F35mi,
                F3rc,
                F3rmi,
                F3rms,//29
                // 4 shorts
                F40sc,//30
                //5 short
                F51l //31  
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		*/
	}
}

DexOpcode OpcodeFormat::getOpcode()
{
	return _opcode;
}
