/*
 * EntryAliasManager.cpp
 *
 *  Created on: Aug 9, 2010
 *      Author: isil
 */

#include "EntryAliasManager.h"
#include "access-path.h"
#include "AccessPath.h"

#include "sail/Function.h"
#include "sail/SuperBlock.h"
#include "sail/BasicBlock.h"
#include "sail/Cfg.h"
#include "sail/CfgEdge.h"
#include "sail/Instruction.h"
#include "sail/AddressVar.h"
#include "MemoryGraph.h"
#include "util.h"
#include "MemoryLocation.h"
#include "compass_assert.h"

#define DISABLE_ENTRY_ALIASING false


EntryAliasManager::EntryAliasManager(MemoryGraph* mg):mg(mg)
{
	su = NULL;

}

EntryAliasManager::~EntryAliasManager()
{

}

bool EntryAliasManager::is_array_pointer(AccessPath* ap)
{
	//cout << "Is array pointer? " << ap->to_string() << endl;
	if(ap->find_outermost_index_var(true) != NULL) return true;

	if(ap->get_ap_type() != AP_ARITHMETIC) return false;
	ArithmeticValue* av = (ArithmeticValue*) ap;

	if(!av->is_structured_ptr_arithmetic()) return false;
	const vector<offset>& offsets = av->get_offsets();
	for(unsigned int i=0; i<offsets.size(); i++) {
		const offset& o = offsets[i];
		if(o.is_index_offset()) return true;
	}
	return false;

}

void EntryAliasManager::process_ap_upgrade(AccessPath* old_ap, AccessPath* new_ap)
{

	old_ap = old_ap->strip_deref();
	new_ap = new_ap->strip_deref();

	{
		map<string, set<AccessPath*> >::iterator it = stored_aps.begin();
		for(; it != stored_aps.end(); it++) {
			set<AccessPath*> new_set;
			set<AccessPath*>::iterator it2 = it->second.begin();
			bool changed = false;
			for(; it2 != it->second.end(); it2++) {
				AccessPath* ap = *it2;
				AccessPath* new_cur_ap = ap;
				if(!is_array_pointer(ap)){
					new_cur_ap = ap->replace(old_ap, new_ap);
				}
				new_set.insert(new_cur_ap);
				if(new_cur_ap != ap) changed = true;
			}
			if(changed)
				it->second = new_set;
		}
	}

	{
		map<string, map<AccessPath*, set<AccessPath*, EntryAliasCompare> > >::
			iterator it = relevant_aliases.begin();
		for(; it != relevant_aliases.end(); it++) {
			map<AccessPath*, set<AccessPath*, EntryAliasCompare> > & inner =
					it->second;

			map<AccessPath*, set<AccessPath*, EntryAliasCompare> > new_inner;

			map<AccessPath*, set<AccessPath*, EntryAliasCompare> >::
				iterator it2 = inner.begin();
			for(; it2 != inner.end(); it2++){
				AccessPath* key = it2->first;
				if(!is_array_pointer(key))
				{
					key = key->replace(old_ap, new_ap);
				}

				set<AccessPath*, EntryAliasCompare> new_set;
				set<AccessPath*, EntryAliasCompare>::iterator it3 =
						it2->second.begin();
				for(; it3 != it2->second.end(); it3++) {
					AccessPath* ap = *it3;
					c_assert(ap != NULL);

					AccessPath* new_cur_ap = ap;
					if(!is_array_pointer(ap)){
						new_cur_ap = ap->replace(old_ap, new_ap);
					}

					c_assert(new_cur_ap != NULL);
					new_set.insert(new_cur_ap);
				}



				new_inner[key] = new_set;

			}


			it->second = new_inner;


		}
	}


}

void EntryAliasManager::set_summary_unit(sail::SummaryUnit* su)
{
	this->su = su;

	if(!su->is_superblock()) return;
	sail::SuperBlock* sb = static_cast<sail::SuperBlock*>(su);
	set<sail::BasicBlock*> pred_blocks;
	set<sail::Block*> work_list;
	set<sail::Block*> visited;
	work_list.insert(sb);
	while(work_list.size() > 0)
	{
		sail::Block* cur = *work_list.begin();
		work_list.erase(cur);
		if(visited.count(cur) > 0) continue;
		visited.insert(cur);
		if(cur->is_basicblock())
			pred_blocks.insert(static_cast<sail::BasicBlock*>(cur));

		set<sail::CfgEdge*>& preds = cur->get_predecessors();
		set<sail::CfgEdge*>::iterator it = preds.begin();
		for(; it != preds.end(); it++) {
			sail::Block* b = (*it)->get_source();
			if(b->is_superblock()) {
				sail::SuperBlock* sb = static_cast<sail::SuperBlock*>(b);
				work_list.insert(sb->get_exit_block());
				work_list.insert(sb);
			}
			else
			{
				work_list.insert(static_cast<sail::BasicBlock*>(b));
			}
		}
	}

	set<sail::BasicBlock*>::iterator it = pred_blocks.begin();
	for(; it != pred_blocks.end(); it++) {
		sail::BasicBlock* b = *it;
		vector<sail::Instruction*> & stmts = b->get_statements();
		vector<sail::Instruction*> ::iterator it2 = stmts.begin();
		for(; it2 != stmts.end(); it2++) {
			sail::Instruction* inst = *it2;
			if(inst->get_instruction_id() != sail::ADDRESS_VAR) continue;
			sail::AddressVar* addr = static_cast<sail::AddressVar*>(inst);
			sail::Symbol* s = addr->get_rhs();

			AccessPath* address_ap = mg->get_access_path_from_symbol(s);
			address_ap = Address::make(address_ap);
			relevant_addresses.insert(address_ap);

		}
	}


}

AccessPath* EntryAliasManager::get_size(AccessPath* ap)
{
	AccessPath* deref = ap->add_deref();
	if(deref->get_ap_type() == AP_ARRAYREF) {
		deref = deref->get_inner();
	}
	FieldSelection* fs = FieldSelection::make_size(deref);
	return fs;
}


AccessPath* EntryAliasManager::get_size_difference(AccessPath* ap,
		AccessPath* alias)
{
	int elem_size;
	IndexVariable* iv = find_relevant_index_var(alias, elem_size);
	if(iv == NULL) return NULL;

	AccessPath* alias_size = get_size(alias);

	int my_elem_size;
	IndexVariable* my_iv = find_relevant_index_var(ap, my_elem_size);
	if(my_iv == NULL) {
		return
		ArithmeticValue::make_minus(alias_size, ConstantValue::make(elem_size));
		//return alias_size;
	}

	AccessPath* my_size = get_size(ap);

	AccessPath* size_diff = ArithmeticValue::make_minus(alias_size, my_size);
	//size_diff = ArithmeticValue::make_plus(size_diff, ConstantValue::make(1));
	return size_diff;

}


Constraint EntryAliasManager::get_index_range(AccessPath* ap, AccessPath* alias)
{
	int alias_elem_size;
	IndexVariable* alias_iv = find_relevant_index_var(alias, alias_elem_size);


	/*
	 * Alias not an array; so we don't any range on the index.
	 */
	if(alias_iv == NULL) return Constraint(true);

	alias_iv = (IndexVariable*)
				IndexVarManager::rename_source_to_target(alias_iv);


	/*
	 * If the current alias is myself, just return iv == 0.
	 * This is an optimization because otherwise we would
	 * generate 0<= iv <= 0.
	 */
	if(ap == alias) {
		return ConstraintGenerator::get_eqz_constraint(alias_iv);
	}


	AccessPath* size_diff = get_size_difference(ap, alias);
	c_assert(size_diff != NULL);

	AccessPath* elem_size_x_index = ArithmeticValue::make_times(alias_iv,
			ConstantValue::make(alias_elem_size));


	Constraint index_pos(true);
	index_pos &= ConstraintGenerator::get_geqz_constraint(elem_size_x_index);
	index_pos &= ConstraintGenerator::get_leq_constraint(elem_size_x_index,
			size_diff);

	int my_elem_size;
	IndexVariable* my_iv = find_relevant_index_var(ap, my_elem_size);
	if(my_iv == NULL) return index_pos;



	Constraint index_neg(true);
	index_neg &= ConstraintGenerator::get_leqz_constraint(elem_size_x_index);
	index_neg &= ConstraintGenerator::get_leq_constraint(size_diff,
			elem_size_x_index);


	Constraint res = index_pos | index_neg;
	return res;

}

bool EntryAliasManager::get_potential_entry_aliases(AccessPath* ap, vector<pair<
		AccessPath*, Constraint> > & entry_aliases)
{

	AccessPath* orig_ap = ap;

	AccessPath* ap_without_index = ap;
	if(ap_to_rep.count(ap) > 0) {
		ap = ap_to_rep[ap];
	}


	il::type* t= ap->get_type();
	if(DISABLE_ENTRY_ALIASING ||
			!has_potential_aliases(ap) ||
			relevant_aliases.count(t->to_string()) == 0 ||
			relevant_aliases[t->to_string()].size() == 0 ||
			relevant_aliases[t->to_string()].count(ap) == 0)
	{
		entry_aliases.push_back(make_pair(orig_ap, Constraint(true)));
		return false;
	}

	set<AccessPath*, EntryAliasCompare> aliases =
			relevant_aliases[t->to_string()][ap];

	bool res = false;
	if(aliases.count(ap) > 0 && ap->get_ap_type() == AP_ARRAYREF) {
		res = true;
	}
	else aliases.insert(ap);

	Constraint neq_before(true);

	map<Term*, int> to_divide_source;
	map<Term*, int> to_divide_target;



	set<AccessPath*,  EntryAliasCompare>::iterator it = aliases.begin();
	for(; it!= aliases.end(); it++)
	{
		AccessPath* cur = *it;




		AccessPath* cur_j = IndexVarManager::rename_source_to_target(cur);
		AccessPath* ap_j =
				IndexVarManager::rename_source_to_target(ap_without_index);
		Constraint eq = ConstraintGenerator::get_eq_constraint(ap_j, cur_j);


		Constraint shared_base(true);
		int elem_size;
		if(find_relevant_index_var(cur, elem_size) != NULL) {
			shared_base = ConstraintGenerator::get_neqz_constraint(
					FunctionValue::make_shared_base(cur, ap));

		}


		Constraint cur_c = eq & shared_base;

		/*
		 * The aliasing between different array elements may be relevant
		 */
		if(res  && ap == cur) {
			c_assert(ap->get_ap_type() == AP_ARRAYREF);
			AccessPath* ap_j = IndexVarManager::rename_source_to_target(ap);
			Constraint elem_alias_c =
					ConstraintGenerator::get_eq_constraint(ap, ap_j);
			cur_c &= elem_alias_c;
		}


		Constraint cur_c_with_before = cur_c & neq_before;

		if(ap == cur) {
			cur = orig_ap;
		}



		entry_aliases.push_back(make_pair(cur, cur_c_with_before));


		/*
		 * We don't want to introduce a-b%4 constraints, so
		 * rename 4j to j before eliminating because
		 * we know alignment restrictions must already be
		 * satisfied.
		 */
		IndexVariable* iv = this->find_relevant_index_var(cur, elem_size);
		if(iv != NULL && elem_size != 1)
		{
			iv = IndexVariable::make_target(iv);
			cur_c.divide(elem_size, iv);

		}


		IndexVarManager::eliminate_target_vars(cur_c);

		/*if(cur != ap && cur_c.sat()) {
			cout << "ADDING EXTRA EDGE FROM ENTRY ALIAS: " << ap->to_string() <<
					" and " << cur->to_string() << endl;
		}*/


		neq_before &= !cur_c;
		if(neq_before.unsat()) break;



	}
	return res;

}

IndexVariable* EntryAliasManager::find_relevant_index_var(AccessPath* ap, int&
		elem_size)
{
	elem_size = -1;
	if(ap->get_ap_type() != AP_ARITHMETIC) return NULL;
	ArithmeticValue* av = (ArithmeticValue*) ap;
	if(!av->is_structured_ptr_arithmetic()) return NULL;

	for(unsigned int i=0; i<av->get_offsets().size(); i++)
	{
		const offset& o = av->get_offsets()[i];
		if(o.is_index_offset()) {
			if(o.index->get_ap_type() == AP_INDEX) {
				elem_size = o.elem_size;
				return (IndexVariable*) o.index;
			}

		}
	}
	return NULL;
}


void EntryAliasManager::add_stored(AccessPath* ap)
{



	/*
	 * Do not add any entry aliasing info for top of the callgraph functions
	 */
	if(mg->is_entry_function()) return;
	if(!has_potential_aliases(ap)) return;




	il::type* t = ap->get_type();

	add_stored_internal(t, ap);




	//cout << "### Added Mapping " << *t << " -> " <<
		//		ap->to_string() << endl;

	il::type* inner_t = t->get_inner_type();
	if(inner_t->is_record_type()) {
		il::record_type* rt = (il::record_type*) inner_t;
		il::record_info* ri = rt->get_field_from_offset(0);
		if(ri == NULL) return;
		il::type* first_field_t = ri->t;
		t = il::pointer_type::make(first_field_t, "");
		add_stored_internal(t, ap);
	//	cout << "### Added Mapping " << *t << " -> " <<
		//			ap->to_string() << endl;
	}

}

string EntryAliasManager::to_string()
{
	string res = "***********Relevant alias sets********** \n";
	res+= " Size: " + int_to_string(count()) + "\n";

	map<string, map<AccessPath*, set<AccessPath*, EntryAliasCompare>
	> >::iterator it =
			this->relevant_aliases.begin();
	for(; it!= relevant_aliases.end(); it++)
	{
		string t = it->first;
		res += "\t Type: " + t + "\n";

		map<AccessPath*, set<AccessPath*, EntryAliasCompare> > & cur = it->second;
		map<AccessPath*, set<AccessPath*, EntryAliasCompare> >::iterator it2 = cur.begin();
		for(; it2!= cur.end(); it2++)
		{
			AccessPath* cur_ap = it2->first;
			set<AccessPath*, EntryAliasCompare>& cur_aliases = it2->second;
			res += "\t\t" + cur_ap->to_string()  + " (" +
					cur_ap->get_type()->to_string() + ") " + " -> {";

			set<AccessPath*>::iterator it3 = cur_aliases.begin();

			for(int i = 0; it3 != cur_aliases.end(); it3++, i++)
			{
				AccessPath* cur_alias = *it3;
				res += cur_alias->to_string();
				if(i != cur_aliases.size()-1) {
					res += ", ";
				}
			}
			res += "} \n";

		}


	}

	return res;


}


int EntryAliasManager::count()
{
	if(DISABLE_ENTRY_ALIASING) return 0;

	int c = 0;
	map<string, map<AccessPath*, set<AccessPath*, EntryAliasCompare>
	> >::iterator it =
			this->relevant_aliases.begin();
	for(; it!= relevant_aliases.end(); it++)
	{

		map<AccessPath*, set<AccessPath*, EntryAliasCompare> > & cur = it->second;
		map<AccessPath*, set<AccessPath*, EntryAliasCompare> >::iterator it2 =
				cur.begin();
		for(; it2!= cur.end(); it2++)
		{
			set<AccessPath*, EntryAliasCompare>& cur_aliases = it2->second;
			c+= cur_aliases.size();

		}


	}
	return c;
}


void EntryAliasManager::add_loaded(AccessPath* ap)
{

	/*
	 * Do not add any entry aliasing info for top of the callgraph functions
	 */
	if(mg->is_entry_function()) return;
	if(!has_potential_aliases(ap)) return;




	il::type* t = ap->get_type();

	add_loaded_internal(t, ap);




	//cout << "### Added Mapping " << *t << " -> " <<
		//		ap->to_string() << endl;

	il::type* inner_t = t->get_inner_type();
	if(inner_t->is_record_type()) {
		il::record_type* rt = (il::record_type*) inner_t;
		il::record_info* ri = rt->get_field_from_offset(0);
		if(ri != NULL)
		{
			il::type* first_field_t = ri->t;
			t = il::pointer_type::make(first_field_t, "");
			add_loaded_internal(t, ap);
		}
	//	cout << "### Added Mapping " << *t << " -> " <<
		//			ap->to_string() << endl;
	}






}

void EntryAliasManager::mark_array_representative(AccessPath* ap)
{
	if(ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(av->is_structured_ptr_arithmetic()) {
			const vector<offset>& offsets = av->get_offsets();
			if(offsets[offsets.size()-1].is_index_offset()) {
				vector<offset> new_offsets = offsets;
				new_offsets.pop_back();
				AccessPath* new_av =
						ArithmeticValue::make_structured_pointer_arithmetic(
								av->get_base_ptr(),new_offsets);
				ap_to_rep[new_av] = ap;
			}
		}
	}
}

void EntryAliasManager::add_stored_internal(il::type* t, AccessPath* ap)
{

	//if(ap->get_type()->to_string() != t->to_string()) return;

	/*if(ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(av->is_structured_ptr_arithmetic()) {
			const vector<offset>& offsets = av->get_offsets();
			if(offsets[offsets.size()-1].is_index_offset()) {
				vector<offset> new_offsets = offsets;
				new_offsets.pop_back();
				ArithmeticValue* new_av =
						ArithmeticValue::make_structured_pointer_arithmetic(av->get_base_ptr(),
								new_offsets);
				ap_to_rep[new_av] = ap;
			}
		}
	}*/

	mark_array_representative(ap);
	stored_aps[t->to_string()].insert(ap);
	map<AccessPath*, set<AccessPath*, EntryAliasCompare> >&
	ap_map = relevant_aliases[t->to_string()];
	//if(ap_map.count(ap) >0) return;
	//ap_map[ap].insert(ap);


}

void EntryAliasManager::clear_stored()
{
	stored_aps.clear();
}

void EntryAliasManager::add_loaded_internal(il::type* t, AccessPath* ap)
{
	//if(ap->get_type()->to_string() != t->to_string()) return;

	/*if(ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(av->is_structured_ptr_arithmetic()) {
			const vector<offset>& offsets = av->get_offsets();
			if(offsets[offsets.size()-1].is_index_offset()) {
				ap_to_rep[av->get_base_ptr()] = ap;
			}
		}
	}*/

	mark_array_representative(ap);
	map<AccessPath*, set<AccessPath*, EntryAliasCompare> >&
	ap_map = relevant_aliases[t->to_string()];
	set<AccessPath*, EntryAliasCompare>& my_aliases = ap_map[ap];
	//my_aliases.insert(ap);

	set<AccessPath*> & stored = stored_aps[t->to_string()];
	set<AccessPath*>::iterator it = stored.begin();
	for(; it!= stored.end(); it++)
	{
		AccessPath* stored_ap = *it;
		ap_map[stored_ap].insert(ap);
		my_aliases.insert(stored_ap);


	}




}




bool EntryAliasManager::has_potential_aliases(AccessPath* ap)
{

	if(ap->is_derived_from_constant()) return false;
	if(ap->to_string().find("$anonymous")!=string::npos) return false;

	if(ap->get_ap_type() == AP_ADDRESS) return false;
	AccessPath* base = ap->get_base();
	if(base == NULL) return false;
	if(base->get_ap_type() != AP_VARIABLE) return false;


	/*
	 * Inside a constructor, anything reachable from "this"
	 * cannot have an entry alias.
	 */
	if(su->is_function()) {
		sail::Function* f = (sail::Function*) su;
		if(f->is_constructor()) {
				Variable* v = (Variable*) base;
				if(v->get_arg_number() == 0) {
					return false;
				}

		}
	}





	if(su->is_superblock()) {
		if(ap->get_ap_type() == AP_ADDRESS &&
				relevant_addresses.count(ap) == 0) return false;
		if(ap->get_ap_type() == AP_ARITHMETIC) {
			ArithmeticValue* av = (ArithmeticValue*) ap;
			AccessPath* base_ptr = av->get_base_ptr();
			if(base_ptr != NULL && base_ptr->get_ap_type() == AP_ADDRESS &&
					relevant_addresses.count(base_ptr) == 0) return false;
		}


	}



	if(ap->get_ap_type() == AP_ARITHMETIC) {
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(av->is_structured_ptr_arithmetic()) {
			const vector<offset>& offsets = av->get_offsets();
			const offset & o = *offsets.rbegin();
			if(o.is_field_offset() && o.field_offset < 0)
				return false;
		}
	}



	if(!ap->maybe_entry_aliased(su->is_superblock())) return false;


	return true;
}


bool EntryAliasCompare::operator()(const AccessPath* _a,
		const AccessPath* _b) const
{
	AccessPath* a = (AccessPath*) _a;
	AccessPath* b = (AccessPath*) _b;


	il::type* a_type = a->get_type();
	il::type* b_type = b->get_type();

	if(a->get_ap_type() == AP_ARITHMETIC){
		AccessPath* base = a->get_base();
		if(base != NULL) a_type = base->get_type();
	}

	if(b->get_ap_type() == AP_ARITHMETIC){
		AccessPath* base = b->get_base();
		if(base != NULL) b_type = base->get_type();
	}

	bool a_record = false;
	if(a_type->get_inner_type() != NULL)
		a_record = a_type->get_inner_type()->is_record_type();

	bool b_record = false;
	if(b_type->get_inner_type() != NULL)
		b_record = b_type->get_inner_type()->is_record_type();

	if(!a_record && b_record) return false;
	if(a_record && !b_record) return true;
	return a->to_string() < b->to_string();
}
