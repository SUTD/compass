#ifndef IL_H_
#define IL_H_

#include "../types.h"

/*
 * Standard header file including all the AST node
 * types used in the COMPASS intermediate language.
 * Note that all these classes live in namespace::il,
 * so use it if you want direct access.
 */


#include <vector>
#include <string>

using namespace std;


#include "addressof_expression.h"
#include "array_ref_expression.h"
#include "assembly.h"
#include "binop_expression.h"
#include "block_expression.h"
#include "block.h"
#include "break_statement.h"
#include "builtin_expression.h"
#include "case_label.h"
#include "cast_expression.h"
#include "complex_const_exp.h"
#include "conditional_expression.h"
#include "const_expression.h"
#include "continue_statement.h"
#include "control_statement.h"
#include "declaration.h"
#include "deref_expression.h"
#include "do_while_loop.h"
#include "expression.h"
#include "expression_instruction.h"
#include "expr_list_expression.h"
#include "field_ref_expression.h"
#include "file.h"
#include "fixed_const_exp.h"
#include "for_loop.h"
#include "function_address_expression.h"
#include "function_call_expression.h"
#include "function_declaration.h"
#include "function_pointer_call_expression.h"
#include "goto_statement.h"
#include "if_statement.h"
#include "il.h"
#include "initializer_list_expression.h"
#include "instruction.h"
#include "integer_const_exp.h"
#include "label.h"
#include "location.h"
#include "modify_expression.h"
#include "node.h"
#include "noop_instruction.h"
#include "real_const_exp.h"
#include "return_statement.h"
#include "set_instruction.h"
#include "statement.h"
#include "string_const_exp.h"
#include "switch_statement.h"
#include "throw_expression.h"
#include "translation_unit.h"
#include "try_catch_statement.h"
#include "try_finally_statement.h"
#include "type.h"
#include "unop_expression.h"
#include "variable_declaration.h"
#include "variable_expression.h"
#include "variable.h"
#include "vector_const_exp.h"
#include "while_loop.h"




#endif /*IL_H_*/
