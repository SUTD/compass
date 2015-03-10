/*
 * MemoryGraph.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef MEMORYGRAPH_H_
#define MEMORYGRAPH_H_

#include "Constraint.h"
#include <set>
using namespace std;

#include "ConstraintGenerator.h"
#include "DisjointVarManager.h"
#include "AssertionTrace.h"
#include "InvariantEnforcer.h"
#include "Sentinels.h"
#include "UnmodeledValue.h"
#include "InvariantChecker.h"
#include "EntryAliasManager.h"


class MemoryLocation;
class Edge;
class AccessPath;
class Variable;
class SummaryGraph;
class ArrayRef;
class IterationCounter;
class AbstractDataStructure;


namespace sail{
class Variable;
class Symbol;
class FunctionCall;
};
#include "Error.h"
#include "IndexVarManager.h"


namespace il{
	class record_info;
	class type;
}

namespace sail {
	class SummaryUnit;
}

#include "il/binop_expression.h"

class MemoryGraphWidget;
class MemoryAnalysis;



class MemoryGraph {
	friend class ConstraintGenerator;
	friend class SummaryGraph;
	friend class Instantiator;
	friend class SummaryClosure;
	friend class MemoryAnalysis;
	friend class DisjointVarManager;
	friend class InvariantChecker;
	friend class EntryAliasManager;
	friend class LoopInvariantChecker;
private:

	/*
	 * Mapping from access paths to the memory locations associated with them.
	 */
	map<AccessPath*, MemoryLocation*> ap_to_locs;


	/*
	 * For visualization purposes, we maintain a set of memory locations
	 * that serve as the roots of the memory graphs.
	 * This set does not include sail temporaries that have been consumed. i.e.,
	 * no longer serve a useful purpose.
	 */
	set<MemoryLocation*> root_locs;




	IndexVarManager ivm;

	ConstraintGenerator cg;

	InvariantEnforcer ie;

	/*
	 * Counter for string constants so we can give them a fresh name.
	 */
	int str_const_counter;

	/*
	 * Current line number in the file.
	 */
	int line;

	/*
	 * Number of current instruction
	 */
	int instruction_number;

	/*
	 * Current source file
	 */
	string file;

	/*
	 * Current summary unit
	 */
	sail::SummaryUnit* su;

	/*
	 * Identifies whether this function is recursive. If it is, fresh memory
	 * allocations also need to be qualified with an index variable.
	 */
	bool recursive;

	/*
	 * The errors found while generating the memory graph, e.g.
	 * illegal casts etc.
	 */
	vector<Error*> & errors;

	/*
	 * Potential errors that need to be propagated up the call chain.
	 */
	set<ErrorTrace*> error_traces;


	/*
	 * Report all errors, including ones not guaranteed to fail.
	 * This is set to true if the currently analyzed function has
	 * no callers.
	 */
	bool report_all_errors;


	/*
	 * Only used to delete all edges at the end.
	 */
	set<Edge*> edges;


	/*
	 * This is a counter used to number the edges; necessary for
	 * applying fnction summaries in the correct order. E.g.
	 * if a and b are entry aliases and foo has 2 statements
	 * *a = 2
	 * *b = 3
	 * we need to instantiate the edge *a->*2 before we instantiate the
	 * edge *b->*3.
	 */
	int edge_counter;

	string fn_id;

	DisjointVarManager dvm;

	set<Constraint> loop_termination_constraints;


	/*
	 * A mapping from access paths to the constant increment amount on
	 * each recursive iteration.
	 */
	map<AccessPath*, set<pair<int, Constraint> > > recurrence_map;



	MemoryAnalysis & ma;

	/*
	 * Sentinel Manager to track sentinels associated with memory locations.
	 */
	Sentinels sentinels;


	EntryAliasManager ea;

	bool is_top_level_function;

	/*
	 * For error diagnosis only
	 */
	map<Constraint, Constraint> uniqueness_to_sum_c;

	/*
	 * Values observed in at least one concrete run
	 */
	Constraint observed_c;



public:

	void clear();
	void set_recursive();
	void set_fn_id(string fn_id);

	MemoryGraph(vector<Error*>& errors, bool recursive, MemoryAnalysis& ma,
			bool is_top_level_function);

	/*
	 * Sets the current line number.
	 */
	void set_line(int line);

	int get_line();

	/*
	 * Sets the number of the current instruction.
	 */
	void set_instruction_number(int n);

	/*
	 * Sets the current file.
	 */
	void set_file(const string & file);

	string get_file();

	/*
	 * Sets the currently analyzed summary unit.
	 */
	void set_summary_unit(sail::SummaryUnit* su);


	/*
	 * Sets whether errors should be immediately reported or
	 * whether they should be propagated up to callers.
	 */
	void set_error_report_status(bool rae);

	/*
	 * Gives the guarded value set for symbol s.
	 */
	void get_value_set(sail::Symbol* s,
				set<pair<AccessPath*, Constraint> >& values);

	/*
	 * Gives the guarded value set for access path ap. The result of
	 * get_value_set is *always* in terms of
     * source variables, both for the constraint as well as the access paths.
	 */
	void get_value_set(AccessPath* ap,
			set<pair<AccessPath*, Constraint> >& values);

	/*
	 * Are the possible values of this symbol/access path mutually exclusive?
	 */
	bool has_precise_value_set(sail::Symbol* s);
	bool has_precise_value_set(AccessPath* ap);


	AccessPath* get_access_path_from_symbol(sail::Symbol* s);

	ConstraintGenerator& get_cg();

	void add_error_trace(ErrorTrace* et);




public:
	/*
	 * Returns the condition under which s evaluates to a non-zero
	 * value.
	 */
	Constraint get_neqz_constraint(sail::Symbol* s);


	/*
	 * a=b
	 */
	void process_assignment(sail::Variable* lhs, sail::Symbol* rhs,
			Constraint c);
	void process_assignment(Variable* lhs, AccessPath* rhs,
			Constraint c);

	/*
	 * a=b.f
	 */
	void process_assignment(sail::Variable* lhs,  sail::Variable* rhs,
			int rhs_offset, Constraint c);
	void process_assignment(Variable* lhs,  AccessPath* rhs,
			int rhs_offset, Constraint c, string pp_expression="");

	/*
	 * a.f=b
	 */
	void process_assignment(sail::Symbol* lhs, int lhs_offset,
			 sail::Symbol* rhs, Constraint c);

	/*
	 * *(a+offset) = b
	 */
	void process_store(sail::Variable* lhs, int lhs_offset,
			sail::Symbol* rhs, Constraint c);

	/*
	 * a = *(b + offset)
	 */
	void process_load(sail::Variable* lhs, sail::Symbol* rhs, int rhs_offset,
			Constraint c);
	bool process_load(AccessPath* lhs, AccessPath* rhs, int rhs_offset, Constraint c,
			string pp_exp= "");

	/*
	 * a = &b
	 */
	void process_address(sail::Variable* lhs, sail::Symbol* rhs, Constraint c);
	void process_address(Variable* lhs, AccessPath* rhs, Constraint c);

	/*
	 * a = (T) b;
	 */
	void process_cast(sail::Variable* lhs, sail::Symbol* rhs, Constraint c);
	void process_cast(Variable* lhs, AccessPath* rhs, Constraint c);

	/*
	 * v = a[i].f;
	 */
	void process_array_ref_read(sail::Variable* lhs, sail::Variable* array,
			sail::Symbol* index, int offset, Constraint c);
	void process_array_ref_read(Variable* lhs, Variable* array,
			AccessPath* index, int offset, Constraint c, string pp_exp = "");

	/*
	 * a[i].f = s;
	 */
	void process_array_ref_write(sail::Variable* array, sail::Symbol* index,
			int offset,  sail::Symbol* rhs, Constraint& c);

	/*
	 * a = -b;
	 */
	void process_negate(sail::Variable* lhs, sail::Symbol* rhs, Constraint& c);

	/*
	 * a = ~b
	 */
	void process_bitwise_not(sail::Variable* lhs, sail::Symbol* rhs,
			Constraint& c);

	/*
	 * a = !b
	 */
	void process_logical_not(sail::Variable* lhs, sail::Symbol* rhs,
			Constraint& c);

	/*
	 * a=b+c
	 */
	void process_plus(sail::Variable* lhs, sail::Symbol* op1,
			sail::Symbol* op2, Constraint c);
	void process_plus(Variable* lhs, AccessPath* op1, AccessPath* op2,
			Constraint c);

	/*
	 * a = b-c, a=b*c, a=b/c, a=b%c
	 */
	void process_binop(sail::Variable* lhs, sail::Symbol* op1,
			sail::Symbol* op2, Constraint c,il::binop_type bt);
	void process_binop(Variable* lhs, AccessPath* op1, AccessPath* op2,
			Constraint c, il::binop_type bt, string pp_exp);

	/*
	 * a = b, a!=b, a<=b, ...
	 */
	void process_predicate_binop(sail::Variable* lhs, sail::Symbol* op1,
			sail::Symbol* op2, Constraint& c,il::binop_type bt);


	/*
	 * static_assert(p)
	 * Issue an error if p does not hold.
	 */
	void process_static_assert(sail::Symbol* pred, Constraint &c, string loc);


	/*
	 * a = &"string_const"
	 */
	void process_address_string(sail::Variable* var, string s, Constraint c);

	/*
	 * a = malloc(size) or a = new Foo
	 */
	void process_memory_allocation(sail::Variable* lhs,
			string alloc_id, sail::Symbol* alloc_size, Constraint& c,
			bool is_nonnull);

	/*
	 * free(p) or delete p
	 */
	void process_memory_deallocation(sail::Variable* ptr, Constraint stmt_guard);

	void process_memory_deallocation(AccessPath* ptr, Constraint stmt_guard,
			string pp_exp);

	/*
	 * observed(x==2) tells the analysis that the constraint given
	 * by s was satisfied in at least one concrete execution.
	 */
	void process_observed(sail::Symbol * s, Constraint c);

	/*
	 * a = buffer_size(buf);
	 */
	void process_get_buffer_size(sail::Variable* size, sail::Symbol* buffer_ptr,
			Constraint c);

	/*
	 * buf.size = a
	 */
	void process_set_buffer_size(sail::Variable* buffer_ptr,  sail::Symbol* size,
			Constraint c);




	void process_assign_function(sail::Symbol* source, sail::Symbol* target,
			sail::Symbol* nc, sail::Symbol* sc, Constraint c);

	bool build_string_from_loc(MemoryLocation* loc, int offset, string& res);


	void process_function_call(sail::FunctionCall* fc, SummaryGraph* s,
			sail::Variable* retvar, vector<sail::Symbol*> *
			args,  Constraint & stmt_guard);

	void process_function_pointer_call(sail::Variable* function_pointer,
			set<SummaryGraph*>& target_summaries,
			sail::Variable* retvar, vector<sail::Symbol*> *
			args,  Constraint & stmt_guard);

	void process_virtual_method_call(sail::Variable* dispatch_ptr,
			set<SummaryGraph*>& target_summaries, sail::Variable* retvar,
			vector<sail::Symbol*> * args,  Constraint & stmt_guard);

	void process_unknown_function_call(
			sail::Variable* retvar, vector<sail::Symbol*> *
			args, string fun_id, Constraint & stmt_guard,
			il::type* ret_type);

	void process_instance_of(sail::Variable* retvar, sail::Symbol* s,
			il::type* t, Constraint & stmt_guard);

	/*
	 * Processes the call to the constructor by setting the dynamic type.
	 * However, this function does not instantiate the summary of the
	 * constructor; the instantiation must be done in addition to calling
	 * this function.
	 */
	void process_constructor_call(sail::FunctionCall* fc,
			vector<sail::Symbol*> * args, Constraint stmt_guard);

	void process_loop_invocation(SummaryGraph* s, Constraint stmt_guard);

	void process_address_label(sail::Variable* var, string label,
			il::type* signature, Constraint c	);

	// -------------ADT related methods------------------------



	/*
	 * ADT_INSERT(adt, key, value);
	 */
	void process_adt_insert(sail::Variable* v, sail::Symbol* key,
			sail::Symbol* value, Constraint stmt_guard);

	/*
	 * ADT_READ(result, adt, key);
	 */
	void process_adt_read(sail::Variable* v, sail::Variable* adt,
			sail::Symbol* key, Constraint stmt_guard, bool get_index_from_key);


	/*
	 * ADT_READ_REF(result, adt, key);
	 */
	void process_adt_read_ref(sail::Variable* v, sail::Variable* adt,
			sail::Symbol* key, Constraint stmt_guard, bool get_index_from_key);

	/*
	 * res = ADT_GET_SIZE(adt);
	 */
	void process_adt_get_size(sail::Variable* res, sail::Variable* adt,
			Constraint stmt_guard);
	void process_adt_get_size(AccessPath* res, AccessPath* adt,
			Constraint stmt_guard);

	/*
	 * ADT_SET_SIZE(adt, size);
	 */
	void process_adt_set_size(sail::Variable* adt, sail::Symbol* size,
			Constraint stmt_guard);



	/*
	 * ADT_REMOVE(adt, key);
	 */
	void process_adt_remove(sail::Variable* adt, sail::Symbol* key,
			Constraint stmt_guard);

	/*
	 * res = ADT_CONTAINS(adt, key);
	 */
	void process_adt_contains(sail::Variable* res, sail::Variable* adt,
			sail::Symbol* key, Constraint stmt_guard);

	void process_adt_read(AccessPath* res, AccessPath* adt_ptr,
			AccessPath* key, Constraint stmt_guard, bool get_index_from_key,
			int offset);

	void process_adt_read_ref(AccessPath* res, AccessPath* adt_ptr,
			AccessPath* key, Constraint stmt_guard, bool get_index_from_key);




	//----------------------------------------------



	MemoryLocation* get_location(AccessPath* ap);

	/*
	 * Is there a memory location associated with this access path?
	 */
	bool has_location(AccessPath* ap);

	/*
	 * Gives the memory location associated with symbol s.
	 */
	MemoryLocation* get_location(sail::Symbol* s);

	/*
	 * Adds the default targets of loc.
	 */
	void add_default_edges(MemoryLocation* loc, bool add_temp_default = false);

	void add_default_edges(MemoryLocation* loc, Constraint c,
			bool add_temp_default, bool force_default_edge);

	/*
	 * Removes a sail temporary that is no longer needed from
	 * the set of root locations used for visualization.
	 */
	void remove_root(sail::Variable* temp);

	/*
	 * Removes a temporary introduced during instantiation
	 * that is no longer needed from
	 * the set of root locations used for visualization.
	 */
	void remove_root(AccessPath* inst_temp);



	virtual ~MemoryGraph();


	string to_dotty();

	string to_dotty(set<AccessPath*> & syms);


	void get_remaining_buffer_size(sail::Symbol* buf,
			set<pair<AccessPath*, Constraint> >& possible_size,
			Constraint c);
	void get_total_buffer_size(sail::Symbol* buf,
			set<pair<AccessPath*, Constraint> >& possible_size,
			Constraint c);

	Edge* get_existing_edge(MemoryLocation* source, MemoryLocation* target,
			int source_offset, int target_offset);

	/*
	 * Returns the constraint under which this memory location
	 * was not deleted
	 */
	Constraint get_not_deleted_constraint(MemoryLocation* loc);

	// -------------------------------------------

	/*
	 * Functions for checking memory safety
	 */

	// Checks for buffer overruns and underruns
	void check_buffer_access(sail::Variable* buffer_ptr, sail::Symbol* offset,
				Constraint c,  bool user_specified_check = true);
	void check_buffer_access(sail::Variable* buffer_ptr, AccessPath* index,
			Constraint c, bool user_specified_check, string pp_expression);

	void check_static_buffer_access(AccessPath* buf_ap, AccessPath* index,
			Constraint c, string pp_expression);







	Constraint get_overrun_constraint(AccessPath* index, Constraint index_c,
			int elem_size, AccessPath* size_ap, Constraint size_ap_c,
			Constraint cur_offset_constraint);

	Constraint get_underrun_constraint(AccessPath* index, Constraint index_c,
			int elem_size, AccessPath* size_ap, Constraint size_ap_c,
			Constraint cur_offset_constraint, IndexVariable* buf_index);


	// Checks for dereferencing invalid memory (null, deleted etc.)
	void check_ptr_deref(sail::Variable* ptr, Constraint c, string pp_expression);
	void check_null_deref(sail::Variable* ptr, Constraint c, string pp_expression);
	void check_access_to_delete_mem(sail::Variable* ptr, Constraint c, string pp_expression);


	void check_access_to_deleted_mem(AccessPath* deleted_loc, Constraint c,
			string pp_exp, bool access_is_delete);

	/*
	 * Checks that static type of an object is not a strict subtype of its
	 * dynamic type.
	 */
	void check_static_type(sail::Variable* ptr, Constraint c,
			string pp_expression);

	/*
	 * Checks if read from the memory location associated with ap
	 * is valid, i.e. it is initialized and not deleted.
	 */
	void check_memory_access(AccessPath* ap, Constraint c, string pp_expression);

	void get_possible_buffer_sizes(AccessPath* buf_ap,
			set<pair<AccessPath*, Constraint> >& sizes);

	void report_errors();
	void report_error(error_code_type error, string message);

	bool is_entry_function();

private:


	/*
	 * Gives the guarded value set for access path ap
	 * under constraint c (filters out values
	 * not possible under constraint c.) The result of
	 * get_value_set is *always* in terms of
     * source variables, both for the constraint as well as the access paths.
	 */
	void get_value_set(AccessPath* ap,
			set<pair<AccessPath*, Constraint> >& values,
			Constraint c);

	void get_buffer_size(sail::Symbol* buf,
			set<pair<AccessPath*, Constraint> >& possible_sizes,
			Constraint c, bool total_size);



	/*
	 * Gives the guarded value set for loc&offset
	 * combination.
	 */
	void get_value_set(MemoryLocation* loc, int offset,
			set<pair<AccessPath*, Constraint> >& values);

	void get_points_to_set(AccessPath* ap,
			set<pair<AccessPath*, Constraint> >& pts_to_set,
			Constraint c);
	void get_points_to_set(MemoryLocation* loc, int offset,
			set<pair<AccessPath*, Constraint> >& pts_to_set,
			Constraint c);

	/*
	 * Gives the guarded value set for loc&offset
	 * combination under constraint c (filters out values
	 * not possible under constraint c.).
	 */
	void get_value_set(MemoryLocation* loc, int offset,
			set<pair<AccessPath*, Constraint> >& values,
			Constraint c);

	void add_default_edge(MemoryLocation* loc, int offset, Constraint c,
			bool add_temp_default = false);



	void print_offset_to_edge_map(map<int, set<Edge*>* >& succs);



	/*
	 * Preserves the outgoing edges of loc under !c.
	 */
	void update_outgoing_edges(MemoryLocation* loc, Constraint c,
			bool update_fake_field = false);
	void update_outgoing_edges(MemoryLocation* loc, int offset, Constraint c,
			bool update_fake_field = false);
	void update_outgoing_edges(AccessPath* ap, Constraint c,
			bool update_fake_field = false);


	/*
	 * Pointer arithmetic on struct offset
	 */
	void process_offset_plus(MemoryLocation* lhs_loc, MemoryLocation* rhs_loc,
			int offset, Constraint c, string pp_exp);
	void process_field_offset(Variable* lhs_var, AccessPath* rhs_var,
			int offset, Constraint c);


	/*
	 * Pointer arithmetic on array pointer
	 */
	void process_index_plus(MemoryLocation* lhs_loc, MemoryLocation* rhs_loc,
			AccessPath* ap, Constraint stmt_guard,
			Constraint index_guard, int elem_size_in_bytes);

	void process_index_plus(Variable* lhs_var, AccessPath* rhs_var,
			AccessPath* ap, Constraint stmt_guard,
			Constraint index_guard, int elem_size_in_bytes);

	void process_pointer_plus(sail::Variable* lhs, sail::Symbol* op1,
			sail::Symbol* op2, Constraint c);
	void process_pointer_plus(Variable* lhs, AccessPath* op1,
			AccessPath* op2, Constraint c, int elem_size=-1);

	// Helper for above pointer plus
	void process_pointer_plus(MemoryLocation* lhs_loc, MemoryLocation* rhs_loc,
			AccessPath* ap, Constraint stmt_guard, Constraint index_guard,
			il::type* elem_type, int elem_size);


	bool process_pointer_pointer_cast(MemoryLocation* loc,
								il::pointer_type* cast_t);

	void process_scalar_pointer_cast(MemoryLocation* lhs_loc,
			MemoryLocation* rhs_loc,
			il::pointer_type* cast_t, Constraint c);



	bool process_struct_cast(MemoryLocation* loc, int shift_offset,
			AccessPath* ap, il::record_type* new_type);




	Edge* put_edge(MemoryLocation* orig_source, int orig_offset,
			Constraint c, MemoryLocation* source_loc, MemoryLocation*
			target_loc,  int source_offset, int target_offset,
			bool default_edge = false, bool overwrite_fake_field = false);




	Edge* put_edge(AccessPath* source_ap, AccessPath* target_ap, Constraint c,
			bool default_edge = false, bool overwrite_fake_field = false);


	// Array-specific
	Constraint get_index_constraint(AccessPath* index, MemoryLocation* array_loc);

	void delete_edge(Edge* e);

	void assign_unmodeled_value(MemoryLocation* lhs_loc, Constraint c,
			il::type* t, unmodeled_type ut,
			const DisplayTag& dt, int optional_offset_only = -1);

	/*
	 *"Upgrades" this memory location to have an array index and updates
	 * its index constraints. Return value is the new index var associated with
	 * the array
	 */
	IndexVariable* change_to_array_loc(MemoryLocation* loc, il::type* elem_type);
	IndexVariable* change_to_array_loc(MemoryLocation* loc, int elem_size);
	IndexVariable* change_to_adt_loc(MemoryLocation* loc, int offset,
			il::type* adt_type);


	void update_memory_location(MemoryLocation* loc, AccessPath* old_prefix,
			AccessPath* new_prefix, AccessPath* old_rep= NULL);



	void print_ap_to_loc_map(bool pp);

	AccessPath* find_index_var_to_increment(AccessPath* ap, int elem_size);


	/*
	 * Substitutes old_term with new_term in all the constraints.
	 */
	void update_constraints(Term* old_term, Term* new_term);

	AccessPath* get_default_target(AccessPath* ap);

	void set_dynamic_type(MemoryLocation* loc, il::type* dyn_type,
			Constraint stmt_guard);



	/*
	 * Adds an edge from lhs_loc to true and false
	 * under true_c and false_c respectively.
	 * If true_c and false_c are imprecise,
	 * we make them precise by adding disjointness constraints
	 * and associating true_c and false_c with these
	 * disjointness constraints as part of background knowledge.
	 */
	void put_predicate_edge(MemoryLocation* lhs_loc,
			Constraint true_c, Constraint false_c);

	/*
	 * Cleaning-up related functions.
	 * The memory graph loses ownership of the edge.
	 */
	void relinquish_ownership(Edge* e);

	/*
	 * Reports an error due to inconsistent memory layout, such
	 * as adding illegal fields.
	 */
	void report_type_inconsistency(AccessPath* ap, int offset);

	void report_unsafe_buffer_access(bool user_specified_check,
			AccessPath* buffer, AccessPath* index, Constraint fail_c,
			bool overrun, string pp_expression);



	void collect_reachable_locations(MemoryLocation* cur,
			set<MemoryLocation*> & res);

	/*
	 * When a location, e.g., a is upgraded to be an array a[i] or ADT,
	 * a size field is added with default target *a[i].size.
	 * But to correctly account for offsets, we need to modify
	 * the default target to be *a.size-i.
	 */
	void update_size_targets_on_upgrade(MemoryLocation* loc, AccessPath*
			new_rep, int elem_size);

	AccessPath* find_cast_field(MemoryLocation* loc, int offset,
			il::type* cast_type);

	Constraint get_adt_index_constraint(AccessPath* key,
			AbstractDataStructure* adt_loc, Constraint incoming_c,
			bool get_index_from_key);

	void process_string_assignment(MemoryLocation* loc, string s);

	/*
	 * Checks that adt_var is a pointer to an abstract data type
	 */
	bool check_adt_usage(sail::Variable* adt_var, string function_name);

	/*
	 * If we are trying to do an ADT operation on adt_ptr_ap,
	 * this function does the necessary upgrades and checks, and returns
	 * NULL if access is illegal.
	 */
	AbstractDataStructure* get_adt_ap(AccessPath* adt_ptr_ap, MemoryLocation*
			target_loc, int offset);

	void initialize_return_value(sail::Variable* retvar);

	void add_subtype_axiom(il::record_type* rt, AccessPath* rtti_field,
			Constraint rtti_constraint);

	/*
	 * If we delete elements of an array inside a loop/recursive function,
	 * we require that there is no aliasing between different array elements.
	 */
	Constraint get_non_alias_requirements_for_delete(AccessPath* ap);



	void add_size_of_constarray(AccessPath* ap);

	AccessPath* get_default_target_of_size(AccessPath* ap);






};

#endif /* MEMORYGRAPH_H_ */
