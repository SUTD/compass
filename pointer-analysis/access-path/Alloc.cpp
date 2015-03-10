/*
 * Alloc.cpp
 *
 *  Created on: Oct 15, 2008
 *      Author: tdillig
 */

#include "Alloc.h"
#include "Variable.h"
#include "util.h"
#include "IndexVariable.h"
#include "ConstantTerm.h"
#include "ConstantValue.h"
#include "Constraint.h"
#include "ConstraintGenerator.h"
#include "compass_assert.h"
#include "StringLiteral.h"

Alloc* Alloc::make(const alloc_info& alloc_id, il::type* t, AccessPath* index_var,
		bool is_nonnull)
{
	if(index_var == NULL) return make(alloc_id, t, is_nonnull);
	Alloc* a = new Alloc(alloc_id, t, index_var);
	Alloc* res= (Alloc*)FunctionTerm::get_term(a, true);
	if(is_nonnull) Constraint::set_gtz_attribute(res);
	return res;
}

Alloc* Alloc::make(const alloc_info& alloc_id, il::type* t, bool is_nonnull)
{
	Alloc* a = new Alloc(alloc_id, t);
	Alloc* res= (Alloc*)FunctionTerm::get_term(a, true);
	if(is_nonnull) Constraint::set_gtz_attribute(res);
	return res;
}




Alloc::Alloc(const alloc_info& alloc_id, il::type* t, AccessPath* index_var):
	FunctionTerm(CNode::get_varmap().get_id("alloc"),
			ConstantValue::make(alloc_id.get_id())->to_term(),
			index_var->to_term(),
			true, ATTRIB_ALLOC)
{

	this->alloc_id = alloc_id;
	this->t = t;
	this->index_var = index_var;
	has_index_var = true;
	this->specialization_type = AP_ALLOC;
	this->apt = AP_ALLOC;
	add_attribute();
}

Alloc::Alloc(const alloc_info& alloc_id, il::type* t):
	FunctionTerm(CNode::get_varmap().get_id("alloc"),
			ConstantValue::make(alloc_id.get_id())->to_term(),
			ConstantValue::make(0), true, ATTRIB_ALLOC)
{

	this->alloc_id = alloc_id;
	this->t = t;
	this->index_var = NULL;
	has_index_var = false;
	this->specialization_type = AP_ALLOC;
	this->apt = AP_ALLOC;
	add_attribute();
}

void Alloc::add_attribute()
{
	Constraint::set_geqz_attribute(this->to_term());
}

bool Alloc::is_nonnull()
{
	return (get_attribute() == TERM_ATTRIB_GTZ);
}



const alloc_info& Alloc::get_alloc_id()
{
	return alloc_id;
}

string Alloc::to_string()
{
	cout << endl <<"calling alloc::to_string() " << endl;
	if(PRINT_AS_TERM)
		return FunctionTerm::to_string();
	string res = "alloc";
	if(get_attribute() == TERM_ATTRIB_GTZ) {
		res += "+";
	}

	res += "(";
	res += alloc_id.get_original_allocation_site().to_string();
	res += "$" + int_to_string(alloc_id.get_id());
	cout << "callsite is :::::::::: " << res << ")"<<  endl;

	if(index_var != NULL) {
		res += ", " + index_var->to_string();
	}
	res += ")";
	return res;
}

IndexVariable* Alloc::get_index_var()
{
	if(index_var == NULL) return NULL;
	c_assert(index_var->get_ap_type() == AP_INDEX);
	return (IndexVariable*)index_var;
}

AccessPath* Alloc::get_index_expression()
{
	return index_var;
}


Term* Alloc::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	if(index_var == NULL) return this;
	Term* index_t = index_var->to_term()->substitute(subs);
	AccessPath* index_ap = AccessPath::to_ap(index_t);
	if(index_ap == index_var) return this;
	return Alloc::make(alloc_id, t, index_ap, is_nonnull());
}

Term* Alloc::substitute(Term* (*sub_func)(Term* term, void* data), void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}

	if(index_var == NULL) {
		return this;
	}

	Term* index_t = index_var->to_term()->substitute(sub_func, my_data);
	AccessPath* index_ap = AccessPath::to_ap(index_t);
	if(index_ap == index_var) return this;
	return Alloc::make(alloc_id, t, index_ap, is_nonnull());
}




Alloc::~Alloc() {
}

//-------------------------------------------------------------------

info_item::info_item(int line, int instruction_number,
		const Identifier& unit_id)
{
	this->line = line;
	this->instruction_number = instruction_number;
	this->function_name = unit_id.get_function_name();
	this->file = unit_id.get_file_id().to_string();
}

info_item::info_item()
{
	this->line = -1;
	this->instruction_number = -1;
}

string info_item::to_string() const
{
	return function_name + ":" + int_to_string(line);
}
bool info_item::operator==(const info_item& other) const
{
	if(this->line != other.line) return false;
	if(this->instruction_number != other.instruction_number) return false;
	if(this->function_name != other.function_name) return false;
	return this->file == other.file;
}

bool info_item::operator!=(const info_item& other) const
{
	return !(*this == other);
}

bool info_item::operator<(const info_item& other) const
{
	if(this->line < other.line) return true;
	if(this->line > other.line) return false;
	if(this->instruction_number < other.instruction_number) return true;
	if(this->instruction_number > other.instruction_number) return false;
	if(this->function_name < other.function_name) return true;
	if(this->function_name > other.function_name) return false;
	return this->file < other.file;
}

bool info_item::operator>(const info_item& other) const
{
	if(this->line > other.line) return true;
	if(this->line < other.line) return false;
	if(this->instruction_number > other.instruction_number) return true;
	if(this->instruction_number < other.instruction_number) return false;
	if(this->function_name > other.function_name) return true;
	if(this->function_name < other.function_name) return false;
	return this->file > other.file;
}

//--------------------------------------------------
map<alloc_info, int> alloc_info::info_ids;
int alloc_info::counter = 0;

alloc_info::alloc_info(int line, int instruction_number,
		const Identifier& unit_id)
{
	info_item ii(line, instruction_number, unit_id);
	callstack.push_back(ii);
}
alloc_info::alloc_info(const alloc_info& other)
{
	this->callstack = other.callstack;
}

alloc_info::alloc_info()
{

}
void alloc_info::push_caller(int line, int instruction_number,
		const Identifier& unit_id)
{
	callstack.push_back(info_item(line, instruction_number, unit_id));
}
const info_item& alloc_info::get_original_allocation_site() const
{
	assert(callstack.size() > 0);
	return callstack[0];
}
bool alloc_info::operator==(const alloc_info& other) const
{
	if(other.callstack.size() != this->callstack.size()) return false;
	vector<info_item>::const_iterator it1 = callstack.begin();
	vector<info_item>::const_iterator it2 = other.callstack.begin();
	for(; it1!=callstack.end(); it1++, it2++)
	{
		const info_item & ii1 = *it1;
		const info_item & ii2 = *it2;
		if(ii1!= ii2) return false;
	}
	return true;
}

bool alloc_info::operator!=(const alloc_info& other) const
{
	return !(*this == other);
}

bool alloc_info::operator<(const alloc_info& other) const
{
	if(this->callstack.size() < other.callstack.size()) return true;
	if(this->callstack.size() > other.callstack.size()) return false;
	vector<info_item>::const_iterator it1 = callstack.begin();
	vector<info_item>::const_iterator it2 = other.callstack.begin();
	for(; it1!=callstack.end(); it1++, it2++)
	{
		const info_item & ii1 = *it1;
		const info_item & ii2 = *it2;
		if(ii1 < ii2) return true;
		if(ii1 > ii2) return false;
	}

	return false;
}

int alloc_info::get_id() const
{
	if(info_ids.count(*this) > 0) {
		cout << "Found !! " << endl;
		cout << "*********get_id is " << info_ids[*this] << endl;
		return info_ids[*this];

	}
	int id = counter++;
	info_ids[*this] = id;

	cout << "not found, create a new one! " << endl;
	cout << "*********get_id is " << id << endl;
	return id;
}

void alloc_info::clear()
{
	info_ids.clear();
	counter = 0;
}


