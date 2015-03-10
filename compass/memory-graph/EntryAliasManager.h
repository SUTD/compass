/*
 * EntryAliasManager.h
 *
 *  Created on: Aug 9, 2010
 *      Author: isil
 */

#ifndef ENTRYALIASMANAGER_H_
#define ENTRYALIASMANAGER_H_

#include <vector>
#include <map>
#include <set>
#include <string>
using namespace std;
class AccessPath;
class IndexVariable;

#include "Constraint.h"


namespace il {
	class type;

}

namespace sail
{

	class SummaryUnit;
}

class MemoryGraph;

class EntryAliasCompare:public binary_function<AccessPath*, AccessPath*, bool>
{

public:
	bool operator()(const AccessPath* b1, const AccessPath* b2) const;
};


class EntryAliasManager {
private:
	map<string, set<AccessPath*> > stored_aps;

	map<string, map<AccessPath*, set<AccessPath*, EntryAliasCompare> > >
	relevant_aliases;

	sail::SummaryUnit* su;
	MemoryGraph* mg;
	set<AccessPath*> relevant_addresses; //for loops only
	set<AccessPath*> empty;

	/*
	 * Due to issues with upgrading to array,
	 * we keep a mapping from, e.g. x -> x+4*i2
	 */
	map<AccessPath*, AccessPath*> ap_to_rep;

public:
	EntryAliasManager(MemoryGraph* mg);
	~EntryAliasManager();

	bool get_potential_entry_aliases(AccessPath* ap, vector<pair<
			AccessPath*, Constraint> > & entry_aliases);
	void add_stored(AccessPath* ap);
	void add_loaded(AccessPath* ap);

	void set_summary_unit(sail::SummaryUnit* su);
	bool has_potential_aliases(AccessPath* ap);
	string to_string();
	int count();
	void process_ap_upgrade(AccessPath* old_ap, AccessPath* new_ap);

	void clear_stored();

private:
	void add_stored_internal(il::type* t, AccessPath* ap);
	void add_loaded_internal(il::type* t, AccessPath* ap);

	bool is_array_pointer(AccessPath* ap);

	/*
	 * Finds the index variable for which we need to add size constraint
	 */
	IndexVariable* find_relevant_index_var(AccessPath* ap, int& elem_size);



	void mark_array_representative(AccessPath* ap);


	/*
	 * These three methods below are currently not used.
	 * For pointers to arrays, we initially wanted to
	 * make the entry aliasing constraint e.g.,:
	 * 0<=4*i<=a-b | a-b<=4*i<=0, but instead ended
	 * up introducing a predicate same_base(a, b)
	 * because when a and b instantiate to things like
	 * alloc1 = alloc2 + 20, there is no easy & efficient
	 * way to encode in the constraint language that
	 * alloc1 and some arbitrary offset from alloc2 cannot alias.
	 */
	AccessPath* get_size(AccessPath* ap);
	AccessPath* get_size_difference(AccessPath* ap1, AccessPath* ap2);
	Constraint get_index_range(AccessPath* ap, AccessPath* alias);
};


#endif /* ENTRYALIASMANAGER_H_ */
