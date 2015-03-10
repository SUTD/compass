/*
 * AccessPath.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef ACCESSPATH_H_
#define ACCESSPATH_H_

#include <string>
#include <iostream>
#include "il/type.h"
#include <unordered_map>
#include <unordered_set>
#include "Term.h"


#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

namespace sail{
	class Symbol;
	class SummaryUnit;
}

#include "il/binop_expression.h"

using namespace std;

Term* _make_ap(long int c);

/*
 * If this flag is enabled, all access paths print themselves as terms.
 * Convenient if we want to feed constraints to the constraint solver
 * directly.
 */
#define PRINT_AS_TERM false






enum ap_type {
	AP_NON_AP = 0,
	AP_ARITHMETIC,
	AP_ARRAYREF,
	AP_CONSTANT,
	AP_DEREF,
	AP_FIELD,
	AP_FUNCTION,
	AP_VARIABLE,
	AP_ADDRESS,
	AP_ALLOC,
	AP_UNMODELED,
	AP_INDEX,
	AP_COUNTER,
	AP_ADT,
	AP_STRING,
	AP_NIL,
	AP_PROGRAM_FUNCTION,
	AP_TYPE_CONSTANT,
	AP_END
};

enum ap_attribute
{
	ATTRIB_NOATTRIB,
	ATTRIB_UNMODELED = 1,
	ATTRIB_SOURCE_INDEX,
	ATTRIB_TARGET_INDEX,
	ATTRIB_FREE_INDEX,
	ATTRIB_INST_SOURCE_INDEX,
	ATTRIB_INST_TARGET_INDEX,
	ATTRIB_ADDRESS,
	ATTRIB_PROGRAM_VAR,
	ATTRIB_DISJOINT,
	ATTRIB_TARGET,
	ATTRIB_STRING_CONST,
	ATTRIB_ITERATION_COUNTER,
	ATTRIB_LOOP_TERMINATION_VAR,
	ATTRIB_SUMMARY_TEMP,
	ATTRIB_SAIL_EXIT_VAR,
	ATTRIB_TEMP_VAR,
	ATTRIB_MISTRAL_VAR,
	ATTRIB_LOOP_ERROR_TEMP,
	ATTRIB_ADT_POS,
	ATTRIB_IS_NIL_FN,
	ATTRIB_NIL,
	ATTRIB_PROGRAM_FUNCTION,
	ATTRIB_IMPRECISE_ARITHMETIC,
	ATTRIB_TYPE_CONSTANT,
	ATTRIB_ALLOC

};





class Variable;
class IndexVarManager;
class IndexVariable;
class IterationCounter;

class AccessPath {
	friend struct std::hash<AccessPath*>;
	friend class SummaryGraph;
	friend class boost::serialization::access;

public:
	il::type* t;
protected:
	bool has_index_var;

	ap_type apt;

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & t;
		ar & has_index_var;
		ar & apt;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & t;
		ar & has_index_var;
		ar & apt;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:
	AccessPath();
	virtual ~AccessPath();
public:

	inline static string safe_string(AccessPath* ap)
	{
		if(ap == NULL) return "null";
		if( ((long int) ap) <= 100) return "null (with cast)";
		return ap->to_string();
	}

	inline il::type* get_type()
	{
		return t;
	}

	inline ap_type get_ap_type()
	{
		return apt;
	}

	Term* to_term();


	static AccessPath* to_ap(Term* t);

	AccessPath* multiply_ap(long int constant);
	AccessPath* divide_ap(long int constant);
	AccessPath* add_ap(AccessPath* ap);
	AccessPath* subtract_ap(AccessPath* ap);

	void add_signedness_attribute();

	/*
	 * Returns the access path directly nested inside this ap, NULL if
	 * "inner" is not defined. Only defined for memory access paths.
	 */
	virtual AccessPath* get_inner();



	/*
	 * Translates expressions like &a.f as &a + offset(f)
	 * After calling this function, everything is of the form
	 * &x+offset
	 */
	AccessPath* push_address_through();

	/*
	 * Does this access path correspond to a constant?
	 */
	bool is_constant();

	/*
	 * Its base is a constant, e.g. *NULL.
	 */
	bool is_derived_from_constant();

	string to_string();


	/*
	 * base means it doesn't contain any derefs, field selectors, or
	 * array references.
	 */
	bool is_base();

	AccessPath* get_base();
	AccessPath* set_base(AccessPath* new_base);


	/*
	 * Returns the access path that would be the representative of the
	 * memory location this access path is part of. The representative
	 * does not contain any field selectors for bounded memory locations.
	 * For unbounded memory locations, the representative is the
	 * access path associated with the recursive field.
	 */
	AccessPath* find_representative();

	/*
	 * Recursive field
	 */
	bool is_recursive();

	virtual IndexVariable* get_index_var();
	virtual AccessPath* get_index_expression();

	bool contains_index_var();
	bool contains_index_var(IndexVariable* iv);

	void get_nested_index_vars(vector<IndexVariable*>& vars);
	void get_nested_index_vars(set<IndexVariable*>& vars,
			bool include_fn_term = true);

	IndexVariable* find_outermost_index_var(bool stop_at_deref = true);

	AccessPath* replace(map<AccessPath*, AccessPath*>& replacements);
	AccessPath* replace(AccessPath* ap, AccessPath* replacement);

	/*
	 * Gives an access path that would be the result of stripping one
	 * level of dereference. If the ap doesn't have a deref in front,
	 * it returns an address of access path.
	 */
	AccessPath* strip_deref();

	/*
	 * Adds one level of dereference to the access path. If the AP is
	 * an address of access path, it returns its inner expression.
	 */
	AccessPath* add_deref();


	/*
	 * Is this access path part of an array? This is the case if there
	 * is an array bracket before a dereference.
	 * For example, a[i], a[i].f, a[i].f.g belong to an array, but not
	 * (*a[i]).f.
	 */
	bool belongs_to_array();

	/*
	 * Factory method that gives an access path for a sail Symbol.
	 */
	static AccessPath* get_ap_from_symbol(sail::Symbol* s);

	/*
	 * Makes an ArithmeticValue access path from two access paths and the
	 * binop type. It may also return an unmodeled access path if we are
	 * unable to reason about this binop precisely (e.g. we ignore
	 * division at the moment.)
	 */
	static AccessPath* make_arithmetic_ap(AccessPath* ap1, AccessPath* ap2,
			il::binop_type bt);

	/*
	 * Makes an access path from non-logical binops. These
	 * can correspond to arithmetic or function terms.
	 */
	static 	AccessPath* make_access_path_from_binop(AccessPath* ap1,
			AccessPath* ap2, il::binop_type bt);

	/*
	 * Gives back all the memory ap's nested inside an access path.
	 * If an access path corresponds to a memory location, such
	 * as a.x, it doesn't give nested ones, such as a.
	 */
	void get_nested_memory_aps(set<AccessPath*>& mem_aps);

	/*
	 * Gives back all nested access paths
	 */
	void get_nested_aps(set<AccessPath*>& all_aps);

	/*
	 * Does this access path correspond to a location that is
	 * reachable to callers of a given function?
	 * This only makes sense for memory access paths.
	 * The check_arg_deref flag controls whether we require arguments
	 * to have at least one level of dereference, e.g., for deciding
	 * if the write to this location is visible to callers.
	 */
	bool is_interface_object(sail::SummaryUnit* su);

	/*
	 * Is this an access path denoting a size field?
	 */
	bool is_size_field_ap();

	/*
	 * Does this access path used for tracking dynamic type of an object?
	 */
	bool is_rtti_field();


	/*
	 * Is this access path used for tracking deallocated memory?
	 */
	bool is_deleted_field();

	/*
	 * Is this access path a fake field introduced by the analysis?
	 * For instance, size, rtti, and is_deleted are all fake fields introduced
	 * by the analysis.
	 */
	bool is_fake_field();


	/*
	 * Does this access path denote the length of some array?
	 */
	bool is_length_function();

	/*
	 * Does this access path become unobservable at the function/loop
	 * boundary? (locals, environment choices, imprecision etc.)
	 * This function only makes for memory aps.
	 */
	bool is_unobservable(bool is_loop);


	/*
	 * Is it possible that this access path has an entry alias
	 * due to "synactic" restrictions?
	 */
	bool maybe_entry_aliased(bool is_loop);

	/*
	 * Returns the set of access paths corresponding to unobservable terms,
	 * such as environment choices, variables that go out of scope,
	 * and imprecision in the analysis. Calling this function only makes
	 * sense at function boundaries.
	 */
	void get_unobservable_aps(set<AccessPath*>& unobservables, bool is_loop,
			set<AccessPath*>& excluded_aps, bool eliminate_counters);

	/*
	 * Does this access path contain any of the access paths
	 * in the aps set?
	 */
	bool contains_nested_access_path(set<AccessPath*>& aps);

	/*
	 * Is the given access path nested inside this one?
	 */
	bool contains_nested_access_path(AccessPath* ap);


	/*
	 * calls clear methods of its subclasses.
	 */
	static void clear();

	/*
	 * Is this an access path whose base is a temporary variable
	 * introduced by SAIL?
	 */
	bool is_sail_temporary();

	void get_counters(set<IterationCounter*>& counters, bool parametric);

	static void n_ary_cross_product(vector<set<pair<AccessPath*, Constraint> > >&
			ordered_value_sets, set<pair<vector<AccessPath*> , Constraint> >&
 			cross_product);

	/*
	 * Does this access path contain a nested access path with the
	 * specified type?
	 */
	bool contains_ap_type(ap_type apt);

	/*
	 * Does this access path make reference to an uninitialized value?
	 */
	bool contains_uninit_value();
	bool contains_uninit_value_rec(bool seen_deref);

	/*
	 * If there are any dereferences in this access path, returns the
	 * outermost dereference, NULL otherwise.
	 */
	AccessPath* get_outermost_deref();


	/*
	 * If we want to make a fake field from this access path, what should its
	 * inner access path be?
	 * For ADT's, the size fields etc. should not refer to any index variables, so
	 * this function strips the necessary parts.
	 */
	AccessPath* get_representative_for_fake_field();



	virtual bool is_structured_ptr_arithmetic();


	/*
	 * Does this access path represent a definitely initialized value?
	 */
	bool is_initialized();
	bool is_initialized_rec(bool seen_deref);

	bool is_exit_variable();

	bool is_imprecise();

	void update_type(il::type* t);

	/*
	 * Return the number of dereferences in this access path.
	 */
	int get_num_derefs();

	/*
	 * Does this access path contain an unmodeled/unknown value?
	 */
	bool contains_unmodeled_value();






private:
	AccessPath*  strip_deref_rec(bool first);
	AccessPath* push_address_through_rec(AccessPath* inner);

	static void n_ary_cross_product_rec(vector<set<pair<AccessPath*, Constraint> > >&
				ordered_value_sets, set<pair<vector<AccessPath*> , Constraint> >&
				cross_product, int cur_pos, vector<AccessPath*>& cur_completed,
				Constraint cur_constraint);




	//------------------------------------------




/*




	//  Walks through the access path and adds a deref to every access path
	 // that corresponds to a memory location or value.

	AccessPath* add_deref_to_leaf_aps();


	 // Reverses the effect of add_deref_to_leaf_aps

	AccessPath* strip_deref_from_leaf_aps();





	void get_nested_aps_with_index_vars(vector<AccessPath*>& aps);


	 // Returns the innermost access path.
	AccessPath* get_innermost_ap();

	AccessPath* get_innermost_ap(vector<AccessPath*> & selectors,
			int & last_deref_pos);



	// Takes set of access paths rather than variables because
	 // it also includes alloc's as variables.

	void get_nested_variables(set<AccessPath*>& vars);

*/

};

#endif /* ACCESSPATH_H_ */
