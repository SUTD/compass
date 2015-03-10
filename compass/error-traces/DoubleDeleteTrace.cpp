/*
 * DoubleDeleteTrace.cpp
 *
 *  Created on: May 3, 2010
 *      Author: isil
 */

#include "DoubleDeleteTrace.h"
#include "compass_assert.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "ConstraintGenerator.h"
#include "util.h"
#include "DeletedAccessTrace.h"

set<DoubleDeleteTrace*, delete_trace_lt>
		DoubleDeleteTrace::double_delete_traces;

bool delete_trace_lt::operator() (const DoubleDeleteTrace* _ddt1,
		const DoubleDeleteTrace* _ddt2) const
{
	bool res;
	DoubleDeleteTrace* ddt1 = (DoubleDeleteTrace*) _ddt1;
	DoubleDeleteTrace* ddt2 = (DoubleDeleteTrace*) _ddt2;
	if(ddt1->deleted_loc < ddt2->deleted_loc) {
		res = true;
	}
	else if(ddt1->deleted_loc > ddt2->deleted_loc) {
		res = false;
	}
	else res = ddt1->counter_id < ddt2->counter_id;


	return res;
}

DoubleDeleteTrace* DoubleDeleteTrace::get_shared(DoubleDeleteTrace* dt)
{
	set<DoubleDeleteTrace*>::iterator it = double_delete_traces.find(dt);
	if(it != double_delete_traces.end())
	{
		delete dt;
		return *it;
	}

	double_delete_traces.insert(dt);
	return dt;
}


DoubleDeleteTrace* DoubleDeleteTrace::make(AccessPath* deleted_loc,
		string expression, int line, string file, Constraint fail_c,
		int counter_id)
{
	access_info ai(expression, line, file);
	DoubleDeleteTrace* dt = new DoubleDeleteTrace(deleted_loc, counter_id);
	dt = get_shared(dt);

	dt->info.insert(ai);
	dt->fail_c |= fail_c;

	return dt;

}

DoubleDeleteTrace* DoubleDeleteTrace::make(AccessPath* deleted_loc,
		Constraint fail_c, int counter_id, set<access_info>& info)
{
	DoubleDeleteTrace* dt = new DoubleDeleteTrace(deleted_loc, counter_id);

	dt =get_shared(dt);

	dt->info.insert(info.begin(), info.end());
	dt->fail_c |= fail_c;
	return dt;
}

DoubleDeleteTrace::DoubleDeleteTrace()
{

}

DoubleDeleteTrace::DoubleDeleteTrace(AccessPath* deleted_memory, int counter_id)
{
	this->deleted_loc = deleted_memory;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = DOUBLE_DELETE_TRACE;

}

void DoubleDeleteTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.callstack.push_back(ce);
	}
}
void DoubleDeleteTrace::report_error(vector<Error*>& errors, const Identifier & id)
{
	c_assert(info.size() > 0);
	int first_line = info.begin()->line;
	string file = info.begin()->file;
	Error* error = new Error(first_line, file, ERROR_DOUBLE_DELETE, to_error(), id);
	errors.push_back(error);
}
string DoubleDeleteTrace::to_string()
{
	string res = "Double delete trace: ";
	res += this->deleted_loc->to_string();
	res += " / " + fail_c.to_string();
	return res;
}
string DoubleDeleteTrace::to_error()
{
	string res = "Double delete error: ";
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}
	return res;
}

string DoubleDeleteTrace::get_id()
{
	return deleted_loc->to_string();
}
ErrorTrace* DoubleDeleteTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = deleted_loc->replace(ap, replacement);
	return DoubleDeleteTrace::make(new_ap, fail_c, counter_id, info);
}

void DoubleDeleteTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{
	Constraint inst_fail_c = inst.get_instantiation(fail_c);



	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) {
		return;
	}

	set<pair<AccessPath*, Constraint> > ap_instantiations;
	inst.get_instantiation(deleted_loc, ap_instantiations);


	if(inst.is_new_calling_context()){
			callstack_entry ce = inst.get_calling_context();
			push_caller(ce);
	}

	set<pair<AccessPath*, Constraint> >::iterator it1 = ap_instantiations.begin();
	for(; it1!= ap_instantiations.end(); it1++)
	{

		Constraint new_fail_c = inst_fail_c & it1->second;
		AccessPath* inst_ap = it1->first;

		while(inst_ap->get_ap_type() == AP_FIELD){
			inst_ap = inst_ap->get_inner();
		}
		IndexVarManager::eliminate_sum_index_vars(new_fail_c);
		if(top_level_function) {
			new_fail_c.replace_terms(replace_deleted_field, NULL);
		}

		if(new_fail_c.unsat()) continue;
		DoubleDeleteTrace* new_dt = DoubleDeleteTrace::make(inst_ap,
				new_fail_c,  inst.get_counter_id(), info);

		inst_traces.insert(new_dt);


	}
}
void DoubleDeleteTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	deleted_loc = IndexVarManager::convert_index_vars_to_instantiation_vars(
			deleted_loc);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);
	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(deleted_loc);
}

ErrorTrace* DoubleDeleteTrace::set_counter(int counter_id)
{
	return DoubleDeleteTrace::make(deleted_loc, fail_c,
			counter_id,  info);
}



DoubleDeleteTrace::~DoubleDeleteTrace()
{

}
