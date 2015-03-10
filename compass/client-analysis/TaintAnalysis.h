/*
 * TaintAnalysis.h
 *
 *  Created on: Sep 1, 2012
 *      Author: tdillig
 */

#ifndef TAINTANALYSIS_H_
#define TAINTANALYSIS_H_
#include "ClientAnalysis.h"

namespace il
{
	class record_type;

}


class TaintAnalysis:public ClientAnalysis{
public:
	TaintAnalysis();


	virtual string get_property_name();


	virtual il::type* get_property_type();

	virtual sail::Symbol* get_initial_value();

	virtual bool is_tracked_type(il::type* t);

	virtual bool process_error(const string & msg, Constraint fail_c);

	virtual ~TaintAnalysis();


	virtual void process_function_call_after(sail::FunctionCall* inst,
			bool known);

	void process_leak_all(sail::Variable *v);

	void process_library_call(sail::FunctionCall* inst);

	/*
	 * Collects all potential sources and sinks reachable from v.
	 * By source, we mean a variable that is potentially tainted on function
	 * entry, and by sink, we mean a variable that is potentially
	 * tainted after the function call.
	 *
	 * insert_v specifies if v should be inserted into sources/sinks.
	 *
	 * ret specifies if v originates from return value
	 */
	void collect_reachable_locs(sail::Variable* v, il::type* t,
			map<sail::Symbol*, string>& sources,
			map<sail::Variable*, pair<bool, string> >& sinks,
			bool insert_v, bool ret, const string & source_name,
			const string & sink_name, set<il::record_type*> & visited_records);

	/*
	 * Collects all locations reachable from fields of v, which is
	 * of record type rt. This is just helper function for
	 * collect_reachable_locs.
	 */
	void collect_all_fields(il::record_type* rt, int cur_offset,
			sail::Variable* v, map<sail::Symbol*, string>& sources,
			map<sail::Variable*, pair<bool, string> >& sinks,  bool ret,
			const string& source_name, const string& sink_name,
			set<il::record_type*> & visited_records);

	/*
	 * Can s flow to v, based on type compatibility?
	 */
	bool can_flow(sail::Symbol* s, sail::Variable* v);

	/*
	 * Does this term correspond to a ghost variable we introduced to track flow
	 * propagation?
	 */
	bool is_flow_var(Term* t);


	/*
	 * Gives a satisfying assignment for c consistent with every constraint
	 * in consistency set
	 */
	bool get_sat_assignment(Constraint c, map<Term*, int>& assignment,
			set<Constraint>& consistency_set);

	/*
	 * Returns true if user answers yes, false if user answers no
	 */
	bool get_user_input();

	/*
	 * Returns true if this method is in a namespace that we want to
	 * ask the user about.
	 */
	bool is_system_namespace(il::namespace_context ns);





};

#endif /* TAINTANALYSIS_H_ */
