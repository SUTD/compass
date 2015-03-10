/*
 * ClientAnalysis.cpp
 *
 *  Created on: Aug 31, 2012
 *      Author: tdillig
 */

#include "ClientAnalysis.h"

ClientAnalysisManager ClientAnalysis::cam;

void ClientAnalysis::clear_clients()
{
	cam.clear();
}

const map<int, ClientAnalysis*> & ClientAnalysis::get_client_analyses()
{
	return cam.get_client_analyses();
}

void ClientAnalysis::register_client(ClientAnalysis* a)
{
	cam.register_client(a);
}

bool ClientAnalysis::report_error(const string & msg, Constraint fail_c)
{
	return cam.report_error(msg, fail_c);
}

bool ClientAnalysis::process_error(const string & msg, Constraint fail_c)
{
	return true;
}

ClientAnalysis::ClientAnalysis() {


}

ClientAnalysis::~ClientAnalysis() {

}

void ClientAnalysis::process_assign_before(sail::Assignment* inst)
{

}

void ClientAnalysis::process_field_read_before(sail::FieldRefRead* inst)
{

}

void ClientAnalysis::process_field_write_before(sail::FieldRefWrite* inst)
{

}

void ClientAnalysis::process_load_before(sail::Load* inst)
{

}

void ClientAnalysis::process_store_before(sail::Store* inst)
{

}

void ClientAnalysis::process_address_var_before(sail::AddressVar* inst)
{

}

void ClientAnalysis::process_unop_before(sail::Unop* inst)
{

}

void ClientAnalysis::process_binop_before(sail::Binop* inst)
{

}

void ClientAnalysis::process_cast_before(sail::Cast* inst)
{

}

void ClientAnalysis::process_array_read_before(sail::ArrayRefRead* inst)
{

}

void ClientAnalysis::process_array_write_before(sail::ArrayRefWrite* inst)
{

}

void ClientAnalysis::process_address_string_before(sail::AddressString* inst)
{

}

void ClientAnalysis::process_assembly_before(sail::Assembly* inst)
{

}

void ClientAnalysis::process_function_call_before(sail::FunctionCall* inst,
		bool known)
{

}

void ClientAnalysis::process_fn_ptr_call_before(sail::FunctionPointerCall* inst)
{

}

void ClientAnalysis::process_address_label_before(sail::AddressLabel* inst)
{

}




void ClientAnalysis::process_assign_after(sail::Assignment* inst)
{

}

void ClientAnalysis::process_field_read_after(sail::FieldRefRead* inst)
{

}

void ClientAnalysis::process_field_write_after(sail::FieldRefWrite* inst)
{

}

void ClientAnalysis::process_load_after(sail::Load* inst)
{

}

void ClientAnalysis::process_store_after(sail::Store* inst)
{

}

void ClientAnalysis::process_address_var_after(sail::AddressVar* inst)
{

}

void ClientAnalysis::process_unop_after(sail::Unop* inst)
{

}

void ClientAnalysis::process_binop_after(sail::Binop* inst)
{

}

void ClientAnalysis::process_cast_after(sail::Cast* inst)
{

}

void ClientAnalysis::process_array_read_after(sail::ArrayRefRead* inst)
{

}

void ClientAnalysis::process_array_write_after(sail::ArrayRefWrite* inst)
{

}

void ClientAnalysis::process_address_string_after(sail::AddressString* inst)
{

}

void ClientAnalysis::process_assembly_after(sail::Assembly* inst)
{

}

void ClientAnalysis::process_function_call_after(sail::FunctionCall* inst,
		bool known)
{

}

void ClientAnalysis::process_fn_ptr_call_after(sail::FunctionPointerCall* inst)
{

}

void ClientAnalysis::process_address_label_after(sail::AddressLabel* inst)
{

}
//-------------------------------------------------------------------------
sail::Variable* ClientAnalysis::get_temp(il::type* t)
{
	return cam.get_temp(t);
}

void ClientAnalysis::write_state(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint c)
{
	cam.write_state(this, lhs, rhs, c);
}


sail::Variable* ClientAnalysis::read_state(sail::Symbol* s)
{
	return cam.read_state(this, s);
}

/*
 * Returns the constraint under which the value of s is not equal to zero.
 */
Constraint ClientAnalysis::get_neqz_constraint(sail::Symbol* s)
{
	return cam.get_neqz_constraint(s);
}

void ClientAnalysis::assert_state(Constraint c, const string & msg)
{
	cam.assert_state(c, msg);
}

sail::Variable* ClientAnalysis::load(sail::Variable* v, int offset)
{
	return cam.load(v, offset);
}

sail::Variable* ClientAnalysis::load(sail::Variable* v, sail::Symbol* index)
{
	return cam.load(v, index);
}

sail::Variable* ClientAnalysis::get_unknown_value(il::type* t,
		const string & name)
{
	return cam.get_unknown_value(t, name);
}

