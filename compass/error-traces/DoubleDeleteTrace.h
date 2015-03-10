/*
 * DoubleDeleteTrace.h
 *
 *  Created on: May 3, 2010
 *      Author: isil
 */

#ifndef DOUBLEDELETETRACE_H_
#define DOUBLEDELETETRACE_H_

#include "ErrorTrace.h"

class DoubleDeleteTrace;

struct delete_trace_lt
{
	bool operator()(const DoubleDeleteTrace* ddt1,
			const DoubleDeleteTrace* ddt2) const;
};

class DoubleDeleteTrace: public ErrorTrace {
	friend class boost::serialization::access;
	friend class delete_trace_lt;

private:
	static set<DoubleDeleteTrace*, delete_trace_lt> double_delete_traces;

	/*
	 * representative access path of the deleted location
	 */
	AccessPath* deleted_loc;
	set<access_info> info;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & deleted_loc;
		ar & info;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & deleted_loc;
		ar & info;
		deleted_loc = AccessPath::to_ap(
						Term::get_term_nodelete(deleted_loc->to_term()));
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
public:
	static DoubleDeleteTrace* make(AccessPath* deleted_loc, string expression,
			int line, string file, Constraint fail_c, int counter_id);

	static DoubleDeleteTrace* make(AccessPath* deleted_loc, Constraint fail_c,
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
	DoubleDeleteTrace();
	DoubleDeleteTrace(AccessPath* deleted_memory, int counter_id);
	virtual ~DoubleDeleteTrace();

private:
	static DoubleDeleteTrace* get_shared(DoubleDeleteTrace* ddt);
};

#endif /* DOUBLEDELETETRACE_H_ */
