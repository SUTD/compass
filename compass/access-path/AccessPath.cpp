/*
 * AccessPath.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "sail/Symbol.h"
#include "sail/Constant.h"
#include "sail/Variable.h"
#include "util.h"
#include "IndexVarManager.h"
#include "MemoryLocation.h"
#include "access-path.h"
#include "Constraint.h"
#include "ConstraintGenerator.h"
#include "sail/SummaryUnit.h"
#include "compass_assert.h"
#include "DisplayTag.h"

#define MAX_CONTEXTS 1
#define EXPLAIN_ERROR false


using namespace std;

Term* _make_ap(long int c)
{
	ConstantTerm* ct;
	ct = new ConstantValue(c);
	return ct;
}

Term* _make_ap(const string & name)
{

	return new Variable(name, il::get_integer_type(), ATTRIB_MISTRAL_VAR);
}

Term* _make_ap(const map<Term*, long int>& elems,long int constant)
{
	ArithmeticTerm* at;
	at = new ArithmeticValue(elems, constant, il::get_integer_type());
	return at;
}

Term* _make_ap(const string & name, vector<Term*>& args, bool invertible)
{
	FunctionValue* fv;
//	const vector<AccessPath*> & ap_args = (const vector<AccessPath*> &) args;

	//
	vector<AccessPath*> new_ap_vec;
	for(auto it = args.begin(); it != args.end(); it++) {
		new_ap_vec.push_back(AccessPath::to_ap(*it));
	}


	//



	fv = new FunctionValue(FN_UNINTERPRETED, name, new_ap_vec,
			il::get_integer_type(), invertible, 0);
	return fv;
}

AccessPath::AccessPath() {
	has_index_var = false;
	t = NULL;

}

Term* AccessPath::to_term()
{
	switch(this->apt)
	{
	case AP_CONSTANT:
		return (ConstantValue*) this;
	case AP_ADDRESS:
		return (Address*) this;
	case AP_ALLOC:
		return (Alloc*) this;
	case AP_ARITHMETIC:
		return (ArithmeticValue*) this;
	case AP_ARRAYREF:
		return (ArrayRef*) this;
	case AP_DEREF:
		return (Deref*) this;
	case AP_FIELD:
		return (FieldSelection*) this;
	case AP_FUNCTION:
		return (FunctionValue*) this;
	case AP_INDEX:
		return (IndexVariable*) this;
	case AP_UNMODELED:
		return (UnmodeledValue*) this;
	case AP_VARIABLE:
		return (Variable*) this;
	case AP_COUNTER:
		return (IterationCounter*) this;
	case AP_ADT:
		return (AbstractDataStructure*) this;
	case AP_STRING:
		return (StringLiteral*) this;
	case AP_PROGRAM_FUNCTION:
		return (ProgramFunction*) this;
	case AP_TYPE_CONSTANT:
			return (TypeConstant*) this;

	case AP_NIL:
		return (Nil*) this;
	default: {
		assert_context("Unexpected access path: " + to_string() );
		c_assert(false);
	}
	}
}

AccessPath* AccessPath::to_ap(Term* t)
{
	if(t == NULL) return NULL;
	if(!t->is_specialized()) {
		c_assert(false);
	}
	switch(t->get_specialization())
	{
	case AP_CONSTANT:
		return (ConstantValue*) t;
	case AP_ADDRESS:
		return (Address*) t;
	case AP_ALLOC:
		return (Alloc*) t;
	case AP_ARITHMETIC:
		return (ArithmeticValue*) t;
	case AP_ARRAYREF:
		return (ArrayRef*) t;
	case AP_DEREF:
		return (Deref*) t;
	case AP_FIELD:
		return (FieldSelection*) t;
	case AP_FUNCTION:
		return (FunctionValue*) t;
	case AP_INDEX:
		return (IndexVariable*) t;
	case AP_UNMODELED:
		return (UnmodeledValue*) t;
	case AP_VARIABLE:
		return (Variable*) t;
	case AP_COUNTER:
		return (IterationCounter*) t;
	case AP_ADT:
		return (AbstractDataStructure*) t;
	case AP_STRING:
		return (StringLiteral*) t;
	case AP_NIL:
		return (Nil*) t;
	case AP_PROGRAM_FUNCTION:
		return (ProgramFunction*) t;
	case AP_TYPE_CONSTANT:
		return (TypeConstant*) t;
	default: {
		c_assert(false);
	}
	}
}


string AccessPath::to_string()
{
	Term* t = this->to_term();
	return t->to_string();
}

AccessPath* AccessPath::get_inner()
{
	return NULL;
}

bool AccessPath::is_constant()
{
	return this->apt == AP_CONSTANT;
}

bool AccessPath::is_derived_from_constant()
{
	assert_context("Checking if access path is derived from constant: " +
			safe_string(this));
	switch(this->apt)
	{
	case AP_CONSTANT:
	case AP_NIL:
		return true;
	case AP_INDEX:
	case AP_UNMODELED:
	case AP_VARIABLE:
	case AP_ALLOC:
	case AP_ARITHMETIC:
	case AP_COUNTER:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
		return false;
	case AP_DEREF:
	case AP_ADDRESS:
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
	{
		return get_inner()->is_derived_from_constant();
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		const vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++) {
			if(!args[i]->is_derived_from_constant()) return false;
		}
		return true;
	}
	default:
		c_assert(false);


	}
}

bool AccessPath::is_length_function()
{
	if(this->get_ap_type() != AP_FUNCTION) return false;
	FunctionValue* fv = (FunctionValue*) this;
	return fv->is_length();
}

bool AccessPath::is_structured_ptr_arithmetic()
{
	return false;
}

AccessPath* AccessPath::get_base()
{
	switch(this->apt)
	{
	case AP_CONSTANT:
	case AP_INDEX:
	case AP_UNMODELED:
	case AP_ALLOC:
	case AP_VARIABLE:
	case AP_COUNTER:
	case AP_STRING:
	case AP_NIL:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
		return this;
	case AP_ADDRESS:
	case AP_DEREF:
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
	{
		return get_inner()->get_base();
	}
	// Base of an arithmetic term only makes sense
	// for pointer arithmetic
	case AP_ARITHMETIC:
	{
		AccessPath* inner = get_inner();
		if(inner == NULL) return NULL;
		return inner->get_base();



	}
	default:
		return NULL;

	}
}

AccessPath* AccessPath::set_base(AccessPath* new_base)
{
	switch(this->apt)
	{
	case AP_CONSTANT:
	case AP_INDEX:
	case AP_UNMODELED:
	case AP_ALLOC:
	case AP_VARIABLE:
	case AP_COUNTER:
	case AP_STRING:
	case AP_NIL:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
		return new_base;
	case AP_ADDRESS:
	{
		return Address::make(new_base);
	}
	case AP_DEREF:
	{
		return Deref::make(get_inner()->set_base(new_base), this->t);
	}
	case AP_FIELD:
	{
		FieldSelection* fs = (FieldSelection*) this;
		return FieldSelection::make(fs->get_field_name(), fs->get_field_offset(),
				t,  get_inner()->set_base(new_base),
				fs->get_index_expression());
	}
	case AP_ARRAYREF:
	{
		ArrayRef* ar = (ArrayRef*) this;
		return ArrayRef::make(ar->get_inner()->set_base(new_base),
				ar->get_index_expression(), t, ar->get_elem_size());
	}
	case AP_ADT:
	{
		AbstractDataStructure* adt = (AbstractDataStructure*) this;
		AccessPath* new_inner = adt->get_inner()->set_base(new_base);
		return AbstractDataStructure::make(new_inner, adt->get_index_expression(),
				adt->get_key_type(), adt->get_value_type(), t);
	}
	// Base of an arithmetic term only makes sense
	// for pointer arithmetic
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		AccessPath* inner = get_inner();
		if(inner == NULL) return this;
		AccessPath* new_inner = inner->set_base(new_base);
		return replace(inner, new_inner);

	}
	default:
		return this;

	}

}

bool AccessPath::is_sail_temporary()
{
	switch(this->apt)
	{
	case AP_VARIABLE:
	{
		Variable* v =(Variable*) this;
		sail::Variable* sail_var = v->get_original_var();
		if(sail_var == NULL) return false;
		return sail_var->is_temp();
	}
	case AP_DEREF:
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
	{
		return get_inner()->is_sail_temporary();
	}
	default:
		return false;

	}
}

/*
 * The representative of an access path does not contain any direct field
 * selectors. e.g, rep(x.f) = x, rep(x->f) = *x, rep (x.f[i]) = x and so on.
 */
AccessPath* AccessPath::find_representative()
{
	assert_context("Finding access path representative: " + safe_string(this));
	AccessPath* ap = this;
	while(true)
	{
		if(ap->get_ap_type() != AP_FIELD) {
			// We could have a field which is an array, so we need to
			// keep going if that's the case
			if(ap->get_ap_type() == AP_ARRAYREF){
				ArrayRef* are = (ArrayRef*) ap;
				AccessPath* inner = are->get_inner();
				if(inner->get_ap_type() == AP_FIELD)
					ap = inner;
				else return ap;
			}
			else if(ap->get_ap_type() == AP_ADT){
				AbstractDataStructure* are = (AbstractDataStructure*) ap;
				AccessPath* inner = are->get_inner();
				if(inner->get_ap_type() == AP_FIELD)
					ap = inner;
				else return ap;
			}


			else return ap;
		}
		FieldSelection* fs = (FieldSelection*) ap;
		if(fs->get_index_var() != NULL) return fs;
		ap = fs->get_inner();
	}
	c_assert(false);
}


AccessPath* AccessPath::push_address_through()
{
	AccessPath* ap =this;
	if(ap->get_ap_type()!= AP_ADDRESS) return ap;
	Address* addr = (Address*)ap;
	AccessPath* inner = addr->get_inner();
	return push_address_through_rec(inner);

}




AccessPath* AccessPath::push_address_through_rec(AccessPath* inner)
{

	if(inner->get_ap_type() == AP_DEREF) {
		Deref* d = (Deref*) inner;
		return d->get_inner();
	}
	if(inner->get_ap_type() == AP_FIELD || inner->get_ap_type() == AP_ARRAYREF)
	{
		map<Term*, long int> terms;
		int constant = 0;
		while(inner->get_ap_type() == AP_FIELD ||
				inner->get_ap_type() == AP_ARRAYREF)
		{
			if(inner->get_ap_type() == AP_FIELD) {
				FieldSelection* fs = (FieldSelection*) inner;
				constant += fs->get_field_offset();
				inner = fs->get_inner();
			}
			else if(inner->get_ap_type() == AP_ARRAYREF) {
				ArrayRef* ar = (ArrayRef*) inner;
				int coef = ar->get_inner()->get_type()->get_size()/8;
				terms[ar->get_index_var()] = coef;
				inner = ar->get_inner();
			}
		}
		AccessPath* new_inner = push_address_through_rec(inner);
		terms[new_inner->to_term()] = 1;
		return ArithmeticValue::make(terms, constant);
	}


	return Address::make(inner);



}

/*
 * base means it doesn't contain any derefs, field selectors, or
 * array references.
 */
bool AccessPath::is_base()
{
	ap_type t = get_ap_type();
	return (t != AP_DEREF && t!=AP_FIELD && t!=AP_ARRAYREF && t!=AP_ARITHMETIC
			&& t!= AP_FUNCTION && t!= AP_ADDRESS && t!= AP_ADT);
}

AccessPath* AccessPath::multiply_ap(long int constant)
{
	return ArithmeticValue::make_times(this, ConstantValue::make(constant));
}
AccessPath* AccessPath::divide_ap(long int constant)
{
	if(this->apt == AP_CONSTANT)
	{
		ConstantValue* cv = (ConstantValue*) this;
		return ConstantValue::make(cv->get_constant()/constant);
	}
	else if(this->apt == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*) this;

		long int gcd = av->get_gcd(true);
		if(gcd % constant == 0)
		{
			const map<Term*, long int>& elems = av->get_elems();
			map<Term*, long int> new_elems;
			map<Term*, long int>::const_iterator it = elems.begin();
			for(; it!= elems.end(); it++)
			{
				new_elems[it->first] = it->second/constant;
			}

			return ArithmeticValue::make(new_elems, av->get_constant()/constant);

		}
		DisplayTag dt;
		return UnmodeledValue::make_imprecise(il::get_integer_type(), dt);

	}
	DisplayTag dt;
	return UnmodeledValue::make_imprecise(il::get_integer_type(), dt);

}
AccessPath* AccessPath::add_ap(AccessPath* ap)
{
	return ArithmeticValue::make_plus(this, ap);
}
AccessPath* AccessPath::subtract_ap(AccessPath* ap)
{
	return ArithmeticValue::make_minus(this, ap);
}

void AccessPath::add_signedness_attribute()
{

	il::type* t = this->t;
	if(!t->is_base_or_enum_type()) return;
	while(t->is_array_type()) {
		t = t->get_inner_type();
	}

	// TODO: Fix me
	if(t->is_pointer_type()) return;

	if(!t->is_signed_type())
	{
		Constraint::set_geqz_attribute(this->to_term());
	}
}

bool AccessPath::is_recursive()
{
	if(apt != AP_FIELD) return false;
	FieldSelection* fs = (FieldSelection*) this;
	return fs->get_index_var() != NULL;
}

bool AccessPath::maybe_entry_aliased(bool is_loop)
{
	if(!get_type()->is_pointer_type()) return false;

	switch(this->get_ap_type())
	{
		case AP_VARIABLE:
		{
			Variable* v= (Variable*) this;
			if(v->is_argument()) return true;
			if(v->is_global_var()) return true;
			if(is_loop && v->is_local_var()) {
				if(v->get_original_var() == NULL) return false;
				if(v->is_sail_temporary()) return false;
				if(v->is_summary_temp()) return false;
				if(v->is_disjointness_var()) return false;
				if(v->is_background_target_var()) return false;
				if(v->is_anonymous()) return false;
				if(v->is_loop_error_temp()) return false;
				return true;
			}
			return false;
		}
		case AP_DEREF:
		case AP_FIELD:
		case AP_ARRAYREF:
		case AP_ADT:
		{
			return get_inner()->maybe_entry_aliased(is_loop);
		}
		case AP_ARITHMETIC:
		{
			ArithmeticValue* av = (ArithmeticValue*) this;
			AccessPath* base_ptr = av->get_base_ptr();
			if(base_ptr == NULL) return false;
			return base_ptr->maybe_entry_aliased(is_loop);
		}

		default:
			return false;
	}
}

/*
 * Does this access path correspond to a location that is
 * reachable to callers of a given function?
 * This only makes sense for memory access paths.
 */
bool AccessPath::is_interface_object(sail::SummaryUnit* su)
{

	assert_context("Checking if access path " + safe_string(this) +
			"is interface object in unit: " + su->get_identifier().to_string());
	switch(this->get_ap_type())
	{
		case AP_ADDRESS:
		case AP_ALLOC:
		case AP_CONSTANT:
		case AP_ARITHMETIC:
		case AP_FUNCTION:
		case AP_UNMODELED:
		case AP_INDEX:
		case AP_COUNTER:
		case AP_STRING:
		case AP_NIL:
		case AP_PROGRAM_FUNCTION:
		case AP_TYPE_CONSTANT:
			return false;
		case AP_VARIABLE:
		{
			Variable* v = (Variable*) this;
			if(su->is_function() && v->is_anonymous()) {
				return false;
			}
			if(v->is_global_var()) return true;
			if(v->is_return_var()) return true;
			if(v->get_original_var() == NULL){
				return false;
			}
			if(su->is_superblock())
			{
				if(v->is_loop_exit_var()) return true;
				if(v->is_argument()) return true;
				if(v->is_sail_temporary()) {
					return false;
				}
				bool res = !su->is_local(v->get_original_var());
				return res;
			}
			else return false;
		}
		case AP_DEREF:
		{
			Deref* d = (Deref*) this;
			AccessPath* inner = d->get_inner();
			if(inner->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) inner;
				if(v->is_argument()) return true;
			}
			return inner->is_interface_object(su);

		}
		case AP_ARRAYREF:
		case AP_ADT:
		case AP_FIELD:
		{
			AccessPath* inner = this->get_inner();
			c_assert(inner != NULL);
			return inner->is_interface_object(su);
		}
		default:
		c_assert(false);
	}

}


bool AccessPath::is_size_field_ap()
{
	ap_type t = this->get_ap_type();
	if(t!=AP_FIELD) {
		return false;
	}
	FieldSelection* fs = (FieldSelection*) this;
	return fs->is_size_field();
}


bool AccessPath::is_rtti_field()
{
	ap_type t = this->get_ap_type();
	if(t!=AP_FIELD) {
		return false;
	}
	FieldSelection* fs = (FieldSelection*) this;
	return fs->is_rtti_field();
}

bool AccessPath::is_deleted_field()
{
	ap_type t = this->get_ap_type();
	if(t!=AP_FIELD) {
		return false;
	}
	FieldSelection* fs = (FieldSelection*) this;
	return fs->is_deleted_field();
}


bool AccessPath::is_fake_field()
{
	ap_type t = this->get_ap_type();
	if(t!=AP_FIELD) {
		return false;
	}
	FieldSelection* fs = (FieldSelection*) this;
	return fs->is_rtti_field() || fs->is_size_field() || fs->is_deleted_field();
}


IndexVariable* AccessPath::get_index_var()
{
	return NULL;
}


AccessPath* AccessPath::get_index_expression()
{
	return NULL;
}

bool AccessPath::contains_index_var()
{
	return has_index_var;
}

bool AccessPath::contains_index_var(IndexVariable* var)
{
	AccessPath* iv = this->get_index_expression();
	if(iv != NULL && iv == var) return true;
	AccessPath* inner =  this->get_inner();
	if(inner != NULL && inner->contains_index_var(var)) return true;

	if(apt == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++){
			AccessPath* arg = args[i];
			if(arg->contains_index_var(var)) return true;
		}
		return false;
	}

	else if(apt == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int> & elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap == NULL) continue;
			if(ap->contains_index_var(var)) return true;
		}
		return false;
	}

	return false;
}


AccessPath* AccessPath::get_ap_from_symbol(sail::Symbol* s)
{


	if(s->is_constant()){
		sail::Constant* c = (sail::Constant*) s;
		if(c->is_integer())
			return ConstantValue::make(c->get_integer());
		else {
			DisplayTag dt;
			return UnmodeledValue::make_imprecise(s->get_type(), dt);
		}
	}
	sail::Variable* v = (sail::Variable*)s;

	return Variable::make_program_var(v);
}



IndexVariable* AccessPath::find_outermost_index_var(bool stop_at_deref)
{
	AccessPath* ap = this;
	if(ap->get_ap_type() == AP_ARRAYREF){
		ArrayRef* ar = (ArrayRef*) ap;
		return ar->get_index_var();
	}

	if(ap->get_ap_type() == AP_ADT) {
		return ap->get_index_var();
	}

	if(ap->get_ap_type() == AP_ALLOC){
		Alloc* alloc = (Alloc*) ap;
		return alloc->get_index_var();
	}

	if(ap->get_ap_type() == AP_UNMODELED) {
		UnmodeledValue* uv = (UnmodeledValue*) ap;
		return uv->get_index_var();
	}

	if(ap->get_ap_type() == AP_FIELD)
	{
		FieldSelection* fs = (FieldSelection*) ap;
		if(fs->get_index_var() != NULL) return fs->get_index_var();
		return fs->get_inner()->find_outermost_index_var(stop_at_deref);
	}
	if(!stop_at_deref && ap->get_ap_type() == AP_DEREF)
	{
		return ap->get_inner()->find_outermost_index_var(false);
	}
	return NULL;
}

void AccessPath::get_nested_index_vars(vector<IndexVariable*>& vars)
{
	AccessPath* iv = this->get_index_expression();
	if(iv != NULL && iv->get_ap_type() == AP_INDEX) {
		vars.push_back((IndexVariable*)iv);
	}
	AccessPath* inner =  this->get_inner();
	if(inner != NULL) inner->get_nested_index_vars(vars);

	if(apt == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++){
			AccessPath* arg = args[i];
			arg->get_nested_index_vars(vars);
		}
	}

	else if(apt == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int> & elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap == NULL) continue;
			if(ap->get_ap_type() == AP_INDEX) {
				vars.push_back((IndexVariable*) ap);
			}
			else ap->get_nested_index_vars(vars);
		}
	}
}
void AccessPath::get_nested_index_vars(set<IndexVariable*>& vars,
		bool include_fn_terms)
{
	if(this->get_ap_type() == AP_INDEX) vars.insert((IndexVariable*)this);

	AccessPath* iv = this->get_index_expression();
	if(iv != NULL && iv->get_ap_type() == AP_INDEX) {
		vars.insert((IndexVariable*)iv);
	}
	AccessPath* inner =  this->get_inner();
	if(include_fn_terms && inner != NULL) inner->get_nested_index_vars(vars,
			include_fn_terms);

	if(include_fn_terms && apt == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++){
			AccessPath* arg = args[i];
			arg->get_nested_index_vars(vars, include_fn_terms);
		}
	}

	else if(apt == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int> & elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap == NULL) continue;
			if(ap->get_ap_type() == AP_INDEX) {
				vars.insert((IndexVariable*) ap);
			}
			else ap->get_nested_index_vars(vars, include_fn_terms);
		}
	}
}

bool AccessPath::contains_ap_type(ap_type apt)
{
	assert_context("Checkign if access path " + safe_string(this)+
			" contains ap type: " + int_to_string(apt)) ;
	if(this->apt == apt) return true;
	switch(this->apt)
	{
	case AP_CONSTANT:
	case AP_NIL:
	case AP_VARIABLE:
	case AP_UNMODELED:
	case AP_INDEX:
	case AP_COUNTER:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
		return false;
	case AP_ADDRESS:
	case AP_DEREF:
	{
		AccessPath* inner = get_inner();
		return inner->contains_ap_type(apt);
	}
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ALLOC:
	case AP_ADT:
	{
		AccessPath* inner = get_inner();
		if(inner->contains_ap_type(apt)) return true;
		if(get_index_expression() == NULL) return false;
		return get_index_expression()->contains_ap_type(apt);
	}


	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++)
		{
			AccessPath* new_arg = args[i];
			if(new_arg->contains_ap_type(apt)) return true;
		}
		return false;
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;

		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* ap = to_ap(t);
			if(ap->contains_ap_type(apt)) return true;

		}
		return false;



	}
	default:
		c_assert(false);


	}
}

AccessPath* AccessPath::replace(map<AccessPath*, AccessPath*>& replacements)
{
	assert_context("Applying replacement to " + safe_string(this));
	if(replacements.count(this) > 0) return replacements[this];
	AccessPath* inner = this->get_inner();
	AccessPath* new_inner = NULL;
	if(inner != NULL)
		new_inner = inner->replace(replacements);

	AccessPath* index = this->get_index_expression();
	AccessPath* new_index = NULL;
	if(index != NULL) {
		AccessPath* res = index->replace(replacements);
		new_index =  res;
	}

	switch(apt)
	{
	case AP_CONSTANT:
	case AP_VARIABLE:
	case AP_INDEX:
	case AP_COUNTER:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
	case AP_NIL:
		return this;
	case AP_ADDRESS:
	{
		if(inner != new_inner) return Address::make(new_inner);
		return this;
	}
	case AP_DEREF:
	{
		if(inner != new_inner) return Deref::make(new_inner);
		return this;
	}
	case AP_FIELD:
	{
		if(inner == new_inner && index == new_index) return this;
		FieldSelection* fs = (FieldSelection*) this;
		return FieldSelection::make(fs->get_field_name(), fs->get_field_offset(),
				fs->get_type(), new_inner, new_index);
	}
	case AP_ARRAYREF:
	{
		if(inner == new_inner && index == new_index) return this;
		ArrayRef* a = (ArrayRef*) this;
		return ArrayRef::make(new_inner, new_index, a->get_type(),
				a->get_elem_size());
	}
	case AP_ADT:
	{
		if(inner == new_inner && index == new_index) return this;
		AbstractDataStructure* a = (AbstractDataStructure*) this;
		return AbstractDataStructure::make(new_inner, new_index,
				a->get_key_type(), a->get_value_type(), a->get_type());
	}
	case AP_ALLOC:
	{
		if(index == new_index) return this;
		Alloc* a = (Alloc*) this;
		return Alloc::make(a->get_alloc_id(), a->get_type(), new_index,
				a->is_nonnull());
	}
	case AP_UNMODELED:
	{
		if(index == new_index) return this;
		UnmodeledValue* uv = (UnmodeledValue*) this;
		AccessPath* res = UnmodeledValue::make(uv->get_unmodeled_id(),
				uv->get_type(), new_index, uv->get_kind(),
				uv->get_display_tag());
		return res;
	}

	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		bool changed = false;
		vector<AccessPath*> new_args;
		for(unsigned int i=0; i<args.size(); i++)
		{
			AccessPath* new_arg = args[i]->replace(replacements);
			new_args.push_back(new_arg);
			if(args[i] != new_arg){
				changed = true;
			}
		}

		if(!changed) return this;
		return FunctionValue::make(fv->get_fun_type(), new_args,
				fv->get_attribute());
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		if(!av->is_structured_ptr_arithmetic())
		{
			const map<Term*, long int>& elems = av->get_elems();
			map<Term*, long int> new_elems;
			bool changed = false;
			map<Term*, long int>::const_iterator it = elems.begin();
			for(; it!= elems.end(); it++)
			{
				Term* t = it->first;
				AccessPath* ap = to_ap(t);
				if(ap == NULL) {
					new_elems[t] += it->second;
					continue;
				}
				AccessPath* new_ap = ap->replace(replacements);
				if(new_ap != ap) changed = true;
				new_elems[new_ap->to_term()] += it->second;

			}
			if(!changed) return this;
			return ArithmeticValue::make(new_elems, av->get_constant());
		}

		else {
			bool changed = false;
			AccessPath* base_ptr = av->get_base_ptr();
			const vector<offset>& offsets = av->get_offsets();

			AccessPath* new_base_ptr = base_ptr->replace(replacements);
			if(new_base_ptr != base_ptr) changed = true;
			vector<offset> new_offsets;

			for(unsigned int i=0; i<offsets.size(); i++)
			{
				offset o = offsets[i];
				if(o.is_field_offset()) {
					new_offsets.push_back(o);
					continue;
				}
				AccessPath* index = o.index;
				AccessPath* new_index = index->replace(replacements);
				if(new_index != index) changed = true;
				new_offsets.push_back(offset(o.elem_size, new_index));

			}

			if(!changed) return this;
			AccessPath* new_av = ArithmeticValue::make_structured_pointer_arithmetic
					(new_base_ptr, new_offsets);
			return new_av;
		}

	}
	default:
		c_assert(false);


	}
}


AccessPath* AccessPath::replace(AccessPath* ap, AccessPath* replacement)
{
	map<AccessPath*, AccessPath*> replacements;
	replacements[ap] = replacement;
	return replace(replacements);
}

AccessPath* AccessPath::strip_deref()
{
	AccessPath* res = strip_deref_rec(true);
	return res;
}

AccessPath* AccessPath::strip_deref_rec(bool first)
{

	AccessPath* ap = this;
	if(ap->get_ap_type() == AP_DEREF){
		AccessPath* res = ((Deref*)ap)->get_inner();
		return res;
	}
	if(ap->get_ap_type() == AP_FIELD) {
		if(!ap->is_recursive()){
			FieldSelection* fs = (FieldSelection*) ap;
			AccessPath* inner = fs->get_inner()->strip_deref_rec(false);
			if(fs->get_field_offset() == 0) return inner;
			AccessPath* res = NULL;
			if(first){
				res = ArithmeticValue::make_field_offset_plus(inner,
						fs->get_field_name(), fs->get_field_offset(),
								fs->get_type());
			}
			else {
				res = inner;
			}

			return res;
		}
		else {
			return ap;
		}
	}
	if(ap->get_ap_type() == AP_ARRAYREF) {
		ArrayRef* ar = (ArrayRef*) ap;
		AccessPath* inner = ar->get_inner()->strip_deref_rec(true);
		int elem_size = ar->get_elem_size();
		AccessPath* res = ArithmeticValue::make_index_offset_plus(inner,
				elem_size, ar->get_index_expression(), inner->get_type());


		return res;

	}

	if(ap->get_ap_type() == AP_ADT) {
		AbstractDataStructure* ads = (AbstractDataStructure*) ap;
		AccessPath* inner = ads->get_inner()->strip_deref_rec(true);
		AccessPath* res = ArithmeticValue::make_index_offset_plus(inner,
				1, ads->get_index_expression(), inner->get_type());
		return res;
	}

	return Address::make(ap);

}


AccessPath* AccessPath::add_deref()
{
	AccessPath* ap = this;
	if(ap->get_ap_type() == AP_ADDRESS){
		Address* adr = (Address*) ap;
		AccessPath* inner = adr->get_inner();
		while(inner->get_type()->is_record_type() ){
			il::record_type* rt = (il::record_type*) inner->get_type();
			if(rt->get_field_from_offset(0) == NULL) break;
			string fname = rt->get_field_from_offset(0)->fname;
			il::type* t = rt->get_field_from_offset(0)->t;
			FieldSelection* fs = FieldSelection::make(fname,
					0, t, inner);
			inner = fs;
		}

		AccessPath* res = inner;
		return res;
	}

	if(ap->get_ap_type() == AP_ARITHMETIC)
	{

		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(av->is_structured_ptr_arithmetic())
		{
			ap = av->get_base_ptr()->add_deref();
			while(ap->get_ap_type() == AP_FIELD)
				ap = ap->get_inner();
			vector<offset> & offsets = (vector<offset> &)av->get_offsets();
			vector<offset>::iterator it = offsets.begin();
			for(; it != offsets.end(); it++)
			{
				offset & cur= *it;
				if(cur.is_field_offset())
				{
					int fo = cur.field_offset;
					il::type* t = cur.field_type;
					const string& fn = cur.field_name;
					ap = FieldSelection::make(fn, fo,t, ap);
				}
				else {
					int s = cur.elem_size;
					AccessPath* ind = cur.index;
					ap = ArrayRef::make(ap, ind, ap->get_type(), s);
				}
			}
			return ap;
		}
	}

	return Deref::make(ap);
}

/*
 * Is this access path part of an array? This is the case if there
 * is an array bracket before a dereference.
 * For example, a[i], a[i].f, a[i].f.g belong to an array, but not
 * (*a[i]).f.
 */
bool AccessPath::belongs_to_array()
{
	AccessPath * cur = this;
	while(cur->get_ap_type() != AP_DEREF){
		if(cur->get_ap_type() == AP_ARRAYREF) {
			return true;
		}
		cur = cur->get_inner();
		if(cur == NULL){
			return false;
		}
	}
	return false;
}


AccessPath* AccessPath::make_arithmetic_ap(AccessPath* ap1, AccessPath* ap2,
		il::binop_type bt)
{
	assert_context("Making arithmetic ap: " + safe_string(ap1) +
			il::binop_expression::binop_to_string(bt) + safe_string(ap2));
	if(bt == il::_MOD)
	{
		if(!ap2->is_constant())
		{
			return NULL;
		}
		AccessPath* mod_ap = FunctionValue::make(FN_MOD, ap1, ap2);
		return mod_ap;

	}

	/*
	 * We first check if the multiplication results in a
	 * linear constraint.
	 */
	if(bt == il::_MULTIPLY)
	{
		return ArithmeticValue::make_times(ap1, ap2);
	}
	if(bt == il::_DIV){
		return ArithmeticValue::make_division(ap1, ap2);
	}
	if(bt == il::_PLUS)
	{
		return ArithmeticValue::make_plus(ap1, ap2);
	}
	if(bt == il::_MINUS)
	{
		return ArithmeticValue::make_minus(ap1, ap2);
	}

	c_assert(false);
}

/*
 * Gives back all nested access paths
 */
void AccessPath::get_nested_aps(set<AccessPath*>& all_aps)
{
	assert_context("Getting nested access paths: " + safe_string(this));
	all_aps.insert(this);
	switch(this->get_ap_type())
	{

	case AP_CONSTANT:
	case AP_STRING:
	case AP_VARIABLE:
	case AP_INDEX:
	case AP_COUNTER:
	case AP_NIL:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
	{
		return;
	}

	case AP_DEREF:
	{
		get_inner()->get_nested_aps(all_aps);
		return;
	}

	case AP_ADDRESS:
	case AP_ALLOC:
	case AP_ARRAYREF:
	case AP_ADT:
	case AP_FIELD:
	case AP_UNMODELED:
	{
		get_inner()->get_nested_aps(all_aps);
		AccessPath* index = get_index_expression();
		if(index != NULL) {
			index->get_nested_aps(all_aps);
		}
		return;
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* cur = to_ap(t);
			if(cur == NULL) continue;
			cur->get_nested_aps(all_aps);
		}
		return;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++)
		{
			args[i]->get_nested_aps(all_aps);
		}
		return;
	}
	default:
		c_assert(false);

	}
}


/*
 * Gives back all the memory ap's nested inside an access path.
 */
void AccessPath::get_nested_memory_aps(set<AccessPath*>& mem_aps)
{
	assert_context("Getting nested memory access paths: " + safe_string(this));
	switch(this->get_ap_type())
	{
	case AP_ADDRESS:
	case AP_ALLOC:
	case AP_VARIABLE:
	case AP_ARRAYREF:
	case AP_ADT:
	case AP_FIELD:
	case AP_UNMODELED:
	{
		mem_aps.insert(this);
		return;
	}
	case AP_CONSTANT:
	case AP_INDEX:
	case AP_COUNTER:
	case AP_STRING:
	case AP_NIL:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
	{
		return;
	}
	case AP_DEREF:
	{
		Deref* d = (Deref*) this;
		AccessPath* inner = d->get_inner();
		ap_type apt = inner->get_ap_type();
		if(apt == AP_ADDRESS || apt == AP_ALLOC || apt == AP_VARIABLE ||
				apt == AP_ARRAYREF || apt == AP_FIELD || apt == AP_DEREF)
		{
			mem_aps.insert(this);
			return;
		}
		inner->get_nested_memory_aps(mem_aps);
		return;
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* cur = to_ap(t);
			if(cur == NULL) continue;
			cur->get_nested_memory_aps(mem_aps);
		}
		return;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++)
		{
			args[i]->get_nested_memory_aps(mem_aps);
		}
		return;
	}
	default:
		c_assert(false);

	}
}

bool AccessPath::is_unobservable(bool is_loop)
{

	switch(get_ap_type())
	{
	case AP_CONSTANT:
	case AP_INDEX:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_NIL:
	case AP_TYPE_CONSTANT:
		return false;
	case AP_VARIABLE:
	{
		Variable* v = (Variable*) this;
		if(!is_loop && v->is_anonymous()) return true;
		if(!is_loop && v->is_local_var()) return true;
		else if(v->is_disjointness_var()) return true;

		/*else if(!is_loop && v->is_iteration_counter()) return true;
		// TODO: FIXME!!!!!
		else if(v->is_loop_termination_var())
		{
			string name = v->get_name();
			int num_colons = 0;
			for(unsigned int i=0; i < name.size(); i++) {
				if(name[i] == ':') {
					num_colons++;
					if(num_colons>=TERMINATION_VAR_PROPAGATE_DEPTH){
						return true;
					}
				}
			}
			return false;
		}*/
		else if(!is_loop && v->is_loop_exit_var()) return true;
		return false;
	}
	case AP_COUNTER:
	{
		IterationCounter* ic = (IterationCounter*) this;
		return ic->get_num_contexts() >= MAX_CONTEXTS;
	}
	case AP_ALLOC:
		return !is_loop;
	case AP_UNMODELED:
	{
		if(EXPLAIN_ERROR) {
			if(to_string().find("@")!=string::npos) return false;
		}
		return true;
	}

	case AP_DEREF:
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
	case AP_ADDRESS:
	{
		return get_inner()->is_unobservable(is_loop);
	}
	default:
		return false;
	}


}



/*
 * Returns the set of access paths corresponding to unobservable terms,
 * such as environment choices, variables that go out of scope,
 * and imprecision in the analysis
 */
void AccessPath::get_unobservable_aps(set<AccessPath*>& unobservables,
		bool is_loop, set<AccessPath*>& excluded_aps, bool eliminate_counters)
{
	assert_context("Getting unobservable access paths from "
			+ safe_string(this));
	switch(get_ap_type())
	{
		case AP_ALLOC:
		case AP_UNMODELED: {
			if(excluded_aps.count(this) > 0) {
				return;
			}
			if(is_unobservable(is_loop)) unobservables.insert(this);
			return;
		}
		case AP_COUNTER:
			if(!eliminate_counters) return;
			if(is_unobservable(is_loop)) unobservables.insert(this);
			return;
		case AP_CONSTANT:
		case AP_VARIABLE:
		case AP_DEREF:
		case AP_FIELD:
		case AP_ARRAYREF:
		case AP_ADDRESS:
		case AP_ADT:
		case AP_INDEX:
		case AP_STRING:
		case AP_PROGRAM_FUNCTION:
		case AP_TYPE_CONSTANT:
		case AP_NIL:
		{
			if(to_string() == "LIB_HAS_EXCEPTION") unobservables.insert(this);
			if(to_string() == "CUR_EXCEPTIONS") unobservables.insert(this);
			if(is_unobservable(is_loop)) unobservables.insert(this);
			return;
		}


		case AP_ARITHMETIC:
		{
			ArithmeticValue* av = (ArithmeticValue*) this;
			const map<Term*, long int>& elems = av->get_elems();
			map<Term*, long int>::const_iterator it = elems.begin();
			for(; it!= elems.end(); it++)
			{
				Term* t = it->first;
				AccessPath* ap = to_ap(t);
				if(ap == NULL) continue;
				ap->get_unobservable_aps(unobservables, is_loop, excluded_aps,
						eliminate_counters);
			}
			return;
		}

		case AP_FUNCTION:
		{
			FunctionValue* fv = (FunctionValue*) this;
			vector<AccessPath*>& args = fv->get_arguments();
			for(unsigned int i=0; i<args.size(); i++)
			{
				args[i]->get_unobservable_aps(unobservables, is_loop,
						excluded_aps, eliminate_counters);
			}
			return;
		}
		default:
			c_assert(false);
	}
}

int AccessPath::get_num_derefs()
{
	int num_derefs = 0;
	AccessPath* inner = this;
	while(true)
	{
		if(inner->get_inner() == NULL) break;
		if(inner->get_ap_type() == AP_ARITHMETIC) break;
		if(inner->get_ap_type() == AP_DEREF) num_derefs++;
		inner = inner->get_inner();
	}
	return num_derefs;
}


/*
 * Does this access path contain any of the access paths
 * in the aps set?
 */
bool AccessPath::contains_nested_access_path(set<AccessPath*>& aps)
{
	assert_context("Checking for access path containment on " +
			safe_string(this));
	if(aps.count(this) > 0) return true;
	switch(this->get_ap_type())
	{
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		const map<Term*, long int>& terms = av->get_elems();
		map<Term*, long int>::const_iterator it = terms.begin();
		for(; it!= terms.end(); it++)
		{
			Term* t = it->first;
			AccessPath* ap = to_ap(t);
			if(ap == NULL) continue;
			if(ap->contains_nested_access_path(aps)) return true;
		}

		return false;
	}
	case AP_ARRAYREF:
	case AP_ADT:
	{
		AccessPath* inner = this->get_inner();
		return inner->contains_nested_access_path(aps);
	}
	case AP_CONSTANT:
	case AP_VARIABLE:
	case AP_ALLOC:
	case AP_UNMODELED:
	case AP_INDEX:
	case AP_COUNTER:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
	case AP_NIL:
	{
		return false;
	}
	case AP_DEREF:
	{
		Deref* d = (Deref*) this;
		AccessPath* inner = d->get_inner();
		return inner->contains_nested_access_path(aps);
	}
	case AP_FIELD:
	{
		FieldSelection* fs = (FieldSelection*) this;
		AccessPath* inner = fs->get_inner();
		return inner->contains_nested_access_path(aps);
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*> & args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++)
		{
			if(args[i]->contains_nested_access_path(aps)) return true;
		}
		return false;
	}
	case AP_ADDRESS:
	{
		Address* adr = (Address*) this;
		AccessPath* inner = adr->get_inner();
		return inner->contains_nested_access_path(aps);
	}
	default:
		c_assert(false);
	}
}

/*
 * Is the given access path nested inside this one?
 */
bool AccessPath::contains_nested_access_path(AccessPath* ap)
{
	set<AccessPath*> aps;
	aps.insert(ap);
	return contains_nested_access_path(aps);
}






AccessPath* AccessPath::make_access_path_from_binop(
		AccessPath* ap1, AccessPath* ap2, il::binop_type bt)
{
	assert_context("Makign access path from binop: " + safe_string(ap1)
			+ il::binop_expression::binop_to_string(bt) + safe_string(ap2));
	c_assert(!il::binop_expression::is_predicate_binop(bt));
	switch(bt)
	{
		case il::_PLUS:
		case il::_MINUS:
		case il::_MULTIPLY:
		case il::_DIV:
		case il::_MOD:
		{
			return AccessPath::make_arithmetic_ap(ap1, ap2, bt);
		}
		case il::_LEFT_SHIFT:
		{
			return FunctionValue::make(FN_LEFT_SHIFT, ap1, ap2);
		}
		case il::_RIGHT_SHIFT:
		{
			return FunctionValue::make(FN_RIGHT_SHIFT, ap1, ap2);
		}
		case il::_BITWISE_AND:
		{
			return FunctionValue::make(FN_BITWISE_AND, ap1, ap2);
		}
		case il::_BITWISE_OR:
		{
			return FunctionValue::make(FN_BITWISE_OR, ap1, ap2);
		}
		case il::_BITWISE_XOR:
		{
			return FunctionValue::make(FN_BITWISE_XOR, ap1, ap2);
		}

		default:
		{
			return NULL;
		}

	}
}

void AccessPath::update_type(il::type* new_t)
{
	il::type* inner_t = t;
	while(true) {
		il::type* temp = inner_t->get_inner_type();
		if(temp == NULL) break;
		inner_t = temp;
	}

	if(!inner_t->is_void_type()) return;
	this->t = new_t;

	if(apt == AP_DEREF) {
		Deref* d = (Deref*) this;
		AccessPath* inner = d->get_inner();
		il::type* inner_t = il::pointer_type::make(new_t, "");
		inner->update_type(inner_t);
	}

}

bool AccessPath::is_imprecise()
{
	switch(this->get_ap_type())
	{
	case AP_UNMODELED:
		return true;
	case AP_DEREF:
	case AP_FIELD:
	case AP_ADDRESS:
	case AP_ARRAYREF:
	case AP_ADT:
		return get_inner()->is_imprecise();
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		const map<Term*, long int> & elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++) {
			Term* t = it->first;
			if(!t->is_specialized()) continue;
			AccessPath* ap = AccessPath::to_ap(t);
			if(ap->is_imprecise()) return true;
		}
		return false;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*> elems = fv->get_arguments();
		for(unsigned int i=0; i<elems.size(); i++) {
			if(elems[i]->is_imprecise()) return true;
		}
		return false;
	}
	default:
		return false;

	}
}

bool AccessPath::is_initialized()
{
	return is_initialized_rec(false);
}

bool AccessPath::is_exit_variable()
{
	if(this->apt != AP_VARIABLE) return false;
	Variable* v = (Variable*) this;
	return (v->is_loop_exit_var());
}

bool AccessPath::is_initialized_rec(bool seen_deref)
{
	assert_context("Checking if " + safe_string(this) + "is initialized.");
	switch(this->get_ap_type())
	{
	case AP_ADDRESS:
	case AP_CONSTANT:
	case AP_INDEX:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
		return true;
	case AP_NIL:
		return true;
	case AP_VARIABLE:
	{
		Variable* v = (Variable*) this;
		if(v->get_original_var() == NULL) return true;
		return false;
	}
	case AP_COUNTER:
		return true;
	case AP_ALLOC:
	{
		return !seen_deref;
	}
	case AP_UNMODELED:

	{
		UnmodeledValue* uv = (UnmodeledValue*) this;
		return (!uv->is_uninitialized());
	}
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
	case AP_DEREF:
		return get_inner()->is_initialized_rec(true);
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			if(!t->is_specialized()) continue;
			AccessPath* elem = AccessPath::to_ap(t);
			if(!elem->is_initialized_rec(seen_deref)) return false;
		}
		return true;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i< args.size(); i++)
		{
			AccessPath* arg = args[i];
			if(!arg->is_initialized_rec(seen_deref)) return false;
		}
		return true;
	}
	default:
		c_assert(false);

	}
}


/*
 * If there are any dereferences in this access path, returns the
 * outermost dereference, NULL otherwise.
 */
AccessPath* AccessPath::get_outermost_deref()
{
	AccessPath* ap =this;
	while(ap!=NULL) {
		if(ap->get_ap_type() == AP_DEREF) return ap;
		ap = ap->get_inner();
	}
	return NULL;
}


/*
 * If we want to make a fake field from this access path, what should its
 * inner access path be?
 * For ADT's, the size fields etc. should not refer to any index variables, so
 * this function strips the necessary parts.
 */
AccessPath* AccessPath::get_representative_for_fake_field()
{
	AccessPath* outermost_deref = this->get_outermost_deref();
	if(outermost_deref == NULL) return this;
	if(outermost_deref->get_inner()->get_ap_type() == AP_ADT) {
		return outermost_deref->get_inner()->get_inner()->add_deref();
	}
	return this;
}

bool AccessPath::contains_uninit_value()
{
	return contains_uninit_value_rec(false);
}

bool AccessPath::contains_uninit_value_rec(bool seen_deref)
{
	assert_context("Checking if " + safe_string(this) +
				" contains uninitialized value");
	switch(this->get_ap_type())
	{
	case AP_ADDRESS:
	case AP_CONSTANT:
	case AP_STRING:
	case AP_PROGRAM_FUNCTION:
	case AP_TYPE_CONSTANT:
	case AP_INDEX:
	case AP_VARIABLE:
	case AP_COUNTER:
		return false;
	case AP_NIL:
		return false;
	case AP_ALLOC:
		return seen_deref;
	case AP_UNMODELED:

	{
		UnmodeledValue* uv = (UnmodeledValue*) this;
		return (uv->is_uninitialized());
	}
	case AP_FIELD:
	case AP_ARRAYREF:
	case AP_ADT:
		return get_inner()->contains_uninit_value_rec(seen_deref);
	case AP_DEREF:
		return get_inner()->contains_uninit_value_rec(true);
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) this;
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			if(!t->is_specialized()) continue;
			AccessPath* elem = AccessPath::to_ap(t);
			if(elem->contains_uninit_value_rec(seen_deref)) return true;
		}
		return false;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i< args.size(); i++)
		{
			AccessPath* arg = args[i];
			if(arg->contains_uninit_value_rec(seen_deref)) return true;
		}
		return false;
	}
	default:
		c_assert(false);

	}
}



void AccessPath::get_counters(set<IterationCounter*>& counters, bool parametric)
{
	switch(this->get_ap_type())
	{

	case AP_COUNTER:
	{
		IterationCounter* ic = (IterationCounter*) this;
		if(parametric) {
			if(ic->is_parametric()) counters.insert(ic);
		}
		else {
			if(ic->is_last()) counters.insert(ic);
		}
		return;
	}
	case AP_DEREF:
	{
		Deref* d = (Deref*) this;
		AccessPath* inner = d->get_inner();
		inner->get_counters(counters, parametric);
		return;
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*)this;
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			AccessPath* cur = to_ap(t);
			if(cur == NULL) continue;
			cur->get_counters(counters, parametric);
		}
		return;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) this;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i<args.size(); i++)
		{
			args[i]->get_counters(counters, parametric);
		}
		return;
	}
	default:
		return;
	}
}

void AccessPath::clear()
{
	UnmodeledValue::clear();
	IndexVariable::clear();
	Variable::clear();
	FunctionValue::clear();
	Address::clear();
	StringLiteral::clear();
	ProgramFunction::clear();
	TypeConstant::clear();
	Term::clear();
	alloc_info::clear();
}

AccessPath::~AccessPath() {
}

void AccessPath::n_ary_cross_product(
		vector<set<pair<AccessPath*, Constraint> > >&
		ordered_value_sets, set<pair<vector<AccessPath*> , Constraint> >&
		cross_product)
{
	vector<AccessPath*> empty;
	Constraint c;
	n_ary_cross_product_rec(ordered_value_sets, cross_product, 0, empty, c);
}

void AccessPath::n_ary_cross_product_rec(
		vector<set<pair<AccessPath*, Constraint> > >&
		ordered_value_sets, set<pair<vector<AccessPath*> , Constraint> >&
		cross_product, int cur_pos, vector<AccessPath*>& cur_completed,
		Constraint cur_constraint)
{
	if(cur_constraint.unsat()) return;
	if(cur_pos == (int)ordered_value_sets.size())
	{
		cross_product.insert(pair<vector<AccessPath*>, Constraint>(
				cur_completed, cur_constraint));
		return;
	}

	set<pair< AccessPath*, Constraint> >& cur_vs = ordered_value_sets[cur_pos];
	set<pair< AccessPath*, Constraint> >::iterator it = cur_vs.begin();
	for(; it!=cur_vs.end(); it++)
	{
		AccessPath* cur_ap = it->first;
		Constraint cur_c = it->second & cur_constraint;
		cur_completed.push_back(cur_ap);
		n_ary_cross_product_rec(ordered_value_sets, cross_product,
				cur_pos+1, cur_completed, cur_c);
		cur_completed.pop_back();
	}

}

bool AccessPath::contains_unmodeled_value()
{
	switch(apt)
	{
	case AP_UNMODELED:
		return true;
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = static_cast<ArithmeticValue*>(this);
		const map<Term*, long int>& elems = av->get_elems();
		map<Term*, long int>::const_iterator it2 = elems.begin();
		for(;it2!= elems.end(); it2++) {
			Term* t = it2->first;
			AccessPath* elem = AccessPath::to_ap(t);
			if(elem->contains_unmodeled_value()) return true;
		}
		return false;
	}
	default:
		return false;


	}
}

