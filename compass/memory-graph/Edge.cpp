/*
 * Edge.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: isil
 */

#include "Edge.h"
#include "MemoryLocation.h"
#include "util.h"
#include "MemoryGraph.h"
#include "AccessPath.h"
#include "Variable.h"
#include "IndexVarManager.h"
#include "IndexVariable.h"
#include "compass_assert.h"

Edge::Edge(IndexVarManager& ivm, int time_stamp,
		Constraint& c, MemoryLocation* source_loc,
		MemoryLocation* target_loc,  int source_offset, int target_offset,
		bool default_edge, int block_id):c(c)
{
	this->c.fresh_id();
	this->time_stamp = time_stamp;
	this->source_loc = source_loc;
	this->target_loc = target_loc;
	this->source_offset = source_offset;
	this->target_offset = target_offset;
	this->to_default_target = default_edge;
	source_loc->add_successor(this, source_offset);
	target_loc->add_predecessor(this, target_offset);
	this->recursive_edge = source_loc == target_loc;
	AccessPath* source_ap = source_loc->get_access_path(source_offset);
	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	ivm.remove_unused_index_vars(this->c, source_ap, target_ap);
	this->in_delete_queue = false;
	final_delete = false;
	if(!default_edge) block_ids.insert(block_id);



}
Edge::Edge(int time_stamp, MemoryLocation* source_loc,
		MemoryLocation* target_loc, int source_offset, int target_offset,
		bool default_edge, int block_id)
{
	this->c.fresh_id();
	this->time_stamp = time_stamp;
	this->source_loc = source_loc;
	this->target_loc = target_loc;
	this->source_offset = source_offset;
	this->target_offset = target_offset;
	this->to_default_target = default_edge;
	source_loc->add_successor(this, source_offset);
	target_loc->add_predecessor(this, target_offset);
	this->recursive_edge = source_loc == target_loc;
	this->in_delete_queue = false;
	final_delete = false;
	if(!default_edge) block_ids.insert(block_id);
}

AccessPath* Edge::get_source_ap()
{
	return source_loc->get_access_path(source_offset);
}
AccessPath* Edge::get_target_ap()
{
	return target_loc->get_access_path(target_offset);
}

bool Edge::has_default_target()
{

	if(to_default_target) return true;


	AccessPath* source_ap = source_loc->get_access_path(source_offset);
	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	if(target_ap->get_ap_type() == AP_ARRAYREF)
			target_ap = target_ap->get_inner();
	target_ap = target_ap->strip_deref();
	bool res = (source_ap == target_ap);
	return res;

}

set<int>& Edge::get_block_ids()
{
	return block_ids;
}

bool Edge::is_default_edge()
{
	if(!to_default_target){
		return false;
	}

	cout << "Default edge? " << this->to_string() << endl;

	Constraint non_default_c = get_non_default_constraint();

	cout << "Non default c: " << non_default_c << endl;

	return non_default_c.unsat();
}

void Edge::set_source(AccessPath* ap)
{
	source_loc->set_access_path(ap, source_offset);
}
void Edge::set_target(AccessPath* ap)
{
	target_loc->set_access_path(ap, target_offset);
}

Constraint Edge::get_non_default_constraint()
{


	if(!to_default_target){
		return c;
	}

	if(source_loc->get_access_path(source_offset)->is_size_field_ap())
		return Constraint(false);

	if(source_loc->get_representative_access_path()->get_ap_type() == AP_ADT) {
		return Constraint(false);
	}


	AccessPath* source_ap = get_source_ap();
	AccessPath* target_ap = get_target_ap();
	vector<IndexVariable*> source_index_vars;
	vector<IndexVariable*> target_index_vars;
	source_ap->get_nested_index_vars(source_index_vars);
	target_ap->get_nested_index_vars(target_index_vars);

	Constraint default_c;
	if(source_index_vars.size() > 0 &&
			source_index_vars.size() == target_index_vars.size())
	{
		IndexVariable* source_var = source_index_vars[0];
		IndexVariable* target_var = IndexVariable::make_target(source_var);
		default_c = ConstraintGenerator::get_eq_constraint(source_var,
				target_var);
	}
	if(source_index_vars.size() < target_index_vars.size())
	{
		IndexVariable* source_var = target_index_vars[0];
		IndexVariable* target_var = IndexVariable::make_target(source_var);
		default_c = ConstraintGenerator::get_eqz_constraint(target_var);
	}


	Constraint new_edge_c = c & !default_c;
	if(new_edge_c.sat()) {
		return new_edge_c;
	}
	return Constraint(false);
}


void Edge::and_constraint(IndexVarManager& ivm, Constraint and_c)
{
	AccessPath* source_ap = source_loc->get_access_path(source_offset);
	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	ivm.remove_unused_index_vars(and_c, source_ap, target_ap);
	c = and_c & c;

}

void Edge::add_disjointness_constraint(Constraint dc)
{
	Constraint nc = c.nc();
	Constraint sc = c.sc();
	nc &= dc;
	sc |= dc;
	c = Constraint(nc, sc);
}

void Edge::and_not_constraint(IndexVarManager& ivm, Constraint and_c)
{
	AccessPath* source_ap = source_loc->get_access_path(source_offset);
	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	ivm.remove_unused_index_vars(and_c, source_ap, target_ap);
	c = (!and_c) & c;
}



void Edge::or_constraint(IndexVarManager& ivm, Constraint or_c, int block_id)
{
	AccessPath* source_ap = source_loc->get_access_path(source_offset);
	AccessPath* target_ap = target_loc->get_access_path(target_offset);
	ivm.remove_unused_index_vars(or_c, source_ap, target_ap);
	c = or_c | c;
	c.sat();
	block_ids.insert(block_id);
}

Constraint& Edge::get_constraint()
{
	return c;
}

void Edge::set_constraint(Constraint c)
{


	this->c = c;

}

MemoryLocation* Edge::get_source_loc()
{
	return source_loc;
}
MemoryLocation* Edge::get_target_loc()
{
	return target_loc;
}
int Edge::get_source_offset()
{
	return source_offset;
}
int Edge::get_target_offset()
{
	return target_offset;
}

string Edge::to_string()
{
	string res = "(" + source_loc->to_string(true) + ", "+
		int_to_string(source_offset) + ") -> ";
	res += "(" + target_loc->to_string(true) + ", "+
	int_to_string(target_offset) + ")";
	return res;
}

bool Edge::is_recursive_edge()
{
	return recursive_edge;
}

int Edge::get_time_stamp() const
{
	return time_stamp;
}

void Edge::set_final_delete()
{
	final_delete = true;
}

Edge::~Edge()
{
	if(final_delete) return;

	source_loc->remove_successor(this, source_offset);
	target_loc->remove_predecessor(this, target_offset);
	c_assert(source_loc->get_successors(source_offset)->count(this) == 0);
	c_assert(target_loc->get_predecessors(target_offset)->count(this) == 0);
	source_loc = NULL;
	target_loc = NULL;

}

bool CompareTimestamp::operator()(const Edge* _e1, const Edge* _e2) const
{
	Edge *e1 = (Edge*) _e1;
	Edge* e2 = (Edge*) _e2;
	if(e1->get_time_stamp() > e2->get_time_stamp()) return false;
	if(e1->get_time_stamp() < e2->get_time_stamp()) return true;
	if(e1->get_source_ap() > e2->get_source_ap()) return false;
	if(e1->get_source_ap() < e2->get_source_ap()) return true;
	return e1->get_target_ap() < e2->get_target_ap();
}

