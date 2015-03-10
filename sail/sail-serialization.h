/*
 * sail-serialization.h
 *
 *  Created on: Apr 10, 2010
 *      Author: tdillig
 */

#ifndef SAILSERIALIZATION_H_
#define SAILSERIALIZATION_H_

#include "il/il.h"
#include "sail/sail.h"


/*
 * Archive type used for boost serialization
 */

#include "serialization-archive.h"

#ifndef IN_COMPASS_SERIALIZATION
#define CUSTOM_CLASS_EXPORT BOOST_CLASS_EXPORT
#endif /* IN_COMPASS_SERIALIZATION */


#ifdef IN_COMPASS_SERIALIZATION
#define CUSTOM_CLASS_EXPORT BOOST_CLASS_EXPORT_KEY
#endif /* IN_COMPASS_SERIALIZATION */

CUSTOM_CLASS_EXPORT(Serializable);


CUSTOM_CLASS_EXPORT(il::type);
CUSTOM_CLASS_EXPORT(il::pointer_type);
CUSTOM_CLASS_EXPORT(il::array_type);
CUSTOM_CLASS_EXPORT(il::constarray_type);
CUSTOM_CLASS_EXPORT(il::vararray_type);
CUSTOM_CLASS_EXPORT(il::enum_info);
CUSTOM_CLASS_EXPORT(il::enum_type);
CUSTOM_CLASS_EXPORT(il::record_info);
CUSTOM_CLASS_EXPORT(il::method_info);
CUSTOM_CLASS_EXPORT(il::record_type);
CUSTOM_CLASS_EXPORT(il::vector_type);
CUSTOM_CLASS_EXPORT(il::complex_type);
CUSTOM_CLASS_EXPORT(il::function_type);
CUSTOM_CLASS_EXPORT(il::base_type);
CUSTOM_CLASS_EXPORT(il::invalid_type);
CUSTOM_CLASS_EXPORT(il::addressof_expression);
CUSTOM_CLASS_EXPORT(il::array_ref_expression);
CUSTOM_CLASS_EXPORT(il::assembly);
CUSTOM_CLASS_EXPORT(il::binop_expression);
CUSTOM_CLASS_EXPORT(il::block_expression);
CUSTOM_CLASS_EXPORT(il::block);
CUSTOM_CLASS_EXPORT(il::break_statement);
CUSTOM_CLASS_EXPORT(il::builtin_expression);
CUSTOM_CLASS_EXPORT(il::case_label);
CUSTOM_CLASS_EXPORT(il::cast_expression);
CUSTOM_CLASS_EXPORT(il::complex_const_exp);
CUSTOM_CLASS_EXPORT(il::conditional_expression);
CUSTOM_CLASS_EXPORT(il::const_expression);
CUSTOM_CLASS_EXPORT(il::control_statement);
CUSTOM_CLASS_EXPORT(il::declaration);
CUSTOM_CLASS_EXPORT(il::deref_expression);
CUSTOM_CLASS_EXPORT(il::expr_list_expression);
CUSTOM_CLASS_EXPORT(il::expression);
CUSTOM_CLASS_EXPORT(il::expression_instruction);
CUSTOM_CLASS_EXPORT(il::field_ref_expression);
CUSTOM_CLASS_EXPORT(il::file);
CUSTOM_CLASS_EXPORT(il::fixed_const_exp);
CUSTOM_CLASS_EXPORT(il::function_address_expression);
CUSTOM_CLASS_EXPORT(il::function_call_expression);
CUSTOM_CLASS_EXPORT(il::function_declaration);
CUSTOM_CLASS_EXPORT(il::function_pointer_call_expression);
CUSTOM_CLASS_EXPORT(il::goto_statement);
CUSTOM_CLASS_EXPORT(il::if_statement);
CUSTOM_CLASS_EXPORT(il::initializer_list_expression);
CUSTOM_CLASS_EXPORT(il::instruction);
CUSTOM_CLASS_EXPORT(il::integer_const_exp);
CUSTOM_CLASS_EXPORT(il::label);
CUSTOM_CLASS_EXPORT(il::location);
CUSTOM_CLASS_EXPORT(il::modify_expression);
CUSTOM_CLASS_EXPORT(il::namespace_context);
CUSTOM_CLASS_EXPORT(il::node);
CUSTOM_CLASS_EXPORT(il::noop_instruction);
CUSTOM_CLASS_EXPORT(il::real_const_exp);
CUSTOM_CLASS_EXPORT(il::return_statement);
CUSTOM_CLASS_EXPORT(il::set_instruction);
CUSTOM_CLASS_EXPORT(il::statement);
CUSTOM_CLASS_EXPORT(il::string_const_exp);
CUSTOM_CLASS_EXPORT(il::switch_statement);
CUSTOM_CLASS_EXPORT(il::throw_expression);
CUSTOM_CLASS_EXPORT(il::translation_unit);
CUSTOM_CLASS_EXPORT(il::try_catch_statement);
CUSTOM_CLASS_EXPORT(il::try_finally_statement);
CUSTOM_CLASS_EXPORT(il::unop_expression);
CUSTOM_CLASS_EXPORT(il::variable_declaration);
CUSTOM_CLASS_EXPORT(il::variable);
CUSTOM_CLASS_EXPORT(il::variable_expression);
CUSTOM_CLASS_EXPORT(il::vector_const_exp);

CUSTOM_CLASS_EXPORT(sail::AddressLabel);
CUSTOM_CLASS_EXPORT(sail::AddressString);
CUSTOM_CLASS_EXPORT(sail::AddressVar);
CUSTOM_CLASS_EXPORT(sail::ArrayRefRead);
CUSTOM_CLASS_EXPORT(sail::ArrayRefWrite);
CUSTOM_CLASS_EXPORT(sail::Assembly);
CUSTOM_CLASS_EXPORT(sail::Assignment);
CUSTOM_CLASS_EXPORT(sail::Assume);
CUSTOM_CLASS_EXPORT(sail::AssumeSize);
CUSTOM_CLASS_EXPORT(sail::InstanceOf);
CUSTOM_CLASS_EXPORT(sail::ExceptionReturn);
CUSTOM_CLASS_EXPORT(sail::BasicBlock);
CUSTOM_CLASS_EXPORT(sail::Binop);
CUSTOM_CLASS_EXPORT(sail::Block);
CUSTOM_CLASS_EXPORT(sail::Branch);
CUSTOM_CLASS_EXPORT(sail::Cast);
CUSTOM_CLASS_EXPORT(sail::Cfg);
CUSTOM_CLASS_EXPORT(sail::CfgEdge);
CUSTOM_CLASS_EXPORT(sail::Symbol);
CUSTOM_CLASS_EXPORT(sail::Constant);
CUSTOM_CLASS_EXPORT(sail::DropVariable);
CUSTOM_CLASS_EXPORT(sail::FieldRefRead);
CUSTOM_CLASS_EXPORT(sail::FieldRefWrite);
CUSTOM_CLASS_EXPORT(sail::Function);
CUSTOM_CLASS_EXPORT(sail::FunctionCall);
CUSTOM_CLASS_EXPORT(sail::FunctionPointerCall);
CUSTOM_CLASS_EXPORT(sail::Instruction);
CUSTOM_CLASS_EXPORT(sail::Jump);
CUSTOM_CLASS_EXPORT(sail::Label);
CUSTOM_CLASS_EXPORT(sail::Load);
CUSTOM_CLASS_EXPORT(sail::LoopInvocationInstruction);
CUSTOM_CLASS_EXPORT(sail::SaveInstruction);
CUSTOM_CLASS_EXPORT(sail::StaticAssert);
CUSTOM_CLASS_EXPORT(sail::Store);
CUSTOM_CLASS_EXPORT(sail::SummaryUnit);
CUSTOM_CLASS_EXPORT(sail::SuperBlock);

CUSTOM_CLASS_EXPORT(sail::TranslationUnit);
CUSTOM_CLASS_EXPORT(sail::Unop);
CUSTOM_CLASS_EXPORT(sail::Variable);

CUSTOM_CLASS_EXPORT(sail::FlowAnnotation);
CUSTOM_CLASS_EXPORT(sail::Annotation);



#endif /* SAILSERIALIZATION_H_ */
