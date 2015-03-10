/*
 * Callgraph.h
 *
 *  Created on: Aug 20, 2008
 *      Author: tdillig
 */

#ifndef CALLGRAPH_H_
#define CALLGRAPH_H_

#include <boost/thread.hpp>
#include "DataManager.h"
#include "call_id.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

namespace sail
{
	class Function;
	class SummaryUnit;
	class Instruction;
}


namespace il{
	class type;
}

class DataManager;
class Error;

#include <set>
#include <string>
#include <map>
using namespace std;
using namespace boost;

/*
 * Should the callgraph collect all written and read field names?
 */
#define COLLECT_FIELDS true

#include "Identifier.h"

#define POSITION_DEPENDENT_ADT_PREFIX "__mark_position_dependent_adt_"
#define SINGLE_VALUED_ADT_PREFIX "__mark_single_valued_adt_"
#define MULTI_VALUED_ADT_PREFIX "__mark_multi_valued_adt_"

#define COMPASS_SPEC_PREFIX "_compass_spec_"

#define ADT_INSERT "_compass_adt_insert"
#define ADT_READ "_compass_adt_read"
#define ADT_READ_REF "_compass_adt_read_ref"
#define ADT_READ_POS "_compass_adt_read_pos"
#define ADT_READ_POS_REF "_compass_adt_read_pos_ref"
#define ADT_GET_SIZE "_compass_adt_get_size"
#define ADT_SET_SIZE "_compass_adt_set_size"
#define ADT_REMOVE "_compass_adt_remove"
#define ADT_CONTAINS "_compass_adt_contains"
#define ADT_RESIZE "_compass_adt_resize"


#define SPECIFY_CHECKS "specify_checks"

struct CGNode;

/*
 * Least significant bit identifies if this is a back edge;
 * next bit identifies whether this is a definite target (for function calls)
 * or whether this a potential target (for function pointer calls)
 */
struct cg_edge_id
{
	int id;
	cg_edge_id(bool definite_target);
	cg_edge_id();
	bool is_definite() const;
	bool is_backedge() const;
	void mark_backedge();

};

struct CGNode
{
	int cg_id;

	// bool identifies whether the edge from me to callee is a backedge
	map<CGNode*, cg_edge_id> callees;
	// bool identifies whether the edge from caller to me is a backedge
	map<CGNode*, cg_edge_id> callers;

	Identifier id;


	/*
	 * Gives the id used for serializing out this summary unit.
	 */
	string get_serial_id();


	/*
	 * Gives the id used for serializing out the function in
	 * which this summary unit is embedded.
	 */
	string get_function_serial_id();



	CGNode(const Identifier& su_id);
	CGNode(const Identifier& su_id, int cg_id);
	void add_callee(CGNode* callee, bool definite_target);
	bool is_function();
	bool has_callers();
	bool has_callees();
	const map<CGNode*, cg_edge_id>& get_callees();
	const map<CGNode*, cg_edge_id>& get_callers();

	/*bool dependencies_met_topdown(bool acqire_lock = true);
	bool dependencies_met_bottomup(bool acquire_lock = true);
	void update_dependents_topdown(set<CGNode*>& resolved);
	void update_dependents_bottomup(set<CGNode*>& resolved);
	void clear_dependencies();*/

private:
	string get_serial_id(string id);



};


class CGNodeLessThan
{
public:
   bool operator( )(const CGNode* n1,
		   const CGNode* n2) const;
};



/*
 * Since il::type's aren't uniquified, we need to define a less than
 * operator on function signatures to be able to put them as keys in the map
 */
class SignatureLessThan
{
public:
   bool operator( )(const il::type* const & sig1,
		   const il::type* const & sig2) const;
};


class Callgraph {
public:
	Callgraph(set<string>& fun_ids, DataManager* dm, map<string, set<Error*> > &
			errors);



	virtual ~Callgraph();
	map<Identifier, CGNode*>& get_nodes();
	set<CGNode*> & get_entry_points();
	bool is_entry_point(CGNode* node);
	CGNode* get_node(const Identifier& id);
	void remove_node(CGNode* node);
	string to_dotty(const string& dir);
	set<CGNode*>& get_global_initializers();
	CGNode* get_checks_fn();
	set<CGNode*>& get_init_functions();
	map<il::type*, set<call_id> >&
		get_signature_to_callid_map();

	/**
	 * @return Whether the analysis should track run-time type information.
	 */
	bool track_rtti();


	/*
	 * Gives the set of CGNode's transitively reachable from "start".
	 */
	void get_transitive_callees(CGNode* start,
			set<CGNode*>& transitive_callees);

	/*
	 * Gives the set of CGNode's transitively reachable *only*
	 * from "start".
	 */
	void get_exclusive_callees(CGNode* start, set<CGNode*>& exclusive_callees);

	const map<call_id, Identifier>& get_stubs();

	static bool field_written(const string & s);
	static bool field_read(const string & s);



private:

	void map_call_ids_to_identifiers(set<string>& sail_serial_ids);
	void build_initial_callgraph(set<string>& sail_serial_ids);
	void assign_ids_postorder();
	void assign_ids_postorder(CGNode* cur, set<CGNode*>& visited, int& cur_id);
	void find_entry_points(set<CGNode*>& all_nodes);
	virtual void process_summary_unit(sail::SummaryUnit* su,
			bool process_function_adress);
	void mark_backedges();
	void process_adt_functions(sail::Function* f);
	void compute_dominators();

	/*
	 * If a stub was provided for a function, remove the original function
	 * and its callees from the callgraph so that they aren't analyzed.
	 */
	void remove_replaced_nodes();

	/*
	 * Called for every instruction in the program
	 */
	void process_instruction(sail::Instruction* inst);






private:
	DataManager* dm;
	map<call_id, Identifier> call_id_to_identifier;

	map<Identifier, CGNode*> nodes;

	set<CGNode*> entry_points;
	set<CGNode*> init_functions;
	CGNode* checks;
	map<string, set<Error*> > & errors;

	/*
	 * For interface function for ADT's this map stores
	 * the corresponding specification.
	 */
	map<call_id, Identifier> orig_to_spec;
	set<Identifier> ids_with_spec;


	/*
	 * A mapping from function signatures to call_ids representing functions
	 * with the matching signature and whose address is taken.
	 */
	map<il::type*, set<call_id> > signature_to_call_id;
	map<CGNode*, set<CGNode*> > dominators;
	bool track_dynamic_types;

public:
	static set<string> read_fields;
	static set<string> written_fields;





};





#endif /* CALLGRAPH_H_ */
