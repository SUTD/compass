/*
 * BufferTrace.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#include "BufferTrace.h"
#include "Error.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "IndexVarManager.h"
#include "compass_assert.h"

set<BufferTrace*, buffer_lt> BufferTrace::buffer_traces;


bool buffer_lt::operator()(const BufferTrace* _l1, const BufferTrace* _l2) const
{
	BufferTrace* l1 = (BufferTrace*) _l1;
	BufferTrace* l2 = (BufferTrace*) _l2;
	if(l1->accessed_memory < l2->accessed_memory) return true;
	if(l1->accessed_memory > l2->accessed_memory) return false;
	if(l1->overrun < l2->overrun) return true;
	if(l1->overrun > l2->overrun) return false;
	if(l1->offset < l2->offset) return true;
	if(l1->offset > l2->offset) return false;
	return l1->counter_id < l2->counter_id;
}

BufferTrace::BufferTrace(AccessPath* accessed_memory, AccessPath* offset,
		int counter_id, bool overrun)
{

	this->accessed_memory = accessed_memory;
	this->offset = offset;
	this->counter_id = counter_id;
	this->overrun = overrun;
	this->fail_c = Constraint(false);
	this->tt = BUFFER_ACCESS_TRACE;

}

BufferTrace* BufferTrace::get_shared(BufferTrace* bt)
{
	set<BufferTrace*>::iterator it = buffer_traces.find(bt);
	if(it != buffer_traces.end())
	{
		delete bt;
		return *it;
	}

	buffer_traces.insert(bt);
	return bt;
}

BufferTrace* BufferTrace::make(AccessPath* accessed_memory, AccessPath* offset,
		bool overrun, string expression, int line,
		string file, Constraint fail_c, int counter_id)
{
	access_info ai(expression, line, file);
	BufferTrace* bt = new BufferTrace(accessed_memory, offset, counter_id,
			overrun);
	bt = get_shared(bt);

	bt->buffer_info.insert(ai);
	bt->fail_c |= fail_c;


	if(bt->to_string().find("temp25") != string::npos) {
		cout << "FAILURE COND: " << fail_c << endl;

		cout << "bg: " <<
				Constraint::background_knowledge_to_string() << endl;
	}


	return bt;
}

BufferTrace* BufferTrace::make(AccessPath* accessed_memory, AccessPath* offset,
		Constraint fail_c, int counter_id, bool overrun,
		set<access_info>& buffer_info)
{
	BufferTrace* bt = new BufferTrace(accessed_memory, offset, counter_id,
			overrun);
	bt = get_shared(bt);

	bt->buffer_info.insert(buffer_info.begin(), buffer_info.end());
	bt->fail_c |= fail_c;


	if(bt->to_string().find("temp25") != string::npos) {
		cout << "FAILURE COND: " << fail_c << endl;

		cout << "bg: " <<
				Constraint::background_knowledge_to_string() << endl;
	}
	return bt;
}

void BufferTrace::push_caller(callstack_entry& ce)
{
	set<access_info>::iterator it = buffer_info.begin();
	for(; it!= buffer_info.end(); it++)
	{
		access_info& ai = (access_info&)*it;
		ai.callstack.push_back(ce);
	}
}

ErrorTrace* BufferTrace::set_counter(int counter_id)
{
	return BufferTrace::make(accessed_memory, offset, fail_c,
			counter_id, overrun, buffer_info);
}

void BufferTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{


	Constraint inst_fail_c = inst.get_instantiation(fail_c);


	inst_fail_c &= inst.get_stmt_guard();
	if(inst_fail_c.unsat()) {
		return;
	}

	set<pair<AccessPath*, Constraint> > buffer_instantiations;
	inst.get_instantiation(accessed_memory, buffer_instantiations);

	set<pair<AccessPath*, Constraint> > offset_instantiations;
	inst.get_instantiation(offset, offset_instantiations);

	if(inst.is_new_calling_context()){
			callstack_entry ce = inst.get_calling_context();
			push_caller(ce);
	}

	set<pair<AccessPath*, Constraint> >::iterator it1 = buffer_instantiations.begin();
	for(; it1!= buffer_instantiations.end(); it1++)
	{
		Constraint new_fail_c1 = inst_fail_c & it1->second;
		AccessPath* inst_buf = it1->first;
		if(new_fail_c1.unsat()) continue;

		set<pair<AccessPath*, Constraint> >::iterator it2 = offset_instantiations.begin();
		for(; it2!=offset_instantiations.end(); it2++)
		{
			AccessPath* inst_offset = it2->first;
			Constraint new_fail_c2 = new_fail_c1 & it2->second;
			IndexVarManager::eliminate_sum_index_vars(new_fail_c2);

			if(new_fail_c2.unsat()) continue;



			BufferTrace* new_bt = BufferTrace::make(inst_buf, inst_offset,
					new_fail_c2, inst.get_counter_id(), overrun,
					buffer_info);



			inst_traces.insert(new_bt);


		}


	}

}

void BufferTrace::report_error(vector<Error*>& errors, const Identifier & id)
{
	c_assert(buffer_info.size() > 0);
	int first_line = buffer_info.begin()->line;
	string file = buffer_info.begin()->file;
	Error* error;
	if(overrun) error = new Error(first_line, file, ERROR_BUFFER_OVERRUN, to_error(),
			id);
	else error = new Error(first_line, file, ERROR_BUFFER_UNDERRUN, to_error(),
			id);
	errors.push_back(error);
}

string BufferTrace::to_string()
{
	string res;
	if(overrun) res += "Overrun trace: ";
	else res += "Underrun trace: ";

	res+= accessed_memory->to_string() + "[" + offset->to_string() + "]";
	res += " / " + fail_c.to_string();
	return res;
}

string BufferTrace::get_id()
{
	return accessed_memory->to_string() + "[" + offset->to_string() + "]";
}

ErrorTrace* BufferTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	AccessPath* new_ap = accessed_memory->replace(ap, replacement);
	AccessPath* new_offset = offset->replace(ap, replacement);

	return BufferTrace::make(new_ap, new_offset, fail_c, counter_id, overrun,
			buffer_info);
}

string BufferTrace::to_error()
{
	string res;
	if(overrun) res= "Buffer overrun error: ";
	else res = res= "Buffer underrun error:";
	set<access_info>::iterator it = buffer_info.begin();
	for(; it!= buffer_info.end(); it++)
	{
		access_info ai = *it;
		res += "  ";
		res += ai.to_string();
	}
	return res;

}

void BufferTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	accessed_memory = IndexVarManager::convert_index_vars_to_instantiation_vars(
			accessed_memory);
	offset = IndexVarManager::convert_index_vars_to_instantiation_vars(offset);
	fail_c = IndexVarManager::convert_index_vars_to_instantiation_vars(fail_c);
	SummaryGraph::collect_access_paths(fail_c, aps);
	aps.insert(accessed_memory);
	aps.insert(offset);


}

BufferTrace::~BufferTrace() {

}
