/*
 * ErrorTrace.cpp
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#include "ErrorTrace.h"
#include "util.h"
#include "compass_assert.h"


Constraint ErrorTrace::get_failure_cond()
{
	return fail_c;
}

trace_type ErrorTrace::get_trace_type()
{
	return tt;
}

string ErrorTrace::trace_type_to_string(trace_type t)
{
	switch(t)
	{
	case ASSERTION_TRACE:
		return "Assertion Trace";
	case READ_TRACE:
		return "Read Trace";
	case DEREF_TRACE:
		return "Deref Trace";
	case BUFFER_ACCESS_TRACE:
		return "Buffer Trace";
	case CAST_ERROR_TRACE:
		return "Cast Error Trace";
	case DOUBLE_DELETE_TRACE:
		return "Double Delete Trace";
	case MEMORY_LEAK_TRACE:
		return "Memory Leak Trace";
	case DELETED_ACCESS_TRACE:
		return "Deleted Access Trace";
	default:
		c_assert(false);
	}
}

void ErrorTrace::set_failure_cond(Constraint c)
{
	fail_c = c;

}

/*
void ErrorTrace::push_caller(string fn_name, string file, int line)
{
	callstack_entry ce(fn_name, file, line);
	failure_path.push_back(ce);
}
*/

int ErrorTrace::get_counter_id()
{
	return counter_id;
}



/*
vector<callstack_entry>& ErrorTrace::get_error_path()
{
	return failure_path;
}
*/

ErrorTrace::~ErrorTrace()
{

}

callstack_entry::callstack_entry(const string & fn_name, const string & file,
		int line)
{
	this->fn_name = fn_name;
	this->file = file;
	this->line = line;
}

string callstack_entry::to_string()
{
	string res = fn_name + ":" + file + " (" + int_to_string(line) + ")";
	return res;
}


string callstack_to_string(callstack_t& callstack)
{
	string res;
	for(unsigned int i=0; i < callstack.size(); i++) {
		callstack_entry& ce = callstack[i];
		string cur_entry = "\t"+ce.fn_name + ":" + int_to_string(ce.line) + "\n";
		res += cur_entry;
	}

	return res;

}


string access_info::to_string()
{
	string res= expression + " at " + file + ":" + int_to_string(line);
	if(callstack.size() > 0) res += "\nCallstack: \n" +
			callstack_to_string(callstack);
	return res;
}

void access_info::push_caller(string fn_name, string file, int line)
{


	callstack_entry ce(fn_name, file, line);

	if(this->file.find("spec_")!= string::npos) {
		this->file = file;
		this->line= line;
	}

	if(callstack.size() > 0)
	{
		callstack_entry& ce = callstack[callstack.size()-1];
		if(ce.file.find("spec_")!= string::npos)
		{
			callstack.clear();
		}

	}
	callstack.push_back(ce);
}

