/*
 * CastErrorTrace.cpp
 *
 *  Created on: May 2, 2010
 *      Author: isil
 */

#include "CastErrorTrace.h"
#include "compass_assert.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "ConstraintGenerator.h"
#include "FieldSelection.h"
#include "type.h"
#include "TypeConstant.h"


set<CastErrorTrace*, cast_error_lt> CastErrorTrace::cast_error_traces;

bool cast_error_lt::operator()(const CastErrorTrace* _l1,
		const CastErrorTrace* _l2) const
{
	CastErrorTrace* l1 = (CastErrorTrace*) _l1;
	CastErrorTrace* l2 = (CastErrorTrace*) _l2;
	if(l1->memory_loc < l2->memory_loc) return true;
	if(l1->memory_loc > l2->memory_loc) return false;
	if(l1->static_type < l2->static_type) return true;
	if(l1->static_type > l2->static_type) return false;
	return l1->counter_id < l2->counter_id;
}

CastErrorTrace* CastErrorTrace::get_shared(CastErrorTrace* cast_t)
{
	set<CastErrorTrace*>::iterator it = cast_error_traces.find(cast_t);
	if(it != cast_error_traces.end())
	{
		delete cast_t;
		return *it;
	}

	cast_error_traces.insert(cast_t);
	return cast_t;
}


CastErrorTrace::CastErrorTrace(AccessPath* accessed_memory,
		il::type* static_type, int counter_id) {

	this->memory_loc = accessed_memory;
	this->static_type = static_type;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = CAST_ERROR_TRACE;
}

CastErrorTrace* CastErrorTrace::make(AccessPath* accessed_memory,
		il::type* static_type, string expression, int line,
		string file, Constraint fail_c,  int counter_id)
{
	access_info ai(expression, line, file);
	CastErrorTrace* cast_t = new CastErrorTrace(accessed_memory, static_type,
			counter_id);
	cast_t = get_shared(cast_t);

	cast_t->info.insert(ai);
	cast_t->fail_c |= fail_c;


	return cast_t;
}

CastErrorTrace* CastErrorTrace::make(AccessPath* accessed_memory,
		il::type* static_type, Constraint fail_c, int counter_id,
		set<access_info>& info)
{
	CastErrorTrace* cast_t = new CastErrorTrace(accessed_memory, static_type,
			counter_id);
	cast_t = get_shared(cast_t);

	cast_t->info.insert(info.begin(), info.end());
	cast_t->fail_c |= fail_c;
	return cast_t;
}

void CastErrorTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.callstack.push_back(ce);
	}
}

Term* replace_rtti_field(Term* t, void* data)
{
	if(!t->is_specialized()) return t;
	AccessPath* ap = AccessPath::to_ap(t);
	if(ap->get_ap_type() != AP_FIELD) {
		return t;
	}
	FieldSelection* fs =  static_cast<FieldSelection*>(ap);
	if(!fs->is_rtti_field()) {
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
	il::type* typ = (il::type*) data;
	TypeConstant* tc = TypeConstant::make(typ);
	return tc->to_term();

}

void CastErrorTrace::report_error(vector<Error*>& errors, const Identifier & id)
{


	c_assert(info.size() > 0);
	int first_line = info.begin()->line;
	string file = info.begin()->file;
	Error* error = new Error(first_line, file, ERROR_ILLEGAL_CAST, to_error(), id);
	errors.push_back(error);
}
string CastErrorTrace::to_string()
{
	string res = "Cast error trace: ";
	res += this->memory_loc->to_string();
	res += " / " + fail_c.to_string();
	return res;
}
string CastErrorTrace::to_error()
{
	string res = "Illegal cast error: ";
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}
	return res;
}
string CastErrorTrace::get_id()
{
	return this->memory_loc->to_string() + ", " + static_type->to_string();
}

ErrorTrace* CastErrorTrace::replace(AccessPath* ap, AccessPath* replacement)
{

	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = memory_loc->replace(ap, replacement);
	return CastErrorTrace::make(new_ap, static_type, fail_c, counter_id, info);
}

void CastErrorTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{

	Constraint inst_fail_c = inst.get_instantiation(fail_c);


	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) {
		return;
	}

	set<pair<AccessPath*, Constraint> > ap_instantiations;
	inst.get_instantiation(memory_loc, ap_instantiations);


	if(inst.is_new_calling_context()){
			callstack_entry ce = inst.get_calling_context();
			push_caller(ce);
	}

	set<pair<AccessPath*, Constraint> >::iterator it1 = ap_instantiations.begin();
	for(; it1!= ap_instantiations.end(); it1++)
	{

		Constraint new_fail_c = inst_fail_c & it1->second;

		IndexVarManager::eliminate_sum_index_vars(new_fail_c);


		AccessPath* inst_ap = it1->first;




		if(top_level_function) {
			new_fail_c.replace_terms(replace_rtti_field, static_type);
		}
		if(new_fail_c.unsat()) continue;


		CastErrorTrace* cast_t = CastErrorTrace::make(inst_ap, static_type,
				new_fail_c,  inst.get_counter_id(), info);


		inst_traces.insert(cast_t);


	}
}
void CastErrorTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	memory_loc = IndexVarManager::convert_index_vars_to_instantiation_vars(
			memory_loc);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);


	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(memory_loc);
}

ErrorTrace* CastErrorTrace::set_counter(int counter_id)
{
	return CastErrorTrace::make(memory_loc, static_type, fail_c,
			counter_id,  info);
}



CastErrorTrace::~CastErrorTrace() {

}
