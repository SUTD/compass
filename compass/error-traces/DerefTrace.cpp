/*
 * DerefTrace.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#include "DerefTrace.h"
#include "access-path.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "ConstraintGenerator.h"
#include "compass_assert.h"

map<pair<AccessPath*, int>, DerefTrace* > DerefTrace::deref_traces;

DerefTrace* DerefTrace::make(AccessPath* dereferenced_ptr,
		string pp_expression, int line,
		string file, Constraint fail_c, int counter_id)
{


	fail_c &= get_unsigned_constraints(dereferenced_ptr);

	if(fail_c.unsat()){
		return NULL;
	}


	access_info ai(pp_expression, line, file);
	DerefTrace* dt = NULL;
	if(deref_traces.count(make_pair(dereferenced_ptr, counter_id)) == 0)
	{
		dt = new DerefTrace(dereferenced_ptr, counter_id);
		deref_traces[make_pair(dereferenced_ptr, counter_id)] = dt;
	}

	else {
		dt = deref_traces[make_pair(dereferenced_ptr, counter_id)];
	}
	dt->deref_info.insert(ai);
	dt->fail_c |= fail_c;
	dt->fail_c.sat();
	return dt;
}

Constraint DerefTrace::get_unsigned_constraints(AccessPath* dereferenced_ptr)
{
	set<AccessPath*> aps;
	dereferenced_ptr->get_nested_memory_aps(aps);
	set<AccessPath*>::iterator it = aps.begin();
	Constraint geqz_c;
	for(; it!= aps.end(); it++) {
		AccessPath* ap = *it;
		if(!ap->get_type()->is_signed_type())
			geqz_c &= ConstraintGenerator::get_geqz_constraint(*it);
	}
	return geqz_c;
}


void DerefTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = deref_info.begin();
	for(; it!= deref_info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.callstack.push_back(ce);
	}
}

DerefTrace* DerefTrace::make(AccessPath* dereferenced_ptr,
		Constraint fail_c, int counter_id,
		set<access_info>& deref_info)
{
	DerefTrace* dt = NULL;
	if(deref_traces.count(make_pair(dereferenced_ptr, counter_id)) == 0)
	{
		dt = new DerefTrace(dereferenced_ptr, counter_id);
		deref_traces[make_pair(dereferenced_ptr, counter_id)] = dt;
	}

	else {
		dt = deref_traces[make_pair(dereferenced_ptr, counter_id)];
	}
	dt->deref_info.insert(deref_info.begin(), deref_info.end());
	dt->fail_c |= fail_c;
	dt->fail_c.sat();
	return dt;
}

ErrorTrace* DerefTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ptr = dereferenced_ptr->replace(ap, replacement);
	return DerefTrace::make(new_ptr, fail_c, counter_id, deref_info);
}

DerefTrace::DerefTrace(AccessPath* dereferenced_ptr, int counter_id)
{
	this->dereferenced_ptr = dereferenced_ptr;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = DEREF_TRACE;

}

void DerefTrace::report_error(vector<Error*>& errors, const Identifier & id)
{
	c_assert(deref_info.size() > 0);
	int first_line = deref_info.begin()->line;
	string file = deref_info.begin()->file;
	Error* error = new Error(first_line, file, ERROR_NULL_DEREF, to_error(), id);
	errors.push_back(error);
}

string DerefTrace::to_string()
{
	string res = "Deref trace: " + dereferenced_ptr->to_string();
	res += " / " + fail_c.to_string();
	return res;
}

string DerefTrace::get_id()
{
	return dereferenced_ptr->to_string();
}

string DerefTrace::to_error()
{
	string res;
	if(dereferenced_ptr->get_ap_type() == AP_CONSTANT) {
		ConstantValue* cv= (ConstantValue*) dereferenced_ptr;
		if(cv->get_constant() == 0) res = "NULL pointer dereference: ";
	}

	if(res == "") {
		res = "Dereference of possibly NULL pointer:";
	}

	set<access_info>::iterator it = deref_info.begin();
	for(; it!= deref_info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}

	return res;

}

ErrorTrace* DerefTrace::set_counter(int counter_id)
{
	return DerefTrace::make(dereferenced_ptr,fail_c, counter_id,deref_info);
}

void DerefTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	dereferenced_ptr = IndexVarManager::convert_index_vars_to_instantiation_vars(
			dereferenced_ptr);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);


	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(dereferenced_ptr);
}

void DerefTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{


	Constraint inst_fail_c = inst.get_instantiation(fail_c);
	inst_fail_c &= inst.get_stmt_guard();




	if(inst_fail_c.unsat()){
		return;
	}


	set<pair<AccessPath*, Constraint> > instantiations;
	inst.get_instantiation(dereferenced_ptr, instantiations);


	if(inst.is_new_calling_context()){
			callstack_entry ce = inst.get_calling_context();
			push_caller(ce);
	}

	set<pair<AccessPath*, Constraint> >::iterator it = instantiations.begin();
	for(; it!= instantiations.end(); it++)
	{
		Constraint new_fail_c = inst_fail_c & it->second;
		AccessPath* inst_trace = it->first;
		IndexVarManager::eliminate_sum_index_vars(new_fail_c);
		new_fail_c &= get_unsigned_constraints(inst_trace);
		if(new_fail_c.unsat()) continue;

		DerefTrace* new_dt = DerefTrace::make(inst_trace, new_fail_c,
				inst.get_counter_id(), deref_info);
		inst_traces.insert(new_dt);

		cout << "NULL DEREF CONDITION: " << new_fail_c << endl;
	}



}

DerefTrace::~DerefTrace() {

}
