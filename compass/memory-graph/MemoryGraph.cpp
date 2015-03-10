/*
 * MemoryGraph.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "MemoryGraph.h"
#include "MemoryLocation.h"
#include "access-path.h"
#include "sail/Variable.h"
#include "sail/Constant.h"
#include "il/type.h"
#include "Edge.h"
#include "util.h"
#include <iostream>
#include "Type.h"
#include "il/binop_expression.h"
#include "Instantiator.h"
#include "SummaryGraph.h"
#include "ErrorTrace.h"
#include "sail/SummaryUnit.h"
#include "MemoryAnalysis.h"
#include "access-path.h"
#include "sail/Block.h"
#include "GlobalAnalysisState.h"
#include "error-traces.h"
#include "compass_assert.h"
#include "sail/FunctionCall.h"
#include "sail/SuperBlock.h"
#include "ClientAnalysisManager.h"
#include "ClientAnalysis.h"

#define CONVERT_BITWISE_NOT_TO_ARITHMETIC true
#define DEBUG false

void MemoryGraph::clear()
{
	str_const_counter = 0;
	errors.clear();
	error_traces.clear();

	loop_termination_constraints.clear();
	recurrence_map.clear();
	Constraint::clear_background();

	map<AccessPath*, MemoryLocation*>::iterator it = ap_to_locs.begin();
	for(; it != ap_to_locs.end(); it++) {
		it->second->clear();
	}

	edges.clear();
	edge_counter = 0;
	UnmodeledValue::clear();
	Variable::clear();
	dvm.clear();
	ie.clear();

	TypeConstant::add_all_axioms();
}

string MemoryGraph::get_file()
{
	return file;
}

int MemoryGraph::get_line()
{
	return line;
}


MemoryGraph::MemoryGraph(vector<Error*>& errors, bool recursive,
		MemoryAnalysis &ma, bool top_level_fn) :ea(this),
	cg(*this), errors(errors), ma(ma)
{
	str_const_counter = 0;
	this->recursive = recursive;
	edge_counter = 0;
	instruction_number = 0;
	report_all_errors = false;
	this->su = NULL;
	this->is_top_level_function = top_level_fn;

	/*
	int k = 1;
	for(long int i=0; i <20000000000; i++)
	{
		k+=7*i;
	}
	cout << k << endl;
	*/
}

void MemoryGraph::set_recursive() {
	recursive = true;
}

void MemoryGraph::set_line(int line) {
	this->line = line;
}

void MemoryGraph::set_file(const string & file) {
	this->file = file;
}

void MemoryGraph::set_error_report_status(bool rae) {
	this->report_all_errors = rae;
}

void MemoryGraph::set_fn_id(string fn_id) {
	this->fn_id = fn_id;
}

/*
 * Returns the guarded value set for the given symbol.
 */
void MemoryGraph::get_value_set(sail::Symbol* s, set<pair<AccessPath*,
		Constraint> >& values) {

	MemoryLocation* loc = get_location(s);
	get_value_set(loc, 0, values);
}

/*
 * Are the possible values of this symbol mutually exclusive?
 */
bool MemoryGraph::has_precise_value_set(sail::Symbol* s) {
	set<pair<AccessPath*, Constraint> > values;
	get_value_set(s, values);
	set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
	for (; it != values.end(); it++) {
		if (!it->second.is_precise())
			return false;
	}
	return true;
}

/*
 * Are the possible values of this access path mutually exclusive?
 */
bool MemoryGraph::has_precise_value_set(AccessPath* ap) {
	set<pair<AccessPath*, Constraint> > values;
	get_value_set(ap, values);
	set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
	for (; it != values.end(); it++) {
		if (!it->second.is_precise())
			return false;
	}
	return true;
}

/*
 * Gives the guarded value set for loc&offset
 * combination. The result of get_value_set is *always* in terms of
 * source variables, both for the constraint as well as the access paths.
 */
void MemoryGraph::get_value_set(MemoryLocation* loc, int offset, set<pair<
		AccessPath*, Constraint> >& values) {
	if(!loc->has_field_at_offset(offset)) return;
	add_default_edges(loc);
	set<Edge*>* targets = loc->get_successors(offset);
	set<Edge*>::iterator it = targets->begin();
	for (; it != targets->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();
		int target_offset = e->get_target_offset();
		AccessPath* target_ap = target->get_access_path(target_offset);
		target_ap = target_ap->strip_deref();
		AccessPath* res = target_ap;
		Constraint target_c = e->get_constraint();
		ivm.rename_index_vars(target_c);
		values.insert(pair<AccessPath*, Constraint> (res, target_c));
	}
}

/*
 * Gives the guarded value set for loc&offset
 * combination under constraint c. The result of get_value_set is *always* in terms of
 * source variables, both for the constraint as well as the access paths.
 */
void MemoryGraph::get_value_set(MemoryLocation* loc, int offset, set<pair<
		AccessPath*, Constraint> >& values, Constraint c) {


	add_default_edges(loc);
	set<Edge*>* targets = loc->get_successors(offset);
	set<Edge*>::iterator it = targets->begin();
	for (; it != targets->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();
		int target_offset = e->get_target_offset();
		AccessPath* target_ap = target->get_access_path(target_offset);
		target_ap = target_ap->strip_deref();
		AccessPath* res = target_ap;

		Constraint target_c = e->get_constraint();
 		ivm.rename_index_vars(target_c);
		Constraint cur = c & target_c;

		if (cur.sat_discard()) {
			values.insert(pair<AccessPath*, Constraint> (res, target_c));
		}
	}
}

void MemoryGraph::get_points_to_set(AccessPath* ap, set<pair<AccessPath*,
		Constraint> >& pts_to_set, Constraint c)
{
	MemoryLocation* loc = get_location(ap);
	ap = ap->replace(ap->find_representative(), loc->get_representative_access_path());
	int offset = loc->find_offset(ap);
	if(offset == -1){
		offset = 0;
	}
	get_points_to_set(loc, offset, pts_to_set, c);
}

void MemoryGraph::get_points_to_set(MemoryLocation* loc, int offset, set<pair<
		AccessPath*, Constraint> >& pts_to_set, Constraint c)
{
	add_default_edges(loc);
	set<Edge*>* targets = loc->get_successors(offset);
	set<Edge*>::iterator it = targets->begin();
	for (; it != targets->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();
		int target_offset = e->get_target_offset();
		AccessPath* target_ap = target->get_access_path(target_offset);
		AccessPath* res = target_ap;
		Constraint target_c = e->get_constraint();
		ivm.rename_index_vars(target_c);
		Constraint cur = c & target_c;
		if (cur.sat_discard()) {
			pts_to_set.insert(pair<AccessPath*, Constraint> (res, target_c));

		}
	}
}

/*
 * Returns the guarded value set for the given access path.
 */
void MemoryGraph::get_value_set(AccessPath* ap, set<pair<AccessPath*,
		Constraint> >& values)
{
	MemoryLocation* loc = get_location(ap);
	ap = ap->replace(ap->find_representative(), loc->get_representative_access_path());
	int offset = loc->find_offset(ap);
	get_value_set(loc, offset, values);

}

/*
 * Returns the guarded value set for the given access path
 * under constraint c.
 */
void MemoryGraph::get_value_set(AccessPath* ap, set<pair<AccessPath*,
		Constraint> >& values, Constraint c)
{

	MemoryLocation* loc = get_location(ap);
	ap = ap->replace(ap->find_representative(), loc->get_representative_access_path());
	int offset = loc->find_offset(ap);
	if(offset == -1) offset = 0;
	get_value_set(loc, offset, values, c);

}

Constraint MemoryGraph::get_neqz_constraint(sail::Symbol* s) {
	return cg.get_neqz_constraint_from_value_set(s);
}

void MemoryGraph::report_error(error_code_type error, string message) {
	Error* e = new Error(line, file, error, message, su->get_identifier());
	errors.push_back(e);

}

void MemoryGraph::remove_root(sail::Variable* temp) {
	AccessPath* ap = get_access_path_from_symbol(temp);
	remove_root(ap);
}

void MemoryGraph::remove_root(AccessPath* root) {
	MemoryLocation* loc = get_location(root);
	root_locs.erase(loc);
}

bool MemoryGraph::has_location(AccessPath* _ap)
{
	AccessPath* ap = _ap->find_representative();
	return (ap_to_locs.count(ap) > 0);
}


void MemoryGraph::add_size_of_constarray(AccessPath* ap)
{
	il::type* il_t = ap->get_type();
	if(!il_t->is_constarray_type()) return;

	MemoryLocation* res = get_location(ap);
	AccessPath* rep = res->get_representative_access_path();

	assert_context("Adding size of constarray: " + ap->to_string());

	if (!res->has_field_at_offset(SIZE_OFFSET)) {
		c_assert(false);
	}
	AccessPath* total_size;
	int elem_size;
	if(il_t->is_array_type()) {
		c_assert(il_t->is_constarray_type());
		il::constarray_type* ca = (il::constarray_type*) il_t;
		elem_size = ca->get_elem_type()->get_size() / 8;
		int size = ca->get_num_elems() * elem_size;
		AccessPath* cv = ConstantValue::make(size);
		total_size = cv;
	}
	else {
		total_size = res->get_access_path(SIZE_OFFSET);
		elem_size = 1;
	}



	IndexVariable* iv = rep->find_outermost_index_var(true);
	c_assert(iv != NULL);
	AccessPath* offset = ArithmeticValue::make_times(iv,
			ConstantValue::make(elem_size));
	AccessPath* size_ap = ArithmeticValue::make_minus(total_size, offset);
	size_ap = size_ap->add_deref();
	MemoryLocation* size_loc = get_location(size_ap);

	set<IndexVariable*> all_ivs;
	rep->get_nested_index_vars(all_ivs);
	set<IndexVariable*>::iterator it = all_ivs.begin();
	Constraint edge_c;
	for(; it!= all_ivs.end(); it++)
	{
		IndexVariable* iv = *it;
		IndexVariable* target_iv = IndexVariable::make_target(iv);
		edge_c &= ConstraintGenerator::get_eq_constraint(iv, target_iv);
	}


	this->put_edge(NULL, -1, edge_c, res, size_loc, SIZE_OFFSET, 0, false,
			true);

}

/*
 * Returns the memory location that this access path is part of.
 */
MemoryLocation* MemoryGraph::get_location(AccessPath* _ap) {
	MemoryLocation* res;

	assert_context("Get location for access path: "+AccessPath::safe_string(_ap));
	AccessPath* ap = _ap->find_representative();



	// Check if we already have this memory location
	if (ap_to_locs.count(ap) > 0) {
		res = ap_to_locs[ap];
		return res;
	}






	res = new MemoryLocation(ivm, ap, ma.get_su(), ma.track_dynamic_type());
	AccessPath* rep = res->get_representative_access_path();


	/*
	 * It is possible that the representative of the new location is
	 * in the map even though ap was not. This happens, for example, if
	 * ap = "a", but rep = "a[i]".
	 */
	if (ap_to_locs.count(rep) != 0) {
		delete res;
		return ap_to_locs[rep];
	}

	ap_to_locs[rep] = res;
	AccessPath* fake_field_rep = rep->get_representative_for_fake_field();
	ap_to_locs[fake_field_rep]= res;



	if (ap->get_ap_type() == AP_VARIABLE) {
		Variable* v = (Variable*) ap;
		if (v->get_original_var() != NULL || v->is_summary_temp())
			root_locs.insert(res);
	}

	return res;
}

MemoryLocation* MemoryGraph::get_location(sail::Symbol* s) {

	assert_context("Get location for symbol: " +
			(s ?  s->to_string() : "null"));
	AccessPath* ap = AccessPath::get_ap_from_symbol(s);
	MemoryLocation* res = get_location(ap);
	c_assert(res!=NULL);
	return res;
}

AccessPath* MemoryGraph::get_default_target(AccessPath* ap) {

	AccessPath* base = ap->get_base();



	if(ap->get_ap_type() == AP_ADT)
	{
		AbstractDataStructure* ads = (AbstractDataStructure*)ap;
		AccessPath* res = Deref::make(ap, ads->get_value_type());
		return res;
	}
	if(ap->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = static_cast<FieldSelection*>(ap);
		int offset = fs->get_field_offset();
		if(offset<=CLIENT_INITIAL_OFFSET && !
				ap->add_deref()->is_interface_object(su))
		{
			ClientAnalysis* a =
					ClientAnalysis::cam.get_analyis_from_offset(offset);
			sail::Symbol* s = a->get_initial_value();
			AccessPath* ap = get_access_path_from_symbol(s)->add_deref();
			return ap;
		}
	}

	if (ap->get_ap_type() != AP_ADDRESS && base != NULL) {
		if (base->get_ap_type() == AP_VARIABLE) {
			Variable* base_var = (Variable*) base;
			if (this->su->is_function() &&
					(base_var->is_local_var() || base_var->is_return_var())) {
					if(ap->is_deleted_field()) {
						ap = ConstantValue::make(0);
					}
					else {
						DisplayTag dt("uninitialized value", this->line,
								this->file);
						AccessPath* new_base =
								UnmodeledValue::make_uninit(
										base_var->get_type(), dt);
						ap = ap->replace(base_var, new_base);
					}
			}
		}


		else if(ap->is_deleted_field())
		{
			/*
			 * With unmodeled values, deleted doesn't make much sense.
			 */
			if(base->get_ap_type() == AP_UNMODELED)
				ap = ConstantValue::make(0);

			/*
			 * If this is a fake pointer we introduced for ADT's,
			 * it also doesn't make sense to reason about being deleted.
			 */

			MemoryLocation* loc = this->get_location(ap);
			AccessPath* rep = loc->get_access_path(0)->find_representative();

			if(rep->get_ap_type() == AP_DEREF &&
					rep->get_inner()->get_ap_type() == AP_ADT) {
				ap = ConstantValue::make(0);
			}
		}
	}

	if(ap->is_size_field_ap()) {
		AccessPath* size_target = get_default_target_of_size(ap);

		return size_target;
	}



	AccessPath* res = ap->add_deref();




	return res;
}



void MemoryGraph::add_default_edge(MemoryLocation* loc, int offset,
		Constraint edge_c, bool add_temp_default)
{




	assert_context("Add default edge to: " + loc->to_string(true) +
			" at offset " + int_to_string(offset));



	c_assert(loc->offset_to_ap.count(offset) > 0);
	AccessPath* ap = loc->offset_to_ap[offset];
	c_assert(ap != NULL);
	//delete me
	if (!add_temp_default && ap->to_string().find("__temp") != string::npos) {
		return;
	}
	if (ap->get_ap_type() == AP_VARIABLE) {
		Variable* v = (Variable*) ap;
		if (v->is_loop_exit_var()) {
			return;
		}
	}

	if(offset == SIZE_OFFSET) {
		AccessPath* rep = loc->get_representative_access_path();
		if(rep->get_type()->is_constarray_type()) {
			add_size_of_constarray(rep);
			return;
		}
	}

	/*
	 * ADT's start out as having no elements.
	 */

	if(ap->get_representative_for_fake_field() != ap) {
		bool nil_target = su->is_function();
		if(ap->get_base()->get_ap_type() == AP_VARIABLE) {
			Variable* v = (Variable*) ap->get_base();
			if(v->is_argument()) {
				nil_target = false;
			}
			else if(v->is_global_var() && !v->is_anonymous())
			{
				if(su->is_function()) {
					sail::Function* f = (sail::Function*) su;
					if(!f->is_init_function()){
						nil_target = false;
					}
				}
			}


		}
		if(nil_target)
		{

			AccessPath* nil = Nil::make();
			nil = nil->add_deref();
			MemoryLocation* nil_loc = get_location(nil);

			Edge *e = put_edge(NULL, 0, Constraint(true),
					loc, nil_loc, offset, 0, true, true);
			if (e != NULL) {
				e->and_not_constraint(ivm, edge_c);
			}

			return;
		}

	}

	if (!ap->is_recursive()) {

		vector<pair<AccessPath*, Constraint> > default_targets;
		bool elem_alias = ea.get_potential_entry_aliases(ap, default_targets);


		vector<pair<AccessPath*, Constraint> >::iterator it = default_targets.begin();
		for(; it!= default_targets.end(); it++)
		{
			AccessPath* cur_alias = it->first;

			AccessPath* target =
					get_default_target(cur_alias);
			MemoryLocation* def_target = get_location(target);

			int target_offset = def_target->find_offset(cur_alias->add_deref());


			if(target_offset == -1) {
				target_offset = 0;
			}


			/*
			 * If the default target introduces an additional index var i,
			 * we need to add i = 0 constraint.
			 */
			Constraint c = it->second;


			AccessPath* target_ap = def_target->get_access_path(target_offset);

			if(ap == cur_alias) {
				c &= IndexVarManager::get_outer_indices_zero_constraint(target_ap);
				/*
				 * All existing index variables are preserved,so add i=j constraints.
				 */
				vector<IndexVariable*> source_vars;
				loc->get_access_path(offset)->get_nested_index_vars(source_vars);

				vector<IndexVariable*>::iterator it2 = source_vars.begin();
				for (int i=0; it2 != source_vars.end(); it2++, i++) {
					if(elem_alias && i== ((int)source_vars.size()-1)) continue;
					IndexVariable *source_index = (*it2);
					IndexVariable* target_index = IndexVariable::make_target(
							source_index);
					Constraint cur_c = cg.get_eq_constraint(source_index, target_index);
					c &= cur_c;
				}
			}




			/*
			 * For static arrays, the default target must be within the range.
			 */
			if(ma.get_su()->is_function() && ap->get_ap_type() == AP_ARRAYREF &&
					ap->get_type()->is_constarray_type() &&
					loc->get_successors(offset)->size() >0) {
				il::constarray_type* at = (il::constarray_type*) ap->get_type();
				int num_elems = at->get_num_elems();
				ArrayRef* ar = (ArrayRef*) ap;
				IndexVariable* iv = ar->get_index_var();
				c &= ConstraintGenerator::get_geqz_constraint(iv);
				c &= ConstraintGenerator::get_lt_constraint(iv,
						ConstantValue::make(num_elems));
			}

			AccessPath* source_ap = loc->get_access_path(offset);
			bool is_default = true;
			if(source_ap->get_ap_type() == AP_VARIABLE &&
					((Variable*)source_ap)->is_return_var()) {
				is_default = false;
			}

			Edge *e = put_edge(NULL, 0, c, loc, def_target, offset,
					target_offset, is_default,
					true);
			if (e != NULL) {
				e->and_not_constraint(ivm, edge_c);
			}

		}

	}


	else {
		IndexVariable* source_index = ap->get_index_var();
		IndexVariable* target_index = ivm.get_target_index_var(loc);
		AccessPath* av = ArithmeticValue::make_plus(source_index,
				ConstantValue::make(1));
		Constraint c = cg.get_eq_constraint(target_index, av);
		vector<IndexVariable*> source_vars;
		loc->get_access_path(offset)->get_nested_index_vars(source_vars);

		// Preserve any other nested index vars by adding i=j constraints
		vector<IndexVariable*>::iterator it3 = source_vars.begin();
		for (; it3 != source_vars.end(); it3++) {
			if (*it3 == source_index)
				continue;
			IndexVariable *source_index = (*it3);
			IndexVariable* target_index = IndexVariable::make_target(
					source_index);
			Constraint cur_c = cg.get_eq_constraint(source_index, target_index);
			c = c & cur_c;

		}

		Edge* e = put_edge(NULL, 0, c, loc, loc, offset, 0, true, true);
		if (e != NULL) {
			e->and_not_constraint(ivm, edge_c);

		}

	}

}

/*
 * Adds the default points to edges for this memory location.
 */
void MemoryGraph::add_default_edges(MemoryLocation* loc, bool add_temp_default)
{
	add_default_edges(loc, Constraint(true), add_temp_default, false);
}

void MemoryGraph::add_default_edges(MemoryLocation* loc, Constraint c, bool
		add_temp_default, bool force_default_edge)
{

	AccessPath* rep = loc->get_representative_access_path();
	if (rep->get_ap_type() == AP_ARRAYREF
			&& ((ArrayRef*) rep)->is_string_constant())
		return;

	/*if (rep->get_ap_type() == AP_DEREF) {
		Deref* d = (Deref*) rep;
		AccessPath* inner = d->get_inner();
		if (inner->get_ap_type() == AP_CONSTANT)
			return;
	}*/
	Constraint false_c = !c;
	map<int, set<Edge*>*>::iterator it = loc->get_successor_map().begin();
	for (; it != loc->get_successor_map().end(); it++) {
		set<Edge*>* edges = it->second;
		if (!force_default_edge&& edges->size() != 0) {
			continue;
		}
		add_default_edge(loc, it->first, false_c, add_temp_default);

	}
}

/*
 * a=b
 */
void MemoryGraph::process_assignment(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint c)
{
	if(rhs->is_constant())
	{
		sail::Constant* c = (sail::Constant*)rhs;
		if(c->is_string_constant())
		{
			string s = c->get_string_constant();
			MemoryLocation* loc = get_location(lhs);
			process_string_assignment(loc, s);
			return;
		}
	}




	Variable* v_lhs = Variable::make_program_var(lhs);
	AccessPath* ap_rhs = get_access_path_from_symbol(rhs);

	if(v_lhs == ap_rhs) return;


	check_memory_access(ap_rhs, c, rhs->to_string(true));
	process_assignment(v_lhs, ap_rhs, c);


}

void MemoryGraph::process_assignment(Variable* lhs, AccessPath* rhs,
		Constraint c) {

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);
	add_default_edges(lhs_loc);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);



	map<int, set<Edge*>*>::iterator it = rhs_loc->get_successor_map().begin();
	for (; it != rhs_loc->get_successor_map().end(); it++) {
		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			Edge* e = *it2;
			Constraint edge_c = e->get_constraint();
			Constraint assign_c = edge_c & c;

			if(assign_c.unsat()) continue;

			put_edge(e->get_source_loc(), e->get_source_offset(), assign_c,
					lhs_loc, e->get_target_loc(), e->get_source_offset(),
					e->get_target_offset());

		}
	}

	check_invariants(lhs_loc, 0, this);
	//InvariantChecker::_check_invariants(lhs, this);
}

/*
 * a=b.f
 */
void MemoryGraph::process_assignment(sail::Variable* lhs, sail::Variable* rhs,
		int rhs_offset, Constraint c) {
	Variable* v_lhs = Variable::make_program_var(lhs);
	Variable* v_rhs = Variable::make_program_var(rhs);
	string pp_expression = rhs->to_string(true);
	process_assignment(v_lhs, v_rhs, rhs_offset, c, pp_expression);

}

void MemoryGraph::process_assignment(Variable* lhs, AccessPath* rhs,
		int rhs_offset, Constraint c, string pp_expression) {



	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);
	

	// If this location doesn't have the correct field, we may
	// need to add an anynomous field after checking consistency.
	if (!rhs_loc->add_offset(rhs_offset, lhs->get_type(), rhs->is_constant())) {
		add_default_edges(lhs_loc);
		report_type_inconsistency(rhs, rhs_offset);
		DisplayTag dt(pp_expression, this->line, this->file);
		assign_unmodeled_value(lhs_loc, c, lhs->get_type(), UNM_IMPRECISE, dt);
		return;
	}

	add_default_edges(rhs_loc);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);



	/*
	 * If b.f is a struct, more than one field may be copied, so compute
	 * the offset at which we should stop.
	 */
	int end_offset = rhs_offset + lhs_loc->get_last_offset();

	map<int, set<Edge*>*>::iterator it = rhs_loc->get_successor_map().begin();
	for (; it != rhs_loc->get_successor_map().end(); it++) {
		int cur_offset = it->first;
		if (cur_offset < rhs_offset)
			continue;
		if (cur_offset > end_offset)
			break;

		string cur_pp_exp = pp_expression;
		AccessPath* cur_ap = rhs_loc->get_access_path(cur_offset);
		if(cur_ap->get_ap_type() == AP_FIELD) {
			FieldSelection* fs = (FieldSelection*) cur_ap;
			string field_name = fs->get_field_name();
			cur_pp_exp += field_name;
		}

		check_memory_access(cur_ap, c, cur_pp_exp);

		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			Edge* e = *it2;
			Constraint edge_c = e->get_constraint();
			Constraint cur_c = c & edge_c;


			put_edge(e->get_source_loc(), e->get_source_offset(), cur_c,
					lhs_loc, e->get_target_loc(), it->first - rhs_offset,
					e->get_target_offset());

		}
	}
}

void MemoryGraph::get_remaining_buffer_size(sail::Symbol* buf, set<pair<
		AccessPath*, Constraint> >& possible_sizes, Constraint c) {
	get_buffer_size(buf, possible_sizes, c, false);
}
void MemoryGraph::get_total_buffer_size(sail::Symbol* buf, set<pair<
		AccessPath*, Constraint> >& possible_sizes, Constraint c) {
	get_buffer_size(buf, possible_sizes, c, true);
}

void MemoryGraph::get_buffer_size(sail::Symbol* buffer, set<pair<AccessPath*,
		Constraint> >& possible_sizes, Constraint c, bool total) {

	assert_context("Get buffer size for: " + buffer->to_string());
	MemoryLocation* buf_loc = get_location(buffer);
	set<Edge*>* succs = buf_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for (; it != succs->end(); it++) {
		Edge* e = *it;

		MemoryLocation* target = e->get_target_loc();
		add_default_edges(target);
		if (!target->get_representative_access_path()->belongs_to_array()) {
			il::type* buf_type = buffer->get_type();
			c_assert(buf_type->is_pointer_type());
			il::type* elem_type = buf_type->get_inner_type();
			c_assert(!elem_type->is_void_type());
			this->change_to_array_loc(target, elem_type);

		}

		Constraint edge_c = e->get_constraint();

		ivm.rename_index_vars(edge_c);
		Constraint target_c = edge_c & c;
		if (target_c.unsat())
			continue;

		AccessPath* rep = target->get_representative_access_path();
		c_assert(rep->get_ap_type() == AP_ARRAYREF);
		ArrayRef* rep_array = (ArrayRef*) rep;

		set<Edge*>* succs = target->get_successors(SIZE_OFFSET);
		set<Edge*>::iterator it2 = succs->begin();
		for (; it2 != succs->end(); it2++) {
			Edge* cur_e = *it2;
			Constraint size_c = cur_e->get_constraint() & target_c;
			AccessPath* size_ap = cur_e->get_target_ap();
			if (total) {
				set<IndexVariable*> indices;
				size_ap->get_nested_index_vars(indices);
				c_assert(indices.size() <= 1);
				if (indices.size() == 1) {
					size_ap = size_ap->replace(*indices.begin(),
							ConstantValue::make(0));
				}
			}

			possible_sizes.insert(make_pair(size_ap, size_c));

		}
	}
}


void MemoryGraph::process_observed(sail::Symbol * s, Constraint c)
{
	assert_context("Processing observed: " + s->to_string() );
	set<pair<AccessPath*, Constraint> > values;
	get_value_set(s, values);
	set<AccessPath*> sat_values;
	c_assert(sat_values.size() <= 2);
	set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
	Constraint observed_c(true);

	ConstantValue * zero = ConstantValue::make(0);

	for (; it != values.end(); it++) {
		if (it->first == zero)
			continue;
		AccessPath* ap = it->first;
		Constraint eqz_c = ConstraintGenerator::get_neqz_constraint(ap);
		Constraint cur_c = it->second;
		Constraint res_c = (!c) | (cur_c & eqz_c);
		res_c.sat();
		observed_c &= res_c;
	}
	if(!observed_c.sat())
	{
		assert_context("Observed value " + s->to_string() + " is impossible.");
		c_assert(false);
	}
	this->observed_c &= observed_c;
	if(this->observed_c.unsat())
	{
		assert_context("Observed value " + s->to_string() + " is impossible "
				"together with other observed values.");
		c_assert(false);
	}
	cout << "------------ observed_c: " << this->observed_c << endl;

}

/*
 * a = buffer_size(buf);
 */
void MemoryGraph::process_get_buffer_size(sail::Variable* lhs,
		sail::Symbol* buffer, Constraint c) {

	assert_context("Processing get_buffer_size: " + lhs->to_string() +
			" = buffer_size(" + buffer->to_string() +")" );

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(buffer);
	add_default_edges(rhs_loc);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);

	set<Edge*>* succs = rhs_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for (; it != succs->end(); it++) {
		Edge* e = *it;

		MemoryLocation* target = e->get_target_loc();
		add_default_edges(target);
		if (!target->get_representative_access_path()->belongs_to_array()) {
			il::type* buf_type = buffer->get_type();
			c_assert(buf_type->is_pointer_type());
			il::type* elem_type = buf_type->get_inner_type();
			c_assert(!elem_type->is_void_type());
			this->change_to_array_loc(target, elem_type);

		}

		Constraint edge_c = e->get_constraint();

		ivm.rename_index_vars(edge_c);
		ivm.rename_source_to_target_index(edge_c);
		Constraint target_c = edge_c & c;
		if (target_c.unsat())
			continue;


		AccessPath* rep = target->get_representative_access_path();
		c_assert(rep->get_ap_type() == AP_ARRAYREF);
		ArrayRef* rep_array = (ArrayRef*) rep;
		int elem_size = rep_array->get_elem_size();

		set<Edge*>* possible_sizes = target->get_successors(SIZE_OFFSET);
		set<Edge*>::iterator it2 = possible_sizes->begin();
		for (; it2 != possible_sizes->end(); it2++) {
			Edge* cur_e = *it2;
			Constraint size_c = cur_e->get_constraint() & target_c;
			AccessPath* size_ap = cur_e->get_target_ap();
			//size_ap = ArithmeticValue::make_division(size_ap, elem_size);
			MemoryLocation* target_loc = get_location(size_ap);

			put_edge(cur_e->get_source_loc(), cur_e->get_source_offset(),
					size_c, lhs_loc, target_loc, 0, 0);



		}

	}

}

void MemoryGraph::check_memory_access(AccessPath* ap, Constraint c,
		string pp_expression)
{
	if(!GlobalAnalysisState::check_uninit()) {
		return;
	}


	MemoryLocation* loc = get_location(ap);
	ap = ap->replace(ap->find_representative(),
			loc->get_representative_access_path());

	int offset = loc->find_offset(ap);
	add_default_edges(loc);


	set<Edge*>* succs = loc->get_successors(offset);
	set<Edge*>::iterator it = succs->begin();
	for(; it!=succs->end(); it++)
	{

		Edge* e = *it;

		AccessPath* target = e->get_target_ap();
		AccessPath* target_stripped = target->strip_deref();

		// If this is a compund expression (e.g., arithmetic),
		// we must have already checked the constituent parts.
		if(target_stripped->get_ap_type() == AP_ARITHMETIC ||
				target_stripped->get_ap_type() == AP_FUNCTION) continue;
		if(target_stripped->is_initialized()) continue;
		Constraint target_c = e->get_constraint();


		Constraint uninit_c = target_c & c;
		if(uninit_c.sat()) {
			IndexVarManager::eliminate_source_vars(uninit_c);
			IndexVarManager::rename_target_to_source_index(uninit_c);
			ReadTrace* rt = ReadTrace::make(target->strip_deref(),
					pp_expression, line, file,
					uninit_c, ma.get_cur_iteration_counter());
			error_traces.insert(rt);
		}


	}
}

void MemoryGraph::check_static_type(sail::Variable* ptr, Constraint c,
		string pp_expression)
{



	if(!GlobalAnalysisState::check_casts()){
		return;
	}




	MemoryLocation* ptr_loc = get_location(ptr);
	add_default_edges(ptr_loc);


	il::type* ptr_t = ptr_loc->get_access_path(0)->get_type();
	c_assert(ptr_t->is_pointer_type());
	il::type* inner_t = ptr_t->get_inner_type();
	if(!inner_t->is_record_type()) {
		return;
	}
	TypeConstant* static_type =  TypeConstant::make(inner_t);

	set<Edge*>* succs = ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		Constraint edge_c = e->get_constraint();
		MemoryLocation* target_loc = e->get_target_loc();
		add_default_edges(target_loc);
		Constraint fail_c(false);
		if(!target_loc->has_field_at_offset(RTTI_OFFSET)) {
			fail_c = edge_c & c;
		}

		else {



			IndexVarManager::rename_index_vars(edge_c);

			AccessPath* rtti_field = target_loc->get_access_path(RTTI_OFFSET);

			set<Edge*>* rtti_edges = target_loc->get_successors(RTTI_OFFSET);
			set<Edge*>::iterator it2 = rtti_edges->begin();
			for(; it2!= rtti_edges->end(); it2++) {
				Edge* rtti_e = *it2;
				AccessPath* rtti_val = rtti_e->get_target_ap()->strip_deref();
				Constraint rtti_c = rtti_e->get_constraint();

				Constraint ok_c = ConstraintGenerator::get_subtype_of_constraint(rtti_val,
						static_type);
				fail_c |= (!ok_c & rtti_c);

			}

			fail_c &= (edge_c & c);
		}

		if(this->report_all_errors) {
			fail_c.replace_terms(replace_rtti_field, inner_t);
		}



		if(fail_c.sat()) {


			IndexVarManager::eliminate_source_vars(fail_c);
			IndexVarManager::rename_target_to_source_index(fail_c);




			CastErrorTrace* cast_t = CastErrorTrace::make(
					target_loc->get_representative_access_path(),
					inner_t, pp_expression, line, file,
					fail_c, ma.get_cur_iteration_counter());
			if(cast_t != NULL) {
				error_traces.insert(cast_t);
			}
		}

	}

}

void MemoryGraph::check_ptr_deref(sail::Variable* ptr, Constraint c,
		string pp_expression)
{
	if(GlobalAnalysisState::check_null()){
		check_null_deref(ptr, c, pp_expression);
	}

	if(GlobalAnalysisState::check_deleted_access()){
		check_access_to_delete_mem(ptr, c, pp_expression);
	}


}



void MemoryGraph::check_access_to_delete_mem(sail::Variable* ptr, Constraint c,
		string pp_exp)
{
	MemoryLocation* ptr_loc = get_location(ptr);
	this->add_default_edges(ptr_loc);
	set<Edge*>* succs = ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* target_loc = e->get_target_loc();
		AccessPath* target_ap = e->get_target_ap();
		add_default_edges(target_loc);
		Constraint edge_c = e->get_constraint();
		IndexVarManager::rename_index_vars(edge_c);

		if(!target_loc->has_field_at_offset(DELETED_OFFSET)) continue;
		check_access_to_deleted_mem(target_ap, edge_c & c, pp_exp, false);

	}
}

void MemoryGraph::check_null_deref(sail::Variable* ptr, Constraint c,
		string pp_expression)
{

	MemoryLocation* ptr_loc = get_location(ptr);
	add_default_edges(ptr_loc);



	set<Edge*>* succs = ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		Constraint edge_c = e->get_constraint();
		AccessPath* target_ap = e->get_target_ap();
		if(target_ap->get_ap_type() ==AP_ARRAYREF) {
			target_ap = target_ap->get_inner();
		}

		target_ap = target_ap->strip_deref();
		Constraint unsafe_deref_c = c & edge_c;


		Constraint null_c = ConstraintGenerator::get_eqz_constraint(target_ap);
		IndexVarManager::rename_source_to_target_index(null_c);



		unsafe_deref_c &= null_c;

		set<AccessPath*> aps;
		target_ap->get_nested_memory_aps(aps);
		Constraint geqz_c;
		set<AccessPath*>::iterator it2 = aps.begin();
		for(; it2!= aps.end(); it2++) {
			AccessPath* ap = *it2;
			if(!ap->get_type()->is_signed_type())
				geqz_c &= ConstraintGenerator::get_geqz_constraint(*it2);
		}

		unsafe_deref_c &= geqz_c;

		if(unsafe_deref_c.sat()) {
			IndexVarManager::eliminate_source_vars(unsafe_deref_c);
			// This is supposed to be like this, don't touch!
			// The error trace can contain i's; see null 2 regression.
			IndexVarManager::rename_target_to_source_index(unsafe_deref_c);





			DerefTrace* dt = DerefTrace::make(target_ap, pp_expression, line, file,
					unsafe_deref_c, ma.get_cur_iteration_counter());
			if(dt != NULL) {
				error_traces.insert(dt);
			}

		}


	}
}


void MemoryGraph::check_buffer_access(sail::Variable* buffer,
		sail::Symbol* offset, Constraint c, bool user_specified) {

	AccessPath* offset_ap = get_access_path_from_symbol(offset);
	string pp_exp = buffer->to_string(true) + "[" + offset->to_string(true) +"]";
	check_buffer_access(buffer, offset_ap, c, user_specified, pp_exp);


}

// We overrun buffer if (offset+1) * elem_size > remaining_size
Constraint MemoryGraph::get_overrun_constraint(AccessPath* index,
		Constraint index_c, int elem_size,
		AccessPath* size_ap, Constraint size_ap_c, Constraint cur_offset_c)
{
	AccessPath* index_plus_one = ArithmeticValue::make_plus(
			index, ConstantValue::make(1));
	AccessPath* total_offset = index_plus_one->multiply_ap(elem_size);
	Constraint violation_constraint =
			ConstraintGenerator::get_gt_constraint(total_offset, size_ap);


	violation_constraint &= (cur_offset_c & size_ap_c & index_c);
	ivm.eliminate_source_vars(violation_constraint);
	return violation_constraint;
}

/*
 * We underrun buffer if (total - remaining size) + index*elem_size < 0
 */
Constraint MemoryGraph::get_underrun_constraint(AccessPath* index,
		Constraint index_c, int elem_size,
		AccessPath* size_ap, Constraint size_ap_c,
		Constraint cur_offset_constraint, IndexVariable* buf_index)
{
	AccessPath* total_size = size_ap->replace(buf_index, ConstantValue::make(0));
	AccessPath* total_minus_remaining = ArithmeticValue::make_minus(total_size,
			size_ap);
	AccessPath* index_times_elem_size=  index->multiply_ap(elem_size);
	AccessPath* total_offset = ArithmeticValue::make_plus(total_minus_remaining,
			index_times_elem_size);
	Constraint violation_c = ConstraintGenerator::get_ltz_constraint(total_offset);
	violation_c &= (size_ap_c & cur_offset_constraint & index_c);
	ivm.eliminate_source_vars(violation_c);
	return violation_c;

}

void MemoryGraph::check_static_buffer_access(AccessPath* _buf_ap,
		AccessPath* index, Constraint c, string pp_expression)
{
	assert_context("Checking static buffer access to " +
			AccessPath::safe_string(_buf_ap) +
			 " at index: " + index->to_string());

	if(!GlobalAnalysisState::check_buffer()){
		return;
	}
	MemoryLocation* buf_loc = get_location(_buf_ap);
	add_default_edges(buf_loc);

	// We expect this to be upgraded to an array by now
	c_assert(_buf_ap->get_ap_type() == AP_ARRAYREF);
	ArrayRef* buf_ap = (ArrayRef*) _buf_ap;
	IndexVariable* index_var = buf_ap->get_index_var();
	int elem_size = buf_ap->get_elem_size();



	set<pair<AccessPath*, Constraint> > index_values;
	get_value_set(index, index_values);

	il::type* t = buf_ap->get_type();
	c_assert(t->is_constarray_type());
	il::constarray_type* cat = (il::constarray_type*) t;
	int num_elems = cat->get_num_elems();
	int size = num_elems * elem_size;
	AccessPath* size_ap = ConstantValue::make(size);
	AccessPath* mul = ArithmeticValue::make_times(index_var,
			ConstantValue::make(elem_size));
	size_ap = ArithmeticValue::make_minus(size_ap, mul);



	Constraint offset_c = ConstraintGenerator::get_eqz_constraint(index_var);

	Constraint overrun_c(false);
	Constraint underrun_c(false);

	set<pair<AccessPath*, Constraint> >::iterator it = index_values.begin();
	for(; it!= index_values.end(); it++)
	{
		AccessPath* index_val = it->first;
		Constraint index_c = it->second;


		overrun_c |= get_overrun_constraint(index_val, index_c, elem_size,
				size_ap, Constraint(true), offset_c);
		underrun_c |= get_underrun_constraint(index_val, index_c, elem_size,
				size_ap,  Constraint(true), offset_c, index_var);

	}

	overrun_c &= c;
	underrun_c &= c;

	if(overrun_c.sat())
	{
		BufferTrace* bt = BufferTrace::make(buf_ap->strip_deref(), index, true, pp_expression,
				line, file, overrun_c, ma.get_cur_iteration_counter());
		error_traces.insert(bt);

	}

	if(underrun_c.sat())
	{
		BufferTrace* bt = BufferTrace::make(buf_ap->strip_deref(), index, false, pp_expression,
				line, file, underrun_c, ma.get_cur_iteration_counter());
		error_traces.insert(bt);
	}


}

void MemoryGraph::report_unsafe_buffer_access(bool user_specified_check,
		AccessPath* buffer, AccessPath* index, Constraint fail_c,
		bool overrun, string pp_exp)
{

	ErrorTrace* et;
	if(user_specified_check) {
		error_code_type ect = (overrun ? ERROR_BUFFER_OVERRUN :
			ERROR_BUFFER_UNDERRUN);
		et = AssertionTrace::make(ect, line, file, fail_c,
				ma.get_cur_iteration_counter(), ma.get_current_stmt_guard());
	}
	else {
		et = BufferTrace::make(buffer, index, overrun,
				pp_exp, line, file, fail_c, ma.get_cur_iteration_counter());
	}
	error_traces.insert(et);

}

void MemoryGraph::get_possible_buffer_sizes(AccessPath* buf_ap,
		set<pair<AccessPath*, Constraint> >& sizes)
{

	assert_context("Getting possible buffer sizes of " + AccessPath::safe_string(
			buf_ap));
	il::type* t = buf_ap->get_type();
	if(t->is_constarray_type())
	{
		il::constarray_type* ct = (il::constarray_type*) t;
		int num_elems = ct->get_num_elems();
		int elem_size = ct->get_elem_type()->get_size()/8;
		int size = num_elems* elem_size;
		c_assert(buf_ap->get_ap_type() == AP_ARRAYREF);
		AccessPath* size_ap = ConstantValue::make(size);
		IndexVariable* iv = ((ArrayRef*) buf_ap)->get_index_var();
		AccessPath* offset = ArithmeticValue::make_times(iv,
				ConstantValue::make(elem_size));
		size_ap = ArithmeticValue::make_minus(size_ap, offset );
		sizes.insert(make_pair(size_ap, Constraint(true)));
	}
	else {
		MemoryLocation* buf_loc = get_location(buf_ap);
		AccessPath* size_ap = buf_loc->get_access_path(SIZE_OFFSET);
		get_value_set(size_ap, sizes);
	}
}


void MemoryGraph::check_buffer_access(sail::Variable* buffer, AccessPath* offset,
				Constraint c, bool user_specified_check, string pp_expression)
{


	assert_context("Checking buffer access to " + buffer->to_string()
			+" at index: " + AccessPath::safe_string(offset));

	if(!user_specified_check && !GlobalAnalysisState::check_buffer()){
		return;
	}

	MemoryLocation* offset_loc = get_location(offset);
	MemoryLocation* buffer_loc = get_location(buffer);
	add_default_edges(buffer_loc);
	add_default_edges(offset_loc);

	set<Edge*>* succs = buffer_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();

	//Constraint fail_c(false);
	Constraint overrun_c(false);
	Constraint underrun_c(false);

	for (; it != succs->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();
		AccessPath* target_ap = e->get_target_ap();
		int target_offset = e->get_target_offset();

		IndexVariable* index = target_ap->find_outermost_index_var(false);

		if (!target_ap->belongs_to_array()) {
			il::type* buf_type = buffer->get_type();
			c_assert(buf_type->is_pointer_type());
			il::type* elem_type = buf_type->get_inner_type();
			c_assert(!elem_type->is_void_type());
			index = this->change_to_array_loc(target, elem_type);

		}


		AccessPath* rep = target->get_access_path(target_offset);
		if(rep->get_ap_type() != AP_ARRAYREF)
		{
			cerr << "Upgrade to array not successful!\n" <<
			 "Initial ap : " << target_ap->to_string() <<
			 " New representative: " << rep->to_string() <<
			 " Line: " << this->line << endl;
			continue;
		}
		int elem_size = ((ArrayRef*) rep)->get_elem_size();

		add_default_edges(target);
		Constraint edge_c = e->get_constraint();

		ivm.rename_index_vars(edge_c);
		Constraint target_c = edge_c;
		if (target_c.unsat())
			continue;

		IndexVariable* index_var = rep->find_outermost_index_var(false);
		if (index_var == NULL)
			continue;

		AccessPath* index_ap = index_var->multiply_ap(elem_size);

		AccessPath* offset_ap = offset_loc->get_access_path(0);
		set<pair<AccessPath*, Constraint> > offset_values;
		get_value_set(offset_ap, offset_values);

		set<pair<AccessPath*, Constraint> > size_values;
		get_possible_buffer_sizes(rep, size_values);


		set<pair<AccessPath*, Constraint> >::iterator it2 = size_values.begin();
		for (; it2 != size_values.end(); it2++) {
			Constraint size_c = it2->second;
			AccessPath* size_ap = it2->first;

			cout << "size_ap:" << size_ap->to_string() << endl;


			set<pair<AccessPath*, Constraint> >::iterator offset_it =
					offset_values.begin();
			for (; offset_it != offset_values.end(); offset_it++) {
				// We overrun buffer if (offset+1) * elem_size > remaining_size

				AccessPath* offset_ap = offset_it->first;
				Constraint offset_c = offset_it->second;
				Constraint violation_constraint = get_overrun_constraint(
						offset_ap, offset_c, elem_size, size_ap, size_c, target_c);
				overrun_c |= violation_constraint;
				Constraint underrun_constraint = get_underrun_constraint(
						offset_ap, offset_c, elem_size, size_ap, size_c, target_c,
						index_var);
				underrun_c |= underrun_constraint;

			}

		}

	}

	if (DEBUG) {
		cout << "Buffer overrun c: " << overrun_c.to_string() << endl;
		cout << "Buffer underrun c: " << underrun_c.to_string() << endl;
	}

	overrun_c &= c;
	underrun_c &= c;


	AccessPath* buf_ap = get_access_path_from_symbol(buffer);
	if(overrun_c.sat())
	{
		cout << "UNSAFE BUFFER ACCESS CONDITION: " << overrun_c << endl;
		report_unsafe_buffer_access(user_specified_check, buf_ap, offset,
				overrun_c, true, pp_expression);
	}

	if(underrun_c.sat())
	{
		report_unsafe_buffer_access(user_specified_check, buf_ap, offset,
				underrun_c, false, pp_expression);
	}

}

bool MemoryGraph::build_string_from_loc(MemoryLocation* loc, int offset,
		string& res) {
	AccessPath* ap = loc->get_access_path(offset);
	if(ap->get_ap_type() == AP_STRING)
	{
		StringLiteral* sl = (StringLiteral*)ap;
		res = sl->get_string_constant();
		return true;
	}
	IndexVariable* i = ap->find_outermost_index_var(true);
	if (i == NULL)
		return false;
	int index = 0;
	while (true) {
		Constraint index_c = ConstraintGenerator::get_eq_constraint(i,
				ConstantValue::make(index));
		AccessPath* target = NULL;
		set<Edge*>::iterator it = loc->get_successors(offset)->begin();
		for (; it != loc->get_successors(offset)->end(); it++) {
			Edge* e = *it;
			AccessPath* cur_target = e->get_target_ap();
			if (!cur_target->is_derived_from_constant())
				return false;
			Constraint res = e->get_constraint() & index_c;
			if (res.sat()) {
				if (target != NULL)
					return false;
				target = cur_target;
			}
		}
		AccessPath* letter = target->strip_deref();
		if (letter->get_ap_type() != AP_CONSTANT)
			return false;
		ConstantValue* cv = (ConstantValue*) letter;
		long int value = cv->get_constant();
		char l = (char) value;
		if (l != '\0')
			res += l;
		else
			break;
		index++;
	}
	return true;
}

void MemoryGraph::process_assign_function(sail::Symbol* source,
		sail::Symbol* target, sail::Symbol* nc, sail::Symbol* sc, Constraint sg) {
	MemoryLocation* nc_loc = get_location(nc);
	set<Edge*>* edges = nc_loc->get_successors(0);
	if (edges->size() != 1)
		return;
	Edge* e = *edges->begin();

	string nc_string;
	bool res = build_string_from_loc(e->get_target_loc(),
			e->get_target_offset(), nc_string);
	if (!res)
		return;

	MemoryLocation* sc_loc = get_location(sc);
	edges = sc_loc->get_successors(0);
	if (edges->size() != 1)
		return;
	e = *edges->begin();

	string sc_string;
	bool res2 = build_string_from_loc(e->get_target_loc(),
			e->get_target_offset(), sc_string);
	if (!res2)
		return;

	Constraint c_nc(nc_string);
	Constraint c_sc(sc_string);

	MemoryLocation* target_loc = get_location(target);

	MemoryLocation* s = get_location(source);
	this->add_default_edges(s);

	edges = s->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		AccessPath* source_ap = e->get_target_ap();
		MemoryLocation* source_loc = e->get_target_loc();
		int source_offset = e->get_target_offset();
		Constraint source_c = e->get_constraint();
		this->add_default_edges(target_loc);
		this->update_outgoing_edges(source_loc, source_offset, sg);
		set<Edge*>::iterator it2 = target_loc->get_successors(0)->begin();
		for (; it2 != target_loc->get_successors(0)->end(); it2++) {
			Edge* e = *it2;

			AccessPath* target_ap = e->get_target_ap();
			MemoryLocation* t_loc = e->get_target_loc();
			int target_offset = e->get_target_offset();
			Constraint cc = source_c & e->get_constraint();
			IndexVariable* source_i =
					source_ap->find_outermost_index_var(false);
			if (source_i != NULL) {
				VariableTerm* v = (VariableTerm*) VariableTerm::make("i");
				map<Term*, Term*> reps;
				reps[v] = source_i;
				c_nc.replace_terms(reps);
				c_sc.replace_terms(reps);
			}
			IndexVariable* target_j =
					target_ap->find_outermost_index_var(false);

			if (target_j != NULL) {
				target_j = IndexVariable::make_target(target_j);
				VariableTerm* v = (VariableTerm*) VariableTerm::make("j");
				map<Term*, Term*> reps;
				reps[v] = target_j;
				c_nc.replace_terms(reps);
				c_sc.replace_terms(reps);
			}
			Constraint res(c_nc, c_sc);
			res &= sg;
			this->put_edge(source_ap, target_ap, res);

		}

	}

}

/*
 * buf->size = a
 */
void MemoryGraph::process_set_buffer_size(sail::Variable* buffer_ptr,
		sail::Symbol* size, Constraint c) {


	MemoryLocation* buf_ptr_loc = get_location(buffer_ptr);
	MemoryLocation* size_loc = get_location(size);
	add_default_edges(buf_ptr_loc);
	add_default_edges(size_loc);

	set<Edge*>* succs = buf_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for (; it != succs->end(); it++) {
		Edge* e = *it;
		Constraint edge_c = e->get_constraint();
		MemoryLocation* buf_loc = e->get_target_loc();
		if (!buf_loc->get_representative_access_path()->belongs_to_array()) {
			il::type* buf_type = buffer_ptr->get_type();
			c_assert(buf_type->is_pointer_type());
			il::type* elem_type = buf_type->get_inner_type();
			c_assert(!elem_type->is_void_type());
			change_to_array_loc(buf_loc, elem_type);
		}

		il::type* elem_t =
				buf_loc->get_representative_access_path()->get_type();
		if (elem_t->is_array_type())
			elem_t = elem_t->get_inner_type();

		AccessPath* _array = e->get_target_ap();
		c_assert(_array->get_ap_type() == AP_ARRAYREF);
		ArrayRef* array = (ArrayRef*) _array;
		int elem_size = array->get_elem_size();
		IndexVariable* iv = array->get_index_var();
		c_assert(iv != NULL);

		Constraint update_c = edge_c & c;
		if (update_c.unsat())
			continue;
		update_outgoing_edges(buf_loc, SIZE_OFFSET, update_c, true);

		set<Edge*>* size_edges = size_loc->get_successors(0);
		set<Edge*>::iterator size_it = size_edges->begin();
		for (; size_it != size_edges->end(); size_it++) {
			Edge* edge = *size_it;
			AccessPath* size_ap = edge->get_target_ap();
			size_ap = size_ap->strip_deref()->multiply_ap(elem_size);
			size_ap = ArithmeticValue::make_minus(size_ap,
					ArithmeticValue::make_times(iv, ConstantValue::make(elem_size)));
			size_ap = size_ap->add_deref();
			Constraint index_eq = ConstraintGenerator::get_eq_constraint(iv,
					IndexVariable::make_target(iv));


			MemoryLocation* size_target = get_location(size_ap);
			Constraint size_target_c = edge->get_constraint();
			Constraint add_edge_c = size_target_c & update_c & index_eq;

			put_edge(edge->get_source_loc(), edge->get_source_offset(),
					add_edge_c, buf_loc, size_target, SIZE_OFFSET,
					edge->get_target_offset(), false, true);

		}

	}
}

/*
 * v=a[i].f
 */
void MemoryGraph::process_array_ref_read(Variable* lhs, Variable* array,
		AccessPath* index, int offset, Constraint c, string pp_exp) {
	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* array_loc = get_location(array);


	add_default_edges(lhs_loc);
	add_default_edges(array_loc);

	AccessPath* array_ap = array_loc->get_representative_access_path();
	check_static_buffer_access(array_ap, index, c, pp_exp);



	if (!array_loc->add_offset(offset, lhs->get_type())) {
		report_type_inconsistency(array, offset);
		DisplayTag dt(pp_exp, this->line, this->file);
		assign_unmodeled_value(lhs_loc, c, lhs->get_type(), UNM_IMPRECISE, dt);
		return;
	}

	update_outgoing_edges(lhs_loc, c);
	Constraint index_constraint = get_index_constraint(index, array_loc);

	int end_offset = lhs_loc->get_last_offset() + offset;
	map<int, set<Edge*> *>::iterator it =
			array_loc->get_successor_map().begin();

	for (; it != array_loc->get_successor_map().end(); it++) {
		int cur_offset = it->first;
		if (cur_offset < offset)
			continue;
		if (cur_offset > end_offset)
			break;


		string cur_pp = pp_exp;
		AccessPath* cur_ap = array_loc->get_access_path(cur_offset);
		if(cur_ap->get_ap_type() == AP_FIELD) {
			FieldSelection* fs = (FieldSelection*) cur_ap;
			cur_pp += fs->get_field_name();

		}
		check_memory_access(cur_ap, c & index_constraint, cur_pp);

		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			Edge* e = *it2;
			Constraint edge_c = e->get_constraint();
			Constraint assign_c = c & edge_c & index_constraint;
			Edge* new_edge = put_edge(e->get_source_loc(),
					e->get_source_offset(), assign_c, lhs_loc,
					e->get_target_loc(), cur_offset - offset,
					e->get_target_offset());

		}
	}

	check_invariants(lhs_loc, offset, this);

}

void MemoryGraph::process_array_ref_read(sail::Variable* lhs,
		sail::Variable* array, sail::Symbol* index, int offset, Constraint c) {


	Variable* v_lhs = Variable::make_program_var(lhs);
	Variable* v_rhs = Variable::make_program_var(array);
	AccessPath* index_ap = get_access_path_from_symbol(index);
	string pp = array->to_string(true) + "[" + index->to_string(true) +"]";
	process_array_ref_read(v_lhs, v_rhs, index_ap, offset, c, pp);

}

/*
 * a[i].f = v
 */
void MemoryGraph::process_array_ref_write(sail::Variable* lhs,
		sail::Symbol* index, int offset, sail::Symbol* rhs, Constraint& c) {
	MemoryLocation* array_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);


	check_memory_access(get_access_path_from_symbol(rhs), c,
			rhs->to_string(true));

	if (!array_loc->add_offset(offset, lhs->get_type(), rhs->is_constant())) {
		report_type_inconsistency(get_access_path_from_symbol(rhs), offset);
		return;
	}

	add_default_edges(array_loc);
	add_default_edges(rhs_loc);
	AccessPath* index_ap = get_access_path_from_symbol(index);
	AccessPath* array_ap = array_loc->get_representative_access_path();


	string pp_exp = lhs->to_string(true) + "[" + index->to_string(true) + "]";
	check_static_buffer_access(array_ap, index_ap, c, pp_exp);

	Constraint index_constraint = get_index_constraint(index_ap, array_loc);
	Constraint update_cond = c & index_constraint;

	map<int, set<Edge*>*>::iterator it = rhs_loc->get_successor_map().begin();
	for (; it != rhs_loc->get_successor_map().end(); it++) {
		int rhs_offset = it->first;
		update_outgoing_edges(array_loc, offset + it->first, update_cond);
		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			Edge* e = *it2;
			Constraint edge_c = e->get_constraint();
			Constraint assign_c = update_cond & edge_c;
			if (assign_c.unsat())
				continue;
			put_edge(e->get_source_loc(), e->get_source_offset(), assign_c,
					array_loc, e->get_target_loc(), offset + it->first,
					e->get_target_offset());

		}

		check_invariants(array_loc, offset + it->first, this);
	}

}

// -------------ADT related methods------------------------

Constraint MemoryGraph::get_adt_index_constraint(AccessPath* key,
		AbstractDataStructure* adt_loc, Constraint incoming_c,
		bool lookup_index)
{



	IndexVariable* source_index = adt_loc->get_index_var();
	set<pair<AccessPath*, Constraint> > key_values;
	get_value_set(key, key_values);
	Constraint index_constraint(false);
	set<pair<AccessPath*, Constraint> >::iterator it = key_values.begin();
	set<VariableTerm*> to_eliminate;
	for (; it != key_values.end(); it++)
	{
		AccessPath* key_value = it->first;
		Constraint value_c = it->second;



		/*
		 * The loop below exists to prevent naming collisions between index
		 * variables.
		 */
		set<IndexVariable*> index_vars;
		key_value->get_nested_index_vars(index_vars);
		map<AccessPath*, AccessPath*> subs;
		set<IndexVariable*>::iterator it2 = index_vars.begin();
		map<Term*, Term*> sub_terms;
		for(; it2!= index_vars.end(); it2++) {
			IndexVariable* iv = *it2;
			if(!iv->is_source()) continue;
			IndexVariable* fv = IndexVariable::make_free();
			subs[iv] = fv;
			to_eliminate.insert(fv);
			sub_terms[iv] = fv;
		}

		key_value = key_value->replace(subs);
		value_c.replace_terms(sub_terms);


		AccessPath* pos;
		if(lookup_index) pos = adt_loc->get_index_from_key(key_value);
		else pos = key_value;
		AccessPath* new_index = ArithmeticValue::make_minus(source_index, pos);
		Constraint pos_constraint = incoming_c;
		pos_constraint.replace_term(source_index->to_term(),
				new_index->to_term());
		Constraint cur_c = pos_constraint & value_c;
		index_constraint |= cur_c;
	}

	index_constraint.eliminate_evars(to_eliminate);
	return index_constraint;

}

/*
 * Checks that adt_var is a pointer to an abstract data type
 */
bool MemoryGraph::check_adt_usage(sail::Variable* v, string function_name)
{
	il::type* t = v->get_type();
	if(!t->is_pointer_type()) {
		report_error(ERROR_ILLEGAL_ADT_FUNCTION, function_name +
				" takes a pointer to the abstract data structure." );
		return false;
	}
	il::type* inner_t = t->get_inner_type();
	if(!inner_t->is_abstract_data_type()) {
		report_error(ERROR_ILLEGAL_ADT_FUNCTION, "Non-abstract data type passed "
				"to "  + function_name +  "." + " Type of var: " + inner_t->to_string() );
		return false;
	}
	return true;
}




/*
 * ADT_INSERT(adt, key, value);
 */
void MemoryGraph::process_adt_insert(sail::Variable* v, sail::Symbol* key,
		sail::Symbol* value, Constraint stmt_guard)
{




	if(!check_adt_usage(v, "ADT_INSERT")) return;
	check_ptr_deref(v, stmt_guard, v->to_string(true));

	MemoryLocation* adt_ptr_loc = get_location(v);
	MemoryLocation* key_loc = get_location(key);
	MemoryLocation* value_loc = get_location(value);
	add_default_edges(adt_ptr_loc);
	add_default_edges(key_loc);
	add_default_edges(value_loc);

	set<Edge*>* adt_ptr_edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = adt_ptr_edges->begin();
	for(; it!= adt_ptr_edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();
		add_default_edges(adt_loc);



		int offset = e->get_target_offset();
		c_assert(offset == 0);

		Constraint edge_c = e->get_constraint();
		Constraint cur_c = edge_c & stmt_guard;
		if(cur_c.unsat_discard()) continue;

		AbstractDataStructure* adt_ap = get_adt_ap(get_access_path_from_symbol(v),
				adt_loc, offset);

		if(adt_ap == NULL) {
			continue;
		}

		Constraint new_c = (*adt_loc->get_successors(0)->begin())->get_constraint();

		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();



		this->add_default_edges(adt_loc);
		edge_c = e->get_constraint(); // might have changed if upgraded
		ivm.rename_index_vars(edge_c);




		Constraint index_c = get_adt_index_constraint(
				get_access_path_from_symbol(key), adt_ap, edge_c, true);



		Constraint update_c = stmt_guard & index_c;




		map<int, set<Edge*>* >& succs= value_loc->get_successor_map();
		map<int, set<Edge*>* >::iterator succs_it = succs.begin();
		for(; succs_it != succs.end(); succs_it++)
		{
			int cur_offset =  succs_it->first;
			update_outgoing_edges(adt_loc, cur_offset, update_c);


			/*
			 * For every position that may be affected, add edges
			 * for each of the possible values of "value"
			 */
			set<Edge*>* value_edges = succs_it->second;
			set<Edge*>::iterator it2 = value_edges->begin();
			for(; it2!= value_edges->end(); it2++)
			{
				Edge* value_e = *it2;
				MemoryLocation* value_loc = value_e->get_target_loc();
				int value_offset = value_e->get_target_offset();
				Constraint value_c = value_e->get_constraint();
				Constraint put_c = value_c & update_c;

				put_edge(value_e->get_source_loc(), value_e->get_source_offset(),
						put_c, adt_loc, value_loc, cur_offset, value_offset);
			}
		}





	}

}

/*
 * ADT_READ(result, adt, key);
 */
void MemoryGraph::process_adt_read(sail::Variable* v, sail::Variable* adt,
		sail::Symbol* key, Constraint stmt_guard, bool get_index_from_key)
{


	if(!check_adt_usage(adt, "ADT_READ")) return;
	check_ptr_deref(adt, stmt_guard, adt->to_string(true));
	AccessPath* res = get_access_path_from_symbol(v);
	AccessPath* adt_ap = get_access_path_from_symbol(adt);
	AccessPath* key_ap = get_access_path_from_symbol(key);
	process_adt_read(res, adt_ap, key_ap, stmt_guard, get_index_from_key, 0);



}

void MemoryGraph::process_adt_read_ref(sail::Variable* v, sail::Variable* adt,
		sail::Symbol* key, Constraint stmt_guard, bool get_index_from_key)
{
	if(!check_adt_usage(adt, "ADT_READ")) return;
	check_ptr_deref(adt, stmt_guard, adt->to_string(true));
	AccessPath* res = get_access_path_from_symbol(v);
	AccessPath* adt_ap = get_access_path_from_symbol(adt);
	AccessPath* key_ap = get_access_path_from_symbol(key);
	process_adt_read_ref(res, adt_ap, key_ap, stmt_guard, get_index_from_key);
}

/*
 * If we are trying to do an ADT operation on adt_ptr_ap,
 * this function does the necessary upgrades and checks, and returns
 * NULL if access is illegal.
 */
AbstractDataStructure* MemoryGraph::get_adt_ap(AccessPath* adt_ptr_ap,
		MemoryLocation* adt_loc, int offset)
{
	assert_context("Getting access path for ADT: " +
			AccessPath::safe_string(adt_ptr_ap) + "  offset: " +
			int_to_string(offset) + " ADT LOC: " + adt_loc->to_string(false));



	//c_assert(offset == 0);

	// AccessPath* _adt_ap =adt_loc->get_representative_access_path();
	AccessPath* _adt_ap =adt_loc->get_access_path(offset);
	cout << "type: " << *adt_ptr_ap->get_type() << endl;
	il::type* adt_type = adt_ptr_ap->get_type()->get_inner_type();



	c_assert(adt_type != NULL);

	if(_adt_ap->get_ap_type() != AP_ADT)
	{
		change_to_adt_loc(adt_loc, offset, adt_type);
		_adt_ap = adt_loc->get_representative_access_path();
		c_assert(_adt_ap->get_ap_type() == AP_ADT);
	}




	AbstractDataStructure* adt_ap = (AbstractDataStructure*) _adt_ap;
	if(adt_type->is_void_type()) return adt_ap;
	/*if(!(*adt_ap->get_type() ==  *adt_type))
	{
		report_error(ERROR_ILLEGAL_CAST, "Inconsistent ADT types used: "
				+ adt_ap->get_type()->to_string() + " is not compatible with "
				+ adt_type->to_string());

		return NULL;
	}*/
	return adt_ap;

}

void MemoryGraph::process_adt_read(AccessPath* v, AccessPath* adt,
		AccessPath* key, Constraint stmt_guard, bool get_index_from_key,
		int val_offset)
{



	assert_context("Processing adt read: " + v->to_string() + " = " +
			adt->to_string() + "[" + key->to_string() + "]" + " val offset: " +
			int_to_string(val_offset));



	MemoryLocation* res_loc = get_location(v);
	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* key_loc = get_location(key);
	add_default_edges(adt_ptr_loc);
	add_default_edges(key_loc);
	add_default_edges(res_loc);
	update_outgoing_edges(res_loc, stmt_guard);




	set<Edge*>* adt_ptr_edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = adt_ptr_edges->begin();
	for(; it!= adt_ptr_edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();
		int offset = e->get_target_offset();
		Constraint edge_c = e->get_constraint();
		Constraint cur_c = edge_c & stmt_guard;

		if(cur_c.unsat_discard()) continue;

		AbstractDataStructure* adt_ap = get_adt_ap(adt, adt_loc, offset);
		if(adt_ap == NULL) {
			DisplayTag dt(adt->to_string() + "[" + key->to_string() + "]",
					this->line, this->file);
			assign_unmodeled_value(res_loc, cur_c, v->get_type(),
					UNM_INVALID, dt);
			continue;
		}


		add_default_edges(adt_loc);




		edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);



		Constraint index_c= get_adt_index_constraint(key, adt_ap, edge_c,
					get_index_from_key);









		Constraint read_c = stmt_guard & index_c;





		/*
		 * Iterate over outgoing edges of the adt
		 */
		map<int, AccessPath*>::iterator source_it = res_loc->get_access_path_map().begin();
		for(; source_it != res_loc->get_access_path_map().end(); source_it++)
		{
			int source_offset = source_it->first;
			int read_offset = offset+val_offset+source_offset;
			assert_context("Updating field " + int_to_string(read_offset) +
					" of location: " + adt_loc->to_string(false));

			if(!adt_loc->has_field_at_offset(read_offset)) continue;
			set<Edge*>* adt_value_edges = adt_loc->get_successors(read_offset);
			set<Edge*>::iterator it2 = adt_value_edges->begin();
			for(; it2!= adt_value_edges->end(); it2++)
			{
				Edge* val_e = *it2;
				MemoryLocation* val_loc = val_e->get_target_loc();
				int edge_target_offset = val_e->get_target_offset();
				Constraint val_c = val_e->get_constraint();

				Constraint put_c = read_c & val_c;
				put_edge(val_e->get_source_loc(), val_e->get_source_offset(), put_c,
						res_loc, val_loc, source_offset, edge_target_offset );
			}


		}



	}
}


void MemoryGraph::process_adt_read_ref(AccessPath* v, AccessPath* adt,
		AccessPath* key, Constraint stmt_guard, bool get_index_from_key)
{

	il::type* value_t =  adt->get_type()->get_inner_type()->get_adt_value_type();
	if(value_t != NULL && value_t->is_abstract_data_type()){
		process_adt_read(v, adt, key, stmt_guard, get_index_from_key, 0);
		return;
	}


	MemoryLocation* res_loc = get_location(v);
	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* key_loc = get_location(key);
	add_default_edges(adt_ptr_loc);
	add_default_edges(key_loc);
	add_default_edges(res_loc);
	update_outgoing_edges(res_loc, stmt_guard);


	set<Edge*>* adt_ptr_edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = adt_ptr_edges->begin();
	for(; it!= adt_ptr_edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();
		int offset = e->get_target_offset();
		Constraint edge_c = e->get_constraint();
		Constraint cur_c = edge_c & stmt_guard;

		if(cur_c.unsat_discard()) continue;

		AbstractDataStructure* adt_ap = get_adt_ap(adt, adt_loc, offset);
		if(adt_ap == NULL) {
			DisplayTag dt(adt->to_string() + "[" + key->to_string() + "]",
					line, file);
			assign_unmodeled_value(res_loc, cur_c, v->get_type(),
					UNM_INVALID, dt);
			continue;
		}


		add_default_edges(adt_loc);
		edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);
		Constraint index_c= get_adt_index_constraint(key, adt_ap, edge_c,
					get_index_from_key);

		IndexVarManager::rename_source_to_target_index(index_c);



		Constraint read_c = stmt_guard & index_c;

		put_edge(NULL, 0, read_c, res_loc, adt_loc, 0, offset);

	}
}




/*
 * ADT_SET_SIZE(adt, size);
 */
void MemoryGraph::process_adt_set_size(sail::Variable* adt, sail::Symbol* size,
		Constraint stmt_guard)
{
	if(!check_adt_usage(adt, "ADT_SET_SIZE")) return;
	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* size_loc = get_location(size);
	add_default_edges(adt_ptr_loc);
	add_default_edges(size_loc);

	set<Edge*>* edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();

		add_default_edges(adt_loc);
		Constraint edge_c = e->get_constraint();
		Constraint sat_c = edge_c & stmt_guard;
		if(sat_c.unsat_discard()) continue;






		AbstractDataStructure* adt_ap = get_adt_ap(
				get_access_path_from_symbol(adt), adt_loc,
				e->get_target_offset());
		if(adt_ap == NULL) {
			continue;
		}


		edge_c = e->get_constraint(); // might have changed if upgraded
		Constraint cur_c = stmt_guard & edge_c;

		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();
		add_default_edges(adt_loc);

		update_outgoing_edges(adt_loc, SIZE_OFFSET, stmt_guard, true);

		/*
		 * Go through possible size values
		 */
		set<Edge*>* size_targets = size_loc->get_successors(0);
		set<Edge*>::iterator it2 = size_targets->begin();
		for(; it2!= size_targets->end(); it2++)
		{
			Edge* e2 = *it2;
			AccessPath* val = e2->get_target_ap();
			MemoryLocation* val_loc = get_location(val);

			Constraint put_c = e2->get_constraint() & cur_c;

			put_edge(e2->get_source_loc(), e2->get_source_offset(),
					put_c, adt_loc,val_loc, SIZE_OFFSET, 0, false, true);
		}

	}
}



void MemoryGraph::process_adt_get_size(sail::Variable* res, sail::Variable* adt,
		Constraint stmt_guard)
{
	if(!check_adt_usage(adt, "ADT_GET_SIZE")) return;
	AccessPath* res_ap = get_access_path_from_symbol(res);
	AccessPath* adt_ap = get_access_path_from_symbol(adt);
	process_adt_get_size(res_ap, adt_ap, stmt_guard);

}

void MemoryGraph::process_adt_get_size(AccessPath* res, AccessPath* adt,
		Constraint stmt_guard)
{



	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* res_loc = get_location(res);
	add_default_edges(adt_ptr_loc);
	add_default_edges(res_loc);
	update_outgoing_edges(res_loc, 0, stmt_guard);

	set<Edge*>* edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++)
	{
		Edge* e = *it;


		MemoryLocation* adt_loc = e->get_target_loc();
		add_default_edges(adt_loc);
		Constraint edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);
		Constraint c = edge_c & stmt_guard;
		if(c.unsat_discard()) continue;

		AbstractDataStructure* adt_ap = get_adt_ap(adt, adt_loc,
				e->get_target_offset());
		if(adt_ap == NULL) {
			DisplayTag dt("Size of " + adt->to_string(), line, file);
			assign_unmodeled_value(res_loc, c, res->get_type(), UNM_INVALID, dt);
			continue;
		}
		edge_c = e->get_constraint();


		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();
		add_default_edges(adt_loc);

		c = edge_c & stmt_guard;

		/*
		 * Get outgoing edges from the size field
		 */
		c_assert(adt_loc->has_field_at_offset(SIZE_OFFSET));
		set<Edge*>* size_edges = adt_loc->get_successors(SIZE_OFFSET);
		set<Edge*>::iterator it2 = size_edges->begin();
		for(; it2!= size_edges->end(); it2++)
		{
			Edge* cur_e = *it2;
			Constraint cur_c = cur_e->get_constraint();
			Constraint put_c = cur_c & c;
			IndexVarManager::eliminate_source_vars(put_c);
			put_edge(cur_e->get_source_loc(), cur_e->get_source_offset(), put_c,
					res_loc, cur_e->get_target_loc(), 0,
					cur_e->get_target_offset());


		}

	}
}

void MemoryGraph::process_adt_remove(sail::Variable* adt, sail::Symbol* key,
		Constraint stmt_guard)
{
	if(!check_adt_usage(adt, "ADT_REMOVE")) return;
	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* key_loc = get_location(key);
	add_default_edges(adt_ptr_loc);
	add_default_edges(key_loc);
	AccessPath* key_ap = key_loc->get_access_path(0);
	Constraint update_c(false);

	set<Edge*>* edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();
		add_default_edges(adt_loc);
		int offset = e->get_target_offset();
		Constraint edge_c = e->get_constraint();



		AbstractDataStructure* adt_ap = get_adt_ap(
				get_access_path_from_symbol(adt), adt_loc,
				e->get_target_offset());
		if(adt_ap == NULL) {
			continue;
		}

		edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);
		Constraint cur_c = edge_c & stmt_guard;
		if(cur_c.unsat_discard()) continue;



		// remove should be absolute
		IndexVariable* index_var = adt_ap->get_index_var();
		Constraint neqz_c = ConstraintGenerator::get_neqz_constraint(index_var);
		Constraint test_c = neqz_c & edge_c;
		if(test_c.sat_discard()) {
			report_error(ERROR_ILLEGAL_ADT_FUNCTION, "ADT_REMOVE does not "
					"allow relative offsets." );
			return;
		}

		Constraint index_c = get_adt_index_constraint(key_ap, adt_ap, edge_c,
				true);
		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();

		Constraint update_c = index_c & stmt_guard;
		update_outgoing_edges(adt_loc, offset, update_c);
		put_edge(NULL, 0, update_c, adt_loc,
				get_location(Nil::make()->add_deref()), 0, 0);

	}

}

void MemoryGraph::process_adt_contains(sail::Variable* res,
		sail::Variable* adt, sail::Symbol* key, Constraint stmt_guard)
{
	if(!check_adt_usage(adt, "ADT_CONTAINS")) return;
	MemoryLocation* adt_ptr_loc = get_location(adt);
	MemoryLocation* key_loc = get_location(key);
	MemoryLocation* res_loc = get_location(res);
	add_default_edges(adt_ptr_loc);
	add_default_edges(key_loc);
	add_default_edges(res_loc);
	update_outgoing_edges(res_loc, stmt_guard);
	AccessPath* key_ap = key_loc->get_access_path(0);



	Constraint contains_c(false);

	set<Edge*>* adt_edges = adt_ptr_loc->get_successors(0);
	set<Edge*>::iterator it = adt_edges->begin();
	for(; it!= adt_edges->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* adt_loc = e->get_target_loc();
		add_default_edges(adt_loc);



		int adt_offset = e->get_target_offset();


		AbstractDataStructure* adt_ap = get_adt_ap(
				get_access_path_from_symbol(adt), adt_loc,
				e->get_target_offset());
		if(adt_ap == NULL) {
			continue;
		}
		Constraint edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);
		edge_c.eliminate_evar((VariableTerm*)adt_ap->get_index_var()->to_term());

		Constraint cur_contains_c(false);
		Constraint no_offset_c = ConstraintGenerator::get_eqz_constraint(
				adt_ap->get_index_var());
		Constraint index_c = get_adt_index_constraint(key_ap, adt_ap,
				no_offset_c, true);

		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();
		AccessPath* new_adt_ap = adt_ap->add_deref();

		this->add_default_edges(adt_loc);

		set<Edge*>* cur_edges = adt_loc->get_successors(adt_offset);
		set<Edge*>::iterator it2 = cur_edges->begin();
		for(; it2!= cur_edges->end(); it2++)
		{
			Edge* e2 = *it2;
			AccessPath* target_ap = e2->get_target_ap();
			if(target_ap->strip_deref()->get_ap_type() == AP_NIL)
				continue;
			Constraint e2_c = e2->get_constraint();

			Constraint not_is_nil_c = e2_c;


			AccessPath* default_target_ap = target_ap;
			default_target_ap = default_target_ap->strip_deref();
			while(default_target_ap->get_ap_type() == AP_FIELD)
				default_target_ap = default_target_ap->get_inner();
			default_target_ap = default_target_ap->add_deref();



			if(get_default_target(new_adt_ap) == default_target_ap) {

				AccessPath* is_nil_fn = FunctionValue::make_is_nil(
						default_target_ap->strip_deref());

				Constraint not_nil =
						ConstraintGenerator::get_eqz_constraint(is_nil_fn);

				not_is_nil_c &= not_nil;
			}


			cur_contains_c |= (not_is_nil_c);
		}
		cur_contains_c &= (edge_c & index_c);
		contains_c |= cur_contains_c;

	}
	


	IndexVarManager::eliminate_source_vars(contains_c);
	IndexVarManager::eliminate_target_vars(contains_c);

	MemoryLocation* true_loc = get_location(ConstantValue::make(1)->add_deref());
	MemoryLocation* false_loc = get_location(ConstantValue::make(0)->add_deref());

	put_edge(NULL, 0, contains_c & stmt_guard, res_loc, true_loc, 0, 0);
	put_edge(NULL, 0, (!contains_c) & stmt_guard, res_loc, false_loc, 0, 0);

}




//----------------------------------------------

/*
 * a.f=b
 */
void MemoryGraph::process_assignment(sail::Symbol* lhs, int lhs_offset,
		sail::Symbol* rhs, Constraint c) {



	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	check_memory_access(rhs_ap, c, rhs->to_string(true));

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);




	if ( !lhs_loc->add_offset(lhs_offset, rhs->get_type(), rhs->is_constant())) {
		report_type_inconsistency(get_access_path_from_symbol(lhs), lhs_offset);
		return;
	}


	add_default_edges(rhs_loc);
	add_default_edges(lhs_loc);


	map<int, set<Edge*> *>::iterator it = rhs_loc->get_successor_map().begin();
	for (; it != rhs_loc->get_successor_map().end(); it++) {

		int rhs_offset = it->first;
		if(rhs_offset < 0) continue;

		update_outgoing_edges(lhs_loc, lhs_offset + it->first, c);
		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			Edge* e = *it2;
			Constraint edge_c = e->get_constraint();
			Constraint cur_c = edge_c & c;
			put_edge(e->get_source_loc(), e->get_source_offset(), cur_c,
					lhs_loc, e->get_target_loc(), lhs_offset
							+ e->get_source_offset(), e->get_target_offset());
		}

	}
}

/*
 * *(a + offset) = b
 */
void MemoryGraph::process_store(sail::Variable* lhs, int lhs_offset,
		sail::Symbol* rhs, Constraint c) {




	bool is_const = rhs->is_constant();
	check_ptr_deref(lhs, c, lhs->to_string(true));
	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	check_memory_access(rhs_ap, c, rhs->to_string(true));
	AccessPath* lhs_ap = get_access_path_from_symbol(lhs);
	check_memory_access(lhs_ap, c, lhs->to_string(true));

	bool is_buf_ptr = false;

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);



	add_default_edges(lhs_loc);
	add_default_edges(rhs_loc);



	map<int, set<Edge*>*> rhs_targets = rhs_loc->get_successor_map();


	set<Edge*>* lhs_targets = lhs_loc->get_successors(0);

	set<Edge*>::iterator it = lhs_targets->begin();
	for (; it != lhs_targets->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();




		int target_offset = e->get_target_offset() + lhs_offset;
		AccessPath* target_ap = target->get_access_path(target_offset);



		Constraint edge_c = e->get_constraint();
		ivm.rename_index_vars(edge_c);

		bool copy_all_elems = (target_ap->get_type()->is_abstract_data_type());

		if(copy_all_elems)
		{
			IndexVariable* outer_index =
					target_ap->find_outermost_index_var(true);

			if(outer_index != NULL) {
				edge_c.eliminate_evar(outer_index);
			}
		}


		Constraint new_c = edge_c & c;

		if(new_c.unsat_discard()) continue;










		if(!target->has_field_at_offset(target_offset)) {
			report_error(ERROR_UNMODELED_STORE,
					"Attempting to store into " + target->to_string() +
					" at non-existing offset " + int_to_string(target_offset));
			continue;
		}





		if(target_ap->get_ap_type() == AP_ARRAYREF) {
			is_buf_ptr = true;
		}

		if (target_ap->get_base()->get_ap_type() == AP_UNMODELED) {
			UnmodeledValue* uv = (UnmodeledValue*) target_ap->get_base();
			if (uv->is_imprecise()) {
				report_error(ERROR_UNMODELED_STORE,
						"Cannot process store into an unmodeled value");
				continue;
			}
		}

		

		if (!target->add_offset(target_offset, rhs->get_type(), is_const)) {
			add_default_edges(target);
			report_type_inconsistency(
					target->get_representative_access_path(), target_offset);
			DisplayTag dt(rhs->to_string(true), line, file);
			assign_unmodeled_value(target, edge_c, rhs->get_type(),
					UNM_IMPRECISE, dt);
			continue;
		}
		add_default_edges(target);


		map<int, set<Edge*>*>::iterator it2 = rhs_targets.begin();
		for (; it2 != rhs_targets.end(); it2++) {
			int rhs_offset = it2->first;
			if(rhs_offset < 0) continue;
			int cur_offset = target_offset + it2->first;

			AccessPath* target_ap = target->get_access_path(cur_offset);
		//	ea.add_stored(target_ap->strip_deref());

			update_outgoing_edges(target, target_offset + it2->first, new_c);
			set<Edge*>::iterator it3 = it2->second->begin();

			for (; it3 != it2->second->end(); it3++) {
				Edge* rhs_edge = *it3;
				MemoryLocation* rhs_target_loc = rhs_edge->get_target_loc();
				int rhs_offset = rhs_edge->get_target_offset();


				Constraint rhs_target_c = rhs_edge->get_constraint() & new_c;


				put_edge(rhs_edge->get_source_loc(),
						rhs_edge->get_source_offset(), rhs_target_c, target,
						rhs_target_loc, target_offset + it2->first, rhs_offset);

			}
			check_invariants(target, target_offset + it2->first, this);
		}




	}

	string pp_exp = lhs->to_string(true) + "[0]";
	if(is_buf_ptr) {
		check_buffer_access(lhs, ConstantValue::make(0), c, false,pp_exp);

	}

}

/*
 * a = *(b+offset)
 */
void MemoryGraph::process_load(sail::Variable* lhs, sail::Symbol* rhs,
		int offset, Constraint c) {


	AccessPath* v_lhs = Variable::make_program_var(lhs);
	AccessPath* v_rhs = get_access_path_from_symbol(rhs);

	string rhs_pp = rhs->to_string(true);

	check_memory_access(v_rhs, c, rhs_pp);
	if(rhs->is_variable())
		check_ptr_deref(static_cast<sail::Variable*>(rhs), c, rhs_pp);


	bool check_buffer = process_load(v_lhs, v_rhs, offset, c, rhs_pp);

	if(check_buffer && rhs->is_variable()) {
		string pp_exp = rhs->to_string(true) + "[0]";
		check_buffer_access(static_cast<sail::Variable*>(rhs),
				ConstantValue::make(0), c, false, pp_exp);
	}
}







bool MemoryGraph::process_load(AccessPath* lhs, AccessPath* rhs, int offset,
		Constraint c, string pp_exp) {



	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);




	add_default_edges(lhs_loc);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);


	bool copy_all_elems = (lhs->get_type()->is_abstract_data_type());

	bool is_buf_ptr = false;


	set<Edge*>::iterator it1 = rhs_loc->get_successors(0)->begin();
	for (; it1 != rhs_loc->get_successors(0)->end(); it1++) {
		Edge* e1 = *it1;


		if(e1->get_target_ap()->get_ap_type() == AP_ARRAYREF)
			is_buf_ptr = true;
		MemoryLocation* b_target = e1->get_target_loc();
		int start_offset = offset + e1->get_target_offset();
		int end_offset = start_offset + lhs_loc->get_last_offset();
		Constraint c1 = e1->get_constraint();
		ivm.rename_index_vars(c1);



		if (!b_target->add_offset(start_offset, lhs->get_type())) {
			report_type_inconsistency(
					b_target->get_representative_access_path(),
					start_offset);
			DisplayTag dt(pp_exp, line, file);
			assign_unmodeled_value(lhs_loc, c & c1, lhs->get_type(),
					UNM_IMPRECISE, dt);
			continue;
		}

		add_default_edges(b_target);


		map<int, set<Edge*>*>::iterator it2 =
				b_target->get_successor_map().begin();
		for (; it2 != b_target->get_successor_map().end(); it2++) {
			int cur_offset = it2->first;


			if (cur_offset < start_offset) {
				continue;
			}
			if (cur_offset > end_offset) {
				break;
			}



			AccessPath* cur_ap = b_target->get_access_path(cur_offset);
			ea.add_loaded(cur_ap->strip_deref());

			check_memory_access(b_target->get_access_path(cur_offset), c1 & c,
					"*" + pp_exp);


			set<Edge*>::iterator it3 = it2->second->begin();
			for (; it3 != it2->second->end(); it3++) {
				Edge* cur_edge = *it3;
				Constraint edge_c = cur_edge->get_constraint();



				if(copy_all_elems)
				{
					AccessPath* ap = cur_edge->get_target_ap();

					IndexVariable* outer_index =
							ap->find_outermost_index_var(false);

					outer_index = IndexVariable::make_target(outer_index);

					if(outer_index != NULL) {
						edge_c.eliminate_evar(outer_index);
					}
				}

				Constraint cur_c = edge_c & c1 & c;

				put_edge(cur_edge->get_source_loc(),
						cur_edge->get_source_offset(), cur_c, lhs_loc,
						cur_edge->get_target_loc(), cur_offset - start_offset,
						cur_edge->get_target_offset());
			}

		}

	}

	check_invariants(lhs_loc, 0, this);
	return is_buf_ptr;
}

AccessPath* MemoryGraph::get_access_path_from_symbol(sail::Symbol* s) {

	if (s->is_constant()) {
		sail::Constant* c = (sail::Constant*) s;
		if(c->is_integer()) {
			return  ConstantValue::make(c->get_integer());
		}
		DisplayTag dt("Non-integer constant", this->line, this->file);
 		return UnmodeledValue::make_imprecise(s->get_type(), dt);
	} else {
		sail::Variable* v = (sail::Variable*) s;
		AccessPath* res =  Variable::make_program_var(v);
		return res;
	}
}

ConstraintGenerator& MemoryGraph::get_cg()
{
	return cg;
}

void MemoryGraph::add_error_trace(ErrorTrace* et)
{
	error_traces.insert(et);
}

void MemoryGraph::process_cast(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint c) {

	Variable* v_lhs = Variable::make_program_var(lhs);

	cout << "type 1: " << v_lhs->get_type()->to_string() << "  type 2: "
			<< lhs->get_type()->to_string() << endl;
	c_assert(*v_lhs->get_type() == *lhs->get_type());
	AccessPath* ap_rhs = get_access_path_from_symbol(rhs);
	process_cast(v_lhs, ap_rhs, c);

}

void MemoryGraph::process_cast(Variable* lhs, AccessPath* rhs, Constraint c) {
	MemoryLocation* rhs_loc = get_location(rhs);
	MemoryLocation* lhs_loc = get_location(lhs);
	add_default_edges(rhs_loc);
	add_default_edges(lhs_loc);
	bool res = true;
	if (lhs->get_type()->is_pointer_type()) {
		il::pointer_type* lhs_type = (il::pointer_type*) lhs->get_type();
		if (rhs->get_type()->is_pointer_type())
			res = process_pointer_pointer_cast(rhs_loc, lhs_type);
		else {
			process_scalar_pointer_cast(lhs_loc, rhs_loc, lhs_type, c);
			return;
		}
	}

	if (!res) {
		string display = "inv_"+int_to_string(line);
		DisplayTag dt(display, line, file);
		assign_unmodeled_value(lhs_loc, c, lhs->get_type(), UNM_IMPRECISE, dt);
	} else {
		process_assignment(lhs, rhs, c);
	}

	check_invariants(lhs_loc, 0, this);
}

/*
 * Assigns lhs_loc to an unmodeled value of type t.
 */
/*
void MemoryGraph::assign_unmodeled_value(MemoryLocation* lhs_loc, Constraint c,
		il::type* t, unmodeled_type ut,
		const DisplayTag& dt, int optional_offset_only)
{


	AccessPath* unmodeled_val;
	IndexVariable* index_var = NULL;
	Constraint target_index_c;

	//
	//Do not add default edges; callers rely on this!
	 //
	//add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);



	if (recursive) {
		IndexVariable* dummy_index = ivm.get_dummy_index();
		UnmodeledValue* dummy_unmodeled  =
			UnmodeledValue::make(t, dummy_index, ut, dt);
		index_var = ivm.get_new_source_index(dummy_unmodeled, false);
		IndexVariable* target_index_var = IndexVariable::make_target(index_var);

		IterationCounter* iteration_counter = IterationCounter::make_parametric(
				ma.get_cur_iteration_counter());
		target_index_c = cg.get_eq_constraint(target_index_var,
				iteration_counter);
		unmodeled_val = dummy_unmodeled->replace(dummy_index, index_var);

	}
	else {
		unmodeled_val = UnmodeledValue::make(t, ut, dt);
	}
	cout << "Unmodeled val before adding deref: " << unmodeled_val->to_string() << " type: " <<
				unmodeled_val->get_type()->to_string() << endl;

	unmodeled_val = Deref::make(unmodeled_val);
	cout << "Unmodeled val: " << unmodeled_val->to_string() << " type: " <<
			unmodeled_val->get_type()->to_string() << endl;
	MemoryLocation* unmodeled_loc = get_location(unmodeled_val);
	cout << "UNMODELED LOC: " << unmodeled_loc->to_string() << endl;
	cout << "Type: " << t->to_string() << endl;
	cout << "Number of fields in source: " << lhs_loc->get_access_path_map().size() << endl;


	add_default_edges(unmodeled_loc);

	//
	 // If a function is known to return an abstract data type,
	 // get location will make it an ADT access path, and we need to
	 // constrain its additional index variable (note there can be at most
	 // one additional index variable because our representation only
	 //allows pointers to ADT's.)
	 //

	AccessPath* ap = unmodeled_loc->get_representative_access_path();
	IndexVariable* iv = ap->find_outermost_index_var(true);
	if(iv != NULL && index_var != iv)
	{
		iv = IndexVariable::make_target(iv);
		target_index_c &= ConstraintGenerator::get_eqz_constraint(iv);
	}

	Constraint edge_c = c & target_index_c;

	if(optional_offset_only == -1) {
		map<int, set<Edge*>*>::iterator it = lhs_loc->get_successor_map().begin();
		for (; it != lhs_loc->get_successor_map().end(); it++)
		{

			put_edge(NULL, -1, edge_c, lhs_loc, unmodeled_loc, it->first, 0);
		}
	}
	else {
		put_edge(NULL, -1, edge_c, lhs_loc, unmodeled_loc,
				optional_offset_only, 0);
	}


}
*/

/*
 * Assigns lhs_loc to an unmodeled value of type t.
 */

void MemoryGraph::assign_unmodeled_value(MemoryLocation* lhs_loc, Constraint c,
		il::type* t, unmodeled_type ut,
		const DisplayTag& dt, int optional_offset_only)
{
	//
	//Do not add default edges; callers rely on this!
	 //
	update_outgoing_edges(lhs_loc, c);
	map<int, set<Edge*>*>::iterator it = lhs_loc->get_successor_map().begin();
	for (; it != lhs_loc->get_successor_map().end(); it++)
	{
		int source_offset = it->first;
		if(optional_offset_only != -1) {
			if(source_offset != optional_offset_only) continue;
		}

		/*
		 * Find type of current field
		 */
		il::type* cur_type = lhs_loc->get_access_path(source_offset)->get_type();

		AccessPath* unmodeled_val;
		IndexVariable* index_var = NULL;
		Constraint target_index_c;

		if (recursive) {
			IndexVariable* dummy_index = ivm.get_dummy_index();
			UnmodeledValue* dummy_unmodeled  =
				UnmodeledValue::make(cur_type, dummy_index, ut, dt);
			index_var = ivm.get_new_source_index(dummy_unmodeled, false);
			IndexVariable* target_index_var = IndexVariable::make_target(index_var);

			IterationCounter* iteration_counter = IterationCounter::make_parametric(
					ma.get_cur_iteration_counter());
			target_index_c = cg.get_eq_constraint(target_index_var,
					iteration_counter);
			unmodeled_val = dummy_unmodeled->replace(dummy_index, index_var);

		}
		else {
			unmodeled_val = UnmodeledValue::make(cur_type, ut, dt);
		}


		unmodeled_val = Deref::make(unmodeled_val);


		MemoryLocation* unmodeled_loc = get_location(unmodeled_val);

		IndexVariable* iv = unmodeled_val->find_outermost_index_var(true);
		if(iv != NULL && index_var != iv)
		{
			iv = IndexVariable::make_target(iv);
			target_index_c &= ConstraintGenerator::get_eqz_constraint(iv);
		}
		Constraint edge_c = c & target_index_c;

		put_edge(NULL, -1, edge_c, lhs_loc, unmodeled_loc, source_offset, 0);
	}
}


bool MemoryGraph::process_pointer_pointer_cast(MemoryLocation* loc,
		il::pointer_type* cast_t) {
	AccessPath* loc_ap = loc->get_access_path(0);
	if (loc_ap->get_type()->is_pointer_type() &&
			!Type::is_supertype(cast_t, loc_ap->get_type()) && !Type::is_supertype(
			loc_ap->get_type(), cast_t)) {
		report_error(ERROR_ILLEGAL_CAST, "Neither upcast nor down cast "
				+ cast_t->to_string() + " " + loc_ap->get_type()->to_string());
		return false;
	}

	il::type* elem_type = cast_t->get_deref_type();
	if (!elem_type->is_record_type()) {
		return true;
	}

	if(elem_type->is_abstract_data_type()) {
		set<Edge*>* edges = loc->get_successors(0);
		set<Edge*>::iterator it = edges->begin();
		for (; it != edges->end(); it++) {
			Edge* e = *it;
			MemoryLocation* target = e->get_target_loc();
			this->change_to_adt_loc(target, e->get_target_offset(), elem_type);


		}


		return true;
	}

	il::record_type* new_type = (il::record_type*) elem_type;



	set<Edge*>* edges = loc->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		MemoryLocation* target = e->get_target_loc();



		int offset = e->get_target_offset();
		AccessPath* inner = find_cast_field(target, offset, new_type);
		bool res = process_struct_cast(target, offset,
				inner, new_type);
		if(!res) {
			return false;
		}

	}

	return true;

}

AccessPath* MemoryGraph::find_cast_field(MemoryLocation* loc, int offset,
		il::type* cast_type)
{

	if(!loc->has_field_at_offset(offset)) {
		return loc->get_representative_access_path();
	}
	int cur_offset = offset;
	AccessPath* ap = loc->get_access_path(offset);
	while(ap->get_ap_type() == AP_FIELD) {
		int next_offset = loc->get_next_offset(ap);
		c_assert(next_offset != -1);
		cout << "cur offset: " << cur_offset << " next offset: " <<
				next_offset << " cast type size: " <<  cast_type->get_size() << endl;
		if(next_offset - cur_offset > cast_type->get_size())
			return ap;
		ap = ap->get_inner();
	}

	return ap;

}

/*
 * Reports an error due to inconsistent memory layout, such
 * as adding illegal fields.
 */
void MemoryGraph::report_type_inconsistency(AccessPath* ap, int offset) {

	if(ap->is_derived_from_constant()) return;

	report_error(ERROR_ILLEGAL_CAST,
			"Inconsistent memory layout. Cannot add field at offset  "
					+ int_to_string(offset) + "to memory location " +
					ap->to_string() + " with type " + ap->get_type()->to_string());
}

/*
 * We want to enforce the invariant that pointers never point to
 * arithmetic values. If you see a cast b = (foo*)a and a's value is
 * the arithmetic value c+4, make b point to the field at offset for
 * of c.
 *
 */
void MemoryGraph::process_scalar_pointer_cast(MemoryLocation* lhs_loc,
		MemoryLocation* rhs_loc, il::pointer_type* cast_t, Constraint cast_c) {


	assert_context("Processing scalar pointer cast.");

	update_outgoing_edges(lhs_loc, cast_c);

	set<Edge*>* edges = rhs_loc->get_successors(0);
	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint cur_c = e->get_constraint() & cast_c;
		MemoryLocation* target = e->get_target_loc();
		int target_offset = e->get_target_offset();

		AccessPath* outer_ap = target->get_access_path(target_offset);

		if (outer_ap->get_ap_type() != AP_DEREF) {
			put_edge(e->get_source_loc(), e->get_source_offset(), cur_c,
					lhs_loc, target, 0, target_offset);
			continue;
		}
		AccessPath* ap = ((Deref*) outer_ap)->get_inner();
		if (ap->get_ap_type() != AP_ARITHMETIC) {
			put_edge(e->get_source_loc(), e->get_source_offset(), cur_c,
					lhs_loc, target, 0, target_offset);
			continue;
		}
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if (av->is_constant()) {
			put_edge(e->get_source_loc(), e->get_source_offset(), cur_c,
					lhs_loc, target, 0, target_offset);
			continue;
		}

		// find a pointer inside the arithmetic term
		map<Term*, long int> new_args;
		//vector<pair<fraction, AccessPath*> > new_args;
		AccessPath* ptr_ap = NULL;
		bool valid_exp = true;
		const map<Term*, long int>& terms = av->get_elems();
		map<Term*, long int>::const_iterator it = terms.begin();
		for (; it != terms.end(); it++) {
			Term* _cur = it->first;
			long int coef = it->second;
			c_assert(_cur->is_specialized());
			AccessPath* cur = AccessPath::to_ap(_cur);

			if (cur->get_type()->is_pointer_type() && coef != 0) {
				if (ptr_ap != NULL || coef != 1) {
					valid_exp = false;
					break;
				} else
					ptr_ap = cur;
			} else
				new_args[_cur] = coef;

		}

		if (ptr_ap == NULL || !valid_exp) {
			DisplayTag dt(((ArithmeticTerm*) av)->to_string(), line, file);
			assign_unmodeled_value(lhs_loc, cur_c, cast_t, UNM_IMPRECISE, dt);
			string message = ((ArithmeticTerm*) av)->to_string()
					+ " cannot be cast to " + cast_t->to_string();
			report_error(ERROR_ILLEGAL_CAST, message);
			return;
		}
		AccessPath* new_av =
				ArithmeticValue::make(new_args, av->get_constant());

		MemoryLocation* pointer_loc = get_location(ptr_ap);
		add_default_edges(pointer_loc);
		c_assert(ptr_ap->get_type()->is_pointer_type());
		il::type* elem_type =
				((il::pointer_type*) ptr_ap->get_type())->get_deref_type();
		while (elem_type->is_array_type()) {
			elem_type = ((il::array_type*) elem_type)->get_elem_type();
		}

		/*
		 * If this arithmetic value contains an index variable, it's in
		 * terms of source variables, but its target is in terms of
		 * target variables. For the index plus to work correctly,
		 * they need to match so that eliminating unused index variables
		 * constrains the index appropriately. (To see why this is necessary,
		 * see regression Intraprocedural/Cast array ptr to int)
		 */
		new_av = IndexVarManager::rename_source_to_target(new_av);


		process_pointer_plus(lhs_loc, pointer_loc, new_av, cur_c, Constraint(),
				elem_type, elem_type->get_size() / 8);

	}


}

bool MemoryGraph::process_struct_cast(MemoryLocation* target, int offset,
		AccessPath* inner, il::record_type* new_type)
{


	if(ma.track_dynamic_type()) {
		target->add_rtti_field();
	}

	vector<il::record_info*>& fields = new_type->get_fields();
	map<int, AccessPath*>& offset_map = target->get_access_path_map();

	vector<int> existing_offsets;
	target->get_offsets(existing_offsets);

	for (unsigned int i = 0; i < fields.size(); i++) {
		il::record_info* cur_field = fields[i];
		int new_offset = cur_field->offset / 8 + offset;

		// Case 1: Offset already exists
		if (offset_map.count(new_offset) > 0) {
			AccessPath* ap = offset_map[new_offset];
			if (!Type::is_type_compatible(cur_field->t, ap->get_type())) {
				string a;

				report_error(ERROR_ILLEGAL_CAST, "Incompatible types in cast: "
						+ cur_field->t->to_string() + " "
						+ ap->get_type()->to_string());
				return false;
			}

		}

		// Case 2: Offset doesn't exist, but illegal
		il::type* cur_t = cur_field->t;
		int size = Type::get_type_of_first_field(cur_t)->get_size() / 8;
		int end_offset = new_offset + size;
		if (!target->new_offset_is_legal(new_offset, size)) {
			report_error(ERROR_ILLEGAL_CAST, cur_field->t->to_string() + " "
					+ new_type->to_string() + "New  offset from "
					+ int_to_string(new_offset) + " to " + int_to_string(
					end_offset) + " is incompatible with struct layout");
			return false;
		}

	}

	AccessPath* old_rep = target->get_representative_access_path();
	target->populate_struct_fields(ivm, inner, new_type, offset, true);
	AccessPath* new_rep = target->get_representative_access_path();
	new_rep->update_type(new_type);
	update_memory_location(target, old_rep, new_rep, old_rep);
	return true;
}

/*
 * a = &b
 */
void MemoryGraph::process_address(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint c) {
	Variable* lhs_ap = (Variable*) get_access_path_from_symbol(lhs);
	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	process_address(lhs_ap, rhs_ap, c);

}

void MemoryGraph::process_address(Variable* lhs, AccessPath* rhs,
		Constraint _c) {

	Constraint c = _c;
	MemoryLocation* rhs_loc = get_location(rhs);
	MemoryLocation* lhs_loc = get_location(lhs);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);

	AccessPath* target_ap = rhs_loc->get_access_path(0);
	set<IndexVariable*> index_vars;
	target_ap->get_nested_index_vars(index_vars);

	set<IndexVariable*>::iterator it = index_vars.begin();
	for (; it != index_vars.end(); it++) {
		IndexVariable* target_index = IndexVariable::make_target(*it);
		Constraint index_c = ConstraintGenerator::get_eqz_constraint(
				target_index);
		c = c & index_c;
	}
	put_edge(NULL, 0, c, lhs_loc, rhs_loc, 0, 0);

	check_invariants(lhs_loc, 0, this);
}

/*
 * var = &label
 */
void MemoryGraph::process_address_label(sail::Variable* var, string label,
		il::type* signature, Constraint c	)
{
	//cout << "Processing address label: " << label <<  " signature: " <<
		//	(signature ? signature->to_string(): "null") << endl;
	MemoryLocation* loc = get_location(var);
	update_outgoing_edges(loc, c);
	ProgramFunction* pf = ProgramFunction::make(label, signature);
	MemoryLocation* fn_loc = get_location(pf);
	put_edge(NULL, 0, c, loc, fn_loc, 0, 0);

}

void MemoryGraph::process_string_assignment(MemoryLocation* loc, string s)
{
	IndexVariable* index_var =
			loc->get_representative_access_path()->find_outermost_index_var(true);
	map<int, Constraint> targets;

	for (unsigned int i = 0; i <= s.size(); i++) {
		ConstantValue* cv = ConstantValue::make(i);
		Constraint edge_c = cg.get_eq_constraint(cv, index_var);
		int cur_c = (i < s.size() ? s[i] : 0);
		if(targets.count(cur_c) == 0)
			targets[cur_c] = Constraint(false);
		targets[cur_c] |= edge_c;
	}
	map<int, Constraint>::iterator it = targets.begin();
	for(; it!= targets.end(); it++)
	{
		int val = it->first;
		ConstantValue* cv = ConstantValue::make(val);
		MemoryLocation *c_loc = get_location(Deref::make(cv));
		Constraint c = it->second;
		/*
		 * We do not use put_edge for performance reasons to avoid
		 * unnecessary constraint simplifications.
		 */
		Edge * e = new Edge(ivm, edge_counter++, c, loc, c_loc,
						0, 0, false, ma.get_cur_block_id());
		edges.insert(e);
	}
}

/*
 * a = &"str const"
 */
void MemoryGraph::process_address_string(sail::Variable* var, string s,
		Constraint c) {

	il::type* t = var->get_type();
	c_assert(t->is_pointer_type());



	MemoryLocation* lhs_loc = get_location(var);



	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);


	StringLiteral* str_lit = StringLiteral::make(s);
	MemoryLocation* str_loc = get_location(str_lit);
	char first_char;
	if(s.size() > 0)  first_char = s[0];
	else first_char = '\0';
	ConstantValue* first_c = ConstantValue::make(first_char);
	put_edge(NULL, 0, c, lhs_loc, str_loc, 0, 0);
	MemoryLocation* char_loc = get_location(first_c->add_deref());
	put_edge(NULL, 0, Constraint(true), str_loc,  char_loc, 0, 0);


}

void MemoryGraph::set_instruction_number(int n) {
	this->instruction_number = n;
}

void MemoryGraph::set_summary_unit(sail::SummaryUnit* su) {
	this->su = su;
	this->ea.set_summary_unit(su);
}

Constraint MemoryGraph::get_non_alias_requirements_for_delete(AccessPath* ap)
{


	/*
	 * The aliasing requirement is on the pointers to the deleted location
	 */
	AccessPath* ptr_ap = ap->strip_deref();


	Constraint evar_ranges;
	Constraint evar_neq_constraints(false);

	map<AccessPath*, AccessPath*> index_mappings1;
	map<AccessPath*, AccessPath*> index_mappings2;

	set<AccessPath*> nested_aps;
	ptr_ap->get_nested_aps(nested_aps);
	set<AccessPath*>::iterator it = nested_aps.begin();
	for(; it!= nested_aps.end(); it++)
	{
		AccessPath* cur = *it;
		if(cur->get_ap_type() != AP_ARRAYREF &&
				cur->get_ap_type() != AP_ADT) continue;
		IndexVariable* iv = cur->get_index_var();
		MemoryLocation* cur_loc = get_location(cur);
		c_assert(cur_loc->has_size_field());
		AccessPath* size = cur_loc->get_access_path(SIZE_OFFSET);
		Variable* e_var1 = Variable::make_loop_error_temp(
				int_to_string(ma.get_cg_id())+":"+
				int_to_string(((sail::SuperBlock*)su)->get_block_id()));
		Variable* e_var2 = Variable::make_loop_error_temp(
					int_to_string(ma.get_cg_id())+":"+
					int_to_string(((sail::SuperBlock*)su)->get_block_id()));
		AccessPath* sum= ArithmeticValue::make_plus(e_var1, e_var2);

		index_mappings1[cur->get_index_var()] = e_var1;
		index_mappings2[cur->get_index_var()]= sum;

		int elem_size =1;
		if(cur->get_ap_type() == AP_ARRAYREF) {
			elem_size = ((ArrayRef*)cur)->get_elem_size();
		}
		Constraint sum_lt_size = ConstraintGenerator::get_lt_constraint(sum,
				ArithmeticValue::make_division(size,
						ConstantValue::make(elem_size)));




		evar_ranges &=  sum_lt_size;
		evar_neq_constraints |= ConstraintGenerator::get_geq_constraint(e_var2,
				ConstantValue::make(1));

	}

	AccessPath* sub_ap1 = ptr_ap->replace(index_mappings1);
	AccessPath* sub_ap2 = ptr_ap->replace(index_mappings2);


	Constraint alias_c = ConstraintGenerator::get_eq_constraint(sub_ap1, sub_ap2);
	Constraint res = alias_c & evar_ranges & evar_neq_constraints;

	return res;

}

Constraint MemoryGraph::get_not_deleted_constraint(MemoryLocation* loc)
{

	if(!loc->has_field_at_offset(DELETED_OFFSET)) {
		return Constraint(false);
	}
	add_default_edges(loc);



	Constraint not_deleted_c(false);
	set<Edge*>* deleted_values =
			loc->get_successors(DELETED_OFFSET);
	set<Edge*>::iterator it = deleted_values->begin();
	for(; it!= deleted_values->end(); it++)
	{
		Edge* e = *it;
		AccessPath* deleted_val = e->get_target_ap()->strip_deref();
		ConstantValue* false_val = ConstantValue::make(0);
		if(deleted_val != false_val) continue;
		Constraint val_c = e->get_constraint();
		not_deleted_c |= val_c;
	}
	return not_deleted_c;

}


void MemoryGraph::check_access_to_deleted_mem(AccessPath* target_ap, Constraint c,
		string pp_exp, bool access_is_delete)
{
	if(access_is_delete && !GlobalAnalysisState::check_double_deletes())
		return;
	if(!access_is_delete && !GlobalAnalysisState::check_deleted_access()) return;



	MemoryLocation* target_loc = get_location(target_ap);
	c_assert(target_loc->has_field_at_offset(DELETED_OFFSET));


	/*
	 * Lookup values of deleted field
	 */
	set<Edge*>* deleted_values =
			target_loc->get_successors(DELETED_OFFSET);
	set<Edge*>::iterator it2 = deleted_values->begin();
	for(; it2!= deleted_values->end(); it2++)
	{
		Edge* e2 = *it2;
		AccessPath* deleted_val = e2->get_target_ap()->strip_deref();
		Constraint val_c= e2->get_constraint() & c;


		ConstantValue* true_val = ConstantValue::make(1);
		Constraint already_deleted_c =
				ConstraintGenerator::get_eq_constraint(deleted_val, true_val);

		Constraint fail_c = already_deleted_c;
		/*
		 * If we are deleting memory inside a loop,
		 * we also require that
		 */
		if(access_is_delete && su->is_superblock())
		{
			Constraint alias_c =
					get_non_alias_requirements_for_delete(target_ap);
			fail_c |= alias_c;
		}

		fail_c &=  val_c;

		if(this->report_all_errors) {
			fail_c.replace_terms(replace_deleted_field, NULL);
		}


		if(fail_c.unsat()) {
			continue;
		}

		AccessPath* rep = target_loc->get_representative_access_path()->strip_deref();

		ErrorTrace* et;
		if(access_is_delete) {

			et = DoubleDeleteTrace::make(
					rep, pp_exp, this->line, this->file,
					fail_c, this->ma.get_cur_iteration_counter());
		}
		else {
			et = DeletedAccessTrace::make(
					rep, pp_exp, this->line, this->file,
					fail_c, this->ma.get_cur_iteration_counter());
		}


		if(et!=NULL) {
			error_traces.insert(et);
		}


	}


}

bool MemoryGraph::is_entry_function()
{
	return report_all_errors;
}

void MemoryGraph::process_memory_deallocation(AccessPath* ptr,
		Constraint stmt_guard, string pp_exp)
{
	if(!GlobalAnalysisState::track_delete()) return;

	MemoryLocation* ptr_loc = get_location(ptr);

	add_default_edges(ptr_loc);



	/*
	 * Set the deleted field of the memory locations pointed to by
	 * ptr to 0.
	 */
	set<Edge*>* succs = ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* target_loc = e->get_target_loc();
		AccessPath* target_ap = e->get_target_ap();
		add_default_edges(target_loc);
		Constraint edge_c = e->get_constraint();
		IndexVarManager::rename_index_vars(edge_c);


		/*
		 * This can sometimes happen; for example, we refuse to add
		 * is_deleted field to NULL or constants.
		 */
		if(!target_loc->has_field_at_offset(DELETED_OFFSET))
		{
			continue;
		}

		// check for double delete
		check_access_to_deleted_mem(target_ap, edge_c & stmt_guard, pp_exp, true);


		AccessPath* true_val = ConstantValue::make(1)->add_deref();
		MemoryLocation* true_loc = get_location(true_val);
		Constraint update_c =  edge_c & stmt_guard;
		update_outgoing_edges(target_loc, DELETED_OFFSET, update_c, true);
		put_edge(NULL, -1, edge_c & stmt_guard, target_loc, true_loc,
				DELETED_OFFSET, 0, false, true);

	}
}

void MemoryGraph::process_memory_deallocation(sail::Variable* ptr,
		Constraint stmt_guard)
{


	AccessPath* ptr_ap =  get_access_path_from_symbol(ptr);
	string pp_exp = "*"+ ptr->to_string(true);

	process_memory_deallocation(ptr_ap, stmt_guard, pp_exp);





}

/*
 * a = malloc(size)
 */
void MemoryGraph::process_memory_allocation(sail::Variable* lhs,
		string _alloc_id, sail::Symbol* alloc_size, Constraint& c,
		bool is_nonnull) {
	IndexVariable* index_var = NULL;

	Constraint source_index_c;
	Constraint target_index_c;

	alloc_info alloc_id(line, instruction_number, su->get_identifier());

	if (recursive) {
		IndexVariable* dummy_index = ivm.get_dummy_index();
		Alloc* dummy_alloc =
				Alloc::make(alloc_id, lhs->get_type(), dummy_index, is_nonnull);
		index_var = ivm.get_new_source_index(dummy_alloc, false);
		IndexVariable* target_index_var = IndexVariable::make_target(index_var);

		IterationCounter* iteration_counter = IterationCounter::make_parametric(
				ma.get_cur_iteration_counter());
		target_index_c = cg.get_eq_constraint(target_index_var,
				iteration_counter);
		source_index_c = cg.get_eq_constraint(index_var, iteration_counter);

	}
	Alloc* alloc_ap1 = Alloc::make(alloc_id, lhs->get_type(), index_var,
			is_nonnull);
	Deref* alloc_ap = Deref::make(alloc_ap1);
	MemoryLocation* alloc = get_location(alloc_ap);


	AccessPath* outer_index = alloc->get_representative_access_path()
			->find_outermost_index_var(true);

	if(outer_index != NULL) {

		AccessPath* outer_index_j = IndexVarManager::rename_source_to_target(
				outer_index);
		target_index_c &= ConstraintGenerator::get_eqz_constraint(outer_index_j);
	}

	alloc->add_size_field();

	MemoryLocation* size_loc = get_location(alloc_size);
	add_default_edges(size_loc);

	set<Edge*>::iterator it = size_loc->get_successors(0)->begin();
	for (; it != size_loc->get_successors(0)->end(); it++) {
		Edge* e = *it;
		Constraint cur_c = e->get_constraint() & source_index_c;
		Edge* new_e = put_edge(e->get_source_loc(), e->get_source_offset(),
				cur_c, alloc, e->get_target_loc(), SIZE_OFFSET,
				e->get_target_offset(), false, true);
	}

	MemoryLocation* lhs_loc = get_location(lhs);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);

	Constraint edge_c = c & target_index_c;
	Edge* e = put_edge(NULL, 0, edge_c, lhs_loc, alloc, 0, 0);



	/*
	 * Set deleted field to false.
	 */
	if(GlobalAnalysisState::track_delete())
	{
		AccessPath* false_ap = ConstantValue::make(0)->add_deref();
		MemoryLocation* false_loc = this->get_location(false_ap);

		Constraint not_deleted_c;
		if(recursive) {
			IterationCounter* iteration_counter = IterationCounter::make_parametric(
					ma.get_cur_iteration_counter());
			not_deleted_c = cg.get_eq_constraint(index_var, iteration_counter);
		}

		put_edge(NULL, 0, not_deleted_c, alloc, false_loc, DELETED_OFFSET, 0, false, true);

	}

	check_invariants(lhs_loc, 0, this);

}

void MemoryGraph::process_unknown_function_call(sail::Variable* retvar, vector<
		sail::Symbol*> * args, string fun_id, Constraint & stmt_guard,
		il::type* return_type) {


	if (retvar == NULL)
		return;
	MemoryLocation* ret_loc = get_location(retvar);
	add_default_edges(ret_loc);
	string display_str = "Return value of " + fun_id;
	DisplayTag dt(display_str, line, file);
	assign_unmodeled_value(ret_loc, stmt_guard, return_type,
			UNM_UNKNOWN_FUNCTION, dt);



}

void MemoryGraph::set_dynamic_type(MemoryLocation* loc, il::type* dyn_type,
		Constraint stmt_guard)
{

	if(loc->get_representative_access_path()->get_type()->
			is_abstract_data_type())
		return;
	if(!loc->get_representative_access_path()->get_type()->is_record_type())
		return;
	//c_assert(loc->get_representative_access_path()->get_type()->is_record_type());

	TypeConstant* tc = TypeConstant::make(dyn_type);
	MemoryLocation* type_loc = get_location(tc->add_deref());
	this->add_default_edges(loc);
	this->update_outgoing_edges(loc, RTTI_OFFSET, stmt_guard, true);
	put_edge(NULL, 0, stmt_guard, loc, type_loc, RTTI_OFFSET, 0, false, true);



}

void MemoryGraph::process_constructor_call(sail::FunctionCall* fc,
		vector<sail::Symbol*> * args, Constraint stmt_guard)
{
	c_assert(fc!=NULL);
	c_assert(fc->is_constructor());

	il::type* sig = fc->get_signature();
	c_assert(sig->is_function_type());
	il::function_type* ft = static_cast<il::function_type*>(sig);
	c_assert(ft->get_arg_types().size() > 0);

	il::type* ptr_t =ft->get_arg_types()[0];
	c_assert(ptr_t->is_pointer_type());
	il::type* rt = ptr_t->get_inner_type();



	c_assert(args->size() > 0);
	sail::Symbol* obj = (*args)[0];
	MemoryLocation* loc = get_location(obj);

	//do not add rtti fields for ADT types
	il::type* t = loc->get_representative_access_path()->get_type();
	t = t->get_inner_type();
	if(t!= NULL && t->is_abstract_data_type())
		return;

	set<Edge*>* succs = loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++) {
		Edge* e = *it;
		MemoryLocation* object_loc = e->get_target_loc();
		set_dynamic_type(object_loc, rt, stmt_guard);
	}
}

void MemoryGraph::process_function_call(sail::FunctionCall* fc, SummaryGraph* s,
		sail::Variable* retvar, vector<sail::Symbol*> * args,
		Constraint & stmt_guard) {



	if (retvar != NULL) {
		MemoryLocation* ret_loc = get_location(retvar);
		add_default_edges(ret_loc);
	}


	if (s != NULL) {
		Instantiator inst(this, s, stmt_guard, args, retvar);
	}

	if(fc != NULL && fc->is_constructor()) {
		process_constructor_call(fc, args, stmt_guard);
	}



}

void MemoryGraph::initialize_return_value(sail::Variable* retvar)
{
	MemoryLocation* ret_loc = get_location(retvar);
	add_default_edges(ret_loc);
}

void MemoryGraph::add_subtype_axiom(il::record_type* rt, AccessPath* rtti_field,
		Constraint rtti_constraint)
{
	Constraint axiom(false);
	set<il::record_type*> subclasses;
	rt->get_transitive_subclasses(subclasses);
	set<il::record_type*>::iterator it = subclasses.begin();
	for(; it!= subclasses.end(); it++) {
		il::record_type* cur = *it;
		if(cur->is_abstract()) continue;
		TypeConstant* tc = TypeConstant::make(cur);
		Constraint eq_c =ConstraintGenerator::get_eq_constraint(rtti_field, tc);
		Constraint cur_c = eq_c & rtti_constraint;
		IndexVarManager::eliminate_source_vars(cur_c);
		IndexVarManager::eliminate_target_vars(cur_c);
		cur_c.assume(rtti_constraint);
		axiom |= cur_c;

	}
	if(axiom.sat()) {
		Constraint::set_background_knowledge(axiom);
		ma.add_dotty("Adding axiom: " + axiom.to_string(), "");
	}
}



void MemoryGraph::process_instance_of(sail::Variable* retvar, sail::Symbol* s,
		il::type* t, Constraint & stmt_guard)
{

}



void MemoryGraph::process_virtual_method_call(sail::Variable* dispatch_ptr,
		set<SummaryGraph*>& target_summaries, sail::Variable* retvar,
		vector<sail::Symbol*> * args,  Constraint & stmt_guard)
{

	if (retvar != NULL) {
		MemoryLocation* ret_loc = get_location(retvar);
		add_default_edges(ret_loc);

		/*
		 * Initially, put an edge from return to an invalid location under the
		 * stmt_guard. This is necessary if some target of the dispatch_ptr
		 * is illegal, e.g. NULL.
		 */
		DisplayTag dt("Virtual method call to " + dispatch_ptr->to_string(),
				line, file);
		assign_unmodeled_value(ret_loc, stmt_guard, retvar->get_type(),
				UNM_INVALID, dt);
	}

	il::type* _obj_type = dispatch_ptr->get_type()->get_inner_type();
	c_assert(_obj_type->is_record_type());
	il::record_type* obj_type = (il::record_type*) _obj_type;

	check_ptr_deref(dispatch_ptr, stmt_guard, dispatch_ptr->to_string(true));
	check_static_type(dispatch_ptr, stmt_guard, "*"+dispatch_ptr->to_string(true));



	/*
	 * First, look up value of rtti value of the object being dispatch on.
	 */
	set<pair<AccessPath*, Constraint> > rtti_values;
	MemoryLocation* ptr_loc = get_location(dispatch_ptr);
	add_default_edges(ptr_loc);
	set<Edge*>* succs = ptr_loc->get_successors(0);
	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* obj = e->get_target_loc();
		if(obj->get_representative_access_path()->is_derived_from_constant())
			continue;
		add_default_edges(obj);
		//int offset = e->get_target_offset();
		//c_assert(offset == 0);
		Constraint edge_c = e->get_constraint();
		IndexVarManager::rename_index_vars(edge_c);

		if(!obj->has_field_at_offset(RTTI_OFFSET))
		{
			c_warn("No RTTI field on dispatched object");
			continue;
		}

		set<Edge*>* rtti_edges = obj->get_successors(RTTI_OFFSET);
		set<Edge*>::iterator it2 = rtti_edges->begin();
		for(; it2!= rtti_edges->end(); it2++) {
			Edge* e2 = *it2;
			AccessPath* rtti_info = e2->get_target_ap()->strip_deref();
			Constraint e2_c = e2->get_constraint();
			Constraint res_c = e2_c & edge_c;
			IndexVarManager::rename_index_vars(res_c);
			rtti_values.insert(make_pair(rtti_info, res_c));

			add_subtype_axiom(obj_type, rtti_info, res_c);


		}
	}


	set<SummaryGraph*>::iterator sum_it = target_summaries.begin();
	for(; sum_it!= target_summaries.end(); sum_it++) {
		SummaryGraph* sg = *sum_it;
		call_id  cid = sg->get_call_id();

		const string& cur_name = cid.get_function_name();
		il::type* cur_sig = cid.get_signature();


		// Figure out which class the current summary belongs to.
		il::record_type* cur_class = sg->get_defining_class();
		c_assert(cur_class != NULL);

		// Now, figure out all the classes that could inherit this
		// method from cur_class.
		set<il::record_type*> inheriting_classes;
		cur_class->get_classes_inheriting_method(cur_name, cur_sig,
				inheriting_classes);

		//delete me
		/*cout << "CUR CLASS: " << cur_class->to_string() << endl;
		{
			cout << "*********** Inherting classes: " << this->line << endl;
			auto it = inheriting_classes.begin();
			for(; it != inheriting_classes.end(); it++) {
				cout << **it << endl;
			}
			cout << "*****************************" << endl;
		}*/
		//delete me end

		TypeConstant* target_class = TypeConstant::make(cur_class);




		/*
		 * Generate conditional equality between value of rtti field and
		 * each class inheriting this method of cur_class.
		 * For example, if rtti field is x under C1 and y under C2,
		 * and classes A and B inherit this method of cur_class,
		 * then the dispatch constraint is:
		 * (x = A & C1) | (x = B & C1) | (y = A & c2) |  (y = B & c2)
		 */
		Constraint dispatch_constraint(false);
		set<pair<AccessPath*, Constraint> >::iterator rtti_it = rtti_values.begin();
		for(; rtti_it != rtti_values.end(); rtti_it++)
		{
			AccessPath* rtti_val = rtti_it->first;
			Constraint rtti_c = rtti_it->second;

			//cout << "rtti: " << rtti_val->to_string() << endl;
			//cout << "rtti_c: " << rtti_c << endl;

			set<il::record_type*>::iterator class_it = inheriting_classes.begin();
			for(; class_it != inheriting_classes.end(); class_it++)
			{
				il::record_type* cur_ic = *class_it;
				TypeConstant* cur_type_ap = TypeConstant::make(cur_ic);
				Constraint eq_c = ConstraintGenerator::get_eq_constraint(
						cur_type_ap, rtti_val);
				Constraint cur_c = eq_c & rtti_c;
				IndexVarManager::eliminate_source_vars(cur_c);
				dispatch_constraint |= cur_c;

			}

		}
		//cout << "dispatch c: " << dispatch_constraint << endl;
		dispatch_constraint &= stmt_guard;
		if(dispatch_constraint.unsat()){

			continue;
		}


		Instantiator(this, sg, dispatch_constraint, args, retvar);


	}
}

void MemoryGraph::process_function_pointer_call(
		sail::Variable* function_pointer, set<SummaryGraph*>& target_summaries,
		sail::Variable* retvar, vector<sail::Symbol*> *
		args,  Constraint & stmt_guard)
{
	if (retvar != NULL) {
		MemoryLocation* ret_loc = get_location(retvar);
		add_default_edges(ret_loc);

		/*
		 * Initially, put an edge from return to an invalid location under the
		 * stmt_guard. This is necessary if some target of the function_pointer
		 * is illegal, e.g. NULL.
		 */
		string display_str = "Function pointer call through " +
				function_pointer->to_string(true);
		DisplayTag dt(display_str, line, file);
		assign_unmodeled_value(ret_loc, stmt_guard, retvar->get_type(),
				UNM_INVALID, dt);
	}

	MemoryLocation* ptr_loc = get_location(function_pointer);
	add_default_edges(ptr_loc);

	check_ptr_deref(function_pointer, stmt_guard, function_pointer->to_string(true));

	set<Edge*>* succs = ptr_loc->get_successors(0);

	/*
	 * Stipulate that target variable must be equal to one of these
	 * functions so that we can add it as background knowledge
	 */
	Variable* temp = Variable::make_temp(function_pointer->get_type()
			->get_deref_type());
	Constraint bg;
	if(target_summaries.size() > 0) bg = Constraint(false);
	set<SummaryGraph*>::iterator it = target_summaries.begin();
	for(; it!= target_summaries.end(); it++) {
		SummaryGraph* sg = *it;
		call_id  cid = sg->get_call_id();
		ProgramFunction* pf = ProgramFunction::make(cid.name, cid.sig);
		Constraint eqc = ConstraintGenerator::get_eq_constraint(temp, pf);
		bg |= eqc;

	}

	/*
	 * Now do the actual work.
	 */
	it = target_summaries.begin();
	for(; it!= target_summaries.end(); it++)
	{
		SummaryGraph* sg = *it;
		call_id  cid = sg->get_call_id();
		ProgramFunction* pf = ProgramFunction::make(cid.name, cid.sig);


		Constraint target_cond(false);


		set<Edge*>::iterator it = succs->begin();
		for(; it!= succs->end(); it++)
		{
			Edge* e = *it;
			AccessPath* target = e->get_target_ap();
			Constraint edge_c = e->get_constraint();
			IndexVarManager::rename_index_vars(edge_c);
			Constraint eqc = ConstraintGenerator::get_eq_constraint(target, pf);
			Constraint cur = eqc & edge_c;
			IndexVarManager::eliminate_source_vars(cur);
			target_cond |= cur;

			// Deal with background knowledge
			Constraint cur_bg = bg;
			cur_bg.replace_term(temp, target->to_term());

			//Constraint::set_background_knowledge(cur_bg);


		}




		Constraint inst_c = stmt_guard & target_cond;
		if(inst_c.unsat()) continue;
		Instantiator(this, sg, inst_c, args, retvar);





	}

}



void MemoryGraph::process_loop_invocation(SummaryGraph* s, Constraint stmt_guard) {
	Instantiator inst(this, s, stmt_guard);

}

// -----------NON-POINTER OPERATIONS--------------------------

/*
 * a = -b
 */
void MemoryGraph::process_negate(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint& c) {

	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	check_memory_access(rhs_ap, c, rhs->to_string(true));

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);
	c_assert(rhs_loc->get_successor_map().size() >= 1);
	set<Edge*>* edges = rhs_loc->get_successors(0);

	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint new_c = e->get_constraint() & c;
		if (new_c.unsat())
			continue;
		MemoryLocation* target = e->get_target_loc();
		AccessPath* ap = target->get_access_path(e->get_target_offset());

		AccessPath* inner = ap->strip_deref();
		MemoryLocation* new_loc = NULL;
		if (inner->get_ap_type() == AP_CONSTANT) {
			ConstantValue* cc = (ConstantValue*) inner;
			AccessPath* ap = ConstantValue::make(-cc->get_constant())->add_deref();
			new_loc = get_location(ap);
		} else {
			AccessPath* av = inner->multiply_ap(-1);
			Deref* d = Deref::make(av, il::get_integer_type());
			new_loc = get_location(d);
		}
		put_edge(e->get_source_loc(), e->get_source_offset(), new_c, lhs_loc,
				new_loc, 0, 0);

	}

}

void MemoryGraph::process_bitwise_not(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint& c) {

	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	check_memory_access(rhs_ap, c, rhs->to_string(true));

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);
	set<Edge*>* edges = rhs_loc->get_successors(0);

	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint new_c = e->get_constraint() & c;
		if (new_c.unsat())
			continue;
		MemoryLocation* target = e->get_target_loc();
		AccessPath* ap = target->get_access_path(e->get_target_offset());
		AccessPath* inner = ap->strip_deref();
		Deref* d = NULL;
		if (inner->get_ap_type() == AP_FUNCTION) {
			FunctionValue* inner_fn = (FunctionValue*) inner;
			// idempotence
			if (inner_fn->get_fun_type() == FN_BITWISE_NOT) {
				AccessPath* inner_ap = inner_fn->get_arguments()[0];
				d = Deref::make(inner_ap, il::get_integer_type());
			}
		}

		if (d == NULL) {
			if (!CONVERT_BITWISE_NOT_TO_ARITHMETIC) {
				FunctionValue* fv = FunctionValue::make(FN_BITWISE_NOT, inner);
				d = Deref::make(fv, il::get_integer_type());
			} else {
				AccessPath* av = inner->multiply_ap(-1);
				av = ArithmeticValue::make_minus(av, ConstantValue::make(1));
				d = Deref::make(av, il::get_integer_type());
			}
		}
		MemoryLocation* new_loc = get_location(d);
		put_edge(e->get_source_loc(), e->get_source_offset(), new_c, lhs_loc,
				new_loc, 0, 0);
	}
}

void MemoryGraph::process_logical_not(sail::Variable* lhs, sail::Symbol* rhs,
		Constraint& c) {


	AccessPath* rhs_ap = get_access_path_from_symbol(rhs);
	check_memory_access(rhs_ap, c, rhs->to_string(true));

	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(rhs);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);

	set<Edge*>* edges = rhs_loc->get_successors(0);

	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint new_c = e->get_constraint() & c;
		if (new_c.unsat())
			continue;
		MemoryLocation* target = e->get_target_loc();
		AccessPath* ap = target->get_access_path(e->get_target_offset());
		AccessPath* inner = ap->strip_deref();
		Deref* d = NULL;
		if(inner->get_ap_type() == AP_CONSTANT)
		{

			ConstantValue* cv = (ConstantValue*) inner;
			if (cv->get_constant() == 0) {
				ConstantValue* one = ConstantValue::make(1);
				d = Deref::make(one, il::get_integer_type());
			} else {
				ConstantValue* zero = ConstantValue::make(0);
				d = Deref::make(zero, il::get_integer_type());
			}

			MemoryLocation* new_loc = get_location(d);
			put_edge(e->get_source_loc(), e->get_source_offset(), new_c, lhs_loc,
					new_loc, 0, 0);
		}

		else {
			Constraint true_c = ConstraintGenerator::get_eqz_constraint(inner);
			IndexVarManager::rename_source_to_target_index(true_c);
			Constraint false_c = !true_c;

			MemoryLocation* true_loc = get_location(Deref::make(
					ConstantValue::make(1)));
			MemoryLocation* false_loc = get_location(Deref::make(
					ConstantValue::make(0)));

			put_edge(e->get_source_loc(), e->get_source_offset(), new_c & true_c,
					lhs_loc, true_loc, 0, 0);
			put_edge(e->get_source_loc(), e->get_source_offset(), new_c & false_c,
					lhs_loc, false_loc, 0, 0);
		}
	}
}

void MemoryGraph::process_predicate_binop(sail::Variable* lhs,
		sail::Symbol* op1, sail::Symbol* op2, Constraint& c, il::binop_type bt)
{

	AccessPath* op1_ap = get_access_path_from_symbol(op1);
	check_memory_access(op1_ap, c, op1->to_string(true));
	AccessPath* op2_ap = get_access_path_from_symbol(op2);
	check_memory_access(op2_ap, c, op2->to_string(true));


	c_assert(il::binop_expression::is_predicate_binop(bt));
	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* op1_loc = get_location(op1);
	MemoryLocation* op2_loc = get_location(op2);

	add_default_edges(op1_loc);
	add_default_edges(op2_loc);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);

	set<Edge*>* edges1 = op1_loc->get_successors(0);
	set<Edge*>* edges2 = op2_loc->get_successors(0);

	Constraint true_c(false);
	Constraint false_c(false);

	set<Edge*>::iterator it1 = edges1->begin();
	for (; it1 != edges1->end(); it1++) {
		Edge* e1 = *it1;
		Constraint edge_c1 = e1->get_constraint();

		MemoryLocation* target1 = e1->get_target_loc();
		AccessPath* ap1 = target1->get_access_path(e1->get_target_offset());
		AccessPath* inner_ap1 = ap1->strip_deref();

		set<Edge*>::iterator it2 = edges2->begin();
		for (; it2 != edges2->end(); it2++) {
			Edge* e2 = *it2;
			Constraint edge_c2 = e2->get_constraint();
			MemoryLocation* target2 = e2->get_target_loc();
			AccessPath* ap2 = target2->get_access_path(e2->get_target_offset());
			AccessPath* inner_ap2 = ap2->strip_deref();

			pair<Constraint, Constraint> res =
					cg.get_constraint_from_pred_binop(inner_ap1, edge_c1,
							inner_ap2, edge_c2, bt);



			Constraint pred_true = res.first;
			Constraint pred_false = res.second;


			true_c |= pred_true;
			false_c |= pred_false;

		}
	}

	true_c &= c;
	false_c &= c;




	put_predicate_edge(lhs_loc, true_c, false_c);

	check_invariants(lhs_loc, 0, this);

}

/*
 * Adds an edge from lhs_loc to true and false
 * under true_c and false_c respectively.
 * If true_c and false_c are imprecise,
 * we make them precise by adding disjointness constraints
 * and associating true_c and false_c with these
 * disjointness constraints as part of background knowledge.
 */
void MemoryGraph::put_predicate_edge(MemoryLocation* lhs_loc,
		Constraint true_c, Constraint false_c) {

	set<Term*> terms;
	true_c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++) {
		Term* t = *it;
		term_attribute_type attrib = t->get_attribute();
	}

	true_c.sat();
	true_c.valid();



	ConstantValue* one = ConstantValue::make(1);
	ConstantValue* zero = ConstantValue::make(0);
	Deref* drf_one = Deref::make(one);
	Deref* drf_zero = Deref::make(zero);
	MemoryLocation* true_loc = get_location(drf_one);
	MemoryLocation* false_loc = get_location(drf_zero);
	Edge* true_edge = put_edge(NULL, -1, true_c, lhs_loc, true_loc, 0, 0);
	Edge* false_edge = put_edge(NULL, -1, false_c, lhs_loc, false_loc, 0, 0);


}

void MemoryGraph::process_binop(Variable* lhs, AccessPath* op1,
		AccessPath* op2, Constraint c, il::binop_type bt, string pp_exp)
{
	if(bt == il::_PLUS)
	{
		if(op1->get_ap_type() == AP_CONSTANT &&
				((ConstantValue*)op1)->get_constant() == 0)
		{
			process_assignment(lhs, op2, c);
			return;
		}

		if(op2->get_ap_type() == AP_CONSTANT &&
					((ConstantValue*)op2)->get_constant() == 0)
		{
			process_assignment(lhs, op1, c);
			return;
		}
	}

	c_assert(!il::binop_expression::is_predicate_binop(bt));
	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* op1_loc = get_location(op1);
	MemoryLocation* op2_loc = get_location(op2);


	add_default_edges(op1_loc);
	add_default_edges(op2_loc);
	add_default_edges(lhs_loc);
	update_outgoing_edges(lhs_loc, c);

	set<Edge*>* edges1 = op1_loc->get_successors(0);
	set<Edge*>* edges2 = op2_loc->get_successors(0);

	set<Edge*>::iterator it1 = edges1->begin();
	for (; it1 != edges1->end(); it1++) {
		Edge* e1 = *it1;
		Constraint edge_c1 = e1->get_constraint();
		Constraint combined_c1 = edge_c1 & c;
		if (combined_c1.unsat())
			continue;
		MemoryLocation* target1 = e1->get_target_loc();
		AccessPath* ap1 = target1->get_access_path(e1->get_target_offset());

		AccessPath* inner_ap1 = ap1->strip_deref();



		set<Edge*>::iterator it2 = edges2->begin();
		for (; it2 != edges2->end(); it2++) {
			Edge* e2 = *it2;

			MemoryLocation* target2 = e2->get_target_loc();
			AccessPath* ap2 = target2->get_access_path(e2->get_target_offset());

			Constraint edge_c2 = e2->get_constraint();
			ap2 = IndexVarManager::refresh_index_vars(ap2, edge_c2);


			AccessPath* inner_ap2 = ap2->strip_deref();

			Constraint c2 = edge_c2 & c;
			if (c2.unsat())
				continue;




			Constraint combined_c2 = combined_c1 & edge_c2;
			if (combined_c2.unsat())
				continue;
			AccessPath* res = AccessPath::make_access_path_from_binop(
					inner_ap1, inner_ap2, bt);


			/* If the above function returns null, we cannot model this
			 * binop precisely.
			 */
			if(res == NULL) {
				DisplayTag dt(pp_exp, line, file);
				assign_unmodeled_value(lhs_loc, combined_c2, lhs->get_type(),
						UNM_IMPRECISE, dt);
				continue;
			}

			set<IndexVariable*> nested_indices;
			res->get_nested_index_vars(nested_indices);
			map<Term*, Term*> subs;
			set<IndexVariable*>::iterator sub_it = nested_indices.begin();
			/*
			 * Any new memory location needs fresh names for embedded
			 * index variables in this location. This only can happen with
			 * arithmetic access paths that contain an index variable. This
			 * only happens when pointers to arrays are cast to integers.
			 */
			for (; sub_it != nested_indices.end(); sub_it++) {
				IndexVariable* cur = *sub_it;
				if (!cur->is_source())
					continue;
				IndexVariable* target_cur = IndexVariable::make_target(cur);
				IndexVariable* new_cur = IndexVariable::make_source();
				IndexVariable* new_target = IndexVariable::make_target(new_cur);

				subs[cur] = new_cur;
				subs[target_cur] = new_target;

			}
			Term* t = res->to_term()->substitute(subs);
			res = AccessPath::to_ap(t);

			combined_c2.replace_terms(subs);

			Deref* new_drf =
					Deref::make(res, lhs->get_type()->get_deref_type());
			MemoryLocation* new_loc = get_location(new_drf);
			put_edge(e2->get_source_loc(), e2->get_source_offset(),
					combined_c2, lhs_loc, new_loc, 0, 0);

		}

	}

	check_invariants(lhs_loc, 0, this);

}

void MemoryGraph::process_binop(sail::Variable* lhs, sail::Symbol* op1,
		sail::Symbol* op2, Constraint c, il::binop_type bt) {
	AccessPath* lhs_ap = get_access_path_from_symbol(lhs);
	c_assert(lhs_ap->get_ap_type() == AP_VARIABLE);
	AccessPath* op1_ap = get_access_path_from_symbol(op1);
	AccessPath* op2_ap = get_access_path_from_symbol(op2);

	check_memory_access(op1_ap, c, op1->to_string(true));
	check_memory_access(op2_ap, c, op2->to_string(true));
	string pp_exp = op1->to_string(true) +
			il::binop_expression::binop_to_string(bt) +
			op2->to_string(true);
	process_binop((Variable*) lhs_ap, op1_ap, op2_ap, c, bt, pp_exp);

}

void MemoryGraph::process_field_offset(Variable* lhs_var, AccessPath* rhs_var,
		int offset, Constraint c) {
	MemoryLocation* lhs_loc = get_location(lhs_var);
	MemoryLocation* rhs_loc = get_location(rhs_var);
	string pp_exp = rhs_var->to_string() + "+" + int_to_string(offset);
	process_offset_plus(lhs_loc, rhs_loc, offset, c, pp_exp);
}

/*
 * Pointer arithmetic on struct offset
 */
void MemoryGraph::process_offset_plus(MemoryLocation* lhs_loc,
		MemoryLocation* rhs_loc, int offset, Constraint c, string pp_exp)
{
	AccessPath* source_ap = lhs_loc->get_access_path(0);
	assert_context("Processing offset plus. LHS:  " + source_ap->to_string() + " = "
			+ "RHS: " + rhs_loc->to_string(true) +
			" offset: " + int_to_string(offset));

	add_default_edges(rhs_loc);



	il::type* source_t = source_ap->get_type();

	assert_context("Type of lhs: " + source_t->to_string());
	c_assert(source_t->is_pointer_type());

	set<Edge*>::iterator it = rhs_loc->get_successors(0)->begin();
	for (; it != rhs_loc->get_successors(0)->end(); it++) {
		Edge* e = *it;
		Constraint edge_c = e->get_constraint();
		Constraint cur_c = edge_c & c;
		MemoryLocation* target = e->get_target_loc();
		int new_offset = offset + e->get_target_offset();


		bool consistent = target->add_offset(new_offset,
				source_t->get_inner_type());
		il::type* t = lhs_loc->get_representative_access_path()->get_type();

		if (!consistent) {

			string inst = lhs_loc->to_string(true) + "= " +
					rhs_loc->to_string(true) + "+" + int_to_string(offset);
			if(!e->get_target_ap()->is_derived_from_constant()) {
				report_error(ERROR_INVALID_POINTER_ARITHMETIC,
						"Invalid pointer arithmetic on struct  (" + inst + ")\n");
			}
			DisplayTag dt(pp_exp, line, file);
			assign_unmodeled_value(lhs_loc, cur_c, t, UNM_INVALID, dt);
			continue;
		}

		Constraint final_c = cur_c;

		if(!target->has_field_at_offset(new_offset)) {
			DisplayTag dt(pp_exp, line, file);
			this->assign_unmodeled_value(lhs_loc, cur_c, t, UNM_INVALID, dt);
			continue;
		}

		AccessPath* target_ap = target->get_access_path(new_offset);
		IndexVariable* index_var = target_ap->find_outermost_index_var();
		if (index_var != NULL) {
			IndexVariable* target_index = IndexVariable::make_target(index_var);
			set<Term*> constraint_vars;
			edge_c.get_free_variables(constraint_vars);
			if (constraint_vars.count(target_index) == 0) {
				Constraint index_c = ConstraintGenerator::get_eqz_constraint(
						index_var);
				ivm.rename_source_to_target_index(index_c);
				final_c = final_c & index_c;
			}
		}
		put_edge(e->get_source_loc(), e->get_source_offset(), final_c, lhs_loc,
				target, e->get_source_offset(), new_offset);
	}
}

/*
 * Pointer arithmetic on array pointer
 */
void MemoryGraph::process_index_plus(MemoryLocation* lhs_loc,
		MemoryLocation* rhs_loc, AccessPath* ap, Constraint stmt_guard,
		Constraint index_guard, int elem_size)
{

	set<Edge*>::iterator it = rhs_loc->get_successors(0)->begin();
	for (; it != rhs_loc->get_successors(0)->end(); it++) {
		Edge* e = *it;

		MemoryLocation* target = e->get_target_loc();

		AccessPath* target_path = target->get_access_path(
				e->get_target_offset());
		if(target_path->find_outermost_index_var(true) == NULL)
		{
			IndexVariable* new_index = change_to_array_loc(target, elem_size);
			AccessPath* new_ap = target->get_access_path(e->get_target_offset());
			new_ap = new_ap->replace(new_index, ConstantValue::make(0));

			stmt_guard.replace_term(target_path->to_term(),
					new_ap->to_term());
			target_path = target->get_access_path(e->get_target_offset());

		}


		Constraint c = stmt_guard & index_guard;
		Constraint edge_c = e->get_constraint();
		Constraint cur_c = edge_c & c;


		AccessPath* ap_to_inc = this->find_index_var_to_increment(target_path,
				elem_size);



		/*
		 * If the elem sizes don't match, report an error
		 */
		if (ap_to_inc->get_ap_type() == AP_ARRAYREF &&
				((ArrayRef*)ap_to_inc)->get_elem_size() != elem_size) {
			report_error(ERROR_INCONSISTENT_ELEM_SIZE, "Array "
					+ rhs_loc->to_string(true)
					+ " accessed with different sizes " + int_to_string(
					elem_size) + " and " + int_to_string(
							((ArrayRef*)ap_to_inc)->get_elem_size()));

			DisplayTag dt(rhs_loc->to_string(true), line, file);
			assign_unmodeled_value(lhs_loc, cur_c, target_path->get_type(),
					UNM_IMPRECISE, dt);
			continue;
		}

		IndexVariable* index_var = ap_to_inc->get_index_var();

		Constraint final_c;
		if (index_var != NULL) {
 			IndexVariable* target_var = IndexVariable::make_target(index_var);
			final_c = ivm.increment_index_var(cur_c, target_var, ap);

		} else {
			final_c = cur_c;
		}

		Edge* new_e = put_edge(e->get_source_loc(), e->get_source_offset(),
				final_c, lhs_loc, target, e->get_source_offset(),
				e->get_target_offset());

	}
}

void MemoryGraph::process_pointer_plus(MemoryLocation* lhs_loc,
		MemoryLocation* rhs_loc, AccessPath* ap, Constraint stmt_guard,
		Constraint index_guard, il::type* elem_type, int elem_size)
{



	if (ap->is_constant()) {
		ConstantValue* cv = (ConstantValue*) ap;
		long int constant = cv->get_constant();


		int index = constant / elem_size;
		int offset = constant - (index * elem_size);




		if (index != 0) {
			ConstantValue* t = ConstantValue::make(index);

			process_index_plus(lhs_loc, rhs_loc, t, stmt_guard, index_guard,
					elem_size);
		}
		if (offset != 0 || index == 0) {
			Constraint cur_c = stmt_guard & index_guard;
			string display_str = rhs_loc->get_access_path(0)->to_string() + "+"
					+ ap->to_string();
			process_offset_plus(lhs_loc, rhs_loc, offset, cur_c, display_str);
		}
	} else {
		if (elem_size == 1) {
			process_index_plus(lhs_loc, rhs_loc, ap, stmt_guard, index_guard,
					elem_size);
		} else if (ap->get_ap_type() == AP_ARITHMETIC) {
			ArithmeticValue* av = (ArithmeticValue*) ap;
			bool divisible = true;
			map<Term*, long int> new_elems;

			if (av->get_constant() % elem_size != 0) {
				divisible = false;
			} else {
				const map<Term*, long int> & elems = av->get_elems();
				map<Term*, long int>::const_iterator it2 = elems.begin();
				for (; it2 != elems.end(); it2++) {
					long int coef = it2->second;
					if (coef % elem_size != 0) {
						divisible = false;
						break;
					}
					long int new_coef = coef / elem_size;
					new_elems[it2->first] = new_coef;
				}
			}
			if (!divisible) {
				report_error(ERROR_INVALID_POINTER_ARITHMETIC,
						"Non-constant Pointer arithmetic "
							"not divisible by elements size not supported");
				il::type* t = lhs_loc->get_representative_access_path()->get_type();
				Constraint cur_c = stmt_guard & index_guard;
				string display_str = rhs_loc->get_access_path(0)->to_string()
						+ "+" + ap->to_string();

				DisplayTag dt(display_str, line, file);
				assign_unmodeled_value(lhs_loc, cur_c, t, UNM_IMPRECISE, dt);
				return;
			}

			long int constant = av->get_constant() / elem_size;
			AccessPath* index_ap = ArithmeticValue::make(new_elems, constant);
			process_index_plus(lhs_loc, rhs_loc, index_ap, stmt_guard,
					index_guard, elem_size);
		}
	}

	check_invariants(lhs_loc, 0, this);
}

void MemoryGraph::process_pointer_plus(Variable* lhs, AccessPath* op1,
		AccessPath* op2, Constraint c, int _elem_size)
{

	assert_context("Processing pointer plus: " + lhs->to_string() + "+ " +
			op1->to_string() + "+" + op2->to_string());
	MemoryLocation* lhs_loc = get_location(lhs);
	MemoryLocation* rhs_loc = get_location(op1);



	add_default_edges(lhs_loc);
	add_default_edges(rhs_loc);
	update_outgoing_edges(lhs_loc, c);

	set<pair<AccessPath*, Constraint> > offset_values;
	get_value_set(op2, offset_values);


	if(!op1->get_type()->is_pointer_type()) {
		string display_str = op1->to_string() + "+" + op2->to_string();
		DisplayTag dt(display_str, line, file);
		assign_unmodeled_value(lhs_loc, c, lhs->get_type(), UNM_IMPRECISE, dt);
		return;
	}

	il::pointer_type* pt = (il::pointer_type*) op1->get_type();
	il::type* elem_type = pt->get_deref_type();
	int elem_size = -1;



	while (elem_type->is_array_type()) {
		il::array_type *at = (il::array_type*) elem_type;
		elem_type = at->get_elem_type();
	}
	elem_size = elem_type->get_size() / 8;



	if (_elem_size != -1)
		elem_size = _elem_size;

	set<pair<AccessPath*, Constraint> >::iterator it = offset_values.begin();
	for (; it != offset_values.end(); it++) {
		AccessPath* ap = it->first;
		Constraint value_c = it->second;


		process_pointer_plus(lhs_loc, rhs_loc, ap, c, value_c, elem_type,
				elem_size);
	}

}

void MemoryGraph::process_pointer_plus(sail::Variable* lhs, sail::Symbol* op1,
		sail::Symbol* op2, Constraint c) {

	assert_context("Processing pointer plus: " + op1->to_string() + "+" +
			op2->to_string());

	AccessPath* lhs_ap = get_access_path_from_symbol(lhs);
	c_assert(lhs_ap->get_ap_type() == AP_VARIABLE);
	AccessPath* op1_ap = get_access_path_from_symbol(op1);
	AccessPath* op2_ap = get_access_path_from_symbol(op2);
	check_memory_access(op1_ap, c, op1->to_string(true));
	check_memory_access(op2_ap, c, op2->to_string(true));
	process_pointer_plus((Variable*) lhs_ap, op1_ap, op2_ap, c);

}

void MemoryGraph::process_plus(Variable* lhs, AccessPath* op1, AccessPath* op2,
		Constraint c)
{

	assert_context("Processing plus: " + lhs->to_string() + "=" +
			op1->to_string() + "+" + op2->to_string());

	if (!op1->get_type()->is_pointer_type()
			&& !op2->get_type()->is_pointer_type()) {
		string pp_exp = op1->to_string() + "+" + op2->to_string();
		process_binop(lhs, op1, op2, c, il::_PLUS, pp_exp);
		return;
	}

	/*
	 * If one of the operands is a pointer, the lhs should also be a pointer.
	 */
	c_assert(lhs->get_type()->is_pointer_type());

	/*
	 * Make sure op2 is the constant
	 */
	if (op2->get_type()->is_pointer_type()) {
		AccessPath* temp = op1;
		op1 = op2;
		op2 = temp;
	}



	process_pointer_plus(lhs, op1, op2, c);
}

void MemoryGraph::process_plus(sail::Variable* lhs, sail::Symbol* op1,
		sail::Symbol* op2, Constraint c) {
	AccessPath* lhs_ap = get_access_path_from_symbol(lhs);
	c_assert(lhs_ap->get_ap_type() == AP_VARIABLE);
	AccessPath* op1_ap = get_access_path_from_symbol(op1);
	AccessPath* op2_ap = get_access_path_from_symbol(op2);
	check_memory_access(op1_ap, c, op1->to_string(true));
	check_memory_access(op2_ap, c, op2->to_string(true));
	process_plus((Variable*) lhs_ap, op1_ap, op2_ap, c);
}

MemoryGraph::~MemoryGraph() {

	set<Edge*>::iterator it2 = edges.begin();
	for (; it2 != edges.end(); it2++) {
		(*it2)->set_final_delete();
		delete *it2;
	}

	set<MemoryLocation*> to_delete;
	map<AccessPath*, MemoryLocation*>::iterator it = ap_to_locs.begin();
	for (; it != ap_to_locs.end(); it++) {
		MemoryLocation* loc = it->second;
		to_delete.insert(loc);
	}
	set<MemoryLocation*>::iterator it3 = to_delete.begin();
	for(; it3 != to_delete.end(); it3++)
		delete *it3;
}

void MemoryGraph::relinquish_ownership(Edge* e)
{
	ap_to_locs.erase(e->get_source_loc()->get_representative_access_path());
	ap_to_locs.erase(e->get_target_loc()->get_representative_access_path());
	edges.erase(e);

}

//-------------------------------------------


void MemoryGraph::print_offset_to_edge_map(map<int, set<Edge*>*>& succs) {
	map<int, set<Edge*>*>::iterator it = succs.begin();
	for (; it != succs.end(); it++) {
		cout << it->first << ": " << "[";
		set<Edge*>::iterator it2 = it->second->begin();
		for (; it2 != it->second->end(); it2++) {
			cout << (*it2)->to_string() << " ";
		}
		cout << "]" << endl;
	}
}

Edge* MemoryGraph::put_edge(AccessPath* source_ap, AccessPath* target_ap,
		Constraint c, bool is_default, bool overwrite_fake_field)
{


	if (c.unsat())
		return NULL;
	c.valid();

	MemoryLocation* source_loc = get_location(source_ap);
	int source_offset = source_loc->find_offset(source_ap);
	MemoryLocation* target_loc = get_location(target_ap);
	int target_offset = target_loc->find_offset(target_ap);
	//TODO: This happens if NULL is upgrade to a struct. Fix this!!
	if(target_offset == -1) target_offset = 0;
	return put_edge(NULL, -1, c, source_loc, target_loc, source_offset,
			target_offset, is_default, overwrite_fake_field);
}

Edge* MemoryGraph::get_existing_edge(MemoryLocation* source,
		MemoryLocation* target, int source_offset, int target_offset)
{

	set<Edge*>* edges = source->get_successors(source_offset);
	if (edges == NULL)
		return NULL;
	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;

 		if (e->get_target_loc() == target && e->get_target_offset()
				== target_offset) {
			return e;

 		}
	}
	return NULL;
}



Edge* MemoryGraph::put_edge(MemoryLocation* orig_source,
		int orig_source_offset, Constraint c, MemoryLocation* source_loc,
		MemoryLocation* target_loc, int source_offset, int target_offset,
		bool default_edge, bool overwrite_fake_field) {


	//if(source_loc->get_access_path(0)->to_string().find("temp1") != string::npos &&
	//		source_offset ==-16 && target_offset == 0)
	//	c_assert(false);


	if (c.unsat())
		return NULL;




	c.valid();
	
	if(!overwrite_fake_field && source_offset < 0 && source_offset > CLIENT_INITIAL_OFFSET) {
		return NULL;
	}
	
	if(source_offset == DELETED_OFFSET && !default_edge)
	{
		AccessPath* rep = source_loc->get_representative_access_path();
		if(rep->get_index_var() != NULL) {
			IndexVariable* iv = rep->get_index_var();
			Constraint old_c = c;
			c.eliminate_evar(iv);
		}
	}

	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	target_ap = target_ap->strip_deref();
	if(target_ap->get_ap_type() == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*)target_ap;
		if(fv->is_adt_pos_selector())
		{
			AccessPath* arg = (fv->get_arguments())[1];
			if(arg->get_base()->get_ap_type() == AP_NIL)
				return NULL;
		}
	}




	/*
	 * Do not put edges to NIL unless 1) you are an ADT 2) you are
	 * a sum temp.
	 */
	/*if(source_loc->get_representative_access_path()->strip_deref()
			->get_ap_type() != AP_ADT &&
			target_loc->get_access_path(target_offset)->
			strip_deref()->get_ap_type() == AP_NIL)
	{

		AccessPath* source_ap = source_loc->get_representative_access_path();
		if(source_ap->get_ap_type() == AP_ADT) source_ap = source_ap->get_inner();
		if(source_ap->get_ap_type() == AP_VARIABLE)
		{
			Variable* v = static_cast<Variable*>(source_ap);
			if(!v->is_summary_temp()){
				return NULL;
			}
		} else{
			return NULL;
		}
	}*/


	/*
	 * The fake targets of ADT's should always have index
	 * constraint i=j.
	 */
	{
		AccessPath* source_ap = source_loc->get_access_path(source_offset);
		if(source_ap->get_ap_type()==AP_ADT)
		{
			IndexVariable* index_var = source_ap->get_index_var();
			AccessPath* target_ap = target_loc->get_access_path(target_offset);
			IndexVariable* target_index =
					target_ap->get_outermost_deref()->find_outermost_index_var(false);
			if(target_index != NULL) {
				target_index = IndexVariable::make_target(target_index);
				c.eliminate_evar(target_index);

				c &= ConstraintGenerator::get_eq_constraint(target_index, index_var);
			}
		}
	}

	/*
	 * If the source is *constant, we cannot put any valid edges. We change the
	 * target to an invalid location.
	 */

	AccessPath* new_source_ap = source_loc->get_access_path(source_offset);
	if (new_source_ap->get_ap_type() == AP_DEREF
			&& new_source_ap->get_inner()->get_ap_type() == AP_CONSTANT) {
		string display_str =  "*" + new_source_ap->get_inner()->to_string();
		DisplayTag dt(display_str, line, file);
		AccessPath* target_ap = Deref::make(UnmodeledValue::make_invalid(
				il::get_integer_type(), dt));
		target_loc = get_location(target_ap);
		target_offset = 0;
	}

	if (orig_source != NULL) {
		IndexVariable* new_index = new_source_ap->find_outermost_index_var();
		if (new_index != NULL) {
			AccessPath* old_source_ap = orig_source->get_access_path(
					orig_source_offset);

			IndexVariable* old_index =
					old_source_ap->find_outermost_index_var();
			if (old_index != NULL) {
				c.replace_term(old_index, new_index);
			}
		}
	}


	Edge* existing = get_existing_edge(source_loc, target_loc, source_offset,
			target_offset);



	Edge* res_e;
	if (existing == NULL) {
		Edge * e = new Edge(ivm, edge_counter++, c, source_loc, target_loc,
				source_offset, target_offset, default_edge,
				ma.get_cur_block_id());
		edges.insert(e);
		res_e =  e;
	} else {

		existing->or_constraint(ivm, c, ma.get_cur_block_id());
		res_e= existing;
	}



	/*
	 * If we added an edge from *X to Y, then register with the
	 * EntryAliasManager that there was a store
	 * into  location X.
	 */
	AccessPath* source_ap = res_e->get_source_ap();
	if(!default_edge)
	{
		//cout << "Calling ad stored on: " << source_ap->strip_deref()->to_string() << endl;
		ea.add_stored(source_ap->strip_deref());
	}

	return res_e;
}


void MemoryGraph::update_outgoing_edges(AccessPath* ap, Constraint c,
		bool update_fake_field) {
	MemoryLocation* loc = get_location(ap);
	int offset = loc->find_offset(ap);
	c_assert(offset!=-1);
	update_outgoing_edges(loc, offset, c, update_fake_field);
}

void MemoryGraph::update_outgoing_edges(MemoryLocation* loc, Constraint c,
		bool update_fake_field) {
	loc->delete_count++;
	set<Edge*> to_delete;
	map<int, set<Edge*>*>::iterator succs_it = loc->get_successor_map().begin();
	for (; succs_it != loc->get_successor_map().end(); succs_it++) {
		int offset = succs_it->first;
		update_outgoing_edges(loc, offset, c, update_fake_field);

	}
	set<Edge*>::iterator it = to_delete.begin();
	for (; it != to_delete.end(); it++)
		delete_edge(*it);
	loc->delete_count--;
}

void MemoryGraph::update_outgoing_edges(MemoryLocation* loc, int offset,
		Constraint c, bool update_fake_field) {

	assert_context("Updating outgoing edges to: " + loc->to_string(false) +
			" offset: " + int_to_string(offset) + " type: " +
			loc->get_representative_access_path()->get_type()->to_string());


	if(!update_fake_field && offset < 0 && offset > CLIENT_INITIAL_OFFSET){
		return;
	}

	if(offset == DELETED_OFFSET)
	{
		AccessPath* rep = loc->get_representative_access_path();
		if(rep->get_index_var() != NULL) {
			IndexVariable* iv = rep->get_index_var();
			c.eliminate_evar(iv);
		}
	}


	if(loc->get_successor_map().count(offset) == 0)
	{
		//cout << this->to_dotty() << endl;
		if(offset < 0) loc->add_offset(offset, il::get_integer_type());
		else c_assert(false);
	}
	loc->delete_count++;

	set<Edge*> to_delete;
	set<Edge*>* edges = loc->get_successors(offset);
	set<Edge*>::iterator it = edges->begin();
	for (; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint orig_c = e->get_constraint();
		e->and_not_constraint(ivm, c);
		Constraint new_c = e->get_constraint();
		if (e->get_constraint().unsat()) {
			if (e->in_delete_queue)
				continue;
			to_delete.insert(e);
			e->in_delete_queue = true;
		}
	}
	it = to_delete.begin();
	for (; it != to_delete.end(); it++)
		delete_edge(*it);
	loc->delete_count--;

}

void MemoryGraph::delete_edge(Edge* e) {
	e->in_delete_queue = true;
	edges.erase(e);
	delete e;

}

Constraint MemoryGraph::get_index_constraint(AccessPath* index,
		MemoryLocation* array_loc) {

	IndexVariable* source_index = ivm.get_source_index_var(array_loc);
	set<pair<AccessPath*, Constraint> > index_values;
	get_value_set(index, index_values);
	Constraint index_constraint(false);
	set<pair<AccessPath*, Constraint> >::iterator it = index_values.begin();
	for (; it != index_values.end(); it++) {
		Constraint ap_constraint =
				cg.get_eq_constraint(source_index, it->first);
		Constraint value_c = it->second;
		Constraint cur_c = ap_constraint & value_c;
		index_constraint = index_constraint | cur_c;
	}
	return index_constraint;

}

void MemoryGraph::update_memory_location(MemoryLocation* loc,
		AccessPath* old_prefix, AccessPath* new_prefix, AccessPath* old_rep) {

	assert_context("Updating memory location. Old prefix: " +
			AccessPath::safe_string(old_prefix) +
			" new prefix: " + AccessPath::safe_string(new_prefix));


	if (old_rep == NULL)
		old_rep = loc->get_representative_access_path();
	AccessPath* new_rep = loc->get_representative_access_path();

	new_rep = new_rep->replace(old_prefix, new_prefix);

	/*
	 * Fix length fields
	 */
	AccessPath* length = FunctionValue::make_length(old_rep)->add_deref();
	if(has_location(length))
	{
		MemoryLocation* length_loc = get_location(length);
		update_memory_location(length_loc, old_prefix, new_prefix,
				length_loc->get_representative_access_path());
	}


	/*
	 * Remember old default locations
	 */
	vector<MemoryLocation*> rec;
	map<int, AccessPath*>::iterator it;
	for (it = loc->get_access_path_map().begin(); it
			!= loc->get_access_path_map().end(); it++) {
		AccessPath* ap = it->second;
		if (ap->is_size_field_ap())
			continue;

		AccessPath* drf = ap->add_deref();
		if (ap_to_locs.count(drf) > 0) {
			MemoryLocation* def_target = get_location(drf);
			rec.push_back(def_target);
		}
	}

	bool changed = loc->update_access_paths(old_prefix, new_prefix);

	if (changed) {
		/* Make recursive call to update default targets
		 */
		for (unsigned int i = 0; i < rec.size(); i++) {
			update_memory_location(rec[i], old_prefix, new_prefix);
		}

		IndexVariable* index_var = new_prefix->get_index_var();
		if (index_var == NULL) {
			return;
		}

		/*
		 * Now update incoming edges
		 */

		map<int, set<Edge*>*>::iterator it2 =
				loc->get_predecessor_map().begin();
		for (; it2 != loc->get_predecessor_map().end(); it2++) {
			set<Edge*>::iterator it3 = it2->second->begin();
			for (; it3 != it2->second->end(); it3++) {
				Edge* e = *it3;
				AccessPath* pred = e->get_source_loc()->get_access_path(
						e->get_source_offset());

				if (pred->contains_index_var(index_var)) {

					IndexVariable* source = IndexVariable::make_source(
							index_var);
					IndexVariable* target = IndexVariable::make_target(
							index_var);
					Constraint c = cg.get_eq_constraint(source, target);
					e->and_constraint(ivm, c);
				}

				else {
					IndexVariable* target = IndexVariable::make_target(
							index_var);
					Constraint c = cg.get_eqz_constraint(target);
					e->and_constraint(ivm, c);
				}

			}
		}

		// and outgoing edges
		it2 = loc->get_successor_map().begin();
		for (; it2 != loc->get_successor_map().end(); it2++) {
			if (loc->is_size_field(it2->first))
				continue;
			if(it2->first == DELETED_OFFSET) continue;
			Constraint update_c(false);
			set<Edge*>::iterator it3 = it2->second->begin();
			for (; it3 != it2->second->end(); it3++) {
				Edge* e = *it3;
				AccessPath* succ = e->get_target_loc()->get_access_path(
						e->get_target_offset());
				if (succ->contains_index_var(index_var))
					continue;
				IndexVariable* source = IndexVariable::make_source(index_var);
				Constraint c = cg.get_eqz_constraint(source);
				e->and_constraint(ivm, c);
				update_c = update_c | c;
			}
			if (update_c.valid())
				continue;

			add_default_edge(loc, it2->first, update_c);
		}
	}

	c_assert(ap_to_locs.count(old_rep) > 0);


	if(new_rep->is_derived_from_constant())
	{
		ap_to_locs.erase(old_rep);
	}

	loc->set_representative(new_rep);
	ap_to_locs[new_rep] = loc;

}

void MemoryGraph::report_errors()
{
	set<ErrorTrace*>::iterator it = error_traces.begin();
	for(; it!= error_traces.end(); it++) {
		ErrorTrace* et = *it;
		et->report_error(errors, su->get_identifier());
	}

	error_traces.clear();
}

AccessPath* MemoryGraph::get_default_target_of_size(AccessPath* size_ap)
{

	assert_context("Getting default target for " + size_ap->to_string());
	c_assert(size_ap->get_ap_type() == AP_FIELD);
	FieldSelection* fs = (FieldSelection*) size_ap;
	c_assert(fs->is_size_field());

	AccessPath* inner = size_ap->get_inner();
	if(inner->get_ap_type() != AP_ARRAYREF)
		return size_ap->add_deref();

	ArrayRef* ar = (ArrayRef*) inner;
	int elem_size = ar->get_elem_size();
	IndexVariable* iv = ar->get_index_var();

	AccessPath* new_size_field = size_ap->replace(ar, ar->get_inner());

	AccessPath* offset = ArithmeticValue::make_times(iv,
			ConstantValue::make(elem_size));
	AccessPath* new_size = ArithmeticValue::make_minus(new_size_field,
			offset);
	new_size = new_size->add_deref();

	return new_size;
}

void MemoryGraph::update_size_targets_on_upgrade(MemoryLocation* loc,
		AccessPath* new_rep, int elem_size) {


	IndexVariable* iv = new_rep->get_index_var();
	IndexVariable* target_iv = IndexVariable::make_target(iv);
	/*
	 * Update targets of the size field to take offsets
	 * into account.
	 */
	set<Edge*>* size_targets = loc->get_successors(SIZE_OFFSET);
	set<Edge*>::iterator it = size_targets->begin();

	set<pair<pair<AccessPath*, AccessPath*> , pair<Constraint, bool> > > to_add;

	vector<Edge*> edges;
	for (; it != size_targets->end(); it++) {
		Edge* e = *it;
		edges.push_back(e);
		AccessPath* size_ap = e->get_target_ap();
		AccessPath* size_ap_stripped = size_ap->strip_deref();
		bool is_default = e->has_default_target();

		if (size_ap_stripped->is_size_field_ap()) {
			FieldSelection* fs = (FieldSelection*) size_ap_stripped;
			AccessPath* inner = size_ap_stripped->get_inner();
			// we are dealing with a default target
			if (inner == new_rep) {
				AccessPath* new_inner = inner->get_inner();
				c_assert(new_inner != NULL);
				size_ap_stripped = FieldSelection::make(fs->get_field_name(),
						fs->get_field_offset(), fs->get_type(), new_inner);
			}
		}

		AccessPath* offset = ArithmeticValue::make_times(iv,
				ConstantValue::make(elem_size));
		AccessPath* new_size = ArithmeticValue::make_minus(size_ap_stripped,
				offset);
		new_size = new_size->add_deref();
		Constraint edge_c = ConstraintGenerator::get_eq_constraint(iv,
				target_iv);
		edge_c &= e->get_constraint();
		to_add.insert(
				make_pair(make_pair(e->get_source_ap(), new_size),
						make_pair(edge_c, is_default)));
	}
	for (unsigned int i = 0; i < edges.size(); i++)
		delete_edge(edges[i]);

	set<pair<pair<AccessPath*, AccessPath*> , pair<Constraint, bool> > >::iterator it2 =
			to_add.begin();
	for (; it2 != to_add.end(); it2++) {
		AccessPath* source = it2->first.first;
		AccessPath* target = it2->first.second;
		Constraint c = it2->second.first;
		bool is_default = it2->second.second;

		put_edge(source, target, c, is_default, true);
	}

}

IndexVariable* MemoryGraph::change_to_adt_loc(MemoryLocation* loc, int offset,
		il::type* adt_type)
{
	assert_context("Change to ADT location: " + loc->to_string() + " type: " +
			(adt_type ? adt_type->to_string() : "null"));

	cout << "Changing to array loc: " << loc->to_string() << endl;

	AccessPath* ap  = loc->get_access_path(offset);

	if(ap->get_ap_type() == AP_ADT)
		return ap->find_outermost_index_var(true);


	cout << "Changing to array loc 222: " << loc->to_string() << endl;
	c_assert(adt_type->is_abstract_data_type());
	il::type* key_t = adt_type->get_adt_key_type();
	il::type* val_t = adt_type->get_adt_value_type();

	IndexVariable* dummy = ivm.get_dummy_index();
	AccessPath* old_rep = ap->find_representative();
	old_rep->update_type(adt_type);

	AccessPath* dummy_adt = AbstractDataStructure::make(old_rep,dummy,
			key_t, val_t, adt_type);
	IndexVariable* index_var = ivm.get_new_source_index(dummy_adt, false);
	AccessPath* new_rep = AbstractDataStructure::make(old_rep,index_var,
			key_t, val_t, adt_type);

	update_memory_location(loc, old_rep, new_rep);

	c_assert(new_rep->get_ap_type() == AP_ADT);
	AbstractDataStructure* new_adt = (AbstractDataStructure*) new_rep;
	IndexVariable* iv = new_adt->get_index_var();


	//loc->add_size_field();
	//if (!loc->size_field_has_target()) {
		//add_default_edge(loc, SIZE_OFFSET, Constraint(false));
	//}


	//update_size_targets_on_upgrade(loc, new_adt, 1);

	map<Term*, Term*> subs;
	subs[iv] = ConstantValue::make(0);
	Term* no_shift = new_adt->substitute(subs);
	update_constraints(old_rep->to_term(), no_shift);
	return index_var;
}

IndexVariable* MemoryGraph::change_to_array_loc(MemoryLocation* loc,
		int elem_size)
{



	assert_context("Changing location to array: " + loc->to_string(true) +
			" element size: " + int_to_string(elem_size));

	IndexVariable* dummy = ivm.get_dummy_index();
	AccessPath* old_rep = loc->get_representative_access_path();
	AccessPath* dummy_array = ArrayRef::make(old_rep, dummy,
			old_rep->get_type(), elem_size);
	IndexVariable* index_var = ivm.get_new_source_index(dummy_array, false);

	AccessPath* new_rep = ArrayRef::make(old_rep, index_var,
			old_rep->get_type(), elem_size);

	update_memory_location(loc, old_rep, new_rep);
	ea.process_ap_upgrade(old_rep, new_rep);
	c_assert(new_rep->get_ap_type() == AP_ARRAYREF);
	ArrayRef* new_array = (ArrayRef*) new_rep;
	IndexVariable* iv = new_array->get_index_var();

	loc->add_size_field();
	if (!loc->size_field_has_target()) {
		add_default_edge(loc, SIZE_OFFSET, Constraint(false));
	}

	//new
	else {
		update_size_targets_on_upgrade(loc, new_array, elem_size);
	}

	map<Term*, Term*> subs;
	subs[iv] = ConstantValue::make(0);
	Term* array_zero = new_array->substitute(subs);
	update_constraints(old_rep->to_term(), array_zero);


	/*
	 * If we are upgrading a string literal to an array, we need to
	 * expand its contents and update its size field.
	 */
	if(old_rep->get_ap_type() == AP_STRING)
	{
		// kill old edges; we don't want any default targets.
		Constraint true_c(true);
		update_outgoing_edges(loc, 0, true_c);
		StringLiteral* sl = (StringLiteral*) old_rep;
		const string& s = sl->get_string_constant();
		map<int, Constraint> targets;

		int string_size = s.size() + 1;
		for (unsigned int i = 0; i <= s.size(); i++) {
			ConstantValue* cv = ConstantValue::make(i);
			Constraint edge_c = cg.get_eq_constraint(cv, index_var);
			int cur_c = (i < s.size() ? s[i] : 0);
			if(targets.count(cur_c) == 0)
				targets[cur_c] = Constraint(false);
			targets[cur_c] |= edge_c;
		}
		map<int, Constraint>::iterator it = targets.begin();
		for(; it!= targets.end(); it++)
		{
			int val = it->first;
			ConstantValue* cv = ConstantValue::make(val);
			MemoryLocation *c_loc = get_location(Deref::make(cv));
			Constraint c = it->second;

			Edge * e = new Edge(ivm, edge_counter++, c, loc, c_loc,
							0, 0, false, ma.get_cur_block_id());
			edges.insert(e);
		}

		AccessPath* size_ap =
				ArithmeticValue::make_minus(ConstantValue::make(string_size),
						index_var)->add_deref();
		update_outgoing_edges(loc, SIZE_OFFSET, true_c, true);
		Constraint edge_c = ConstraintGenerator::get_eq_constraint(index_var,
				IndexVariable::make_target(index_var));
		put_edge(NULL, 0, edge_c, loc, get_location(size_ap), SIZE_OFFSET, 0,
				false, true);


	}


	return index_var;
}

IndexVariable* MemoryGraph::change_to_array_loc(MemoryLocation* loc,
		il::type* elem_type) {
	return change_to_array_loc(loc, elem_type->get_size() / 8);
}

void MemoryGraph::update_constraints(Term* old_term, Term* new_term) {

	set<Edge*>::iterator it = edges.begin();
	for (; it != edges.end(); it++) {
		Edge* e = *it;

		Constraint& c = e->get_constraint();
		c.replace_term(old_term, new_term);

	}

	ma.update_statement_guards(old_term, new_term);

	set<ErrorTrace*>::iterator it2 = error_traces.begin();
	for (; it2 != error_traces.end(); it2++) {
		ErrorTrace* et = *it2;
		Constraint error_c = et->get_failure_cond();
		error_c.replace_term(old_term, new_term);
		et->set_failure_cond(error_c);
	}

	Constraint::replace_term_in_axioms(old_term, new_term);

	AccessPath* old_size = FieldSelection::make_size(AccessPath::to_ap(old_term));
	AccessPath* new_size = FieldSelection::make_size(AccessPath::to_ap(new_term));

	Constraint::replace_term_in_axioms(new_size->to_term(), old_size->to_term());



}

AccessPath* MemoryGraph::find_index_var_to_increment(AccessPath* ap,
		int elem_size) {

	//IndexVariable* last_index_var = NULL;
	AccessPath* last_ar = NULL;

	while (true) {
		if (ap->get_ap_type() != AP_FIELD && ap->get_ap_type() != AP_ARRAYREF &&
				ap->get_ap_type() != AP_ADT) {
			return last_ar;
		}

		if (ap->get_ap_type() == AP_FIELD) {
			FieldSelection* fs = (FieldSelection*) ap;
			ap = fs->get_inner();
			continue;
		}

		if (ap->get_ap_type() == AP_ARRAYREF) {
			ArrayRef* are = (ArrayRef*) ap;
			il::type* cur_type = are->get_type();
			while (cur_type->is_array_type()) {
				il::array_type* at = (il::array_type*) cur_type;
				cur_type = at->get_elem_type();
			}
			if (cur_type->get_size() / 8 == elem_size)
				return are;

			ap = are->get_inner();
			last_ar = are;

		}

		if(ap->get_ap_type() == AP_ADT)
		{
			return ap;
		}
	}

	return NULL;
}

//----------------------------------------------------------

void MemoryGraph::process_static_assert(sail::Symbol* pred, Constraint &c,
		string loc) {
	set<pair<AccessPath*, Constraint> > values;
	get_value_set(pred, values);
	set<AccessPath*> sat_values;
	c_assert(sat_values.size() <= 2);
	set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
	Constraint fail_c(false);

	ConstantValue * one = ConstantValue::make(1);

	for (; it != values.end(); it++) {
		if (it->first == one)
			continue;
		AccessPath* ap = it->first;
		Constraint eqz_c = ConstraintGenerator::get_eqz_constraint(ap);
		Constraint cur_c = it->second;
		Constraint res_c = cur_c & eqz_c & c;
		if (res_c.unsat())
			continue;
		fail_c |= cur_c;

	}


	if (fail_c.unsat()) {
		return;
	}

	cout << "Made static assert fail c: " << fail_c << endl;
	cout << "Statement guard: " << c << endl;


	AssertionTrace* et = AssertionTrace::make(ERROR_STATIC_ASSERT, line, file,
					fail_c & c, ma.get_cur_iteration_counter(), c);
	error_traces.insert(et);

}

//----------------------------------------------------------
void MemoryGraph::print_ap_to_loc_map(bool pp) {
	cout << "****AP TO LOCS MAP*****" << endl;
	map<AccessPath*, MemoryLocation*>::iterator it = ap_to_locs.begin();
	for (; it != ap_to_locs.end(); it++) {
		string ap = it->first->to_string();
		MemoryLocation* loc = it->second;
		cout << ap << "-> " << loc->to_string(pp) << endl;
	}
	cout << "**** END AP TO LOCS MAP*****" << endl;
}

void MemoryGraph::collect_reachable_locations(MemoryLocation* cur, set<
		MemoryLocation*> & res) {
	if (res.count(cur) > 0)
		return;


	res.insert(cur);
	map<int, set<Edge*>*> & succs = cur->get_successor_map();
	map<int, set<Edge*>*>::iterator it = succs.begin();
	for (; it != succs.end(); it++) {
		set<Edge*>* suc = it->second;
		set<Edge*>::iterator it2 = suc->begin();
		for (; it2 != suc->end(); it2++) {
			Edge* e = *it2;
			collect_reachable_locations(e->get_target_loc(), res);
		}
	}

}

string MemoryGraph::to_dotty() {

	set<MemoryLocation*> to_print;
	set<MemoryLocation*>::iterator it_loc = root_locs.begin();
	for (; it_loc != root_locs.end(); it_loc++) {
		MemoryLocation* root = *it_loc;
		collect_reachable_locations(root, to_print);
	}

	map<AccessPath*, MemoryLocation*>::iterator ii = ap_to_locs.begin();
	for(; ii != ap_to_locs.end(); ii++)
		to_print.insert(ii->second);


	string res;
	res = "digraph G { rankdir=LR;\n";
	res += "ranksep=1.2;\n";
	res += "nodesep=0.9;\n";
	set<MemoryLocation*>::iterator it = to_print.begin();
	for (; it != to_print.end(); it++) {
		MemoryLocation* loc = *it;

		res += string("node") + int_to_string((long int) loc)
				+ " [shape = record]";
		res += "[port=e][height=0.1]";
		if (loc->is_interface_object())
			res += "[color=orange]";
		AccessPath* ap = loc->get_representative_access_path();
		if (ap->get_ap_type() == AP_VARIABLE) {
			Variable* v = (Variable*) ap;
			if (v->is_summary_temp())
				res += "[color=green]";
		}
		res += "[label= \"";
		res += loc->to_dotty();
		res += "\"]\n";

		std::map<int, set<Edge*>*> & succs = loc->get_successor_map();
		std::map<int, set<Edge*>*>::iterator it2 = succs.begin();
		for (; it2 != succs.end(); it2++) {
			set<Edge*> & ss = (*it2->second);
			set<Edge*>::iterator it3 = ss.begin();
			for (; it3 != ss.end(); it3++) {
				Edge* e = *it3;
				//e->get_constraint().sat();
				if (e->is_recursive_edge())
					res += "edge[color=red]\n";
				else if (e->has_default_target())
					res += "edge[color=blue]\n";
				else
					res += "edge[color = black]\n";
				string s = e->get_constraint().to_string();
				s += " [" + int_to_string(e->get_time_stamp()) + "]";
				res += "\"node" + int_to_string((long int) loc) + "\"";
				if (e->get_source_offset() >= 0)
					res += ":off_s" + int_to_string(e->get_source_offset());
				else res += ":off_s_minus" + int_to_string(-e->get_source_offset());
				//else if(e->get_source_offset() == SIZE_OFFSET)
				//	res += ":off_size_s";

				res += " -> ";
				res += "\"node" + int_to_string((long int) e->get_target_loc())
						+ "\"";
				if (e->get_target_offset() >= 0)
					res += ":off_e" + int_to_string(e->get_target_offset());
				else
					//res += ":off_size_e";
					res += ":off_e_minus" + int_to_string(-e->get_target_offset());
				res += " [label = \"";
				res += s;
				res += "\"]\n";

			}
		}

	}

	res += "}\n";
	return res;

}

string MemoryGraph::to_dotty(set<AccessPath*> & syms)
{
	set<Edge*> to_print;
	set<AccessPath*>::iterator it = syms.begin();
	for(; it != syms.end(); it++)
	{
		AccessPath* ap = *it;
		MemoryLocation* loc = get_location(ap);
		ap = ap->replace(ap->find_representative(),
				loc->get_representative_access_path());
		set<int> offsets;

		map<int, AccessPath*>::iterator off_it =
				loc->get_access_path_map().begin();
		for(; off_it!= loc->get_access_path_map().end(); off_it++) {
			//if(off_it->first < 0)
			offsets.insert(off_it->first);
			//if(off_it->first > 0) break;
		}
		int offset = loc->find_offset(ap);

		if(offset != -1) offsets.insert(offset);
		else {
			const map<int, AccessPath*> & offset_map =
					loc->get_access_path_map();
			map<int, AccessPath*>::const_iterator it = offset_map.begin();
			for(; it!= offset_map.end(); it++) {
				offsets.insert(it->first);
			}
		}


		set<int>::iterator it = offsets.begin();
		for(; it!= offsets.end(); it++) {
			int offset = *it;
			to_print.insert(loc->get_successors(offset)->begin(),
					loc->get_successors(offset)->end());
		}


	}

	set<MemoryLocation*> valid_sources;
	set<Edge*>::iterator it3 = to_print.begin();
	for(; it3 != to_print.end(); it3++)
	{
		valid_sources.insert((*it3)->get_source_loc());
		valid_sources.insert((*it3)->get_target_loc());
	}

	set<Edge*> worklist = to_print;
	while(worklist.size() > 0)
	{
		Edge* e = *worklist.begin();
		worklist.erase(e);
		if(valid_sources.count(e->get_source_loc()) == 0) continue;

		to_print.insert(e);

		map<int, set<Edge*>* >& succs_map =
				e->get_target_loc()->get_successor_map();
		map<int, set<Edge*>* >::iterator it = succs_map.begin();
		for(; it!= succs_map.end(); it++) {
			int offset = it->first;
			//if(offset < 0 || offset == e->get_target_offset())
			{
				set<Edge*>* succs = it->second;
				set<Edge*>::iterator it2 = succs->begin();
				for(; it2 != succs->end(); it2++)
				{
					Edge* cur_suc = *it2;
					if(to_print.count(cur_suc) > 0) continue;
					worklist.insert(cur_suc);
				}
			}
		}


	}

	set<MemoryLocation*> printed;

	string res;
	res = "digraph G { rankdir=LR;\n";
	res+= "ranksep=1.2;\n";
	res+= "nodesep=0.7;\n";
	set<Edge*>::iterator it2 = to_print.begin();
	for(; it2!= to_print.end(); it2++)
	{
		Edge* e = *it2;
		if(e->get_source_ap()->is_constant()) continue;

		MemoryLocation* loc = e->get_source_loc();
		if(printed.count(loc) == 0){
			res += string("node") + int_to_string((long int) loc)
							+ " [shape = record]";
			res += "[port=e][height=0.1]";
			if (loc->is_interface_object())
				res += "[color=orange]";
			AccessPath* ap = loc->get_representative_access_path();
			if (ap->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) ap;
				if (v->is_summary_temp())
					res += "[color=green]";
			}
			res += "[label= \"";
			res += loc->to_dotty();
			res += "\"]\n";
		}

		loc = e->get_target_loc();
		if(printed.count(loc) == 0){
			res += string("node") + int_to_string((long int) loc)
							+ " [shape = record]";
			res += "[port=e][height=0.1]";
			if (loc->is_interface_object())
				res += "[color=orange]";
			AccessPath* ap = loc->get_representative_access_path();
			if (ap->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) ap;
				if (v->is_summary_temp())
					res += "[color=green]";
			}
			res += "[label= \"";
			res += loc->to_dotty();
			res += "\"]\n";
		}



		if (e->is_recursive_edge())
			res += "edge[color=red]\n";
		else if (e->has_default_target())
			res += "edge[color=blue]\n";
		else
			res += "edge[color = black]\n";
		string s = e->get_constraint().to_string();
		s += " [" + int_to_string(e->get_time_stamp()) + "]";
		res += "\"node" + int_to_string((long int) e->get_source_loc()) + "\"";
		if (e->get_source_offset() >= 0)
			res += ":off_s" + int_to_string(e->get_source_offset());
		else
			//res += ":off_size_s";
			res += ":off_s_minus" + int_to_string(-e->get_source_offset());
		res += " -> ";
		res += "\"node" + int_to_string((long int) e->get_target_loc())
				+ "\"";
		if (e->get_target_offset() >= 0)
			res += ":off_e" + int_to_string(e->get_target_offset());
		else
			//res += ":off_size_e";
			res += ":off_e_minus" + int_to_string(-e->get_target_offset());
		res += " [label = \"";
		res += s;
		res += "\"]\n";

	}



	res+="}\n";

	return res;
}

