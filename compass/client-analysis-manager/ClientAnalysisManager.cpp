/*
 * ClientAnalysisManager.cpp
 *
 *  Created on: Sep 1, 2012
 *      Author: tdillig
 */

#include "ClientAnalysisManager.h"
#include "ClientAnalysis.h"
#include "MemoryAnalysis.h"
#include "MemoryGraph.h"
#include "sail.h"
#include "compass_assert.h"
#include "access-path.h"



ClientAnalysisManager::ClientAnalysisManager()
{
	next_offset  = CLIENT_INITIAL_OFFSET;
	var_counter = 0;

}

const map<int, ClientAnalysis*> &
ClientAnalysisManager::get_client_analyses()
{
	return rev_offsets;
}


void ClientAnalysisManager::register_client(ClientAnalysis* a)
{
	analyses.push_back(a);
	offsets[a] = next_offset;
	rev_offsets[next_offset] = a;
	il::type* t = a->get_property_type();
	int size = t->get_size()/8;
	next_offset -= size;
}

ClientAnalysis* ClientAnalysisManager::get_analyis_from_offset(int offset)
{
	c_assert(rev_offsets.count(offset) >0);
	return rev_offsets[offset];
}

void ClientAnalysisManager::register_memory_analysis(MemoryAnalysis* ma)
{
	this->ma = ma;
}

void ClientAnalysisManager::clear()
{
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		delete *it;
	}

	analyses.clear();
	offsets.clear();
	rev_offsets.clear();
	next_offset  = CLIENT_INITIAL_OFFSET;
	var_counter = 0;
}

void
ClientAnalysisManager::process_instruction_before(sail::Instruction* inst)
{
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		process_inst_before(inst, *it);
	}
}
void
ClientAnalysisManager::process_instruction_after(sail::Instruction* inst)
{
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		process_inst_after(inst, *it);
	}
}

void
ClientAnalysisManager::process_function_call_before(
		sail::FunctionCall* inst, bool known)
{
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		(*it)->process_function_call_before(inst, known);
	}
}

void
ClientAnalysisManager::process_function_call_after(
		sail::FunctionCall* inst, bool known)
{
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		(*it)->process_function_call_after(inst, known);
	}

}


void ClientAnalysisManager::process_inst_before(sail::Instruction* inst,
		ClientAnalysis* a)
{
	switch(inst->get_instruction_id())
	{

	case sail::ASSIGNMENT:
	{
		a->process_assign_before((sail::Assignment*)inst);
		break;
	}
	case sail::FIELD_REF_READ:
	{
		a->process_field_read_before((sail::FieldRefRead*)inst);
		break;
	}
	case sail::FIELD_REF_WRITE:
	{
		a->process_field_write_before((sail::FieldRefWrite*) inst);
		break;
	}
	case sail::LOAD:
	{
		a->process_load_before((sail::Load*) inst);
		break;
	}
	case sail::STORE:
	{
		a->process_store_before((sail::Store*)inst);
		break;
	}
	case sail::ADDRESS_VAR:
	{
		a->process_address_var_before((sail::AddressVar*) inst);
		break;
	}
	case sail::UNOP:
	{
		a->process_unop_before((sail::Unop*) inst);
		break;
	}

	case sail::BINOP:
	{
		a->process_binop_before((sail::Binop*) inst);
		break;
	}

	case sail::CAST:
	{
		a->process_cast_before((sail::Cast*)inst);
		break;
	}

	case sail::ARRAY_REF_READ:
	{
		a->process_array_read_before((sail::ArrayRefRead*)inst);
		break;
	}
	case sail::ARRAY_REF_WRITE:
	{
		a->process_array_write_before((sail::ArrayRefWrite*)inst);
		break;
	}

	case sail::ADDRESS_STRING:
	{
		a->process_address_string_before((sail::AddressString*)inst);
		break;
	}

	case sail::SAIL_ASSEMBLY:
	{
		a->process_assembly_before((sail::Assembly*) inst);
		break;
	}

	case sail::FUNCTION_CALL:
	{

		c_assert(false);
	}
	case sail::FUNCTION_POINTER_CALL:
	{
		a->process_fn_ptr_call_before((sail::FunctionPointerCall*)inst);
		break;
	}
	case sail::ADDRESS_LABEL:
	{
		a->process_address_label_before((sail::AddressLabel*) inst);
		break;
	}

	default:
	{
		break;
	}
	}

}


void ClientAnalysisManager::process_inst_after(sail::Instruction* inst,
		ClientAnalysis* a)
{
	switch(inst->get_instruction_id())
	{

	case sail::ASSIGNMENT:
	{
		a->process_assign_after((sail::Assignment*)inst);
		break;
	}
	case sail::FIELD_REF_READ:
	{
		a->process_field_read_after((sail::FieldRefRead*)inst);
		break;
	}
	case sail::FIELD_REF_WRITE:
	{
		a->process_field_write_after((sail::FieldRefWrite*) inst);
		break;
	}
	case sail::LOAD:
	{
		a->process_load_after((sail::Load*) inst);
		break;
	}
	case sail::STORE:
	{
		a->process_store_after((sail::Store*)inst);
		break;
	}
	case sail::ADDRESS_VAR:
	{
		a->process_address_var_after((sail::AddressVar*) inst);
		break;
	}
	case sail::UNOP:
	{
		a->process_unop_after((sail::Unop*) inst);
		break;
	}

	case sail::BINOP:
	{
		a->process_binop_after((sail::Binop*) inst);
		break;
	}

	case sail::CAST:
	{
		a->process_cast_after((sail::Cast*)inst);
		break;
	}

	case sail::ARRAY_REF_READ:
	{
		a->process_array_read_after((sail::ArrayRefRead*)inst);
		break;
	}
	case sail::ARRAY_REF_WRITE:
	{
		a->process_array_write_after((sail::ArrayRefWrite*)inst);
		break;
	}

	case sail::ADDRESS_STRING:
	{
		a->process_address_string_after((sail::AddressString*)inst);
		break;
	}

	case sail::SAIL_ASSEMBLY:
	{
		a->process_assembly_after((sail::Assembly*) inst);
		break;
	}

	case sail::FUNCTION_CALL:
	{

		c_assert(false);
		break;
	}
	case sail::FUNCTION_POINTER_CALL:
	{
		a->process_fn_ptr_call_after((sail::FunctionPointerCall*)inst);
		break;
	}
	case sail::ADDRESS_LABEL:
	{
		a->process_address_label_after((sail::AddressLabel*) inst);
		break;
	}

	default:
	{
		break;
	}
	}
}


void ClientAnalysisManager::write_state(ClientAnalysis* a,
		sail::Variable* lhs, sail::Symbol* rhs, Constraint c)
{
	cout << "############### writing state ########## " << endl;
	int offset = offsets[a];
	Constraint stmt_guard = ma->get_current_stmt_guard();
	Constraint new_c = stmt_guard & c;
	MemoryGraph* mg = ma->get_mg();

	string display_str = "*(" + lhs->to_string() + int_to_string(offset) +
			") = " +rhs->to_string();
	mg->process_store(lhs, offset, rhs, new_c);
	AccessPath* ap1 = mg->get_access_path_from_symbol(lhs);
	AccessPath* ap2 = mg->get_access_path_from_symbol(rhs);
	ma->add_dotty(display_str, ap1, ap2);

}


sail::Variable*
ClientAnalysisManager::read_state(ClientAnalysis* a, sail::Symbol* s)
{
	il::type* t = a->get_property_type();
	int offset = offsets[a];
	sail::Variable* v = get_temp(t);
	Constraint stmt_guard = ma->get_current_stmt_guard();
	MemoryGraph* mg = ma->get_mg();
	string display_str = v->to_string() + " = *(" + s->to_string() + "+" +
			int_to_string(offset) +")";
	mg->process_load(v, s, offset, stmt_guard);
	AccessPath* ap1 = mg->get_access_path_from_symbol(v);
	AccessPath* ap2 = mg->get_access_path_from_symbol(s);
	ma->add_dotty(display_str, ap1, ap2);
	return v;
}

Constraint ClientAnalysisManager::get_neqz_constraint(sail::Symbol* s)
{

	MemoryGraph* mg = ma->get_mg();
	Constraint c = mg->get_cg().get_neqz_constraint_from_value_set(s);
	return c;

}

void ClientAnalysisManager::assert_state(Constraint c, const string & msg)
{
	MemoryGraph* mg = ma->get_mg();
	c &= ma->get_current_stmt_guard();
	if(c.unsat()) return;
	AssertionTrace* a =
			AssertionTrace::make(ERROR_CLIENT_ANALYSIS, mg->get_line(), mg->get_file(),
			c, ma->get_cur_iteration_counter(), ma->get_current_stmt_guard(), msg);
	mg->add_error_trace(a);
}

sail::Variable* ClientAnalysisManager::get_temp(il::type* t)
{
	return new sail::Variable("$client" + int_to_string(var_counter++), t, true);
}

sail::Variable* ClientAnalysisManager::load(sail::Variable* v, int offset)
{

	//c_assert(v->get_type()->is_pointer_type());
	il::type* t = v->get_type()->get_inner_type();
	if(t == NULL) t = il::get_integer_type();

	//cout << "Variable: " << v->to_string() << " type: " << t->to_string() << endl;
	if(t->is_record_type()) {
		auto rt = static_cast<il::record_type*>(t);
		/*for(auto it = rt->elems.begin(); it != rt->elems.end(); it++)
		{
			cout << "Name: " << (*it)->fname << " Offset: " << (*it)->offset <<
					" Type: " << *(*it)->t << endl;

		}*/

		t = rt->get_type_from_offset(offset);
	}
	MemoryGraph* mg = ma->get_mg();
	Constraint c = ma->get_current_stmt_guard();
	sail::Variable* res = get_temp(t);

	string display_str = res->to_string() + " = *(" + v->to_string() + "+" +
				int_to_string(offset/8) +")";


	mg->process_load(res, v, offset/8, c);

	AccessPath* ap1 = mg->get_access_path_from_symbol(res);
	AccessPath* ap2 = mg->get_access_path_from_symbol(v);
	ma->add_dotty(display_str, ap1, ap2);
	return res;
}

sail::Variable* ClientAnalysisManager::load(sail::Variable* v,
		sail::Symbol* index)
{
	c_assert(v != NULL);
	c_assert(index != NULL);
	c_assert(v->get_type()->is_pointer_type());
	il::type* t = v->get_type()->get_inner_type();
	MemoryGraph* mg = ma->get_mg();
	Constraint c = ma->get_current_stmt_guard();
	sail::Variable* offset = get_temp(il::get_integer_type());
	mg->process_binop(offset, index, new sail::Constant(t->get_size()/8, true, 32),
			c, il::_MULTIPLY);

	sail::Variable* temp = get_temp(v->get_type());
	mg->process_plus(temp, v, offset, c);

	sail::Variable* res = get_temp(t);

	string display_str = "*(" + res->to_string() + int_to_string(0) +
				") = " +temp->to_string();

	mg->process_load(res, temp, 0, c);
	AccessPath* ap1 = mg->get_access_path_from_symbol(res);
	AccessPath* ap2 = mg->get_access_path_from_symbol(temp);
	ma->add_dotty(display_str, ap1, ap2);
	return res;


}


sail::Variable* ClientAnalysisManager::get_unknown_value
	(il::type* t, const string & name)
{
	AccessPath* ap = Variable::make_client_var(name, t);
	//cout << "client var: " << ap->to_string() << endl;
	DisplayTag dt(name, 0, "");
	//AccessPath* ap = UnmodeledValue::make_unknown_fn_return(t, dt);
	sail::Variable * v = this->get_temp(t);
	MemoryGraph* mg = ma->get_mg();
	AccessPath* v_ap = mg->get_access_path_from_symbol(v);
	c_assert(v_ap->get_ap_type() == AP_VARIABLE);
	Variable* v_var = static_cast<Variable*>(v_ap);
	mg->process_assignment(v_var, ap, Constraint(true));
	return v;

}

bool ClientAnalysisManager::report_error(const string & msg, Constraint fail_c)
{
	bool report = false;
	for(auto it = analyses.begin(); it != analyses.end(); it++) {
		bool res = (*it)->process_error(msg, fail_c);
		if(res) report = true;
	}
	return report;


}



ClientAnalysisManager::~ClientAnalysisManager()
{

}
