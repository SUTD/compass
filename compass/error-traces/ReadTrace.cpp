/*
 * ReadTrace.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#include "ReadTrace.h"

#include "access-path.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "compass_assert.h"

map<pair<AccessPath*, int>, ReadTrace* > ReadTrace::read_traces;

ReadTrace* ReadTrace::make(AccessPath* accessed_memory,
		string pp_expression, int line,
		 string file, Constraint fail_c, int counter_id)
{
	access_info ai(pp_expression, line, file);
	ReadTrace* rt = NULL;

	if(read_traces.count(make_pair(accessed_memory, counter_id)) == 0)
	{
		rt = new ReadTrace(accessed_memory, counter_id);
		read_traces[make_pair(accessed_memory, counter_id)] = rt;
	}

	else {
		rt = read_traces[make_pair(accessed_memory, counter_id)];
	}


	rt->read_info.insert(ai);
	rt->fail_c |= fail_c;



	return rt;
}


ErrorTrace* ReadTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = accessed_memory->replace(ap, replacement);
	if(new_ap == accessed_memory) return this;
	return ReadTrace::make(new_ap, fail_c, counter_id, read_info);
}


void ReadTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = read_info.begin();
	for(; it!= read_info.end(); it++)
	{
		access_info& ai = (access_info&)*it;

		ai.callstack.push_back(ce);
	}
}

ErrorTrace* ReadTrace::set_counter(int counter_id)
{
	return ReadTrace::make(accessed_memory, fail_c, counter_id, read_info);
}

ReadTrace* ReadTrace::make(AccessPath* accessed_memory,
		Constraint fail_c, int counter_id,
		set<access_info>& read_info)
{



	ReadTrace* rt = NULL;
	if(read_traces.count(make_pair(accessed_memory, counter_id)) == 0)
	{
		rt = new ReadTrace(accessed_memory, counter_id);
		read_traces[make_pair(accessed_memory, counter_id)] = rt;
	}

	else {
		rt = read_traces[make_pair(accessed_memory, counter_id)];
	}
	rt->read_info.insert(read_info.begin(), read_info.end());
	rt->fail_c |= fail_c;
	return rt;
}


void ReadTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{
	Constraint inst_fail_c = inst.get_instantiation(fail_c);
	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) return;

	set<pair<AccessPath*, Constraint> > instantiations;
	inst.get_instantiation(accessed_memory, instantiations);

	if(inst.is_new_calling_context()){
			callstack_entry ce = inst.get_calling_context();
			push_caller(ce);
	}


	set<pair<AccessPath*, Constraint> >::iterator it = instantiations.begin();
	for(; it!= instantiations.end(); it++)
	{
		Constraint new_fail_c = inst_fail_c & it->second;
		AccessPath* inst_trace = it->first;
		if(inst_trace->is_initialized()) continue;
		IndexVarManager::eliminate_sum_index_vars(new_fail_c);
		if(new_fail_c.unsat()) continue;

		ReadTrace* new_dt = ReadTrace::make(inst_trace, new_fail_c,
				inst.get_counter_id(),
				read_info);


		inst_traces.insert(new_dt);
	}

}

ReadTrace::ReadTrace(AccessPath* accessed_memory, int counter_id)
{
	this->accessed_memory = accessed_memory;
	this->counter_id = counter_id;
	this->fail_c = Constraint(false);
	this->tt = READ_TRACE;

}

void ReadTrace::report_error(vector<Error*>& errors, const Identifier & id)
{
	if(!accessed_memory->contains_uninit_value()) return;
	c_assert(read_info.size() > 0);
	int first_line = read_info.begin()->line;
	string file = read_info.begin()->file;

	Error* error = new Error(first_line, file, ERROR_UNINITIALIZED_READ, to_error(), id);
	errors.push_back(error);
}
string ReadTrace::to_error()
{
	string res= "Uninitialized memory read: \n";
	set<access_info>::iterator it = read_info.begin();
	for(; it!= read_info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}
	return res;

}

string ReadTrace::to_string()
{
	string res= "Read trace: " + accessed_memory->to_string();
	res += " / "+ fail_c.to_string();
	return res;
}

string ReadTrace::get_id()
{
	return accessed_memory->to_string();
}

void ReadTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	accessed_memory = IndexVarManager::convert_index_vars_to_instantiation_vars(
			accessed_memory);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);


	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(accessed_memory);
}

ReadTrace::~ReadTrace() {

}
