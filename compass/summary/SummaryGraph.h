/*
 * SummaryGraph.h
 *
 *  Created on: Oct 18, 2008
 *      Author: tdillig
 */

#ifndef SUMMARYGRAPH_H_
#define SUMMARYGRAPH_H_

#include <set>
#include <map>
#include <vector>
#include <queue>

#include "Constraint.h"
#include "Edge.h"
#include "AccessPath.h"
#include "call_id.h"


//#define ENABLE_USER_INTERACTION false


enum summary_type {
	SUM_LOOP,
	SUM_FUNCTION
};

/*
 * A constraint is represented as a pair of access paths in the summary
 * to keep serialization uniform.
 */
typedef pair<AccessPath*, AccessPath*> sum_c_type;

class MemoryGraph;
class MemoryLocation;
class ErrorTrace;
class IterationCounter;

class Callgraph;
namespace sail{
class SummaryUnit;
}
using namespace std;

namespace sail{
	class SuperBlock;
	class Function;
};


class SummaryGraph {
	friend class SummaryClosure;
	friend class LoopAbstractor;
	friend class Instantiator;
	friend class boost::serialization::access;
	friend class InvariantChecker;
	friend class LoopInvariantChecker;
private:


	/*
	 * The set of edges representing side effects
	 */
	set<Edge*, CompareTimestamp> edges;


	/*
	 * The set of atomic access paths that need to be instantiated to
	 * instantiate this summary.
	 */
	set<AccessPath*> aps_to_instantiate;

	/*
	 * A mapping from access paths to their update conditions.
	 * Note that the disjunction of the edge constraints may not
	 * precisely characterize the constraint under which the source will be
	 * updated. This happens when there are choice variables because
	 * sufficient conditions do not distribute over or's.
	 */
	map<AccessPath*, Constraint> update_conditions;


	call_id sum_call_id;



	string sum_short_id;
	string sum_id;

	bool is_loop;


	/*
	 * Is this the summary of an init function?
	 */
	bool is_init_fn;


	/*
	 * Set of error traces that need to be instantiated.
	 */
	set<ErrorTrace*> error_traces;

	/*
	 * What kind of summary unit is this SummaryGraph associated with?
	 * (loop, function etc.)
	 */
	summary_type sum_t;



	/*
	 * The condition under which this SummaryUnit will reach its
	 * exit block. This should be used to modify the statement
	 * guard when the summary is applied.
	 */
	Constraint return_cond;

	Constraint loop_continuation_cond;

	/*
	 * In case of loops and tail-recursive functions, we may pick up constraints
	 * on the N representing the number of times the loop executes. Since N
	 * is existentially quantified, it cannot be negated and therefore has to
	 * be treated specially as assumed background knowledge by the Constraint.
	 */
	set<Constraint> loop_termination_constraints;

	// --------------------------------------------------
	/*
	 * Member variables used only during constructing the summary graph
	 */

	/*
	 * These two are NULL after loading.
	 */
	sail::SummaryUnit* su;
	MemoryGraph* mg;


	/*
	 * The set of locations directly reachable in the calling context.
	 */
	set<MemoryLocation*> entry_points;

	/*
	 * Set of objects transitively reachable from an interface object
	 */
	set<MemoryLocation*> reachable_objects;

	/*
	 * The set of access paths that have incoming edges to them.
	 * This is used for figuring out what should not be eliminated from
	 * constraints.
	 */
	set<AccessPath*> reachable_aps;

	/*
	 * Defining class if this summary graph is associated with
	 * a member function, NULL otherwise.
	 */
	il::record_type* defining_class;

/*
 * The following variables are used only for the purposes of
 * diagnosing error reports without having to re-analyze functions
 */
#ifdef ENABLE_USER_INTERACTION
	map<Edge*, Constraint> original_edge_c;
	map<AccessPath*, Constraint> original_update_conditions;
	map<Constraint, Constraint> uniqueness_to_sum_c;

#endif /*ENABLE_USER_INTERACTION */



private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & edges;
		ar & aps_to_instantiate;
		ar & sum_short_id;
		ar & sum_id;
		ar & is_loop;
		ar & is_init_fn;
		ar & error_traces;
		ar & sum_t;
		ar & return_cond;
		ar & loop_continuation_cond;
		ar & loop_termination_constraints;
		ar & sum_call_id;
		ar & update_conditions;
		ar & defining_class;
#ifdef ENABLE_USER_INTERACTION
		ar & original_edge_c;
		ar & original_update_conditions;
		ar & uniqueness_to_sum_c;
#endif /*ENABLE_USER_INTERACTION */

	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & edges;
		set<AccessPath*> aps_to_inst;
		ar & aps_to_inst;
		set<AccessPath*>::iterator it = aps_to_inst.begin();
		for(; it!= aps_to_inst.end(); it++)
		{
			AccessPath* cur = *it;
			cur = AccessPath::to_ap(Term::get_term_nodelete(cur->to_term()));
			aps_to_instantiate.insert(cur);

		}
		ar & sum_short_id;
		ar & sum_id;
		ar & is_loop;
		ar & is_init_fn;
		ar & error_traces;
		ar & sum_t;
		ar & return_cond;
		ar & loop_continuation_cond;
		ar & loop_termination_constraints;
		ar & sum_call_id;

		map<AccessPath*, Constraint> _update_conditions;
		ar & _update_conditions;
		map<AccessPath*, Constraint>::iterator it2 = _update_conditions.begin();
		for(; it2 != _update_conditions.end(); it2++)
		{
			AccessPath* cur = it2->first;
			cur = AccessPath::to_ap(Term::get_term_nodelete(cur->to_term()));
			update_conditions[cur] = it2->second;
		}


		ar & defining_class;
		il::type::register_loaded_typeref((il::type**)&defining_class);

#ifdef ENABLE_USER_INTERACTION
		ar & original_edge_c;

		map<AccessPath*, Constraint> _original_update_conditions;
		ar & _original_update_conditions;
		map<AccessPath*, Constraint>::iterator it3 =
				_original_update_conditions.begin();
		for(; it3 != _original_update_conditions.end(); it3++)
		{
			AccessPath* cur = it3->first;
			cur = AccessPath::to_ap(Term::get_term_nodelete(cur->to_term()));
			original_update_conditions[cur] = it3->second;
		}

		ar & uniqueness_to_sum_c;
#endif /*ENABLE_USER_INTERACTION */
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	SummaryGraph()
	{
		mg = NULL;
		su = NULL;
	};



public:

	SummaryGraph(MemoryGraph* mg, Constraint return_cond,
			sail::Function* f);


	SummaryGraph(MemoryGraph* mg, Constraint return_cond,
			Constraint continuation_cond,
			sail::SuperBlock* su);

	const set<ErrorTrace*> & get_error_traces() const;

	/*
	 * Adds an edge between the given memory locations
	 */
	Edge* add_edge(MemoryLocation* source_loc, int source_offset,
			MemoryLocation* target_loc, int target_offset,
			Constraint c, int timestamp =-1);

	void clear_edges(MemoryLocation* source_loc, int source_offset);

	/*
	 * Yields the constraint on the edge between source and target, false if
	 * there is no edge.
	 */
	Constraint get_constraint(AccessPath* source, AccessPath* target);

	/*
	 * Gives the constraint under which the summary updates the source location.
	 */
	Constraint get_update_condition(AccessPath* source);

	/*
	 * Gives the set of successor and predecessor edges of this location,
	 * but disregards edges that are not part of the summary, such
	 * as default edges.
	 */
	void get_successors(MemoryLocation* loc, int offset, set<Edge*>& succs);
	void get_predecessors(MemoryLocation* loc, int offset, set<Edge*>& preds);

	void remove_edge(Edge* e);

	string get_sum_id();
	string get_sum_short_id();

	call_id get_call_id();

	/**
	 * For mmethod summaries, the defining class of the method.
	 */
	il::record_type* get_defining_class();


	string to_dotty();

	/*
	 * Only shows edges whose source access path
	 * contains the string "filter" and anything reachable from them.
	 */
	string to_dotty(const string& filter);

	/*
	 * Collects all the access paths in a constraint that need to be instantiated
	 * to allow translating this constraint
	 */
	static void collect_access_paths(Constraint c, set<AccessPath*>& aps);

	/*
	 * Collects the set of access paths used in a term. Note that not
	 * all terms have to correspond to access paths; for example
	 * arithmetic terms may have been introduced from
	 * quantifier elimination.
	 */
	static void collect_access_paths(Term* t, set<AccessPath*>& aps);


	bool is_function_summary();
	bool is_loop_summary();
	bool is_init_fn_summary();

	Constraint get_loop_termination_constraints();
	
	Constraint get_return_cond();
	const set<Constraint> & get_loop_counter_relations();

	Identifier  get_identifier();

	/*
	 * Only used for error diagnosis
	 */
	Constraint get_original_edge_c(Edge* e);
	Constraint get_original_update_c(AccessPath* ap);
	Constraint get_constraint_for_disjointness_leaf(Constraint c);
	map<Constraint, Constraint> & get_disjointness_map();


private:

	/*
	 * Collects the set of atomic access paths that need to be instantiated
	 * in the calling context and replaces index variables
	 * with summary index variables to avoid naming collisions.
	 */
	void collect_access_paths_to_instantiate();




	/*
	 * Replaces disjointness variables in constraints with what they stand for.
	 */
	void replace_disjointness_constraints();

	void get_value_set(AccessPath* ap,
			set<pair<AccessPath*, Constraint> >& values, Constraint c);



	void collect_loop_termination_constraints();


	void eliminate_unobservables();
	Constraint eliminate_unobservables(Constraint c, bool eliminate_counters,
			bool keep_reachable_unobservables);




	/*
	 * Determines the part of the points to graph reachable from
	 * callers.rges any errors that are provably safe under the
	 * loop entry condition and reports errors that are guaranteed to
	 * happen under the loop entry condition.
	 */
	void construct_reachable_graph();
	void construct_reachable_graph(MemoryLocation* loc);



	/*
	 * Remove all reachable memory location from the memory graph and
	 * claim their ownership.
	 */
	void update_memory_graph();

	void compute_update_conditions();



	string info_to_string();




	Edge* get_existing_edge(MemoryLocation* source_loc, int source_offset,
			MemoryLocation* target_loc, int target_offset);

	void eliminate_unused_iteration_counters();

	/*
	 * If error traces contain N's, we want to explicitly add constraints
	 * we know about N.
	 */
	Constraint include_counter_dependecies(Constraint c, set<Constraint>&
			counter_info);

	/*
	 * We refine edge constraints to a location L (not originally accessible in
	 * the calling context) by assuming the disjunction of
	 * the incoming edge constraints to L. This is useful if an unobservable
	 * variable appears in the incoming edge constraints to L and the elimination
	 * causes this edge constraint to become imprecise. This transformation
	 * is correct because future referents to L in the calling context must
	 * access it through one of the referents to L in the summary.
	 */
	void refine_edge_constraints();
	void get_postordering(MemoryLocation* loc,
			vector<MemoryLocation*> & order,
			set<MemoryLocation*>& processed);

	void print_update_conditions();

	void check_leaks();

	void collect_aps_in_error_traces();

	void postprocess_imprecise_values();
	void postprocess_bracketing_constraints();
	void get_summary_entry_points(set<pair<MemoryLocation*, int> >& entry_points);




};

#endif /* SUMMARYGRAPH_H_ */
