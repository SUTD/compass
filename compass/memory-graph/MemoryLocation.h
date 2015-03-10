/*
 * MemoryLocation.h
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#ifndef MEMORYLOCATION_H_
#define MEMORYLOCATION_H_

#include <map>
#include <set>
#include "il/type.h"
using namespace std;

#define INVALID_OFFSET -9999

#include "AccessPath.h"
class Edge;
class IndexVarManager;

namespace il{
	class record_type;
};

namespace sail{
	class SummaryUnit;
};


/*
 * A memory location is a mapping from offsets to
 * access paths.
 */
class MemoryLocation {
	friend class MemoryGraph;
	friend class boost::serialization::access;
public:
	int delete_count;
private:
	map<int, AccessPath*> offset_to_ap;

	/*
	 * Points to edges outgoing from each offset in this memory location.
	 */
	map<int, set<Edge*>*> succs;

	/*
	 * Incoming points-to edges to a given offset in this memory location.
	 */
	map<int, set<Edge*>*> preds;

	map<int, AccessPath* > default_target_map;

	/*
	 * The access path representing this memory location.
	 */
	AccessPath* representative;

	/*
	 * True for arrays, lists etc.
	 */
	bool unbounded;

	/*
	 * Is this memory location accessible by a caller of the
	 * currently analyzed function?
	 */
	bool interface_object;
private:

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		ar & offset_to_ap;
		ar & representative;


	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & offset_to_ap;
		map<int, AccessPath*>::iterator it = offset_to_ap.begin();
		for(; it!= offset_to_ap.end(); it++)
		{
			it->second = AccessPath::to_ap(
					Term::get_term_nodelete(it->second->to_term()));
		}
		ar & representative;
		unbounded = false;
		interface_object = false;

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()



	MemoryLocation(){};


public:
	MemoryLocation(IndexVarManager& ivm, AccessPath* ap, sail::SummaryUnit* su,
			bool track_rtti);
	void add_successor(Edge* e, int offset = 0);
	void add_predecessor(Edge* e, int offset = 0);
	void clear();

	/*
	 * Adds an entry at offset to the fields associated with this memory location
	 * if one does not already exist. It returns false if such an offset cannot be
	 * added; i.e., the existing memory layout is incompatible
	 * with the addition of such a field.
	 */
	bool add_offset(int offset, il::type* t, bool is_const = false);
	void remove_successor(Edge* e, int offset = 0);
	void remove_predecessor(Edge* e, int offset = 0);
	set<Edge*>* get_successors(int offset = 0);
	set<Edge*>* get_predecessors(int offset = 0);
	bool has_successors();
	bool has_predecessors();
	map<int, set<Edge*>* >& get_successor_map();
	map<int, set<Edge*>* >& get_predecessor_map();
	map<int, AccessPath*>& get_access_path_map();
	AccessPath* get_access_path(int offset);
	void add_field(IndexVarManager& ivm, AccessPath* ap, int offset);
	int get_last_offset();
	void set_representative(AccessPath* rep);
	void set_access_path(AccessPath* ap, int offset);
	bool has_field_at_offset(int offset);
	bool update_access_paths(AccessPath* old_prefix, AccessPath* new_prefix);
	void replace(map<AccessPath*, AccessPath*>& replacements);
	void set_default_target(int offset, AccessPath* ap);
	AccessPath* get_default_target(int offset);
	bool is_size_field(int offset);
	bool has_size_field();
	void add_size_field();
	void add_rtti_field();
	void add_deleted_field();
	bool size_field_has_target();

	int get_next_offset(int offset);
	int get_prev_offset(int offset);

	/*
	 * Gives the next offset not containing ap.
	 * For example, if the struct looks like:
	 * a.f.x
	 * a.f.y
	 * a.g
	 * and we call get_next_offset(a.f), the result is a.g.
	 */
	int get_next_offset(AccessPath* ap);




	bool is_interface_object();

	/*
	 * Does this location have a successor that is not its default target?
	 */
	bool has_non_default_target();



	/*
	 * Finds the offset of the given ap, -1 if not found.
	 * Note: this requires a linear search over the map.
	 */
	int find_offset(AccessPath* ap);

	/*
	 * If this memory location is not a struct, it returns
	 * the ap at offset 0; otherwise returns an access path
	 * representing the whole struct.
	 */
	AccessPath* get_representative_access_path();

	/*
	 * True for arrays, lists, and so on.
	 */
	bool is_unbounded();

	/*
	 * This memory location has no incoming edges.
	 */
	bool is_orphaned();


	~MemoryLocation();

	string to_string(bool pp);
	string to_string();
	string to_dotty();


	void populate_struct_fields(IndexVarManager& ivm, AccessPath* ap,
			il::record_type* rt,
			int start_offset, bool find_rep = false);
	void populate_fields(IndexVarManager& ivm, AccessPath* ap, int start_offset,
			bool find_rep = false);

	/*
	 * Checks whether an entry from start_offset to start_offset+size
	 * is comptabile with the existing memory layout.
	 */
	bool new_offset_is_legal(int start_offset, int size);

	void get_offsets(vector<int>& offsets);

	static int find_previous_offset(int offset, vector<int>& offsets);
	static int find_next_offset(int offset, vector<int>& offsets);

	/*
	 * Adds "shadow" fields for properties tracked by client analyses
	 */
	void add_client_fields();
};

#endif /* MEMORYLOCATION_H_ */
