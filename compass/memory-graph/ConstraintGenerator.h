/*
 * ConstraintGenerator.h
 *
 *  Created on: Oct 16, 2008
 *      Author: tdillig
 */

#ifndef CONSTRAINTGENERATOR_H_
#define CONSTRAINTGENERATOR_H_

class MemoryGraph;
class Constraint;
class AccessPath;
class FunctionValue;
#include "il/binop_expression.h"

namespace sail {
class Symbol;
}



class ConstraintGenerator {
private:
	MemoryGraph & mg;
public:
	ConstraintGenerator(MemoryGraph & mg);
	virtual ~ConstraintGenerator();


	Constraint get_neqz_constraint_from_value_set(sail::Symbol* s);

	static Constraint get_eqz_constraint(AccessPath* ap);
	static Constraint get_neqz_constraint(AccessPath* ap);

	/*
	 * ap > 0
	 */
	static Constraint get_gtz_constraint(AccessPath* ap);

	/*
	 * ap >= 0
	 */
	static Constraint get_geqz_constraint(AccessPath* ap);

	/*
	 * ap < 0
	 */
	static Constraint get_ltz_constraint(AccessPath* ap);

	/*
	 * ap < =0
	 */
	static Constraint get_leqz_constraint(AccessPath* ap);

	/*
	 * ap1= ap2
	 */
	static Constraint get_eq_constraint(AccessPath* ap1, AccessPath* ap2);

	/*
	 * ap1!= ap2
	 */
	static Constraint get_neq_constraint(AccessPath* ap1, AccessPath* ap2);

	/*
	 * ap1 <= ap2
	 */
	static Constraint get_geq_constraint(AccessPath* ap1, AccessPath* ap2);

	/*
	 * ap1 <= ap2
	 */
	static Constraint get_leq_constraint(AccessPath* ap1, AccessPath* ap2);

	/*
	 * ap1 < ap2
	 */
	static Constraint get_lt_constraint(AccessPath* ap1, AccessPath* ap2);

	/*
	 * ap1 > ap2
	 */
	static Constraint get_gt_constraint(AccessPath* ap1, AccessPath* ap2);

	static Constraint get_mod_constraint(AccessPath* ap1, long int c);

	/*
	 * ap1 binop ap2
	 */
	static Constraint get_constraint_from_binop(AccessPath* ap1,
			AccessPath* ap2, il::binop_type binop);

	/*
	 * subtypeof(ap1, ap2), i.e. ap1 is subtype of ap2
	 */
	static Constraint get_subtype_of_constraint(AccessPath* ap1,AccessPath* ap2);



	/*
	 * sum = ap1 + ap2
	 */
	static Constraint get_sum_constraint(AccessPath* sum, AccessPath* ap1,
			AccessPath* ap2);

	/*
	 * If vs = {<ap1, c1>, ... <ak, ck>}, this function generates the
	 * constraint (v=ap1 & c1) | .... | (v=apk & ck)
	 */
	static Constraint get_eq_to_value_set_constraint(AccessPath* v,
			const set<pair<AccessPath*, Constraint> >& vs,
			bool eliminate_counters);

	/*
	 * Eliminates the existentially quantified access path ap from c.
	 */
	static void eliminate_evar(Constraint& c, AccessPath* ap);
	static void eliminate_uvar(Constraint& c, AccessPath* ap);
	static void eliminate_free_var(Constraint& c, AccessPath* ap);
	static void eliminate_evars(Constraint& c, set<AccessPath*>& aps);
	static void eliminate_free_vars(Constraint& c, set<AccessPath*>& aps);

	static void eliminate_unobservables(Constraint & c, bool is_loop,
			set<AccessPath*>& excluded_aps, bool eliminate_counters);

	/*
	 * The InvariantEnforcer may introduce artifical j =t(i) functions
	 * to connect the background constraint to the actual constraint.
	 * Once we propagate background constraints, these placeholder
	 * target functions/variables should be eliminated.
	 */
	static void eliminate_background_target_variables(Constraint& c);
	static void eliminate_disjointness_terms(Constraint& c);

	bool contains_access_path(Constraint c, AccessPath* ap);

	bool contains_access_path(Constraint c, set<AccessPath*>& ap);


	static bool disjoint(set<Constraint>& constraints);

	static void get_nested_aps(Constraint c, set<AccessPath*> & aps);





	/*
	 * Yields the constraint under which the given binop is true and false
	 * respectively.
	 */
	pair<Constraint, Constraint> get_constraint_from_pred_binop(AccessPath* _ap1,
			Constraint ap1_c, AccessPath* _ap2, Constraint ap2_c,
			il::binop_type bt);

	static void get_nested_memory_aps(Constraint c, set<AccessPath*>& aps);




private:

	inline Constraint make_constraint_from_binop(AccessPath* ap1,
			AccessPath* ap2, il::binop_type bt, Constraint& to_and);


};

#endif /* CONSTRAINTGENERATOR_H_ */
