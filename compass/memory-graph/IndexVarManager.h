/*
 * IndexVarManager.h
 *
 *  Created on: Oct 16, 2008
 *      Author: isil
 */

#ifndef INDEXVARMANAGER_H_
#define INDEXVARMANAGER_H_
#include <string>
#include <vector>
#include <map>
#include <set>
using namespace std;

class Constraint;
class MemoryLocation;
class IndexVariable;
class AccessPath;
class Term;




class IndexVarManager {
private:

	/*
	 * A mapping from access paths to the index variables
	 * associated with them.
	 */
	map<AccessPath*, IndexVariable*> ap_to_index_map;

	// Used for checking if we already got an index variable for some location
	IndexVariable* dummy_index;

public:
	IndexVarManager();

	IndexVariable* get_dummy_index();


	/*
	 * Gives a fresh index variable i.
	 */
	IndexVariable* get_new_source_index(AccessPath* inner, bool is_unsigned);



	/*
	 * Returns a new constraint where the value of the index var
	 * in orig c is index_var + inc_value.
	 * For example, if constraint is j=1, index var is j, and
	 * inc_value is 2, this function will return the constraint j=3.
	 */
	Constraint increment_index_var(Constraint orig_c, IndexVariable* index_var,
			AccessPath* inc_value);

	static void get_index_vars(Constraint c, set<IndexVariable*>& index_vars);

	static void get_sum_index_vars(AccessPath* ap, set<IndexVariable*>&
			sum_indices);


	/*
	 * Given a constraint that should relate source and target, it renames all
	 * the unused index variables in the constraint. E.g.,
	 * if the constarint is i=2 but the source access path doesn't
	 * contain an i, the constraint becomes f=2, which gets simplified to
	 * true because f is implicitly existentially quantified.
	 */
	void remove_unused_index_vars(Constraint& c, AccessPath* source,
					AccessPath* target);

	/*
	 * Removes all source and target variables from the constraint.
	 */
	void remove_all_index_vars(Constraint& c);

	/*
	 * Removes all k's.
	 */
	static void remove_parametric_loop_vars(Constraint & c);


	static void rename_source_to_target_index(Constraint& c);
	static void rename_target_to_source_index(Constraint& c);
	static void rename_source_to_fresh_index(Constraint& c);
	static void rename_target_to_fresh_index(Constraint& c);

	static void rename_inst_source_to_inst_target(Constraint& c);

	/*
	 * Renames the index variables in ap to fresh vars and updates the
	 * index variables in the constraint consistently. E.g., if ap is a[i1]
	 * and constraint is j1 = 2, then it returns a[i2] and updates constraint to
	 * j2=2.
	 */
	static AccessPath* refresh_index_vars(AccessPath* ap, Constraint& c);


	static AccessPath* convert_index_vars_to_instantiation_vars(AccessPath* ap);
	static Constraint convert_index_vars_to_instantiation_vars(Constraint c);

	static void eliminate_target_vars(Constraint& c);
	static void eliminate_source_vars(Constraint& c);
	static void eliminate_fresh_vars(Constraint& c);
	static void eliminate_sum_index_vars(Constraint& c);
	static void eliminate_index_vars(Constraint& c,
			set<IndexVariable*> index_vars);

	static Constraint get_source_eq_target_constraint(AccessPath* ap);

	/*
	 * Gives a constraint setting all outer target indices (before a dereference)
	 * to zero.
	 */
	static Constraint get_outer_indices_zero_constraint(AccessPath* ap);




	IndexVariable* get_source_index_var(MemoryLocation* loc);

	IndexVariable* get_target_index_var(MemoryLocation* loc);

	static void rename_index_vars(Constraint& c);

	static void rename_inst_index_vars(Constraint& c);

	/*
	 * Changes all occurences of source indices in this access path
	 * to target indices.
	 */
	static AccessPath* rename_source_to_target(AccessPath* ap);

	static void increment_index(Constraint& c, map<AccessPath*, AccessPath*>&
			base_to_index);

	static AccessPath* set_summary_indices_to_zero(AccessPath* ap);

	virtual ~IndexVarManager();

private:
	static bool is_index_term(Term* t);
	static bool is_counter_term(Term* t, bool parametric);






};

#endif /* INDEXVARMANAGER_H_ */
