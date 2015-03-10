/*
 * ErrorSummary.cpp
 *
 *  Created on: Mar 29, 2009
 *      Author: tdillig
 */

#include "AssertionTrace.h"
#include "Error.h"
#include "access-path.h"
#include "ConstraintGenerator.h"
#include <assert.h>
#include <algorithm>
#include "util.h"
#include "IndexVarManager.h"
#include "SummaryGraph.h"
#include "Instantiator.h"
#include "compass_assert.h"
#include "ClientAnalysis.h"

#define DEBUG false
#define REPORT_FULL_PATH_INFO false

#define EXPLAIN_ERROR false



/*
 * Any variable coming from a callee (as supposed to the current function)
 * contains CALLEE_ID.
 */
#define CALLEE_ID "^"




AssertionTrace::AssertionTrace(error_code_type ect, int line, string file,
		Constraint fail_c, int counter_id, Constraint stmt_guard, string msg)
{
	this->line = line;
	this->file = file;
	this->fail_c = fail_c;
	this->fail_c.fresh_id();
	this->counter_id = counter_id;
	this->ect = ect;
	this->tt = ASSERTION_TRACE;
	this->stmt_guard = stmt_guard;
	this->msg = msg;
}

Constraint AssertionTrace::get_stmt_guard()
{
	return stmt_guard;
}
/*
AssertionTrace::AssertionTrace(const AssertionTrace & other, int block_id)
{
	this->line = other.line;
	this->file = other.file;
	this->failure_path = other.failure_path;
	this->fail_c = other.fail_c;
	this->counter_id = counter_id;
}*/

AssertionTrace* AssertionTrace::make(error_code_type ect, int line, string file,
		Constraint fail_c, int counter_id, Constraint stmt_guard, string msg)
{
	return new AssertionTrace(ect, line,  file, fail_c, counter_id, stmt_guard,
			msg);
}


void AssertionTrace::get_access_paths_to_instantiate(set<AccessPath*>& aps)
{
	SummaryGraph::collect_access_paths(fail_c, aps);
}

void AssertionTrace::report_error(vector<Error*> & errors, const Identifier & id)
{
	if(EXPLAIN_ERROR)
	{
		map<Term*, int> costs;
		Constraint fail_c = this->fail_c;
		set<Term*> terms;
		fail_c.get_terms(terms, false);
		for(auto it = terms.begin(); it!= terms.end(); it++){
			Term* t = *it;
			AccessPath* ap = AccessPath::to_ap(t);
			if(!ap->is_imprecise() ||
					ap->to_string().find("@") == string::npos) {
				costs[t] = 100;
				cout << "High cost: " << t->to_string() << endl;
			}
		}

		Constraint bg = Constraint::get_general_background();
		set<Constraint> consistent;
		while(true){
			Constraint assert_c = !fail_c;
			if(bg.implies(assert_c)) break;
			cout << "FINDING EXPLANATION FOR: " << assert_c.to_string() << endl;
			cout << "BG: " << bg.to_string() << endl;
			Constraint expl = assert_c.abduce(bg, consistent, costs);
			cout << "Abductive explanation: " << expl << endl;
			if(expl.unsat()) {
				Error* e = new Error(line,file, ect, to_error(), id);
				errors.push_back(e);
				return;
			}

			cout << "Open another instance of Compass and add the assertion "
					 << expl << " into the closest loop" << endl;
			cout << "Can you verify the assertion? " << endl;

			char answer;
			cin >> answer;
			if(answer == 'y') break;
			consistent.insert(!expl);


		}


		return;
	}


	if(this->ect == ERROR_CLIENT_ANALYSIS)
	{
		if(ClientAnalysis::report_error(to_error(), fail_c) == false)
			return;
	}
	Error* e = new Error(line,file, ect, to_error(), id);
	errors.push_back(e);
}



void AssertionTrace::instantiate_trace(Instantiator& inst,
		set<ErrorTrace*>& inst_traces, bool top_level_function)
{

	cout << "INSTANTIATING TRACE: " << this->to_string() << endl;

	Constraint inst_fail_c = inst.get_instantiation(fail_c);
	inst_fail_c &= inst.get_stmt_guard();

	cout << "INST FAIL C: "<< inst_fail_c << endl;

	IndexVarManager::eliminate_sum_index_vars(inst_fail_c);

	if(inst_fail_c.unsat()) return;

	Constraint inst_stmt = inst.get_instantiation(stmt_guard);



	AssertionTrace* new_at = AssertionTrace::make(ect, line, file,
			inst_fail_c, inst.get_counter_id(), inst_stmt, msg);
	new_at->callstack = callstack;
	if(inst.is_new_calling_context())
		new_at->callstack.push_back(inst.get_calling_context());
	inst_traces.insert(new_at);






}

ErrorTrace* AssertionTrace::replace(AccessPath* ap, AccessPath* replacement)
{
	fail_c.replace_term(ap->to_term(), replacement->to_term());
	return this;
}

string AssertionTrace::witness_to_string()
{
	return callstack_to_string(callstack);
	/*
	set<pair<string, string> > assignments;
	int id = find_witnessing_failure_path(assignments);
	if(id == -1) return "";
	string res = "Callstack:\n";
	callstack_t & cs = failure_path[id].first;
	for(unsigned int i=0; i < cs.size(); i++) {
		callstack_entry& ce = cs[i];
		string fn = ce.fn_name;
		string trace = filter_assignments_by_function(assignments, fn,
				i==cs.size()-1);
		res+=ce.to_string();
		if(trace != "") res += " (" + trace +")\n";
		else res += "\n";
	} */

/*	res += "Constraint: " + failure_paths[id].second.error_c.to_string() + "\n";
	res+="Trace: \n";
	set<pair<string, string> >::iterator it2 = assignments.begin();
	for(; it2!= assignments.end(); it2++)
	{
		res += "\t" + it2->first + ": " + it2->second + "\n";
	}

*/
	//return res;
}

/*
int ErrorTrace::find_witnessing_failure_path(set<pair<string, string> >&
		assignments)
{
	for(unsigned int i=0; i<failure_path.size(); i++)
	{
		Constraint ec = failure_path[i].second;
		bool sat = ec.get_assignment(assignments);
		if(sat) {
			return i;
		}
	}
	return -1;
}
*/
/*
 * The access paths from the callee mentioned in this error trace
 * that need to be translated.
 */
/*
void AssertionTrace::get_aps_to_instantiate(set<AccessPath*>& aps)
{
	set<AccessPath*> all_aps;
	for(unsigned int i=0;i<failure_paths.size(); i++)
	{
		error_cond c = failure_paths[i].second;
		ap_constraint_t ap_c = c.error_c_ap;
		AccessPath* nc_ap = ap_c.first;
		AccessPath* sc_ap = ap_c.second;
		assert(nc_ap != NULL && sc_ap != NULL);
		nc_ap->get_nested_memory_aps(all_aps);
		sc_ap->get_nested_memory_aps(all_aps);

	}

	set<AccessPath*>::iterator it = all_aps.begin();
	for(; it!= all_aps.end(); it++)
	{
		AccessPath* cur = *it;
		set<string> vars;
		cur->get_nested_varnames(vars);
		if(vars.size() == 0) continue;
		bool instantiate = true;
		set<string>::iterator it2 = vars.begin();
		for(; it2!=vars.end(); it2++) {
			string cur = *it2;
			if(cur.find(CALLEE_ID) != string::npos){
				instantiate = false;
				break;
			}
		}
		if(!instantiate) continue;
		aps.insert(cur);
	}
}

*/
/*
 * renaming map maps each access path in to_rename to another
 * access path where all nested variable names are appended with
 * fn_name.
 */
void AssertionTrace::build_renaming_map(set<AccessPath*>& to_rename, string& fn_name,
		map<AccessPath*, AccessPath*>& renaming_map)
{
	/*set<AccessPath*>::iterator it = to_rename.begin();
	for(; it!= to_rename.end(); it++)
	{
		AccessPath* ap = *it;
		set<AccessPath*> vars;
		ap->get_nested_variables(vars);

		set<AccessPath*>::iterator it2= vars.begin();
		map<AccessPath*, AccessPath*> renaming;
		for(; it2!= vars.end(); it2++)
		{
			AccessPath* cur = *it2;

			// Allocs are already prepended with their function name
			if(cur->get_ap_type() != AP_VARIABLE) continue;
			Variable* cur_var = (Variable*) cur;
			string var_name = cur_var->get_name();
			string new_name = var_name + CALLEE_ID + fn_name;
			Variable* new_var = Variable::make_variable(new_name,
					cur_var->get_type(), cur_var->is_unmodeled_value());
			renaming[cur_var] = new_var;
		}

		AccessPath* new_ap = ap->apply_replacements(renaming);
		renaming_map[ap] = new_ap;
	}*/
}

Constraint AssertionTrace::get_guarded_value_constraint(AccessPath* ap,
		map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
		instantiation_map,
		map<AccessPath*, AccessPath*>& renaming_map)
{
	Constraint res(false);
	set<pair<AccessPath*, Constraint> >&  inst_set = instantiation_map[ap];
	set<pair<AccessPath*, Constraint> >::iterator it = inst_set.begin();
	for(; it!= inst_set.end(); it++)
	{
		AccessPath* cur_inst = it->first;
		Constraint cur_constraint = it->second;
		c_assert(renaming_map.count(ap) > 0);
		Constraint eq_c = ConstraintGenerator::get_eq_constraint(
				renaming_map[ap], cur_inst);
		Constraint cur_inst_c = cur_constraint & eq_c;
		res |= cur_inst_c;
	}
	return res;

}

string AssertionTrace::filter_assignments_by_function(set<pair<string, string>  >&
		assignments, const string & fn_name, bool last)
{
	string res = "";
	set<pair<string, string> >::iterator it = assignments.begin();
	int i=0;
	for(;it!= assignments.end(); it++, i++)
	{
		string var_name = it->first;
		string fun_id = CALLEE_ID + fn_name;
		if(last && (var_name.find(CALLEE_ID)!=string::npos)) continue;
		if(!last)
		{
			int pos = var_name.find(fn_name);
			if(pos == string::npos) continue;
			int end_pos = pos+fn_name.size();
			if(end_pos != var_name.size()) {
				char x = var_name[end_pos];
				if(x != ')' && x!=',' && x!=':') continue;
			}
			int callee_id_pos =var_name.find(CALLEE_ID);

			var_name = var_name.replace(callee_id_pos, 1+fn_name.size(), "");
		}

		res += var_name + "=" + it->second;
		res += ", ";
	}
	return res.substr(0, res.size()-2);
}

ErrorTrace* AssertionTrace::set_counter(int counter_id)
{
	return this;
}

Constraint AssertionTrace::get_instantiation_constraint(AccessPath* ap,
			set<AccessPath*>& aps_to_rename,
			map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
			instantiation_map,
			map<AccessPath*, AccessPath*>& renaming_map)
{
	set<AccessPath*> nested_aps;
	ap->get_nested_memory_aps(nested_aps);
	set<AccessPath*> intersection;
	set_intersection(nested_aps.begin(), nested_aps.end(),
			aps_to_rename.begin(), aps_to_rename.end(),
			insert_iterator<set<AccessPath*> >(intersection,
					intersection.begin()));

	ap = ap->replace(renaming_map);
	Constraint new_c(ap);

	set<AccessPath*>::iterator it = intersection.begin();
	for(; it!= intersection.end(); it++)
	{
		AccessPath* cur = *it;

		Constraint cur_inst_c = get_guarded_value_constraint(cur,
				instantiation_map, renaming_map);
		new_c &= cur_inst_c;
	}
	IndexVarManager::eliminate_fresh_vars(new_c);
	return new_c;

}

/*
 * Pushes the given function on top of the callstack.
 * Uses the instantiation map to add appropriate instantiation constraints.
 * For instance, if x maps to a under flag and b under
 * !flag, all the callstack entries will and the constraint
 * (x = a & flag) | (x=b & !flag)
 */
void AssertionTrace::push_caller(string fn_name, string file, int line,
		map<AccessPath*, set<pair<AccessPath*, Constraint> > >&
		instantiation_map)
{
	return;
	/*set<AccessPath*> aps_to_rename;
	map<AccessPath*, set<pair<AccessPath*, Constraint> > >::iterator it =
		instantiation_map.begin();
	for(; it!= instantiation_map.end(); it++) {
		aps_to_rename.insert(it->first);
	}

	callstack_entry cur_caller(fn_name, file, line);
	for(unsigned int i=0; i<failure_paths.size(); i++)
	{
		callstack_t& cur_callstack = failure_paths[i].first;
		error_cond& cur_cond = failure_paths[i].second;
		string prev_fun = cur_callstack[cur_callstack.size()-1].fn_name;
		map<AccessPath*, AccessPath*> renaming_map;
		build_renaming_map(aps_to_rename, prev_fun, renaming_map);
		cur_callstack.push_back(cur_caller);
		AccessPath* nc_ap = cur_cond.error_c_ap.first;
		assert(nc_ap != NULL);
		Constraint new_nc = get_instantiation_constraint(nc_ap,
				aps_to_rename, instantiation_map, renaming_map);
		AccessPath* sc_ap = cur_cond.error_c_ap.second;
		assert(sc_ap != NULL);
		Constraint new_sc = get_instantiation_constraint(sc_ap,
				aps_to_rename, instantiation_map, renaming_map);
		Constraint new_c(new_nc, new_sc);
		cur_cond.error_c = new_c;
		cur_cond.error_c_ap = pair<AccessPath*, AccessPath*>((AccessPath*)NULL,
				(AccessPath*)NULL);




	}*/
}

string AssertionTrace::to_error()
{
	string res;
	if(msg == "") res = string(error_names[ERROR_STATIC_ASSERT]);
	res += msg;
			//+ " at " + file+ ":" + int_to_string(line);
	if(callstack.size() > 0) res += "\nCallstack: \n" +
			callstack_to_string(callstack);
	return res;
}

string AssertionTrace::to_string()
{
	string res = "Assertion trace: " + file+ ":" + int_to_string(line);
	res += " / " + fail_c.to_string();
	return res;
}

string AssertionTrace::get_id()
{
	return "line " + int_to_string(line);
}

AssertionTrace::~AssertionTrace()
{

}
