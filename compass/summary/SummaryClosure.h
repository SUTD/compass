/*
 * SummaryClosure.h
 *
 *  Created on: Mar 9, 2009
 *      Author: isil
 */

#ifndef SUMMARYCLOSURE_H_
#define SUMMARYCLOSURE_H_
#include <vector>
#include <string>
#include <set>
#include <map>
#include "Constraint.h"
#include "Edge.h"
using namespace std;



class SummaryGraph;
class AccessPath;
class MemoryLocation;
class Variable;
class Edge;
class IterationCounter;

namespace sail{
	class Block;
};

enum gen_kind_t {PARAMETRIC, LAST};


/*
 * Generalization of an access path within a recursive summary unit.
 * The parametric field is parameterized over a variable k which
 * represents any iteration number.
 * The last field refers to iteration N, which is the iteration at which
 * the loop terminates.
 * constraint is the constraint under which this generalization holds (e.g.
 * i may have generalization i+k under flag and i+2k under !flag).
 */
struct generalization {
	AccessPath* base;
	AccessPath* parametric;
	AccessPath* last;
	Constraint constraint;
	// Increment has to be an integer, otherwise we get non-linear things
	long int increment;
	static int counter;


	generalization(AccessPath* rec_base, AccessPath* to_generalize,
			Constraint c,  int cur_counter);
	generalization();
	AccessPath* get_generalization(gen_kind_t kind, int counter_id);

	long int get_increment();
	string to_string();


};

struct generalization_set {
	set<generalization*> generalizations;
	generalization_set();
	~generalization_set();
	bool add_generalization(AccessPath* rec_base, AccessPath*
			to_generalize,
			Constraint c,  int cur_counter);
	void add_generalization(generalization* g);
	string to_string();
	int size();
	void clear();

};


/*
 * SummaryClosure computes the transitive closure of the
 * summary graph for loops and tail-recursive functions.
 * This is an alternative way of doing a fixed-point
 * computation that makes it easier to recover invariants.
 */
class SummaryClosure {
private:
	SummaryGraph& sg;
	vector<string>* visuals;

	/*
	 * The constraint under which the loop/recursion terminates.
	 */
	Constraint termination_cond;


	/*
	 * The set of all locations used in the summary graph.
	 */
	set<MemoryLocation*> locs;
	set<AccessPath*> all_sources;

	/*
	 * A mapping from memory access paths to the set of edges in
	 * which this access path appears in the target.
	 * This is used in the fixpoint computation for graph closure
	 * to re-enqueue the relevant edges.
	 */
	map<AccessPath*, set<Edge*> > target_aps_to_edges;


	/*
	 * A mapping from access paths pairs to their closed value set
	 * (i.e. values they can take in any possible iteration.)
	 */
	map<AccessPath*, generalization_set> generalization_map;
	/*
	 * The key set of the above map.
	 */
	set<AccessPath*> generalized_aps;

	/*
	 * All the iteration counters used in the summary.
	 */
	set<IterationCounter*> counters;


	/*
	 * This is just used to avoid making the constraint
	 * k1= k2 = ... kn multiple times (used for checking
	 * whether source/target can be overwritten.)
	 */
	Constraint counter_ids_eq_constraint;

	/*
	 * Access paths that are not a linear function of the iteration counter
	 * but that are modified in the loop.
	 */
	set<AccessPath*> non_generalizable_aps;

	/*
	 * Edges that need to be updated.
	 * For instance, if i is incremented by, then summary graph
	 * contains edge from i to *(i+1) but this edge
	 * will need to be replaced with i->*(i+k) etc.
	 */
	map<Edge*, set<generalization*> > edges_to_update;

	/*
	 * Counter for variables used for preprocessing pointer
	 * arithmetic
	 */
	int pointer_index_counter;

	/*
	 * Fake l variables introduced by the analysis to
	 * treat pointer arithmetic as index-based access.
	 */
	set<AccessPath*> pointer_indices;

	/*
	 * If a is an array that has pointer arithmetic applied to it
	 * in the loop, this map will map a to the fake l variable
	 * that expresses pointer arithmetic as index-based access.
	 */
	map<AccessPath*, AccessPath*> base_to_index;

	/*
	 * i-> i+l
	 */
	map<Term*, Term*> pointer_arithmetic_subs;


public:
	SummaryClosure(SummaryGraph& sg, vector<string>* visuals = NULL);
	~SummaryClosure();

	static void get_iteration_counters(Constraint c,
			set<IterationCounter*>& counters);
	static void get_termination_vars(Constraint c,
			set<IterationCounter*>& counters);

private:
	/*
	 * We classify scalars modified by the loop into 3 categories:
	 * 1) Scalars that are linear functions of the iteration counter;
	 * these can be generalized meaningfully as i -> i+c*N +c'
	 * 2) Scalars that are non-linear or unknown function of the iteration
	 * counter, these lead to imprecise generalizations of the form
	 * i->imprecise(N) or i->imprecise(k)
	 * 3) Scalars written to in the loop, but are not even recursively defined;
	 * these are not in the generalization_map.
	 *
	 * Scalars that belong to (2) and (3) are also added to
	 * non_generalizable_aps so that they can be treated as non-deterministic
	 * environment choices when deciding whether two constraints are disjoint.
	 */
	void compute_generalizations();

	/*
	 * Prepares graph for closure by determining the incoming edges of
	 * each target, i.e., initializes target_aps_to_edges.
	 */
	void prepare_graph();

	/*
	 * Eliminates access paths in the non_generalizable_aps set
	 * from the constraints.
	 */
	void eliminate_non_linear_aps();

	/*
	 * Where possible, determines the number of times the loop
	 * executes.
	 */
	void compute_termination_constraint();

	/*
	 * Computes the last iteration condition for an exit point out of the loop.
	 */
	Constraint get_last_iteration_constraint(Constraint break_cond,
			sail::Block* exit_pred_block);

	/*
	 * Replaces values used in the constraint with their
	 * generalizations.
	 */
	void generalize_constraints();

	/*
	 * Recomputes the termination (resp. last iteration) condition of the
	 * loop using the appropriate N's for each block.
	 */
	Constraint compute_generalized_termination_cond();



	/*
	 * Returns the generalization of a constraint
	 * c is the constraint to be generalized, and parametric indicates whether
	 * k or N should be used in the generalization, and counter_id
	 * indicates which counter id should be used in the generalization.
	 */
	Constraint generalize_constraint(Constraint c, gen_kind_t parametric,
			int counter_id);
	Constraint generalize_ap_in_constraint(Constraint c, AccessPath* old_ap,
			gen_kind_t kind, int counter_id);

	bool target_overwritten_in_prev_it(AccessPath* target, Constraint target_c,
			Constraint overwrite_c);

	/*
	 * Can the target of this edge be overwritten in a previous iteration?
	 * We only need to check this for self recursive edges because
	 * for other edges, closing the graph makes sure sufficient conditions
	 * are weakened appropriately.
	 */
	bool target_overwritten_in_prev_it(Edge* e);
	bool source_overwritten_in_future_it(Edge* e);

	Constraint increment_iteration_counters(Constraint c);
	Constraint decrement_iteration_counters(Constraint c);

	Constraint increment_iteration_counters(Constraint c,
			AccessPath* inc_amount);

	/*
	 * Eliminates the iteration counters used in the constraints
	 * where appropriate.
	 */
	void eliminate_iteration_counters();

	Constraint get_counters_eq_constraint();

	/*
	 * Checks whether the ap is in terms of the iteration counter,
	 * e.g., i+k is parametric.
	 */
	bool is_parametric_ap(AccessPath* ap);





	/*
	 * Closing the summary graph G means the following:
	 * If there is an edge from X to *Y in G as
	 * well as an edge from Y to *Z, then add the edge
	 * X to *Z. The NC for the edge constraint is the
	 * conjunction of the necessary condition for the individual
	 * edge constraints, but the sufficient condition is false.
	 */
	void close_graph();

	void enqueue_dependencies(AccessPath* source_ap, AccessPath* new_target,
			set<Edge*,CompareTimestamp>& edges, Edge* new_edge);


	/*
	 * Populates locs, i.e., the set of locations
	 * used in the summary.
	 */
	void collect_summary_locs();




	/*
	 * Makes pointer arithmetic explicit by introducing integer variables
	 * so that we can treat pointer arithmetic and explicit index based
	 * side effects more uniformly. For instance, if there is a side
	 * effect a++, this is turned into a points to (*a)[i] under the constraint
	 * i=l_a and l_a points to *(l_a + 1).
	 */
	void preprocess_pointer_arithmetic();
	void preprocess_pointer_arithmetic_in_constraints();
	void preprocess_pointer_arithmetic(Constraint & c);
	/*
	 * Removes the fake l_i variables introduces during preprocessing.
	 */
	void postprocess_pointer_arithmetic();

	/*
	 * Given an access path such as *(a+b) where a's value set is <c/flag,
	 * d/!flag> and where b's value set is <e/flag2, f/!flag2> gives back
	 * the set {*(c+e)/flag&flag2, *(c+f)/flag&!flag2, *(d+e)/!flag&flag2,
	 * *(d+f)/ !flag&!flag2} and so on.
	 *
	 * If the return value is false, it means that there may be infinitely many
	 * possible targets (and cannot be generalized), and the target should
	 * become imprecise.
	 */
	bool get_closed_targets(AccessPath* ap, Constraint c,
			set<pair<AccessPath*, Constraint> >& closed_targets);
	bool get_closed_targets_rec(AccessPath* ap, Constraint c,
			set<pair<AccessPath*, Constraint> >& closed_targets);

	/*
	 * Get the iteration counter for this edge.
	 * INVALID_COUNTER if it does not have consistent
	 * block ids.
	 */
	int get_iteration_counter(Edge* e);

	/*
	 * Generalizes the graph by updating the targets of
	 * summary edges with their correct generalizations.
	 */
	void generalize_targets();

	/*
	 * Is ap1 contained in ap2?
	 */
	bool occurs_check(AccessPath* ap1, AccessPath* ap2);

	/*
	 * If we were able to generalize an access path like "i",
	 * we need to update any other access paths involving i, such
	 * as i+1 etc.
	 */
	void update_generalization_dependencies();

	void cross_product_generalizations(
			map<AccessPath*, generalization_set>& map_set,
			set<map<AccessPath*, generalization*> > & set_map);

	void cross_product_generalizations_rec(
			map<AccessPath*, generalization_set>& map_set,
			set<map<AccessPath*, generalization*> > & set_map,
			map<AccessPath*, generalization*>& cur_map);

	/*
	 * Determines if the given ap contains any N.
	 */
	bool contains_termination_var(AccessPath* ap);


	void generalize_error_traces();

	inline Variable* get_pointer_index();


	/*
	 * Is the source of this edge guaranteed to represent
	 * a single concrete location?
	 */
	bool has_concrete_source(Edge* e);

	/*
	 * Yields the constraint for the edge that gave rise to this generalization.
	 * For example, if one of the generalizations for i is i+N, then this
	 * function looks up the constraint on the edge i->*(i+1).
	 */
	Constraint get_original_constraint(generalization* g);

	/*
	 * Collects the set of counter ids used in the summary.
	 */
	void collect_used_counters();

	/*
	 * Minimizes the number of N's used in the summary.
	 */
	void minimize_iteration_counters();

	/*
	 * Uniquifies error traces by disregarding the block in which
	 * they were generated.
	 */
	void uniquify_error_traces();



	/* ---------------------------
	 * Debugging functions
	 * ---------------------------
	 */
	void print_target_ap_to_edges();
	void print_non_generalizable_aps();
	void print_base_to_index();
	void print_linear_scalars();
	void print_used_counters();


};

#endif /* SUMMARYCLOSURE_H_ */
