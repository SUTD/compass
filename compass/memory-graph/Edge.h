/*
 * Edge.h
 *
 *  Created on: Sep 24, 2008
 *      Author: isil
 */

#ifndef EDGE_H_
#define EDGE_H_
#include "constraint.h"

class MemoryLocation;
class IndexVarManager;






class Edge {
	friend class boost::serialization::access;
public:
	bool in_delete_queue;
	bool final_delete;
private:
	Constraint c;
	MemoryLocation* source_loc;
	int source_offset;
	MemoryLocation* target_loc;
	int target_offset;
	bool recursive_edge;

	/*
	 * Is this an edge from a source ap to its default target?
	 */
	bool to_default_target;

	/*
	 * The time stamp allows us to keep track of the order in which the edges
	 * were added. This is necessary for applying function summaries
	 * in the correct order.
	 */
	int time_stamp;

	/*
	 * The set of blocks that added this edge.
	 */
	set<int> block_ids;

private:
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & source_loc;
		ar & source_offset;
		ar & target_loc;
		ar & target_offset;
		ar & c;
		ar & recursive_edge;
		ar & to_default_target;
		ar & time_stamp;
		ar & block_ids;
		ar & in_delete_queue;
		ar & final_delete;
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & source_loc;
		ar & source_offset;
		ar & target_loc;
		ar & target_offset;
		ar & c;
		ar & recursive_edge;
		ar & to_default_target;
		ar & time_stamp;
		ar & block_ids;
		ar & in_delete_queue;
		ar & final_delete;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	Edge() {};

public:
	Edge(IndexVarManager& ivm, int time_stamp,
			Constraint& c, MemoryLocation* source_loc,
			MemoryLocation* target_loc,  int source_offset,
			int target_offset, bool default_edge, int block_id);
	Edge(int time_stamp, MemoryLocation* source_loc, MemoryLocation*
			target_loc,
			int source_offset, int target_offset, bool default_edge,
			int block_id);
	bool has_default_target();
	bool is_default_edge();

	/*
	 * Returns the constraint under which this edge will not point
	 * to its default target
	 */
	Constraint get_non_default_constraint();
	Constraint& get_constraint();
	void set_constraint(Constraint c);
	void set_source(AccessPath* ap);
	void set_target(AccessPath* ap);
	void and_constraint(IndexVarManager& ivm, Constraint c);
	void and_not_constraint(IndexVarManager& ivm, Constraint and_c);
	void or_constraint(IndexVarManager& ivm, Constraint c, int block_id);
	void add_disjointness_constraint(Constraint c);
	MemoryLocation* get_source_loc();
	MemoryLocation* get_target_loc();
	int get_source_offset();
	int get_target_offset();
	AccessPath* get_source_ap();
	AccessPath* get_target_ap();
	int get_time_stamp() const;
	string to_string();
	bool is_recursive_edge();
	void set_final_delete();
	~Edge();
	set<int>& get_block_ids();

};

class CompareTimestamp:public binary_function<Edge*, Edge*, bool> {

public:
   bool operator()(const Edge* b1, const Edge* b2) const;
};

#endif /* EDGE_H_ */
