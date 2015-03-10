#ifndef _ALLOC_CONTEXT_CONTROLLER_H_
#define _ALLOC_CONTEXT_CONTROLLER_H_


#include "sail.h"
#include <set>
#include "access-path.h"

class MemNode;

class AllocContext{

	MemNode* alloc_node;
	sail::FunctionCall* alloc_ins;
	AccessPath* lhs_ap;


public:
	AllocContext();

	~ AllocContext(){}

	void set_alloc_ins(sail::FunctionCall* alloc);

	void set_alloc_memnode(MemNode* mnode);
	void set_alloc_lhs_ap(AccessPath* lhs_ap);

	sail::FunctionCall* get_alloc_ins() const;

	AccessPath* get_alloc_lhs_ap() const;

	MemNode* get_alloc_mem_node() const;

	const string to_string() const;
};

class AllocContextController{

	map<MemNode*, AllocContext*> alloccs;

	//AllocContext* unfinished_alloc_context;

	set<call_id> constructor_ids;

	//e.g temp = alloc_sth(); temp is lhs and alloc ap is rhs;
	map<AccessPath*, AccessPath*> lhs_rhs_aps;


public:

	AllocContextController(){/*unfinished_alloc_context = NULL;*/}

	~AllocContextController();

	void collect_alloc_context(AllocContext* allctx, AccessPath* alloc_deref_ap);

	AccessPath* get_alloc_deref_ap(AccessPath* lhs_ap);

	sail::FunctionCall* get_alloc_ins(MemNode* mn);

	sail::Variable* get_alloc_ins_lhs_var(MemNode* mn);

	AccessPath* get_alloc_lhs_ap(MemNode* mn);

	map<MemNode*, AllocContext*>& get_allocs_map();

	bool has_node(MemNode* mn);

	const string to_string() const;




};

#endif
