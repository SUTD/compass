#ifndef _MEMORYGRAPH_H
#define _MEMORYGRAPH_H
#include "MemEdge.h"

#include "access-path.h"
#include "SummaryUnit.h"


#define SIZE_OFFSET -4
//#define RECORD_TYPE_SIZE_OFFSET -4
#define RECORT_RIIT_FIELD_OFFSET -8

class MemEdge;
class StaticAssertElement;
class Type;
class MemNode;
class MemGraph {
private:

	set<MemNode*>* _mem_nodes;
	set<MemEdge*>* _edges;

	//for create unique access and node;
	map<AccessPath*, MemNode*>* _ap_map;

	map<AccessPath*, MemNode*> _arg_locs;

	//haven't processed yet, representative ap and its node;
	map<AccessPath*, MemNode*> _global_locs;

	set<MemNode*> _root_locs;

	//used when create new edge/node
	int _edge_counter;
	int _node_counter;


	AccessPath* return_ap;

	MemNode* unmodeled_val_node;

	MemNode* _init_node;
	MemNode* _init_target_node;



public:
	MemGraph();

	MemGraph(const MemGraph&);

	~MemGraph();


	inline set<MemEdge*>* get_edges(){
		return _edges;
	}

	MemNode* get_init_node();

	MemNode* get_init_deref_node() ;


	bool has_location(AccessPath* ap);

	map<AccessPath*, MemNode*>& get_argument_location_map();

	AccessPath* get_return_var_ap();

	MemNode* get_return_var_node();


	//unsigned int undefined_target_node_size() const;
	//for all type of ap;
	MemNode* get_memory_node(AccessPath* ap);

	MemEdge* get_edge(MemNode* source,
			MemNode* target,
			int source_offset,
			int target_offset,
			bool init = false);

	void add_edge(MemEdge* edge);

	void add_node(MemNode* node);


	void add_edge_to_next_layer(MemNode* node,
			AccessPath* ap);

	//those for " = " functionality, source is unique;
	void update_graph_for_ins(AccessPath* lhs_variable,
			AccessPath* rhs_variable_1,
			AccessPath* rhs_variable_2,
			sail::Instruction* ins);


	UnmodeledValue* get_UnmodeledValue_access_path(il::type* t){
		string name = "UNCERTAINTY";

		DisplayTag dt(name, 0, "");
		return UnmodeledValue::make(t, UNM_IMPRECISE, dt);

	}

	void build_combination_arithmetic_nodes(set<MemNode*>& op1_targets,
			set<MemNode*>& op2_targets,
			il::binop_type bt,
			set<MemNode*>* combi_targets);

	void add_edge_for_single_source_and_single_target(
			pair<MemNode*, int>& source_pair,
			MemNode* t,
			bool default_flag = false,
			bool init_flag = false);

	void add_edge_for_source_and_targets(pair<MemNode*, int> & source_pair,
			set<MemNode*>* targets,
			bool init = false);

	void add_edge_for_source_and_targets_based_on_types(
			pair<MemNode*, int>& source_pair,
			set<MemNode*>* targets);

	void update_for_cast_ins(AccessPath* lhs_variable,
			AccessPath* rhs_variable_1,
			sail::Cast* cast);

	void update_for_unop_ins(AccessPath* lhs, AccessPath* rhs,
			sail::Unop* ins);

	void update_for_ap_alloc(AccessPath* lhs_variable,
			AccessPath* rhs_variable_1,
			AccessPath* rhs_variable_2,
			sail::Instruction* ins);

	void update_for_address_string_ins(AccessPath* lhs_variable,
			AccessPath* rhs_variable_1,
			sail::AddressString* ins);

	void process_bitwise_not(set<MemNode*>*& target,
			set<MemNode*>*& new_target,
			pair<MemNode*, int>& lhs_node);

	void process_negation(set<MemNode*>*& target,
			set<MemNode*>*& new_target,
			pair<MemNode*, int>& lhs_node);

	void collect_reachable_locations(MemNode* cur,
			set<MemNode*>* res) const;

	const string to_dotty() const;



private:
	MemNode* set_init_node();
	//void delete_edge(MemEdge* edge);
	//void delete_node(MemNode* node);

	void making_deref_node_for_next_level(MemNode* source, AccessPath* inner,
			MemNode*& deref_node, int source_offset, int target_offset);

	void find_source_nodes(AccessPath* lhs, sail::Instruction* ins,
			vector<pair<MemNode*, int>>& s_nodes);

	//target are multiple
	void find_target_node(AccessPath* rhs, sail::Instruction* ins,
			set<MemNode*>*& targets);

	void update_graph_for_adding_edge(MemEdge* e);

	bool edge_existing(MemEdge* edge);
};
#endif
