/*
 * ErrorTrace.h
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#ifndef ERRORTRACE_H_
#define ERRORTRACE_H_

#include <vector>
using namespace std;
#include "AccessPath.h"
#include "Constraint.h"
#include <boost/serialization/string.hpp>

class Error;
class Instantiator;
class Identifier;

enum trace_type
{
	ASSERTION_TRACE,
	READ_TRACE,
	DEREF_TRACE,
	BUFFER_ACCESS_TRACE,
	CAST_ERROR_TRACE,
	DOUBLE_DELETE_TRACE,
	MEMORY_LEAK_TRACE,
	DELETED_ACCESS_TRACE
};



struct callstack_entry
{
	friend class boost::serialization::access;
	callstack_entry(const string & fn_name, const string & file, int line);
	string fn_name;
	string file;
	int line;
	string to_string();

private:
	callstack_entry(){};
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & fn_name;
		ar & file;
		ar & line;
	}



};

typedef vector<callstack_entry> callstack_t;

string callstack_to_string(callstack_t& callstack);

struct access_info {
	friend class boost::serialization::access;
	string expression;
	int line;
	string file;
	callstack_t callstack;
	access_info(){file="unknown"; line = -1; expression="unknown";};

private:

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{

		ar & line;
		ar & file;
		ar & callstack;
		ar & expression;


	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{

		ar & line;
		ar & file;
		ar & callstack;
		ar & expression;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:

	access_info(string expression, int line, string file)
	{
		this->expression = expression;
		this->line = line;
		this->file = file;
	}

	bool operator<(const access_info& other) const
	{

		if(line > other.line) return false;
		if(line < other.line) return true;
		if(file > other.file) return false;
		if(file < other.file) return true;
		if (expression > other.expression) return false;
		if(expression < other.expression) return true;
		if(callstack.size() > other.callstack.size()) return false;
		if(callstack.size() < other.callstack.size()) return true;
		for(unsigned int i=0; i<callstack.size(); i++) {
			if(callstack[i].file > other.callstack[i].file) return false;
			if(callstack[i].file < other.callstack[i].file) return true;
			if(callstack[i].fn_name > other.callstack[i].fn_name) return false;
			if(callstack[i].fn_name < other.callstack[i].fn_name) return true;
			if(callstack[i].line > other.callstack[i].line) return false;
			if(callstack[i].line < other.callstack[i].line) return true;
		}
		return false;

	}

	string to_string();

	void push_caller(string fn_name, string file, int line);


};



class ErrorTrace {
	friend class boost::serialization::access;
protected:
	Constraint fail_c;
	//int line;
	//string file;
	int counter_id; // only meaningful for loops
	//vector<callstack_entry> failure_path;

	trace_type tt;
private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & fail_c;
		//ar & line;
		//ar & file;
		ar & counter_id;
		ar & tt;
		//ar & failure_path;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & fail_c;
		//ar & line;
		//ar & file;
		ar & counter_id;
		ar & tt;
		//ar & failure_path;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	ErrorTrace(){};

public:
	trace_type get_trace_type();
	virtual void report_error(vector<Error*> & errors, const Identifier & id) = 0;
	virtual string to_string() = 0;
	virtual string to_error() = 0;
	virtual string get_id() = 0;
	virtual int get_line() = 0;
	virtual ErrorTrace* replace(AccessPath* ap, AccessPath* replacement) = 0;
	Constraint get_failure_cond();
	virtual void instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function) = 0;
	virtual void get_access_paths_to_instantiate(set<AccessPath*>& aps) =0;

	void set_failure_cond(Constraint c);
	//void push_caller(string fn_name, string file, int line);
	//vector<callstack_entry>& get_error_path();
	//string callstack_to_string();
	int get_counter_id();
	virtual ~ErrorTrace();
	virtual ErrorTrace* set_counter(int counter_id) = 0;
	static string trace_type_to_string(trace_type t);

};

#endif /* ERRORTRACE_H_ */
