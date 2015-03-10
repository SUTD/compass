/*
 * MemoryLocation.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "MemoryLocation.h"
#include "MemoryGraph.h"
#include "access-path.h"
#include "Edge.h"
#include <assert.h>
#include "util.h"
#include "il/type.h"
#include "IndexVarManager.h"
#include "sail/SummaryUnit.h"
#include "compass_assert.h"
#include "GlobalAnalysisState.h"
#include "ClientAnalysis.h"


MemoryLocation::MemoryLocation(IndexVarManager& ivm, AccessPath* ap,
		sail::SummaryUnit* su, bool track_rtti) {


	assert_context("Making memory location for: " + AccessPath::safe_string(ap));


	unbounded = false;
	representative = NULL;
	populate_fields(ivm, ap, 0, true);

	if(track_rtti && representative->get_type()->is_record_type())
		add_rtti_field();

	if(GlobalAnalysisState::track_delete()) {
		add_deleted_field();
	}

	if(this->representative->get_ap_type() == AP_DEREF &&
			this->representative->get_inner()->get_ap_type() == AP_ADT) {
		add_size_field();
	}

	add_client_fields();


	c_assert(representative != NULL);
	delete_count = 0;
	interface_object = representative->is_interface_object(su);

}

bool MemoryLocation::is_size_field(int offset) {
	if (offset != SIZE_OFFSET)
		return false;
	AccessPath* field = offset_to_ap[offset];
	if (field->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = (FieldSelection*) field;
		if (fs->get_field_name() == SIZE_FIELD_NAME)
			return true;
	}
	return false;
}

bool MemoryLocation::has_size_field() {
	if (offset_to_ap.count(SIZE_OFFSET) == 0)
		return false;
	AccessPath* field = offset_to_ap[SIZE_OFFSET];
	if (field->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = (FieldSelection*) field;
		if (fs->get_field_name() == SIZE_FIELD_NAME)
			return true;
	}
	return false;

}

void MemoryLocation::add_rtti_field() {
	if (offset_to_ap.count(RTTI_OFFSET) > 0)
		return;


	AccessPath* rtti_field = FieldSelection::make_rtti(representative);

	offset_to_ap[RTTI_OFFSET] = rtti_field;
	succs[RTTI_OFFSET] = new set<Edge*> ();
	preds[RTTI_OFFSET] = new set<Edge*> ();
}

void MemoryLocation::add_deleted_field()
{
	if (offset_to_ap.count(DELETED_OFFSET) > 0)
		return;

	AccessPath* base = representative->get_base();
	if(base == NULL){
		return;
	}
	ap_type apt = base->get_ap_type();

	/*
	 * It is non-sensical to add is_deleted fields to other kinds
	 * of access paths.
	 */
	if(apt != AP_ALLOC && apt != AP_VARIABLE && apt != AP_UNMODELED) {
		return;
	}

	if(representative->get_ap_type() == AP_ADDRESS) return;

	if(	representative->strip_deref()->get_ap_type() == AP_FIELD){
		FieldSelection* fs = (FieldSelection*) representative->strip_deref();
		if(fs->is_deleted_field() || fs->is_size_field() || fs->is_rtti_field()) {
			return;
		}
	}



	AccessPath* deleted_field = FieldSelection::make_deleted(representative);

	offset_to_ap[DELETED_OFFSET] = deleted_field;
	succs[DELETED_OFFSET] = new set<Edge*> ();
	preds[DELETED_OFFSET] = new set<Edge*> ();

}

void MemoryLocation::add_size_field() {
	if (offset_to_ap.count(SIZE_OFFSET) > 0)
		return;
	AccessPath* size_field = FieldSelection::make_size(representative);

	offset_to_ap[SIZE_OFFSET] = size_field;
	succs[SIZE_OFFSET] = new set<Edge*> ();
	preds[SIZE_OFFSET] = new set<Edge*> ();
}

bool MemoryLocation::add_offset(int offset, il::type* t, bool is_const) {




	assert_context("Adding offset to: " + to_string(true) + " with type: " +
			(t ? t->to_string() : "null") + " record type: " +
			this->get_representative_access_path()->get_type()->to_string());

	if(offset == SIZE_OFFSET) {
		add_size_field();
		return true;
	}
	if(offset == DELETED_OFFSET) {
		add_deleted_field();
		return true;
	}
	if(offset == RTTI_OFFSET) {
		add_rtti_field();
		return true;
	}
	if(offset <= CLIENT_INITIAL_OFFSET)
	{
		if(offset_to_ap.count(offset) >0 ) return true;
		add_client_fields();
		return (offset_to_ap.count(offset) >0 );
	}

	if(t->is_abstract_data_type()) {
		t = t->get_adt_value_type();
		//return offset == 0;
	}




	// Constants (e.g., *NULL) don't have offsets
	if(offset != 0 && get_representative_access_path()->is_derived_from_constant())
		return false;

	while(t->is_record_type() || t->is_array_type())
	{
		if(t->is_record_type())
		{
			il::record_type* rt = (il::record_type*)t;
			if(rt->get_field_from_offset(0) == NULL){
				break;
			}

			t = rt->get_field_from_offset(0)->t;
		}
		else
		{
			il::array_type* at = (il::array_type*)t;
			t = at->get_elem_type();
		}
	}

	//if(t->get_size() == 0) {
	//	cout << "Void type: " << offset << endl;
	//	assert(false);
	//}

	int size = t->get_size() / 8;
	int next_offset = get_next_offset(offset);



	// offset doesn't exist
	if (offset_to_ap.count(offset) == 0) {
		if(offset == SIZE_OFFSET)
		{
			add_size_field();
			return true;
		}

		if(offset == RTTI_OFFSET) {
			add_rtti_field();
			return true;
		}

		int prev_offset = get_prev_offset(offset);
		if (prev_offset != INVALID_OFFSET) {
			AccessPath* ap = offset_to_ap[prev_offset];
			il::type* t = ap->get_type();
			c_assert(t!= NULL);
			if(!t->is_void_type())
			{
				int prev_size = t->get_size() / 8;
				if (prev_offset + prev_size > offset)
				{
					return false;
				}
			}
		}

		if (next_offset != INVALID_OFFSET) {
			if (!is_const && offset + size > next_offset){
				return false;
			}
		}

		// If we got here, adding the offset is legal; so
		// we add an anonymous field selector.
		AccessPath* ap = FieldSelection::make_anonymous(offset, t,
				representative);
		offset_to_ap[offset] = ap;
		succs[offset] = new set<Edge*> ();
		preds[offset] = new set<Edge*> ();

		return true;

	}
	// offset already exists; we just need to check for consistency
	else {
		if (next_offset == INVALID_OFFSET || is_const)
			return true;
		bool res= offset + size <= next_offset;
		if(!res) {
			cout << "ALREADY EXISTS problem: " << " offset: " << offset <<
					" size: " << size << " next offset: " << next_offset << endl;
			c_assert(false);
		}


		return res;
	}

}

int MemoryLocation::get_next_offset(int offset) {
	map<int, AccessPath*>::iterator it = offset_to_ap.find(offset);
	if(it!= offset_to_ap.end())
	{
		it++;
		if (it == offset_to_ap.end())
			return INVALID_OFFSET;
		return it->first;
	}

	it = offset_to_ap.begin();
	for(; it!= offset_to_ap.end(); it++)
	{
		int cur = it->first;
		if(cur > offset) return cur;
	}

	return INVALID_OFFSET;

}


int MemoryLocation::get_prev_offset(int offset) {
	map<int, AccessPath*>::iterator it = offset_to_ap.find(offset);
	if(it!= offset_to_ap.end())
	{
		if (it == offset_to_ap.begin())
			return INVALID_OFFSET;
		it--;
		return it->first;
	}

	map<int, AccessPath*>::reverse_iterator rit = offset_to_ap.rbegin();
	for(; rit!= offset_to_ap.rend(); rit++)
	{
		int cur = rit->first;
		if(cur < offset) return cur;
	}

		return INVALID_OFFSET;


}

int MemoryLocation::get_next_offset(AccessPath* ap)
{
	int start = -1;
	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	for(; it!= offset_to_ap.end(); it++) {
		AccessPath* cur = it->second;
		if(cur->contains_nested_access_path(ap)) {
			start = it->first;
			break;
		}
	}

	// such an access path doesn't exist
	if(start == -1) return -1;
	int cur_offset = start;

	while(true)
	{
		AccessPath* cur = get_access_path(cur_offset);
		if(!cur->contains_nested_access_path(ap))
			return cur_offset;

		int next_offset = get_next_offset(cur_offset);

		// If this was the last offset, then give
		// cur offset + size_of(cur_offset) as the next available offset
		if(next_offset == INVALID_OFFSET) {
			AccessPath* start_ap = get_access_path(start);
			if(start_ap->get_type() == NULL) return INVALID_OFFSET;
			return start + start_ap->get_type()->get_size()/8;
		}

		cur_offset = next_offset;
	}


}

void MemoryLocation::add_field(IndexVarManager& ivm, AccessPath* ap, int offset) {
	populate_fields(ivm, ap, offset, false);

}

void MemoryLocation::set_access_path(AccessPath* ap, int offset) {
	assert_context("Setting access path " + AccessPath::safe_string(ap) +
			"at offset " + int_to_string(offset) +
			" of location " + to_string(true));
	c_assert(offset_to_ap.count(offset) > 0);
	offset_to_ap[offset] = ap;
}

void MemoryLocation::set_representative(AccessPath* rep) {
	this->representative = rep;
}

bool MemoryLocation::size_field_has_target() {
	if (succs.count(SIZE_OFFSET) == 0)
		return false;
	return succs[SIZE_OFFSET]->size() > 0;
}

void MemoryLocation::populate_fields(IndexVarManager& ivm, AccessPath* ap,
		int start_offset, bool find_rep) {


	c_assert(ap!=NULL);
	il::type* t = ap->get_type();
	c_assert(t!=NULL);



	/*
	 * If this is an array, strip all array types to find the base type
	 * and set unbounded and representative.
	 */
	if (ap->get_type()->is_array_type()) {
		il::array_type* at = (il::array_type*) t;
		il::type* elem_type = at;
		while(elem_type->is_array_type())
			elem_type = elem_type->get_inner_type();

		if(elem_type->is_abstract_data_type()) {
			cerr << "WARNING: It is illegal to have an array of stack-allocated "
					"abstract data structures!" << endl;
		}

		if (ap->get_ap_type() != AP_ARRAYREF) {
			ArrayRef* dummy_array = ArrayRef::make(ap, ivm.get_dummy_index(),
					ap->get_type(), elem_type->get_size()/8);
			IndexVariable* index_var = ivm.get_new_source_index(dummy_array,
					false);
			ap = ArrayRef::make(ap, index_var, ap->get_type(),
					elem_type->get_size()/8);

		}



		/*
		 * Add a size field if this is a static array *not* nested
		 * inside a struct. Arrays inside structs do not get size fields
		 * because their size is already known from their type and we can't
		 * put it at offset -4 because the location may not be an array.
		 */
		if(find_rep)
		{
			AccessPath* size_field = FieldSelection::make_size(ap);
			offset_to_ap[SIZE_OFFSET] = size_field;
			succs[SIZE_OFFSET] = new set<Edge*> ();
			preds[SIZE_OFFSET] = new set<Edge*> ();
		}

		// In our world, everything is converted to a single array.
		while (t->is_array_type()) {
			at = (il::array_type*) t;
			t = at->get_elem_type();

		}
		if (find_rep) {
			unbounded = true;
			representative = ap->find_representative();
			find_rep = false;
		}
	}

	else if(ap->get_type()->is_abstract_data_type() && (find_rep ||
			representative->get_ap_type() != AP_ADT))
	{
		il::type* adt_t = ap->get_type();


		AbstractDataStructure* dummy_adt = AbstractDataStructure::make(ap,
				ivm.get_dummy_index(), adt_t->get_adt_key_type(),
				adt_t->get_adt_value_type(), ap->get_type());
		IndexVariable* index_var = ivm.get_new_source_index(dummy_adt,
				false);
		ap = AbstractDataStructure::make(ap, index_var, adt_t->get_adt_key_type(),
				adt_t->get_adt_value_type(), ap->get_type());

		/*
		 * The type of the inner is the value type;
		 */
		//cout << "Changing t to value type of: " << t->to_string() << endl;
		//t = adt_t->get_adt_value_type();

		if(find_rep)
		{
			AccessPath* size_field = FieldSelection::make_size(ap);
			offset_to_ap[SIZE_OFFSET] = size_field;
			succs[SIZE_OFFSET] = new set<Edge*> ();
			preds[SIZE_OFFSET] = new set<Edge*> ();
		}

		if (find_rep) {
			unbounded = true;
			representative = ap->find_representative();
			find_rep = false;
		}
	}

	/*
	 * If this is a struct (but not an abstract data structure),
	 * populate fields of the struct. For abstract data structures,
	 * we don't want to model their fields since they are supposed
	 * to be abstract.
	 */
	if (t->is_record_type() &&
			((il::record_type*) t)->get_fields().size() > 0) {
		il::record_type* rt = (il::record_type*) t;

		populate_struct_fields(ivm, ap, rt, start_offset, find_rep);
	}

	/*
	 * If it's not a struct, map has only one entry.
	 */
	else {
		offset_to_ap[start_offset] = ap;
		if (succs.count(start_offset) == 0) {
			succs[start_offset] = new set<Edge*> ();
			preds[start_offset] = new set<Edge*> ();
		}

		if (find_rep) {
			unbounded = false;
			representative = ap->find_representative();
		}

	}

}

void MemoryLocation::populate_struct_fields(IndexVarManager& ivm,
		AccessPath* ap, il::record_type* rt, int start_offset, bool find_rep)
{
	vector<il::record_info*>& fields = rt->get_fields();



	il::record_info* rec_field = NULL;

	/*
	 * TODO: For now, we only deal with single recursive fields --
	 * this will need to change.
	 */
	if (rt->is_recursive()) {
		rec_field = rt->get_recursive_fields()[0];
		FieldSelection* dummy_fs = FieldSelection::make(rec_field->fname,
				rec_field->offset / 8, rec_field->t, ap, ivm.get_dummy_index());

		IndexVariable* index_var = ivm.get_new_source_index(dummy_fs, true);
		ap = FieldSelection::make(rec_field->fname, rec_field->offset / 8,
				rec_field->t, ap, index_var);
		if (find_rep) {
			unbounded = true;
			representative = ap->find_representative();
		}
	}

	else if (find_rep) {
		unbounded = false;
		representative = ap->find_representative();
	}


	for (unsigned int i = 0; i < fields.size(); i++) {
		il::record_info* ri = fields[i];
		int offset = start_offset + ri->offset / 8;
		FieldSelection* field_ap = NULL;
		if (rec_field != ri) {
			field_ap = FieldSelection::make(ri->fname, offset, ri->t, ap);
		} else {
			field_ap = (FieldSelection*) ap;
		}

		populate_fields(ivm, field_ap, offset, false);

	}

}

bool MemoryLocation::new_offset_is_legal(int start_offset, int size) {
	vector<int> existing_offsets;
	get_offsets(existing_offsets);

	int prev_offset = find_previous_offset(start_offset, existing_offsets);
	AccessPath* prev_ap = NULL;
	if (offset_to_ap.count(prev_offset) > 0)
		prev_ap = offset_to_ap[prev_offset];
	if (prev_offset != -1 && start_offset < prev_offset
			+ prev_ap->get_type()->get_size() / 8)
		return false;

	int next_offset = find_next_offset(start_offset, existing_offsets);
	if (next_offset > 0 && next_offset < start_offset + size)
		return false;

	return true;
}

bool MemoryLocation::is_orphaned() {
	map<int, set<Edge*>*>::iterator it = preds.begin();
	for (; it != preds.end(); it++) {
		if (it->second->size() > 0)
			return false;
	}
	return true;
}

AccessPath* MemoryLocation::get_access_path(int offset) {
	assert_context("Getting access path from " + to_string() + " at offset " +
			int_to_string(offset));
	assert_context("Type of memory location: " +
			get_representative_access_path()->get_type()->to_string());
	if(offset_to_ap.count(offset) == 0) return offset_to_ap[0];

	c_assert(offset_to_ap.count(offset) > 0);


	return offset_to_ap[offset];
}

bool MemoryLocation::has_field_at_offset(int offset) {
	return (offset_to_ap.count(offset) > 0);
}

void MemoryLocation::add_successor(Edge* e, int offset) {
	assert_context("Adding successor to " + to_string() + " at offset: " +
			int_to_string(offset));
	c_assert(offset_to_ap.count(offset) != 0);
	set<Edge*>* succ_edges = succs[offset];
	if (succ_edges == NULL) {
		succ_edges = new set<Edge*> ();
		succs[offset] = succ_edges;
	}
	if (e->has_default_target())
		default_target_map[offset] = e->get_target_ap();
	succ_edges->insert(e);

}

AccessPath* MemoryLocation::get_default_target(int offset) {
	return default_target_map[offset];
}

void MemoryLocation::add_predecessor(Edge* e, int offset) {
	assert_context("Adding predecessor to " + to_string() + " at offset: " +
			int_to_string(offset));
	c_assert(offset_to_ap.count(offset) != 0);
	set<Edge*>* pred_edges = preds[offset];
	if (pred_edges == NULL) {
		pred_edges = new set<Edge*> ();
		preds[offset] = pred_edges;
	}
	pred_edges->insert(e);
}

void MemoryLocation::remove_successor(Edge* e, int offset) {
	if(offset_to_ap.count(offset) == 0) return;
	set<Edge*>* succ_edges = succs[offset];
	c_assert(succ_edges != NULL);
	c_assert(succ_edges->count(e) > 0);
	succ_edges->erase(e);

	map<int, set<Edge*>*>::iterator it = succs.begin();
	for (; it != succs.end(); it++) {
		set<Edge*>*cur = it->second;
		c_assert(cur->count(e) == 0);
	}

}
void MemoryLocation::remove_predecessor(Edge* e, int offset) {
	if(offset_to_ap.count(offset) == 0) return;
	set<Edge*>* pred_edges = preds[offset];
	c_assert(pred_edges != NULL);
	c_assert(pred_edges->count(e) > 0);
	pred_edges->erase(e);

	map<int, set<Edge*>*>::iterator it = preds.begin();
	for (; it != preds.end(); it++) {
		set<Edge*>*cur = it->second;
		c_assert(cur->count(e) == 0);
	}
}

void MemoryLocation::clear()
{
	map<int, set<Edge*>*>::iterator it = preds.begin();
	for (; it != preds.end(); it++) {
		set<Edge*>*cur = it->second;
		cur->clear();
	}

	it = succs.begin();
	for (; it != succs.end(); it++) {
		set<Edge*>*cur = it->second;
		cur->clear();
	}
}

set<Edge*>* MemoryLocation::get_successors(int offset) {
	assert_context("Getting successors of: " + to_string() + " at offset" +
			int_to_string(offset));

	c_assert(offset_to_ap.count(offset) != 0);
	set<Edge*>* succ_edges = succs[offset];
	if (succ_edges == NULL) {
		succ_edges = new set<Edge*> ();
		succs[offset] = succ_edges;
	}
	return succ_edges;
}
set<Edge*>* MemoryLocation::get_predecessors(int offset) {
	assert_context("Getting predecessors of: " + to_string() + " at offset" +
			int_to_string(offset));
	c_assert(offset_to_ap.count(offset) != 0);
	set<Edge*>* pred_edges = preds[offset];
	if (pred_edges == NULL) {
		pred_edges = new set<Edge*> ();
		preds[offset] = pred_edges;
	}
	return pred_edges;
}

map<int, set<Edge*>*>& MemoryLocation::get_successor_map() {
	return succs;
}
map<int, set<Edge*>*>& MemoryLocation::get_predecessor_map() {
	return preds;
}

map<int, AccessPath*>& MemoryLocation::get_access_path_map() {
	return offset_to_ap;
}

AccessPath* MemoryLocation::get_representative_access_path() {
	return representative;
}

bool MemoryLocation::is_unbounded() {
	return unbounded;
}

MemoryLocation::~MemoryLocation() {
	map<int, set<Edge*>*>::iterator it2 = succs.begin();

	for (; it2 != succs.end(); it2++) {
		it2->second->clear();
		delete it2->second;
	}

	it2 = preds.begin();
	for (; it2 != preds.end(); it2++) {
		it2->second->clear();
		delete it2->second;
	}

	succs.clear();
	preds.clear();

}

int MemoryLocation::get_last_offset() {
	return offset_to_ap.rbegin()->first;
}

string escape_dotty_string(string s) {
	string res;
	for (unsigned int i = 0; i < s.size(); i++) {
		if (s[i] == '>') {
			res += "\\>";
			continue;
		}
		if (s[i] == '<') {
			res += "\\<";
			continue;
		}
		if (s[i] == '|') {
			res += "\\|";
			continue;
		}
		if (s[i] == '{') {
			res += "\\{";
			continue;
		}
		if (s[i] == '}') {
			res += "\\}";
			continue;
		}
		res += s[i];
	}
	return res;
}

void MemoryLocation::get_offsets(vector<int>& offsets) {
	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	for (; it != offset_to_ap.end(); it++) {
		int offset = it->first;
		offsets.push_back(offset);
	}
}

int MemoryLocation::find_previous_offset(int offset, vector<int>& offsets) {
	int prev = -1;
	for (unsigned int i = 0; i < offsets.size(); i++) {
		int cur = offsets[i];
		if (cur >= offset)
			return prev;
		prev = cur;
	}
	return prev;
}

int MemoryLocation::find_next_offset(int offset, vector<int>& offsets) {
	for (unsigned int i = 0; i < offsets.size(); i++) {
		int cur = offsets[i];
		if (cur > offset) {
			return cur;
		}
	}
	return -1;
}

int MemoryLocation::find_offset(AccessPath* ap) {

	if(ap->is_size_field_ap()) {
		return SIZE_OFFSET;
	}
	else if(ap->is_deleted_field()) {
		return DELETED_OFFSET;
	}
	else if(ap->is_rtti_field()) {
		return RTTI_OFFSET;
	}


	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	for (; it != offset_to_ap.end(); it++) {
		AccessPath* cur = it->second;
		if (cur == ap) {
			return it->first;
		}
	}
	return -1;
}

string MemoryLocation::to_string(bool pp) {
	if (!pp)
		return to_string();
	return representative->to_string();
}

bool MemoryLocation::is_interface_object() {
	return this->interface_object;
}

bool MemoryLocation::has_successors() {
	map<int, set<Edge*>*>::iterator it = succs.begin();
	for (; it != succs.end(); it++) {
		if (it->second->size() > 0)
			return true;
	}
	return false;
}
bool MemoryLocation::has_predecessors() {
	map<int, set<Edge*>*>::iterator it = preds.begin();
	for (; it != preds.end(); it++) {
		if (it->second->size() > 0)
			return true;
	}
	return false;
}

/*
 * Does this location have a successor that is not its default target?
 */
bool MemoryLocation::has_non_default_target() {
	map<int, set<Edge*>*>::iterator it = succs.begin();
	for (; it != succs.end(); it++) {
		set<Edge*>* cur_succs = it->second;
		int num_succs = cur_succs->size();
		if (num_succs > 1)
			return true;
		if (num_succs == 0)
			continue;
		Edge* e = *cur_succs->begin();
		if (e->get_target_ap() != default_target_map[it->first])
			return true;

	}
	return false;

}
void MemoryLocation::set_default_target(int offset, AccessPath* ap) {
	default_target_map[offset] = ap;
}

/*
 * For every access path contained in this memory location,
 * replaces old_prefix with new_prefix. This is used when memory
 * locations are upgraded to arrays.
 */
bool MemoryLocation::update_access_paths(AccessPath* old_prefix,
		AccessPath* new_prefix) {
	bool changed = false;
	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	for (; it != offset_to_ap.end(); it++) {
		//if(it->first == DELETED_OFFSET) continue;
		AccessPath* ap = it->second;
		AccessPath* new_ap = ap->replace(old_prefix, new_prefix);
		if (new_ap != ap) {
			set_access_path(new_ap, it->first);
			changed = true;
		}
	}

	if (!changed)
		return false;

	map<int, set<Edge*>*>::iterator it2 = this->preds.begin();
	for (; it2 != preds.end(); it2++) {
		set<Edge*>* cur_preds = it2->second;
		set<Edge*>::iterator it3 = cur_preds->begin();
		for (; it3 != cur_preds->end(); it3++) {
			Edge* e = *it3;
			if (!e->has_default_target())
				continue;
			MemoryLocation* source = e->get_source_loc();
			source->set_default_target(e->get_source_offset(),
					offset_to_ap[it2->first]);

		}
	}

	return changed;

}

void MemoryLocation::replace(map<AccessPath*,
		AccessPath*>& replacements)
{
	map<AccessPath*, AccessPath*>::iterator it = replacements.begin();
	for(; it!= replacements.end(); it++)
	{
		AccessPath* old_ap = it->first;
		AccessPath* new_ap = it->second;
		update_access_paths(old_ap, new_ap);


	}
}

string MemoryLocation::to_string()
{
	string res;
	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	int i=0;
	int size = offset_to_ap.size();
	for(; it!= offset_to_ap.end(); it++, i++)
	{
		int offset = it->first;
		AccessPath* ap = it->second;
		res += "[" + int_to_string(offset) + "->" + ap->to_string() + "]";
		if(i != size -1) res += ",";
	}
	return res;
}

string MemoryLocation::to_dotty() {

	string s = "{{";
	map<int, AccessPath*>::iterator it = offset_to_ap.begin();
	int i = 0;
	for (; it != offset_to_ap.end(); it++, i++) {
		s += "{";
		if (it->first >= 0)
			s += "<off_e" + int_to_string(it->first) + ">";
		else
			//s += "<off_size_e>";
			s += "<off_e_minus" + int_to_string(-it->first) + ">";
		s += int_to_string(it->first);
		s += "}";
		if (i < (int) offset_to_ap.size() - 1)
			s += "|";
	}
	s += "}|{";

	i = 0;
	it = offset_to_ap.begin();
	for (; it != offset_to_ap.end(); it++, i++) {
		s += "{";
		if (it->first >= 0)
			s += "<off_s" + int_to_string(it->first) + ">";
		else
			//s += "<off_size_s>";
			s += "<off_s_minus" + int_to_string(-it->first) + ">";
		s += escape_dotty_string(it->second->to_string() + ":" +
				it->second->get_type()->to_string());
		s += "}";
		if (i < (int) offset_to_ap.size() - 1)
			s += "|";
	}

	s += "}}";
	return s;
}

void MemoryLocation::add_client_fields()
{
	AccessPath* rep = get_representative_access_path();
	il::type* loc_type = rep->get_type();


	/*
	 * Do not add shadow fields to shadow fields
	 */
	if(rep->get_ap_type() == AP_DEREF) {
		AccessPath* ap_inner = rep->get_inner();
		if(ap_inner->get_ap_type() == AP_FIELD) {
			FieldSelection* fs = static_cast<FieldSelection*>(ap_inner);
			if(fs->get_field_offset() < 0) return;
		}
	}





	const map<int, ClientAnalysis*>& clients = ClientAnalysis::get_client_analyses();
	auto it = clients.begin();
	for(; it!= clients.end(); it++)
	{
		int cur_offset = it->first;
		ClientAnalysis* a = it->second;

		if (offset_to_ap.count(cur_offset) > 0)
			continue;


		if(!a->is_tracked_type(loc_type)) continue;


		const string & name = a->get_property_name();
		il::type* field_type = a->get_property_type();

		AccessPath* cur_field =  FieldSelection::make(name, cur_offset,
				field_type, representative->find_representative());


		offset_to_ap[cur_offset] = cur_field;
		succs[cur_offset] = new set<Edge*> ();
		preds[cur_offset] = new set<Edge*> ();

	}
}

