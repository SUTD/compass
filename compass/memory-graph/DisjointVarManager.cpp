/*
 * DisjointVarManager.cpp
 *
 *  Created on: Mar 21, 2009
 *      Author: tdillig
 */

#include "DisjointVarManager.h"
#include "util.h"
#include "il/type.h"
#include "Variable.h"
#include "ConstraintGenerator.h"
#include "ConstantValue.h"
#include "MemoryGraph.h"
#include "compass_assert.h"
#include "SummaryGraph.h"
#include "access-path.h"


DisjointVarManager::DisjointVarManager()
{
	this->count = 0;
	this->id = "";

}

void DisjointVarManager::set_id(int id)
{
	this->id = int_to_string(id);
}

void DisjointVarManager::clear()
{
	this->count = 0;
}


Variable* DisjointVarManager::get_disjointness_constraints(int num,
		vector<Constraint> & res)
{
	cout << "---- D count is: " << count << endl;
	//il::type* t = num<=2 ? il::get_integer_type():il::get_unsigned_integer_type();
	il::type* t = il::get_integer_type();
	Variable* v = Variable::make_disjointness_var(count++, t);
	c_assert(num >1);
	if(num == 2) {
		Constraint c1 = ConstraintGenerator::get_eqz_constraint(v);
		res.push_back(c1);
		Constraint c2 = ConstraintGenerator::get_neqz_constraint(v);
		res.push_back(c2);
		return v;
	}

	//----------
	{
	AccessPath* ap = ConstantValue::make(0);
		Constraint c = ConstraintGenerator::get_leq_constraint(v, ap);
		res.push_back(c);
	}
	//---------
	for(int i=1; i < num-1; i++) {
		AccessPath* ap = ConstantValue::make(i);
		Constraint c = ConstraintGenerator::get_eq_constraint(v, ap);
		res.push_back(c);
	}
	AccessPath* ap = ConstantValue::make(num-1);
	Constraint c = ConstraintGenerator::get_geq_constraint(v, ap);
	res.push_back(c);
	return v;

}


void DisjointVarManager::eliminate_disjoint_vars(Constraint & c)
{
	set<Term*> vars;
	c.get_free_variables(vars);
	set<VariableTerm*> disjoint_vars;
	set<Term*>::iterator it = vars.begin();
	for(; it!= vars.end(); it++)
	{
		Term* t = *it;
		if(is_disjoint_var(t)) {
			disjoint_vars.insert((VariableTerm*)t);
		}

	}
	c.eliminate_evars(disjoint_vars);
}



bool DisjointVarManager::is_disjoint_var(Term* t)
{
	if(t->get_specialization() != AP_VARIABLE) return false;
	VariableTerm* vt = (VariableTerm*) t;
	return vt->get_id_attribute() == ATTRIB_DISJOINT;
}



bool DisjointVarManager::contains_disjointness_vars(Constraint c)
{
	set<Term*> vars;
	c.get_free_variables(vars);
	set<Term*>::iterator it = vars.begin();
	for(; it!= vars.end(); it++)
	{
		if(is_disjoint_var(*it)) return true;
	}
	return false;
}

/*
 * Replaces disjointness constraints with what they actually mean, as
 * given by the map in the SummaryGraph. This function is currently
 * only used for the purposes of explaining/diagnosing errors.
 *
 *
 * If constraint c contains a disjointness variable, e.g., d=3,
 * use information in summary graph to conjoin what we know about d=3, i.e.
 * c & d=3=> phi
 * and eliminate this d variable.
 */

 Constraint DisjointVarManager::replace_disjointness_constraints(Constraint c,
		SummaryGraph* sg)
{

	 cout << "REPLACING DISJOINTNESS CONSTRAINTS IN " << c << endl;

	set<Term*> disjointness_terms;
	set<Term*> terms;

	/*
	 * Mapping from each d(i) function to instantiation set of i.
	 */
	map<string, set<Term*> > inst_set;

	/*
	 * Find all disjointness terms in this constraint
	 */
	c.get_terms(terms, false);
	for(set<Term*>::iterator it = terms.begin(); it!= terms.end(); it++) {
		Term* t = *it;
		AccessPath* ap =AccessPath::to_ap(t);
		cout << "=> cur: " << ap->to_string() << endl;

		if(ap->get_ap_type() == AP_VARIABLE) {
			Variable* v = static_cast<Variable*>(ap);
			if(v->is_disjointness_var()) {
				cout << "Disjointness var: " << v->to_string() << endl;
				disjointness_terms.insert(t);
			}
		}

		else if(ap->get_ap_type() == AP_FUNCTION) {
			FunctionValue* fv = static_cast<FunctionValue*>(ap);
			if(fv->is_disjoint()) {
				assert(fv->get_arguments().size() > 0);
				disjointness_terms.insert(t);
				inst_set[fv->get_name()].insert(fv->to_term());
				cout << "Disjointness fun: " << fv->to_string() << endl;
			}
		}

	}

	Constraint expanded_c = c;

	set<VariableTerm*> to_eliminate;

	/*
	 * Iterate over translation map and conjoin what we know about each d=c
	 * constraint
	 */

	cout << "===============" << endl;
	map<Constraint, Constraint>& d_map = sg->get_disjointness_map();
	for(map<Constraint, Constraint>::iterator it = d_map.begin();
			it != d_map.end(); it++)
	{
		Constraint key_c = it->first;
		Constraint value_c = it->second;

		cout << "MAP entry: " << key_c << " -> " << value_c << endl;

		set<Term*> key_terms;
		key_c.get_terms(key_terms, false);
		AccessPath* disjointness_term = NULL;

		/*
		 * Find the disjointness term in the key
		 */
		set<Term*>::iterator it2 = key_terms.begin();
		for(; it2!= key_terms.end(); it2++)
		{
			Term* t= *it2;
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap->get_ap_type() == AP_VARIABLE) {
				Variable* v = static_cast<Variable*>(ap);
				if(v->is_disjointness_var()) {
					disjointness_term = v;
					break;
				}
			}
			else if(ap->get_ap_type() == AP_FUNCTION)
			{
				FunctionValue* fv= static_cast<FunctionValue*>(ap);
				if(fv->is_disjoint()) {
					disjointness_term = fv;
					break;
				}
			}

		}

		assert(disjointness_term != NULL);
		if(disjointness_terms.count(disjointness_term->to_term()) > 0) {
			Term* t = disjointness_term->to_term();
			assert(t->get_term_type() == VARIABLE_TERM);
			to_eliminate.insert(static_cast<VariableTerm*>(t));
			Constraint key_implies_value = (!key_c) | value_c;
			Constraint val_implies_key = (!value_c) | key_c;
			expanded_c &= key_implies_value;
			expanded_c &= val_implies_key;
		}

		else {
			if(disjointness_term->get_ap_type() == AP_FUNCTION) {
				FunctionValue* fv = (FunctionValue*) disjointness_term;

				cout << "Replacing term: " << fv->to_string() << endl;

				string name = fv->get_name();
				if(inst_set.count(name) > 0) {
					set<Term*>& inst = inst_set[name];
					for(set<Term*>::iterator it = inst.begin();
							it!= inst.end(); it++)
					{
						Term* inst_exp = *it;

						cout << "Inst exp: " << inst_exp->to_string() << endl;
						Constraint inst_key_c = key_c;
						inst_key_c.replace_term(fv, inst_exp);

						cout << "Inst key c: " << inst_key_c << endl;

						Constraint inst_value_c = value_c;
						inst_value_c.replace_term(fv, inst_exp);
						cout << "Inst value c: " << inst_value_c << endl;

						Constraint key_imp_val = (!inst_key_c) | inst_value_c;
						cout << "key imp val: " << key_imp_val << endl;

						cout << "Cur expanded_c 1: " << expanded_c << endl;

						expanded_c &=  key_imp_val;

						cout << "Cur expanded_c 2: " << expanded_c << endl;

						Constraint val_imp_key = (!inst_value_c) | inst_key_c;
						cout << "val imp key: " << val_imp_key << endl;
						expanded_c &=  val_imp_key;

						cout << "Cur expanded_c 3: " << expanded_c << endl;

						Variable* temp = Variable::make_temp(il::get_integer_type());
						expanded_c.replace_term(inst_exp, temp);

						cout << "Replaced: " << expanded_c << endl;
						to_eliminate.insert(temp);

					}
				}


			}

		}
	}

	cout << "Expanded c: " << expanded_c << endl;

	expanded_c.eliminate_evars(to_eliminate);
	cout << "AFTER ELIMINATE: " << expanded_c << endl;

	IndexVarManager::eliminate_source_vars(expanded_c);
	IndexVarManager::eliminate_target_vars(expanded_c);
	IndexVarManager::eliminate_sum_index_vars(expanded_c);
	expanded_c.sat();
	return expanded_c;
}

