/*
 * ErrorSummary.h
 *
 *  Created on: Mar 29, 2009
 *      Author: tdillig
 */

#ifndef ASSERTIONTRACE_H_
#define ASSERTIONTRACE_H_

#include <string>
#include <map>
#include <set>
#include "Constraint.h"
#include "Error.h"
#include "ErrorTrace.h"
using namespace std;

class AccessPath;
class AssertionTrace: public ErrorTrace {

private:
	int line;
	string file;
	callstack_t callstack;
	error_code_type ect;
	Constraint stmt_guard;
	string msg;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<ErrorTrace>(*this);
		ar & line;
		ar & file;
		ar & callstack;
		ar & ect;
		ar & stmt_guard;
		ar & msg;
	}


protected:
	AssertionTrace(){};
	AssertionTrace(error_code_type ect, int line, string file,
			Constraint fail_c, int counter_id, Constraint stmt_guard,
			string msg = "");
	//AssertionTrace(const AssertionTrace & other, int block_id);

public:
	static AssertionTrace* make(error_code_type ect, int line, string file,
			Constraint fail_c, int counter_id, Constraint stmt_guard,
			string msg = "");

	virtual void report_error(vector<Error*> & errors, const Identifier & id);
	virtual string to_string();
	virtual string to_error();
	virtual string get_id();
	virtual int get_line() {return line;};
	virtual ErrorTrace* replace(AccessPath* ap, AccessPath* replacement);

	virtual void instantiate_trace(Instantiator& inst,
			set<ErrorTrace*>& inst_traces, bool top_level_function);

	virtual void get_access_paths_to_instantiate(set<AccessPath*>& aps);



	/*
	 * Pushes the given function on top of the callstack.
	 * Uses the instantiation map to add appropriate instantiation constraints.
	 * For instance, if x maps to a under flag and b under
	 * !flag, all the callstack entries will and the constraint
	 * (x = a & flag) | (x=b & !flag)
	 */
	void push_caller(string fn_name, string file, int line,
			map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
			instantiation_map);

	virtual ErrorTrace* set_counter(int counter_id);

	Constraint get_stmt_guard();





	virtual ~AssertionTrace();

private:
	/*
	 * renaming map maps each access path in to_rename to another
	 * access path where all nested variable names are appended with
	 * fn_name.
	 */
	void build_renaming_map(set<AccessPath*>& to_rename, string& fn_name,
			map<AccessPath*, AccessPath*>& renaming_map);

	Constraint get_guarded_value_constraint(AccessPath* ap,
			map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
			instantiation_map, map<AccessPath*, AccessPath*>& renaming_map);

	Constraint get_instantiation_constraint(AccessPath* ap,
			set<AccessPath*>& aps_to_rename,
			map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
			instantiation_map,
			map<AccessPath*, AccessPath*>& renaming_map);

	//int find_witnessing_failure_path(set<pair<string, string> >& assignments);
	string witness_to_string();
	string filter_assignments_by_function(set<pair<string, string>  >& assignments,
			const string & fn_name, bool last);



};

#endif /* ERRORTRACE_H_ */
