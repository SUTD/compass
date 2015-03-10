/*
 * MemoryLeakTrace.cpp
 *
 *  Created on: May 3, 2010
 *      Author: isil
 */

#include "MemoryLeakTrace.h"
#include "compass_assert.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "ConstraintGenerator.h"
#include "util.h"
#include "Alloc.h"

set<MemoryLeakTrace*, leak_trace_lt> MemoryLeakTrace::leak_traces;

bool leak_trace_lt::operator() (const MemoryLeakTrace* mlt1,
		const MemoryLeakTrace* mlt2) const
{
	MemoryLeakTrace* lt1 = (MemoryLeakTrace*) mlt1;
	MemoryLeakTrace* lt2 = (MemoryLeakTrace*) mlt2;
	if(lt1->leaked_loc < lt2->leaked_loc) return true;
	if(lt1->leaked_loc > lt2->leaked_loc) return false;
	return lt1->counter_id < lt2->counter_id;
}

MemoryLeakTrace* MemoryLeakTrace::get_shared(MemoryLeakTrace* lt)
{
	set<MemoryLeakTrace*>::iterator it = leak_traces.find(lt);
	if(it != leak_traces.end())
	{
		delete lt;
		return *it;
	}
	leak_traces.insert(lt);
	return lt;
}


MemoryLeakTrace* MemoryLeakTrace::make(AccessPath* leaked_loc, string expression,
			int line, string file, Constraint fail_c, int counter_id)
{

	if(file.find("spec_")!=string::npos) return NULL;

	access_info ai(expression, line, file);
	MemoryLeakTrace* lt = new MemoryLeakTrace(leaked_loc, counter_id);

	lt = get_shared(lt);

	lt->info.insert(ai);
	lt->fail_c |= fail_c;


	return lt;
}

MemoryLeakTrace* MemoryLeakTrace::make(AccessPath* leaked_loc, Constraint fail_c,
			int counter_id, set<access_info>& info)
{
	MemoryLeakTrace* lt = new MemoryLeakTrace(leaked_loc, counter_id);
	lt = get_shared(lt);

	lt->info.insert(info.begin(), info.end());
	lt->fail_c |= fail_c;



	return lt;
}

void MemoryLeakTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.callstack.push_back(ce);
	}
}

void MemoryLeakTrace::report_error(vector<Error*>& errors, const Identifier & id)
{
	c_assert(info.size() > 0);
	int first_line = info.begin()->line;
	string file = info.begin()->file;
	Error* error = new Error(first_line, file, ERROR_MEMORY_LEAK, to_error(), id);
	errors.push_back(error);
}

string MemoryLeakTrace::to_string()
{
	string res = "Memory leak trace: ";
	res += this->leaked_loc->to_string();
	res += " / " + fail_c.to_string();
	return res;
}

string MemoryLeakTrace::to_error()
{
	string res = "Memory leak error: \n";
	Alloc* a = (Alloc*) leaked_loc;
	const info_item& ii = a->get_alloc_id().get_original_allocation_site();
	res+= "Allocated at: " + ii.file + ":" + int_to_string(ii.line);
	const vector<info_item>& ct = a->get_alloc_id().callstack;
	if(ct.size() > 1) res += "\nCalled through: \n";
	for(int i=1; i<ct.size(); i++)
	{
		const info_item& ii = ct[i];
		res += "\t" + ii.file + ":" + int_to_string(ii.line) + "\n";
	}




	/*set<access_info>::iterator it = info.begin();
	for(; it!= info.end(); it++)
	{
		access_info ai = *it;
		string cur = callstack_to_string(ai.callstack);
		string trimmed = trim_whitespaces(cur);
		if(trimmed == "") continue;
		res += "Access info: \n";

		res += callstack_to_string(ai.callstack);
	} */

	return res;
}
string MemoryLeakTrace::get_id()
{
	return leaked_loc->to_string();
}
ErrorTrace* MemoryLeakTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = leaked_loc->replace(ap, replacement);
	return MemoryLeakTrace::make(new_ap, fail_c, counter_id, info);
}

void MemoryLeakTrace::instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function)
{
	Constraint inst_fail_c = inst.get_instantiation(fail_c);


	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) {
		return;
	}

	set<pair<AccessPath*, Constraint> > ap_instantiations;
	inst.get_instantiation(leaked_loc, ap_instantiations);


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


		if(new_fail_c.unsat()) continue;

		MemoryLeakTrace* new_dt = MemoryLeakTrace::make(inst_ap,
				new_fail_c,  inst.get_counter_id(), info);

		inst_traces.insert(new_dt);


	}
}
void MemoryLeakTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	leaked_loc = IndexVarManager::convert_index_vars_to_instantiation_vars(
			leaked_loc);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);
	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(leaked_loc);
}

ErrorTrace* MemoryLeakTrace::set_counter(int counter_id)
{
	return MemoryLeakTrace::make(leaked_loc, fail_c,
			counter_id,  info);
}


MemoryLeakTrace::MemoryLeakTrace()
{

}

MemoryLeakTrace::MemoryLeakTrace(AccessPath* leaked_memory, int counter_id)
{
	this->leaked_loc = leaked_memory;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = MEMORY_LEAK_TRACE;
}
MemoryLeakTrace::~MemoryLeakTrace()
{

}
