/*
 * DeletedAccessTrace.cpp
 *
 *  Created on: May 13, 2010
 *      Author: isil
 */

#include "DeletedAccessTrace.h"
#include "compass_assert.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "ConstraintGenerator.h"
#include "util.h"
#include "FieldSelection.h"
#include "ConstantValue.h"
#include "sail/Variable.h"

set<DeletedAccessTrace*, deleted_access_lt>
		DeletedAccessTrace::deleted_access_traces;

bool deleted_access_lt::operator() (const DeletedAccessTrace* _ddt1,
		const DeletedAccessTrace* _ddt2) const
{
	bool res;
	DeletedAccessTrace* ddt1 = (DeletedAccessTrace*) _ddt1;
	DeletedAccessTrace* ddt2 = (DeletedAccessTrace*) _ddt2;
	if(ddt1->accessed_loc < ddt2->accessed_loc) {
		res = true;
	}
	else if(ddt1->accessed_loc > ddt2->accessed_loc) {
		res = false;
	}
	else res = ddt1->counter_id < ddt2->counter_id;


	return res;
}

DeletedAccessTrace* DeletedAccessTrace::get_shared(DeletedAccessTrace* dt)
{



	set<DeletedAccessTrace*>::iterator it = deleted_access_traces.find(dt);
	if(it != deleted_access_traces.end())
	{
		delete dt;
		return *it;
	}

	deleted_access_traces.insert(dt);
	return dt;
}


DeletedAccessTrace* DeletedAccessTrace::make(AccessPath* deleted_loc,
		string expression, int line, string file, Constraint fail_c,
		int counter_id)
{

	if(!check_eligible(deleted_loc)) {
		return NULL;
	}
	access_info ai(expression, line, file);
	DeletedAccessTrace* dt = new DeletedAccessTrace(deleted_loc, counter_id);
	dt = get_shared(dt);

	dt->info.insert(ai);
	dt->fail_c |= fail_c;



	return dt;

}

bool DeletedAccessTrace::check_eligible(AccessPath* ap)
{

	bool res = true;
	ap_type apt = ap->get_ap_type();

	AccessPath* orig_ap = ap;

	if(apt == AP_DEREF) {
		ap = ap->get_inner();
	}
	apt = ap->get_ap_type();

	if(apt == AP_ADDRESS) res=  false;
	else {

		AccessPath* base = ap->get_base();
		if(base == NULL) res = false;
		else{
			if(base->get_ap_type() == AP_ALLOC) res = true;

			else if(base->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) base;
				sail::Variable* sail_var = v->get_original_var();
				if(sail_var == NULL) res = false;
				//else if(sail_var->is_global() || sail_var->is_argument())
					//res = true;
			}
		}
	}

	return res;
}

DeletedAccessTrace* DeletedAccessTrace::make(AccessPath* deleted_loc,
		Constraint fail_c, int counter_id, set<access_info>& info)
{

	if(!check_eligible(deleted_loc)) {
		return NULL;
	}
	DeletedAccessTrace* dt = new DeletedAccessTrace(deleted_loc, counter_id);



	dt =get_shared(dt);

	dt->info.insert(info.begin(), info.end());
	dt->fail_c |= fail_c;




	return dt;
}

DeletedAccessTrace::DeletedAccessTrace()
{

}

DeletedAccessTrace::DeletedAccessTrace(AccessPath* accessed_memory, int counter_id)
{
	this->accessed_loc = accessed_memory;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = DELETED_ACCESS_TRACE;


}

void DeletedAccessTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.push_caller(ce.fn_name, ce.file, ce.line);
	}
}
void DeletedAccessTrace::report_error(vector<Error*>& errors, const Identifier & id)
{



	c_assert(info.size() > 0);
	int first_line = info.begin()->line;
	string file = info.begin()->file;


	Error* error = new Error(first_line, file, ERROR_DELETED_ACCESS, to_error(),
			id);
	errors.push_back(error);
}
string DeletedAccessTrace::to_string()
{
	string res = "Deleted access trace: ";
	res += this->accessed_loc->to_string();
	res += " / " + fail_c.to_string();
	return res;
}
string DeletedAccessTrace::to_error()
{
	string res = "Access to deleted memory: ";
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}


	return res;
}

string DeletedAccessTrace::get_id()
{
	return accessed_loc->to_string();
}
ErrorTrace* DeletedAccessTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = accessed_loc->replace(ap, replacement);
	return DeletedAccessTrace::make(new_ap, fail_c, counter_id, info);
}

void DeletedAccessTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{
	Constraint inst_fail_c = inst.get_instantiation(fail_c);

	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) {
		return;
	}


	set<pair<AccessPath*, Constraint> > ap_instantiations;
	inst.get_instantiation(accessed_loc, ap_instantiations);


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



		DeletedAccessTrace* new_dt = DeletedAccessTrace::make(inst_ap,
				new_fail_c,  inst.get_counter_id(), info);
		if(new_dt != NULL) inst_traces.insert(new_dt);


	}
}
void DeletedAccessTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	accessed_loc = IndexVarManager::convert_index_vars_to_instantiation_vars(
			accessed_loc);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);
	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(accessed_loc);
}

ErrorTrace* DeletedAccessTrace::set_counter(int counter_id)
{
	return DeletedAccessTrace::make(accessed_loc, fail_c,
			counter_id,  info);
}

Term* replace_deleted_field(Term* t, void* data)
{
	if(!t->is_specialized()) return t;
	AccessPath* ap = AccessPath::to_ap(t);
	if(ap->get_ap_type() != AP_FIELD) {
		return t;
	}
	FieldSelection* fs =  static_cast<FieldSelection*>(ap);
	if(!fs->is_deleted_field()) {
		return t;
	}
	AccessPath* base = fs->get_base();
	if(base->get_ap_type() != AP_VARIABLE){
		return t;
	}
	Variable* v = (Variable*) base;
	if(!v->is_argument()){
		return t;
	}
	return ConstantValue::make(0);
}



DeletedAccessTrace::~DeletedAccessTrace()
{

}

