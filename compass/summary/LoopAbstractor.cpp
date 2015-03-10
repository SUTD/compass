/*
 * LoopAbstractor.cpp
 *
 *  Created on: Apr 15, 2012
 *      Author: tdillig
 */

#include "LoopAbstractor.h"
#include "SummaryGraph.h"
#include <set>
#include <map>
#include "Edge.h"
#include "Identifier.h"
#include "SummaryUnit.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "util.h"
#include "DisplayTag.h"
#include "access-path.h"
#include "MemoryGraph.h"
#include "AssertionTrace.h"



using namespace std;

LoopAbstractor::LoopAbstractor(SummaryGraph& sg):sg(sg)
{

	//discharge_assertions();
	//assume_loop_continuation_cond();
	add_after_loop_targets();


}




void LoopAbstractor::assume_loop_continuation_cond()
{
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!=sg.edges.end(); it++)
	{
		Edge* se = *it;
		Constraint edge_c = se->get_constraint();
		edge_c.assume(sg.loop_continuation_cond);
		se->set_constraint(edge_c);
	}
}


void LoopAbstractor::add_after_loop_targets()
{
	// First, build a mapping from access paths to new access paths
	// to substitute later


	int loop_id =  sg.get_identifier().get_loop_id();
	int line = (*sg.su->get_entry_block()->get_statements().begin())->line;

	/*
	for(; it!= sg.edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* source = e->get_source_ap();
		string new_name = source->to_string() + "@" + int_to_string(loop_id);
		DisplayTag dt(new_name, line, "");
		AccessPath* new_ap = UnmodeledValue::make_imprecise(source->get_type(), dt);
		ap_replacements[source] = new_ap;
		cout << "Adding to ap_replacements: " << source->to_string() << "  -> "
				<< new_ap->to_string() << endl;
		term_replacements[source->to_term()] = new_ap->to_term();
		cout << source->to_string() << " ------>  "<< new_ap->to_string() << endl;
	}*/

	set<Edge*, CompareTimestamp> edges_copy = sg.edges;

	// Now, go through each edge and update targets and constraints
	set<pair<MemoryLocation*, int> > processed_sources;
	set<Edge*>::iterator it = sg.edges.begin();
	for(it = edges_copy.begin(); it!= edges_copy.end(); it++)
	{
		Edge* se = *it;
		AccessPath* source_ap = se->get_source_ap();
		string new_name = source_ap->to_string() + "@" + int_to_string(loop_id);
		DisplayTag dt(new_name, line, "");

		AccessPath* new_ap;
		Constraint c;
		if(source_ap->find_outermost_index_var(true)!= NULL) {
			IndexVariable* iv = IndexVariable::make_source();
			new_ap = UnmodeledValue::make_imprecise(
							source_ap->get_type(), iv, dt);
			c &= ConstraintGenerator::get_eq_constraint(
					source_ap->find_outermost_index_var(true),
					IndexVariable::make_target(iv));
		}
		else
		{
			new_ap = UnmodeledValue::make_imprecise(
							source_ap->get_type(),  dt);
		}


		new_ap = Deref::make(new_ap);

		MemoryLocation* source = se->get_source_loc();
		int source_offset = se->get_source_offset();
		pair<MemoryLocation*, int> p = make_pair(source, source_offset);
		sg.remove_edge(se);
		if(processed_sources.count(p) > 0) continue;
		processed_sources.insert(p);
		MemoryLocation* target = sg.mg->get_location(new_ap);

		sg.add_edge(source, source_offset, target, 0, c,
				se->get_time_stamp());
	}


	sg.return_cond = Constraint();//.replace_terms(term_replacements);
	//conjoin_term_cond();

}


void LoopAbstractor::conjoin_term_cond()
{
	Constraint term_cond = !sg.loop_continuation_cond;

	cout << "Original term cond:  " << term_cond << endl;

	set<Term*> terms;
	term_cond.get_terms(terms,false);


	set<Term*>::iterator it =terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		AccessPath* ap = AccessPath::to_ap(t);
		cout << "Current term: " << ap->to_string() << endl;
		if(ap_replacements.count(ap) == 0) {
			cout << "no replacement...skipping" << endl;
			continue;
		}

		set<pair<AccessPath*, Constraint> > vs;
		sg.get_value_set(ap, vs, Constraint(true));

		AccessPath* renamed_ap = UnmodeledValue::make_imprecise(ap->get_type(),
				DisplayTag("",-1,""));

		Constraint final_c(false);
		Constraint changed_c(false);
		set<pair<AccessPath*, Constraint> >::iterator it2 = vs.begin();
		for(; it2 != vs.end(); it2++) {
			AccessPath* val = it2->first;
			Constraint val_c= it2->second;
			changed_c |= val_c;
			Constraint cur =
					ConstraintGenerator::get_eq_constraint(renamed_ap, val);
			cur&=val_c;
			final_c |= cur;
		}

		Constraint default_c = !changed_c;
		AccessPath* replaced_ap = ap_replacements[ap];
		assert(replaced_ap != NULL);
		Constraint cur =
				ConstraintGenerator::get_eq_constraint(renamed_ap, replaced_ap);
		cur &= default_c;
		final_c  |= cur;



		term_cond.replace_term(ap->to_term(), renamed_ap->to_term());
		term_cond &= final_c;

		cout << "Current term cond: " << term_cond << endl;



		ConstraintGenerator::eliminate_evar(term_cond, renamed_ap);

		cout << "After eliminate: " << term_cond << endl;


	}

	sg.return_cond &= term_cond;




}


LoopAbstractor::~LoopAbstractor()
{

}
