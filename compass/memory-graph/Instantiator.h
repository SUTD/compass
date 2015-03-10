/*
 * Instantiator.h
 *
 *  Created on: Oct 25, 2008
 *      Author: isil
 */

#ifndef INSTANTIATOR_H_
#define INSTANTIATOR_H_

#include <set>
#include <vector>
#include <map>
using namespace std;

#include "ArithmeticValue.h"
#include "Constraint.h"
#include "MemoryLocation.h"
#include "ErrorTrace.h"

class MemoryGraph;
class SummaryGraph;
class ArrayRef;
class FunctionValue;
class AccessPath;
class FieldSelection;
class Edge;
class Alloc;
class AbstractDataStructure;

namespace sail
{
	class Variable;
	class Symbol;
	class SummaryUnit;

}



// -----------------------------------------------

class Instantiator {
	friend class InvariantEnforcer;
private:
	MemoryGraph* mg;
	SummaryGraph* sg;


	/*
	 * A mapping from access paths to be instantiated to a temporary variable
	 * associated with each access path. The instantiation of the access path
	 * is the value set of this variable.
	 */
	map<AccessPath*, Variable*> temporaries;

	/*
	 * The instantiation of each access path used in the summary.
	 */
	map<AccessPath*, set< pair<AccessPath*, Constraint> > > instantiations;


	/*
	 * The condition under which the processed summary unit
	 * returns (instantiated)
	 */
	Constraint inst_return_cond;

	vector<sail::Symbol*>* args;
	sail::Variable* retvar;
	Constraint stmt_guard;


	bool is_loop;


	Constraint inst_guard;

	/*
	 * The set of all temporary variables introduced during instantiation.
	 */
	set<Variable*> all_temps;

	/*
	 * All locations in the memory graph that were updated as
	 * a result of the function call. This is used for
	 * enforcing existence and uniqueness.
	 */
	set<AccessPath*> inst_sources;

	bool report_leaked_stack_vars;

	Constraint loop_termc_inst;

	/*
	 * We can't directly use instantiate_constraint(e->get_constraint())
	 * because if constraints are imprecise, we might have needed to
	 * enforce and uniqueness.
	 */
	map<Edge*, Constraint> edge_to_inst_constraints;
	map<AccessPath*, Constraint> inst_update_c;



	map<AccessPath*, AccessPath*> unmodeled_inst_map;





public:

	/*
	 * Constructor for instantiating function summaries
	 */
	Instantiator(MemoryGraph* mg, SummaryGraph* sg, Constraint& stmt_guard,
			vector<sail::Symbol*>* args, sail::Variable* retvar);

	/*
	 * Constructor for instantiating loop summaries
	 */
	Instantiator(MemoryGraph* mg, SummaryGraph* sg, Constraint& stmt_guard);

	Constraint get_instantiation(Constraint);
	void get_instantiation(AccessPath* sum_ap,
			set<pair<AccessPath*, Constraint> >& instantiation,
			bool get_value = true);

	int get_counter_id();
	callstack_entry get_calling_context();
	// Only instantiations in a function (not a loop) introduce a
	// new calling context.
	bool is_new_calling_context();
	Constraint get_stmt_guard();
	~Instantiator();

	static string value_set_to_string(set<pair<AccessPath*, Constraint> >& vs);
	MemoryGraph* get_memory_graph();
	sail::SummaryUnit* get_current_unit();
private:
	void apply_summary();
	Constraint simplify_pos(Constraint c);
	void instantiate_access_paths();
	AccessPath* instantiate_ap(AccessPath* ap);
	Variable* instantiate_ap_internal(AccessPath* ap, Constraint inst_c);
	AccessPath* instantiate_index(AccessPath* index);
	Variable* instantiate_arithmetic_value(ArithmeticValue* av);
	Variable* instantiate_function_value(FunctionValue* av);
	Constraint instantiate_constraint(Constraint c, bool eliminate_counters = false);

	Constraint instantiate_constraint_helper(Constraint c, bool eliminate_counter);
	/*
	 * Can the instantiation of this access path be reduced
	 * to processing a single instruction by calling
	 * memory graph primitives?
	 */
	bool is_directly_instantiable(AccessPath* ap);
	bool is_directly_instantiable(AccessPath* ap, bool ok);

	/*
	 * Given an access path that is not directly instantiable,
	 * identifies a smaller subpart that we can recursively
	 * instantiate.
	 */
	AccessPath* get_next_unit(AccessPath*& ap);

	/*
	 * Given a "directly instantiable ap" as defined above,
	 * instantiate this access path by calling a memory
	 * graph primitive.
	 */
	Variable* instantiate_unit_ap(AccessPath* ap, Constraint inst_c);
	Variable* instantiate_unit_pointer_array_read(ArrayRef* ar,
			int offset, const string& field_name, il::type* result_type,
			Constraint inst_c);
	Variable* instantiate_unit_pointer_arithmetic(ArithmeticValue* av,
			il::type* result_type, Constraint inst_c);
	Variable* instantiate_structured_pointer_arithmetic(ArithmeticValue* av,
			il::type* result_type, Constraint inst_c);
	Variable* instantiate_unstructured_pointer_arithmetic(ArithmeticValue* av,
			il::type* result_type, Constraint inst_c);
	Variable* instantiate_unit_static_array_read(ArrayRef* ar,
			int offset, const string& field_name, il::type* result_type,
			Constraint inst_c);
	Variable* instantiate_unit_adt_read(AbstractDataStructure* abs, int offset,
			const string& field_name, il::type* result_type, Constraint inst_c);


	void add_instantiation(AccessPath* sum_ap, AccessPath* inst_ap);
	void add_instantiation(AccessPath* sum_ap, Variable* inst_temp);

	AccessPath* get_base_instantiation(AccessPath* v, Constraint & c);


	AccessPath* get_unique_ap(AccessPath* a, Constraint & c);

	void remove_temps();

	/*
	 * Instantiates the trigger conditions for errors and reports any
	 * errors if (i) they are implied by the statement guard or (ii) if
	 * the current function has no callers. Otherwise, if they cannot be
	 * discharged propagates error traces up the call chain.
	 */
	void instantiate_error_traces();

	/*
	 * After applying the summary, the edge constraints may no longer
	 * encode that every source must point to exactly one target due
	 * to imprecision. This function adds additional constraints to
	 * summary edges to maintain the invariant that the necessary
	 * conditions for edge constraints are mutually exclusive and that
	 * sufficient conditions sum up to true.
	 */
	void enforce_existence_and_uniqueness();

	void instantiate_loop_termination_constraints();

	void get_aps_inside_function(FunctionValue* fv, set<AccessPath*>&
		aps);

	/*
	 * We assume argument locations are distinct from fresh allocations
	 * and addresses of other locations. DUe to entry aliasing, some constraints
	 * in the summary may be of the form arg1 != arg2, which may
	 * for example instantiate to alloc != arg1. We explicitly simplify
	 * instantiated constraints with respect to these
	 * non-aliasing assumptions to rule out spuriously satisfiable
	 * constraints.
	 *
	 */
	Constraint enforce_aliasing_assumptions(Constraint inst_c,
			set<AccessPath*>& instantiations);

	/*
	 * Helper for enforce_aliasing_assumptions.
	 * This function generates a constraint stipulating that each of the
	 * terms in t1 is distinct from each of the terms in t2.
	 */
	Constraint get_disequality_constraints(set<Term*>& t1, set<Term*>& t2);



};

#endif /* INSTANTIATOR_H_ */
