/*
 * TaintAnalysis.cpp
 *
 *  Created on: Sep 1, 2012
 *      Author: tdillig
 */

#include "TaintAnalysis.h"
#include "sail.h"
#include "ConstraintGenerator.h"
#include "access-path.h"
#include "Callgraph.h"

#define FLOW_PREFIX ("@flow_")

TaintAnalysis::TaintAnalysis() {


}


string TaintAnalysis::get_property_name()
{
	return "taint";
}


il::type* TaintAnalysis::get_property_type()
{
	return il::get_integer_type();
}


sail::Symbol* TaintAnalysis::get_initial_value()
{
	sail::Symbol* zero = new sail::Constant(0, true, 32);
	return  zero;
}

bool TaintAnalysis::is_tracked_type(il::type* t)
{
	return true;
}

TaintAnalysis::~TaintAnalysis() {

}



void TaintAnalysis::process_function_call_after(sail::FunctionCall* inst,
		bool known)
{




	/*
	 * Marks return value as tainted
	 */
	if(inst->get_function_name()== "taint")  {
		sail::Variable* ret_var = inst->get_return_variable();
		if(ret_var == NULL) return;
		sail::Symbol* one = new sail::Constant(1, true, 32);
		Constraint c;
		write_state(ret_var, one, c);
	}

	/*
	 * Marks dereference of first argument as tainted
	 */
	else if(inst->get_function_name()== "taint_drf")  {
		vector<sail::Symbol*> * args = inst->get_arguments();
		if(args->size() == 0) return;
		sail::Symbol* s = (*args)[0];
		if(!s->is_variable()) return;
		sail::Variable* v = static_cast<sail::Variable*>(s);
		sail::Variable* t = load(v, 0);
		sail::Symbol* one = new sail::Constant(1, true, 32);
		Constraint c;
		write_state(t, one, c);

	}

	/*
	 * Marks  second element of first argument as tainted
	 */
	else if(inst->get_function_name()== "taint_array")  {
		vector<sail::Symbol*> * args = inst->get_arguments();
		if(args->size() == 0) return;
		sail::Symbol* s = (*args)[0];
		if(!s->is_variable()) return;
		sail::Variable* v = static_cast<sail::Variable*>(s);
		sail::Symbol* two = new sail::Constant(2, true, 32);
		sail::Variable* t = load(v, two);
		sail::Symbol* one = new sail::Constant(1, true, 32);
		Constraint c;
		write_state(t, one, c);

	}




	/*
	 * Checks if first argument is tainted
	 */
	else if(inst->get_function_name()== "leak")  {
		vector<sail::Symbol*> * args = inst->get_arguments();
		if(args->size() == 0) return;
		sail::Symbol* s = (*args)[0];
		if(!s->is_variable()) return;
		sail::Variable* v = static_cast<sail::Variable*>(s);
		sail::Variable* res = read_state(v);
		cout << "TAINTED VALUE: " << res->to_string() << endl;
		Constraint neqz_c = get_neqz_constraint(res);
		assert_state(neqz_c, "Tainted value leaked");

	}
	/*
	 * Checks if aything reachable from first argument is tainted
	 */
	else if(inst->get_function_name()== "leak_all")  {
		vector<sail::Symbol*> * args = inst->get_arguments();
		if(args->size() == 0) return;
		sail::Symbol* s = (*args)[0];
		if(!s->is_variable()) return;
		sail::Variable* v = static_cast<sail::Variable*>(s);
		process_leak_all(v);




	}

	else if (!known)
	{
		process_library_call(inst);
	}

}

void TaintAnalysis::process_leak_all(sail::Variable *v)
{
	set<il::record_type*> visited_records;
	map<sail::Symbol*, string> sources;
	map<sail::Variable*, pair<bool, string> > sinks;
	collect_reachable_locs(v, v->get_type(), sources, sinks, true, false, "arg0",
					"arg0", visited_records);

	for(auto it = sources.begin(); it!=sources.end(); it++) {
		sail::Symbol* s = it->first;
		if(!s->is_variable()) continue;
		sail::Variable* cur_var = static_cast<sail::Variable*>(s);
		sail::Variable* res = read_state(cur_var);
		cout << "TAINTED VALUE: " << res->to_string() << endl;
		Constraint neqz_c = get_neqz_constraint(res);
		string error = "Tainted value leaked " + it->second;
		assert_state(neqz_c, error);

	}
}


bool begins_with(const string & s1, const string & s2)
{
	if(s2.size()> s1.size()) return false;
	string begin = s1.substr(0, s2.size());
	return begin == s2;
}

bool TaintAnalysis::is_system_namespace(il::namespace_context ns)
{
	string s = ns.to_string();
	if(begins_with(s, "Ljava")) return true;
	if(begins_with(s, "Landroid")) return true;
	if(begins_with(s, "Lorg/apache/")) return true;
	return false;
}



void TaintAnalysis::process_library_call(sail::FunctionCall* inst)
{
	string fn_name = inst->get_function_name();


	if(!is_system_namespace(inst->get_namespace())){
		return;
	}




	cout << "&&&&&&&&&&&&&&& fn name: " << fn_name  << endl;

	/*
	 * All potential sources/sinks. Key is a variable representing the source
	 * and string is its "access path" encoding for error reporting.
	 */
	map<sail::Symbol*, string> sources;
	map<sail::Variable*, pair<bool, string> > sinks;

	vector<sail::Symbol*>* args = inst->get_arguments();
	il::type* _fn_t = inst->get_signature();
	assert(_fn_t->is_function_type());
	il::function_type* fn_t = static_cast<il::function_type*>(_fn_t);
	cout << "sig: " << fn_t->to_string() << endl;
	cout << "arg size: " << args->size() << endl;


	for(unsigned int i=0; i<args->size(); i++) {
		sail::Symbol* s = (*args)[i];
		string source_name = "arg"  + int_to_string(i);
		string sink_name = "arg"  + int_to_string(i);
		sources[s] = source_name;
		if(s->is_constant()) continue;
		sail::Variable* v = static_cast<sail::Variable*>(s);
		set<il::record_type*> visited_records;
		il::type* cur_formal_t = v->get_type();
		if(i < fn_t->get_arg_types().size())
			cur_formal_t = fn_t->get_arg_types()[i];
		collect_reachable_locs(v, cur_formal_t, sources, sinks, false, false, source_name,
				sink_name, visited_records);
	}

	sail::Variable* ret = inst->get_return_variable();
	if(ret != NULL) {
		string source_name = "ret";
		string sink_name = "ret";
		set<il::record_type*> visited_records;
		collect_reachable_locs(ret, fn_t->get_return_type(),
				sources, sinks, true, true, source_name,
				sink_name, visited_records);
	}


	cout << "=============POTENTIAL SOURCES+++++++++++++" << endl;
	for(auto it = sources.begin(); it!= sources.end(); it++){
		sail::Symbol* s = it->first;
		cout << "\t" << s->to_string() << ":" << it->second << " -> "
				<< *s->get_type() << endl;
	}

	cout << "=============POTENTIAL SINKS+++++++++++++" << endl;
	for(auto it = sinks.begin(); it!= sinks.end(); it++){
		sail::Symbol* s = it->first;
		cout << "\t" << s->to_string() << ":" << it->second.second << "->" <<
				*s->get_type()  << endl;
	}

	map<sail::Variable*, set<sail::Symbol*> > potential_flows;

	/*
	 * Build a map from each potential sink to each potential source
	 */
	for(auto it = sinks.begin(); it!= sinks.end(); it++) {
		sail::Variable* v = it->first;
		for(auto it2 = sources.begin(); it2!= sources.end(); it2++) {
			sail::Symbol* s = it2->first;
			if(s->is_variable()){
				sail::Variable* source_var = static_cast<sail::Variable*>(s);
				if(*source_var == *v) continue;
			}
			if(can_flow(s, v)) {
				potential_flows[v].insert(s);
			}

		}

	}
	cout << "$$$$$$$flows\t" << potential_flows.size() << endl;
	cout << "================POTENTIAL FLOWS=================" << endl;
	for(auto it = potential_flows.begin(); it!= potential_flows.end(); it++)
	{
		sail::Variable* v = it->first;
		set<sail::Symbol*>&s = it->second;
		cout << v->to_string() << " <-- ";
		for(auto it = s.begin(); it!= s.end(); it++)
		{
			cout << (*it)->to_string() <<":" << (*it)->get_type()->to_string() <<  "\t";

		}
		cout << endl;
	}


	//cout << ">>>>>>>>>>>>>>>WRITING TAINT CONDITIONS <<<<<<<<<<<<" << endl;

	int c = 0;
	for(auto it = potential_flows.begin(); it!= potential_flows.end(); it++)
	{
		sail::Variable* v = it->first;
		Constraint taint_c(false);
		bool is_ret = sinks[v].first;
		string sink_name = sinks[v].second;

	//	cout << "SINK: " << v->to_string() << ":" << sink_name << endl;

		if(!is_ret) {
			sail::Variable* old_taint = read_state(v);
			Constraint old_tainted = get_neqz_constraint(old_taint);
			taint_c |= old_tainted;
		}

		set<sail::Symbol*>& s = it->second;
		for(auto it = s.begin(); it!= s.end(); it++)
		{
			sail::Symbol* source = *it;
			string source_name = sources[source];
			//cout << "SOURCE: " << source->to_string() << ":" << source_name << endl;

			sail::Variable* source_taint = read_state(source);
			Constraint source_tainted = get_neqz_constraint(source_taint);
			string name = FLOW_PREFIX + fn_name+ "_" + source_name + "_" +
					sink_name;

			sail::Variable* unknown = get_unknown_value(il::get_integer_type(),
					 name);
			Constraint flow_c = get_neqz_constraint(unknown);
			//cout << "SOURCE TAINTED C: " << source_tainted << endl;
			//cout << "FLOW C: " << flow_c << endl;
			taint_c |= (flow_c & source_tainted);
		}

		//cout << "TAINT  1111 CONDITION FOR " << v->to_string() << ": " << taint_c << endl;

		taint_c.sat();

		//cout << "TAINT CONDITION FOR " << v->to_string() << ": " << taint_c << endl;

		sail::Symbol* one = new sail::Constant(1, true, 32);
		Constraint final_taint_c(taint_c, false);
		write_state(v, one, final_taint_c);

	}

}


bool TaintAnalysis::can_flow(sail::Symbol* s, sail::Variable* v)
{
	il::type* source_type = s->get_type();
	il::type* sink_type = v->get_type();

	//if(sink_type->to_string().find("int")!=string::npos) return false;
	//if(sink_type->to_string().find("ContactAdder")!=string::npos) return false;
	//if(sink_type->to_string().find("ArrayList")!=string::npos) return false;
	//if(source_type->to_string().find("ArrayList")!=string::npos) return false;
	//if(sink_type->to_string().find("widget")!=string::npos) return false;

	//if(sink_type->to_string().find("widget")!=string::npos) return false;

	/*if(sink_type->to_string().find("widget")!=string::npos) return false;
	if(sink_type->to_string().find("int")!=string::npos) return false;
	if(sink_type->to_string().find("ArrayList")!=string::npos) return false;
	if(sink_type->to_string().find("ContactAdder")!=string::npos) return false;
	*/
	if(source_type == sink_type){
		//cout << "********* FLOW TYPE: " << source_type->to_string() << endl;
		return true;
	}


	//delete me
	//return false;
	//delete me end


	if(!source_type->is_pointer_type()) return false;
	if(!sink_type->is_pointer_type()) return false;

	il::type* source_inner = source_type->get_inner_type();
	il::type* sink_inner = sink_type->get_inner_type();

	if(!source_inner->is_record_type()) return false;
	if(!sink_inner->is_record_type()) return false;

	il::record_type* source_rt = static_cast<il::record_type*>(source_inner);
	il::record_type* sink_rt = static_cast<il::record_type*>(sink_inner);

	return (source_rt->is_subtype_of(sink_rt));
}


void TaintAnalysis::collect_reachable_locs(sail::Variable* v, il::type* t,
		map<sail::Symbol*, string>& sources,
		map<sail::Variable*, pair<bool, string> >& sinks,
		bool insert_v, bool ret, const string & source_name,
		const string & sink_name,
		set<il::record_type*> & visited_records)
{


	if(insert_v) {
		if(!ret) sources[v] = source_name;
		sinks[v] = make_pair(ret, sink_name);
	}


	if(!t->is_pointer_type()) return;

	il::pointer_type* pt = static_cast<il::pointer_type*>(t);
	il::type* inner_t = pt->get_inner_type();

	string new_source_name = "(*" + source_name + ")";
	string new_sink_name = "(*" + sink_name + ")";

	if(!inner_t->is_record_type()){
		sail::Variable* new_var = load(v, 0);
		collect_reachable_locs(new_var, inner_t, sources, sinks, true, ret,
				new_source_name, new_sink_name, visited_records);
	}

	else {
		il::record_type* rt = static_cast<il::record_type*>(inner_t);
		collect_all_fields(rt, 0, v, sources, sinks, ret, new_source_name, new_sink_name,
				visited_records);


	}




}

void TaintAnalysis::collect_all_fields(il::record_type* rt, int cur_offset,
		sail::Variable* v, map<sail::Symbol*, string>& sources,
		map<sail::Variable*, pair<bool, string> >& sinks,  bool ret,
		const string& source_name, const string& sink_name,
		set<il::record_type*> & visited_records)
{
	//cout << "CURRENT RECORD TYPE: " << rt->to_string() << endl;


	if(visited_records.count(rt) > 0) return;
	visited_records.insert(rt);
	vector<il::record_info*>& fields = rt->get_fields();
	cout << "_____ record type: " << rt->to_string() << endl;
	for(unsigned int i=0; i<fields.size(); i++) {
		il::record_info* ri = fields[i];
		int offset = ri->offset;
		cout << "---> field: " << ri->fname << endl;
		string new_source_name = source_name + "." + ri->fname;
			string new_sink_name = sink_name + "." + ri->fname;
		if(ri->t->is_record_type()) {


			il::record_type* inner_rt = static_cast<il::record_type*>(ri->t);
			collect_all_fields(inner_rt, cur_offset+ offset, v, sources,
				sinks, ret, new_source_name, new_sink_name, visited_records);

		}
		else
		{
		//	cout << "FIELD: " << ri->fname << endl;
			//cout << "TYPE:" << *ri->t << endl;
			sail::Variable* new_var = load(v, cur_offset+offset);

			collect_reachable_locs(new_var, ri->t,
					sources, sinks, true, ret,
					new_source_name, new_sink_name, visited_records);
		}
	}
}





bool  TaintAnalysis::process_error(const string & msg, Constraint fail_c)
{
	cout << "failure condition: " << fail_c << endl;
	Constraint success_c = (!fail_c).sc();

	/*
	 * Universally quantify over inputs since we only want to generate queries
	 * about flow propagation behavior of library functions
	 */
	set<Term*> all_terms;
	success_c.get_terms(all_terms, false);

	for(auto it = all_terms.begin(); it!= all_terms.end(); it++)
	{
		Term* t = *it;
		if(!is_flow_var(t)) {
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap->is_constant()) continue;
			cout << "Eliminating ap: " << ap->to_string() << endl;
			ConstraintGenerator::eliminate_uvar(success_c, ap);
		}
	}

	cout << "Constraint after eliminating input vars: " << success_c << endl;

	/*
	 * If it turns unsat after eliminating variables, report error
	 */
	if(success_c.unsat()) return true;

	set<Constraint> consistency_set;
	Constraint known_inv(true);
	map<Term*, int> costs;

	while(true) {

		Constraint sol = success_c.abduce(known_inv, consistency_set, costs);
		if(sol.unsat()) return true;
		if(sol.valid()) return false;
		map<Term*, int> assignment;
		set<Constraint> consistency = consistency_set;
		consistency.insert(known_inv);
		bool res = get_sat_assignment(sol, assignment, consistency_set);
		if(res == false) return true;

		for(auto it = assignment.begin(); it!= assignment.end(); it++)
		{

			Term* cur_t = it->first;
			int value = it->second;
			if(value == 0) {
				cout << "Is " << cur_t->to_string() << " always false?" << endl;
			}
			else {
				cout << "May " << cur_t->to_string() << " be true?" << endl;
			}

			bool answer = get_user_input();

			Constraint inv;
			Constraint new_consistency;
			ConstantValue* zero = ConstantValue::make(0);
			if(value == 0) {
				if(answer) {
					inv = Constraint(cur_t, zero, ATOM_EQ);
				}
				else {
					new_consistency = Constraint(cur_t, zero, ATOM_NEQ);
				}
			}
			else {
				if(answer) {
					new_consistency =  Constraint(cur_t, zero, ATOM_NEQ);
				}
				else {
					inv = Constraint(cur_t, zero, ATOM_EQ);
				}
			}


			known_inv &= inv;
			if(!new_consistency.valid()) {
				consistency_set.insert(new_consistency);
			}

			if(!answer) break;




		}


	}




	return true;
}

bool TaintAnalysis::get_user_input()
{

	while(true) {
		string answer;
		std::getline(cin, answer);
		if(answer == "yes") {
			return true;
		}
		else if(answer == "no") {
			return false;
		}
		else{
			cout << "Please answer yes or no" << endl;

		}
	}
}



bool TaintAnalysis::is_flow_var(Term* t)
{
	AccessPath* ap = AccessPath::to_ap(t);
	if(ap->get_ap_type() != AP_VARIABLE) return false;
	Variable* v = static_cast<Variable*>(ap);
	if(v->get_name().find(FLOW_PREFIX)!= string::npos) return true;
	return false;
}

bool TaintAnalysis::get_sat_assignment(Constraint c,
		map<Term*, int>& assignment, set<Constraint>& consistency_set)
{
	set<Term*> all_vars;
	c.get_free_variables(all_vars);

	Constraint conjunct = c;

	for(auto it = consistency_set.begin(); it!= consistency_set.end(); it++)
	{
		Constraint cur = *it;
		set<Term*> cur_vars;
		cur.get_free_variables(cur_vars);
		for(auto it = cur_vars.begin(); it!= cur_vars.end(); it++) {
			Term* t = *it;
			if(all_vars.count(t) == 0) {
				cur.eliminate_evar(static_cast<VariableTerm*>(t));
			}
		}
		conjunct &= cur;

	}

	map<Term*, SatValue> sat_assign;
	bool res = conjunct.get_assignment(sat_assign);

	for(auto it = sat_assign.begin(); it!= sat_assign.end(); it++)
	{

		Term* t = it->first;
		SatValue s = it->second;
		assert(s.integer);
		assignment[t] = s.value.to_int();
	}
	return res;

}




