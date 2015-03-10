/*
 * MemoryLeakTrace.h
 *
 *  Created on: May 3, 2010
 *      Author: isil
 */

#ifndef MEMORYLEAKTRACE_H_
#define MEMORYLEAKTRACE_H_

#include "ErrorTrace.h"

class MemoryLeakTrace;

struct leak_trace_lt
{
	bool operator()(const MemoryLeakTrace* ddt1,
			const MemoryLeakTrace* ddt2) const;
};

class MemoryLeakTrace: public ErrorTrace {
	friend class boost::serialization::access;
	friend class leak_trace_lt;

private:
	static set<MemoryLeakTrace*, leak_trace_lt> leak_traces;

	AccessPath* leaked_loc;
	set<access_info> info;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & leaked_loc;
		ar & info;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & leaked_loc;
		ar & info;
		leaked_loc = AccessPath::to_ap(
						Term::get_term_nodelete(leaked_loc->to_term()));
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

public:
	static MemoryLeakTrace* make(AccessPath* leaked_loc, string expression,
			int line, string file, Constraint fail_c, int counter_id);

	static MemoryLeakTrace* make(AccessPath* leaked_loc, Constraint fail_c,
			int counter_id, set<access_info>& info);

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

	virtual ErrorTrace* set_counter(int counter_id);

protected:
	MemoryLeakTrace();
	MemoryLeakTrace(AccessPath* leaked_memory, int counter_id);
	virtual ~MemoryLeakTrace();
private:
	static MemoryLeakTrace* get_shared(MemoryLeakTrace* mlt);
};

#endif /* MEMORYLEAKTRACE_H_ */
