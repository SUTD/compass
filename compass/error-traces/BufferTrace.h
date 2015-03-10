/*
 * BufferTrace.h
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#ifndef BUFFERTRACE_H_
#define BUFFERTRACE_H_

#include "ErrorTrace.h"
class BufferTrace;
struct buffer_lt
{
  bool operator()(const BufferTrace* l1, const BufferTrace * l2) const;
};

class BufferTrace: public ErrorTrace {
	friend struct buffer_lt;
	friend class boost::serialization::access;

private:
	static set<BufferTrace*, buffer_lt> buffer_traces;

	set<access_info> buffer_info;
	AccessPath* accessed_memory;
	AccessPath* offset;
	bool overrun;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & buffer_info;

		/*int num_info = buffer_info.size();
		ar & num_info;
		set<access_info>::iterator it = buffer_info.begin();
		for(; it!= buffer_info.end();  it++)
		{
			access_info ai = *it;
			ar & ai.file;
			ar & ai.line;
			ar & ai.expression;
			ar & ai.callstack;
		}*/
		ar & accessed_memory;
		ar & offset;
		ar & overrun;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);

		/*int num_info;
		ar & num_info;
		for(int i=0; i<num_info; i++) {
			access_info ai;
			ar & ai.file;
			ar & ai.line;
			ar & ai.expression;
			ar & ai.callstack;
			buffer_info.insert(ai);
		}*/

		ar & buffer_info;
		ar & accessed_memory;
		ar & offset;
		ar & overrun;
		accessed_memory = AccessPath::to_ap(
				Term::get_term_nodelete(accessed_memory->to_term()));
		offset = AccessPath::to_ap(
				Term::get_term_nodelete(offset->to_term()));

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	static BufferTrace* make(AccessPath* accessed_memory, AccessPath* offset,
			bool overrrun, string expression, int line,
			string file, Constraint fail_c, int counter_id);

	static BufferTrace* make(AccessPath* accessed_memory, AccessPath* offset,
			Constraint fail_c, int counter_id, bool overrun,
			set<access_info>& buffer_info);

	void push_caller(callstack_entry& ce);

	virtual ErrorTrace* set_counter(int counter_id);
	virtual int get_line() {return -1;};

	virtual void report_error(vector<Error*>& errors, const Identifier & id);
	virtual string to_string();
	virtual string to_error();
	virtual string get_id();
	virtual ErrorTrace* replace(AccessPath* ap, AccessPath* replacement);

	virtual void instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function);

	virtual void get_access_paths_to_instantiate(set<AccessPath*>& aps);

protected:
	BufferTrace() {};
	BufferTrace(AccessPath* accessed_memory, AccessPath* offset, int counter_id,
			bool overrrun);
	virtual ~BufferTrace();
private:
	static BufferTrace* get_shared(BufferTrace* bt);
};

#endif /* BUFFERTRACE_H_ */
