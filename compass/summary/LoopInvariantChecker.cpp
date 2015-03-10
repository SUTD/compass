/*
 * LoopInvariantChecker.cpp
 *
 *  Created on: May 6, 2012
 *      Author: tdillig
 */

#include "LoopInvariantChecker.h"
#include "SummaryGraph.h"
#include "Constraint.h"
#include "AssertionTrace.h"
#include "access-path.h"
#include "util.h"
#include "ConstraintGenerator.h"
#include "MemoryGraph.h"
#include "SummaryUnit.h"
#include "Identifier.h"


LoopInvariantChecker::LoopInvariantChecker(SummaryGraph& sg,
		MemoryGraph* mg):sg(sg)
{
	Constraint::disable_background();
	this->mg = mg;
	discharge_assertions();

}

LoopInvariantChecker::~LoopInvariantChecker() {

}


Constraint LoopInvariantChecker::replace_vars_with_values(Constraint c)
{
	set<Term*> vars;
	c.get_free_variables(vars);
	set<Term*>::iterator it = vars.begin();
	int counter = 0;
	set<VariableTerm*> to_eliminate;
	Constraint eqs;
	for(; it != vars.end(); it++) {
		Term* t = *it;
	//	cout << "term: " << t->to_string() << endl;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type()!=AP_VARIABLE) continue;
		set<pair<AccessPath*, Constraint> > values;
		mg->get_value_set(ap, values, sg.loop_continuation_cond);
		if(values.size() == 0) continue;
		Variable* temp = Variable::make_temp("elim_" + int_to_string(counter++));
		Constraint eq_c(false);
		for(set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
				it != values.end(); it++) {
			AccessPath* val = it->first;
			Constraint c = it->second;
			Constraint eq = ConstraintGenerator::get_eq_constraint(val, temp);
			//Constraint val_c = (!c) | eq;
			Constraint val_c = c & eq;
			//eq_c &= val_c;
			eq_c |= val_c;
		}

		eq_c.assume(sg.loop_continuation_cond);
		cout << "VALUE SET FOR: " << ap->to_string() << " is: " << eq_c << endl;
		c.replace_term(ap->to_term(), temp->to_term());
		cout << "c replaced " << c << endl;
		cout << "eq_c " << eq_c << endl;
		IndexVarManager::eliminate_source_vars(eq_c);
		//c &= eq_c;
		eqs &= eq_c;
	//	cout << "&&: " << c << endl;
		to_eliminate.insert(static_cast<VariableTerm*>(temp->to_term()));
	}
	c&= eqs;
	cout << "COnstraint before elim: " << c << endl;
	c.eliminate_evars(to_eliminate);
	cout << "Constraint after elim: " << c << endl;
	return c;
}


void LoopInvariantChecker::discharge_assertions()
{
	set<ErrorTrace*> traces = mg->error_traces;
	mg->error_traces.clear();

	set<ErrorTrace*>::iterator it = traces.begin();
	for(; it !=traces.end(); it++) {
		ErrorTrace* et = *it;
		if(et->get_trace_type() == ASSERTION_TRACE)
			discharge_assertion(static_cast<AssertionTrace*>(et));
	}


}

bool LoopInvariantChecker::query_precondition(Constraint query)
{
	cout << "Open another instance of Compass and add the assertion " << query <<
			" before the loop  " << sg.su->get_identifier().to_string()
			<< endl;
	cout << "Can you show the assertion?" << endl;
	char answer;
	cin >> answer;
	if(answer != 'y') return false;
	cout << "Add assumption " << query << " at the beginning of the loop " <<
			 sg.su->get_identifier().to_string() << endl;
	return true;
}

bool LoopInvariantChecker::query_client(Constraint query,
		Constraint assumption)
{
	cout << "Can any analysis show that " << query <<
			" is loop invariant assuming " <<
			  assumption << " is a loop invariant?" << endl;
	char i;
	cin >> i;
	if(i == 'y') return true;
	return false;
}

bool LoopInvariantChecker::prove_invariant(Constraint to_show,
		Constraint bg, set<Constraint> consistency, Constraint loop_cont_c)
{


	/*
	 * Compute the weakest precondition of to_show
	 */
	Constraint pre_c = replace_vars_with_values(to_show);
	Constraint lhs = to_show & bg & loop_cont_c;


	set<AccessPath*> ignore;
	ConstraintGenerator::eliminate_unobservables(lhs, true, ignore, true);
	lhs = lhs.nc();

	cout << "OBSERVED C: " << this->mg->observed_c << endl;
	Constraint o_c = this->mg->observed_c;
	o_c.assume(loop_cont_c);
	cout << "OBSERVED C 2: " << o_c << endl;
	consistency.insert(o_c);
	map<Term*, int> costs;
	while(true) {
		cout << "TO SHOW: " << to_show << endl;
		cout << "PERFORMING ABDUCTION: " << endl;
		cout << "Want to show: " << pre_c <<  " LHS: " << lhs << endl;

		if(lhs.implies(pre_c)) return true;

		Constraint abduction = pre_c.abduce(lhs, consistency, costs);
		if(abduction.unsat()) {
			return false;
		}

		Constraint query = abduction;
		query.assume(to_show);
		query.assume(bg);

		if(!query_precondition(query)) {
			consistency.insert(!abduction);
			continue;
		}


		/*
		 * Ask client analyses
		 */
		bool answer = query_client(query, to_show & bg);
		if(answer) return true;

		/*
		 * If client can't show query, make recursive call to
		 * prove the query assuming to_show holds
		 */
		Constraint new_bg = bg & to_show;
		answer = prove_invariant(query, new_bg, consistency, loop_cont_c);
		if(answer) return true;


		cout << "Inserting into consistency: " << !abduction << endl;
		consistency.insert(!abduction);
	}



}


void LoopInvariantChecker::discharge_assertion(AssertionTrace* at)
{
	set<Constraint> cc_set;

	Constraint bg = sg.return_cond;
	bg.assume(sg.loop_continuation_cond);



	Constraint assert_c = !at->get_failure_cond();
	assert_c.assume(sg.loop_continuation_cond);

	//bool res = query_precondition(assert_c);

	at->set_failure_cond(!assert_c);
	mg->error_traces.insert(at);



	//if(res)
	bool	res = prove_invariant(assert_c, bg, cc_set, sg.loop_continuation_cond);

	if(res) {
		cout << "******Assertion discharged*******" << endl;
	}
	else
	{
		cout << "********Assertion cannot be shown********" << endl;
	}

}
