/*
 * DerefTrace.h
 *
 *  Created on: Nov 11, 2009
 *      Author: isil
 */

#ifndef DEREFTRACE_H_
#define DEREFTRACE_H_

#include "ErrorTrace.h"

class DerefTrace;
class AccessPath;


class DerefTrace: public ErrorTrace {
	friend class boost::serialization::access;
private:
	static map<pair<AccessPath*, int>, DerefTrace* > deref_traces;

	set<access_info> deref_info;
	AccessPath* dereferenced_ptr;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		assert(deref_info.size() >=1);
		access_info ai = *deref_info.begin();
		ar & ai.file;
		ar & ai.line;
		ar & ai.expression;
		ar & ai.callstack;
		ar & dereferenced_ptr;
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);

		access_info ai;
		ar & ai.file;
		ar & ai.line;
		ar & ai.expression;
		ar & ai.callstack;
		deref_info.insert(ai);

		ar & dereferenced_ptr;
		dereferenced_ptr = AccessPath::to_ap(
			Term::get_term_nodelete(dereferenced_ptr->to_term()));
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()


protected:
	DerefTrace() {};
	DerefTrace(AccessPath* dereferenced_value, int counter_id);
	virtual ~DerefTrace();

public:
	static DerefTrace* make(AccessPath* dereferenced_ptr,
			string pp_expression, int line,
			string file, Constraint fail_c, int counter_id);

	static DerefTrace* make(AccessPath* dereferenced_ptr,
			Constraint fail_c, int counter_id,
			set<access_info>& deref_info);

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
private:
	static Constraint get_unsigned_constraints(AccessPath* dereferenced_ptr);


};

#endif /* DEREFTRACE_H_ */
