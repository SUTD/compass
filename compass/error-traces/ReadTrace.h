/*
 * ReadTrace.h
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#ifndef READTRACE_H_
#define READTRACE_H_

#include "ErrorTrace.h"

class ReadTrace: public ErrorTrace {
	friend class boost::serialization::access;

private:
	static map<pair<AccessPath*, int>, ReadTrace* > read_traces;

	set<access_info> read_info;
	AccessPath* accessed_memory;

private:

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & accessed_memory;
		ar & read_info;

		/*set<access_info>::iterator it =  read_info.begin();
		int num_ai = read_info.size();
		ar & num_ai;
		for(; it!= read_info.end(); it++)
		{
			access_info ai = *it;
			ar & ai.line;
			ar & ai.file;
			ar & ai.expression;
			ar & ai.callstack;
			//ar & ai;
		}

		ar & accessed_memory; */

	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{

		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & accessed_memory;
		ar & read_info;

		accessed_memory = AccessPath::to_ap(
				Term::get_term_nodelete(accessed_memory->to_term()));

		/*int num_ai;
		ar & num_ai;
		for(int i=0; i<num_ai; i++)
		{
			access_info ai;
			ar & ai.line;
			ar & ai.file;
			ar & ai.expression;
			ar & ai.callstack;
			//ar & ai;
			read_info.insert(ai);
		}
		ar & accessed_memory;
		accessed_memory = AccessPath::to_ap(
				Term::get_term_nodelete(accessed_memory->to_term())); */
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	ReadTrace(){};
	ReadTrace(AccessPath* accessed_memory, int counter_id);
	virtual ~ReadTrace();

public:
	static ReadTrace* make(AccessPath* accessed_memory,
			string pp_expression, int line,
			string file, Constraint fail_c, int counter_id);

	static ReadTrace* make(AccessPath* accessed_memory,
			Constraint fail_c, int counter_id,
			set<access_info>& deref_info);

	virtual ErrorTrace* set_counter(int counter_id);



	void push_caller(callstack_entry& ce);

	virtual void report_error(vector<Error*>& errors, const Identifier & id);
	virtual string to_string();
	virtual string to_error();
	virtual string get_id();
	virtual int get_line() {return -1;};
	virtual ErrorTrace* replace(AccessPath* ap, AccessPath* replacement);
	virtual void instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function);


	virtual void get_access_paths_to_instantiate(set<AccessPath*>& aps);


};

#endif /* READTRACE_H_ */
