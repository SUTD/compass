/*
 * FunctionValue.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "FunctionValue.h"
#include "ArrayRef.h"
#include "AbstractDataStructure.h"
#include "ConstantValue.h"
#include "ArithmeticValue.h"
#include "util.h"
#include "compass_assert.h"

int FunctionValue::imprecise_id = 0;
map< pair<AccessPath*, AccessPath*>, int> FunctionValue::shared_bases;

FunctionValue* FunctionValue::make(ap_fun_type special_fun,
		AccessPath* ap)
{
	FunctionValue* fv = new FunctionValue(special_fun, "", ap,
			il::get_integer_type(), false, ATTRIB_NOATTRIB);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}



AccessPath* FunctionValue::make_times(AccessPath* ap1, AccessPath* ap2)
{
	return ArithmeticValue::make_times(ap1, ap2);
}
AccessPath* FunctionValue::make_division(AccessPath* ap1, AccessPath* ap2)
{
	return ArithmeticValue::make_division(ap1, ap2);
}

AccessPath* FunctionValue::_make_times(AccessPath* ap1, AccessPath* ap2)
{
	/*
	 * Since mutliplication commutes, we always try to make
	 * the access path with the smaller address the first argument.
	 */
	if(ap2 < ap1) {
		AccessPath* temp = ap2;
		ap2 = ap1;
		ap1 = temp;
	}
	FunctionValue* fv = new FunctionValue(FN_TIMES, "",
			ap1, ap2, il::get_integer_type(), false,
			ATTRIB_IMPRECISE_ARITHMETIC);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}
AccessPath* FunctionValue::_make_division(AccessPath* ap1, AccessPath* ap2)
{
	FunctionValue* fv = new FunctionValue(FN_DIVIDE, "",
			ap1, ap2, il::get_integer_type(), false,
			ATTRIB_IMPRECISE_ARITHMETIC);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

AccessPath* FunctionValue::make(ap_fun_type special_fun,
		AccessPath* ap1,
		AccessPath* ap2)
{

	assert_context("Making function value of type " +
			int_to_string(special_fun) + " args: "  + safe_string(ap1) +
			", " +  safe_string(ap2));

	if(special_fun == FN_LENGTH)
	{
		return FunctionValue::make_length(ap1);
	}

	if(special_fun == FN_TIMES) {
		return FunctionValue::make_times(ap1, ap2);
	}
	if(special_fun == FN_DIVIDE) {
		return FunctionValue::make_division(ap1, ap2);
	}

	if(special_fun == FN_SUBTYPE) {
		return FunctionValue::make_subtype(ap1, ap2);
	}

	if(special_fun == FN_SHARED_BASELOC){
		return FunctionValue::make_shared_base(ap1, ap2);
	}


	c_assert(special_fun <= FN_MOD);
	/*
	 * We should not make a function term if both ap1 and ap2 are constants
	 */
	if(ap1->get_ap_type() == AP_CONSTANT && ap2->get_ap_type()==AP_CONSTANT)
	{
		long int c1 = ((ConstantValue*) ap1)->get_constant();
		long int c2 = ((ConstantValue*) ap2)->get_constant();
		if(special_fun == FN_MOD)
		{
			long int res = c1%c2;
			return ConstantValue::make(res);
		}
		else if(special_fun == FN_BITWISE_AND)
		{
			long int res = c1&c2;
			return ConstantValue::make(res);
		}
		else if(special_fun == FN_BITWISE_OR)
		{
			long int res = c1|c2;
			return ConstantValue::make(res);
		}
		else if(special_fun == FN_BITWISE_XOR)
		{
			long int res = c1^c2;
			return ConstantValue::make(res);
		}
		else if(special_fun == FN_LEFT_SHIFT)
		{
			long int res = c1<<c2;
			return ConstantValue::make(res);
		}
		else if(special_fun == FN_RIGHT_SHIFT)
		{
			long int res = c1>>c2;
			return ConstantValue::make(res);
		}

	}

	FunctionValue* fv = new FunctionValue(special_fun, "",ap1, ap2,
			il::get_integer_type(), false, ATTRIB_NOATTRIB);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

AccessPath* FunctionValue::make(ap_fun_type special_fun,
		const vector<AccessPath*>& args, int attribute)
{
	if(special_fun == FN_LENGTH)
	{
		return FunctionValue::make_length(args[0]);
	}
	else if(special_fun == FN_TIMES){
		return FunctionValue::make_times(args[0], args[1]);
	}
	else if(special_fun == FN_DIVIDE) {
		return FunctionValue::make_division(args[0], args[1]);
	}
	else if(special_fun == FN_SUBTYPE) {
		return FunctionValue::make_subtype(args[0], args[1]);
	}

	else if(special_fun == FN_SHARED_BASELOC) {
		return FunctionValue::make_shared_base(args[0], args[1]);
	}

	else if(special_fun == FN_ADT_POS_SELECTOR)
	{
		return FunctionValue::make_adt_position_selector(args[0], args[1]);
	}
	else if(special_fun == FN_IS_NIL)
	{
		return FunctionValue::make_is_nil(args[0]);
	}
	if(args.size() == 2 && special_fun <= FN_MOD)
		return FunctionValue::make(special_fun, args[0], args[1]);


	FunctionValue* fv = new FunctionValue(special_fun, "", args,
			il::get_integer_type(), special_fun == FN_ADT_POS_SELECTOR,
			attribute);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

AccessPath* FunctionValue::make(ap_fun_type apt, int id,
		const vector<AccessPath*>& args, il::type* t, int attribute)
{
	if(apt == FN_LENGTH)
	{
		return FunctionValue::make_length(args[0]);
	}

	else if(apt == FN_TIMES) {
		return FunctionValue::make_times(args[0], args[1]);
	}
	else if(apt == FN_DIVIDE) {
		return FunctionValue::make_division(args[0], args[1]);
	}
	else if(apt == FN_SUBTYPE) {
		return FunctionValue::make_subtype(args[0], args[1]);
	}

	else if(apt == FN_SHARED_BASELOC) {
		return FunctionValue::make_shared_base(args[0], args[1]);
	}

	else if(apt == FN_ADT_POS_SELECTOR)
	{
		return FunctionValue::make_adt_position_selector(args[0], args[1]);
	}
	else if(apt == FN_IS_NIL)
	{
		return FunctionValue::make_is_nil(args[0]);
	}
	if(args.size() == 2 && apt <= FN_MOD)
			return FunctionValue::make(apt, args[0], args[1]);

	FunctionValue* fv = new FunctionValue(apt, id, args,t, apt == FN_ADT_POS_SELECTOR,
			attribute);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

FunctionValue* FunctionValue::make_disjoint(int id, AccessPath* ap, il::type* t)
{
	FunctionValue* fv = new FunctionValue(FN_DISJOINT, int_to_string(id),
			ap, t, false, ATTRIB_DISJOINT);
	return (FunctionValue*)FunctionTerm::get_term(fv);

}


FunctionValue* FunctionValue::make_target(int id, AccessPath* ap)
{
	FunctionValue* fv = new FunctionValue(FN_TARGET, int_to_string(id),
			ap, il::get_integer_type(), false, ATTRIB_TARGET);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

FunctionValue* FunctionValue::make_adt_position_selector(AccessPath*
		abs, AccessPath* key)
{

	if(abs->get_ap_type() == AP_ARITHMETIC && abs->get_base() != NULL)
	{
		abs = abs->get_inner();
	}

	if(abs->get_ap_type() == AP_ADT)
	{
		abs = abs->get_inner();
	}



	FunctionValue* fv = new FunctionValue(FN_ADT_POS_SELECTOR, "", abs,
			key, il::get_integer_type(), true, ATTRIB_ADT_POS);
	return (FunctionValue*)FunctionTerm::get_term(fv);
}

AccessPath* FunctionValue::make_length(AccessPath* arg)
{
	if(arg->get_ap_type() == AP_ARRAYREF)
	{
		ArrayRef* ar = (ArrayRef*) arg;
		AccessPath* inner = ar->get_inner();

		AccessPath* index = ar->get_index_expression();
		index = ArithmeticValue::make_times(index, ConstantValue::make(
				ar->get_elem_size()));


		//AccessPath* new_inner = ArrayRef::make(inner, ConstantValue::make(0),
		//		ar->get_elem_size());

		FunctionValue* fv = new FunctionValue(FN_LENGTH, "", inner,
					il::get_unsigned_integer_type(), false, ATTRIB_NOATTRIB);
		fv = (FunctionValue*) FunctionTerm::get_term(fv);
		return ArithmeticValue::make_minus(fv, index);
	}
	if(arg->get_ap_type() == AP_DEREF)
	{
		AccessPath* inner = arg->get_inner();
		if(inner->get_ap_type() == AP_ARITHMETIC)
		{
			ArithmeticValue* av = (ArithmeticValue*) inner;
			AccessPath* base = inner->get_base();
			if(base != NULL)
			{

				AccessPath* residue = ArithmeticValue::make_minus(av, base);
				FunctionValue* fv = new FunctionValue(FN_LENGTH, "",
						base->add_deref(), il::get_unsigned_integer_type(),
						false, ATTRIB_NOATTRIB);
				AccessPath* res1 = AccessPath::to_ap(FunctionTerm::get_term(fv));
				return ArithmeticValue::make_minus(res1, residue);

			}
		}
	}

	FunctionValue* fv = new FunctionValue(FN_LENGTH, "", arg,
			il::get_unsigned_integer_type(), false, ATTRIB_NOATTRIB);

	return (FunctionValue*)FunctionTerm::get_term(fv);
}

AccessPath* FunctionValue::make_subtype(AccessPath* ap1, AccessPath* ap2)
{
	FunctionValue* fv = new FunctionValue(FN_SUBTYPE, "subtype", ap1, ap2,
			il::get_integer_type(), false, ATTRIB_NOATTRIB);
	return (FunctionValue*) FunctionTerm::get_term(fv);
}

bool FunctionValue::is_locally_allocated(AccessPath* ap)
{
	bool local = false;
	if(ap->get_ap_type() == AP_ADDRESS)
	{
		local = true;
	}
	else if(ap->get_base() != NULL)
	{
		AccessPath* base = ap->get_base();
		if(base->get_ap_type() != AP_VARIABLE) {
			local = true;
		}
		else {
				Variable* v1 = (Variable*) base;
				if(v1->is_local_var() && !v1->is_summary_temp()) {
					local = true;
				}
		}
	}


	return local;
}

AccessPath* FunctionValue::make_shared_base(AccessPath* ap1, AccessPath* ap2)
{

	cout << "Making shared base: " << ap1->to_string() << " and " <<
			ap2->to_string() << endl;

	if(ap1 == ap2) return ConstantValue::make(1);
	if(ap1->is_derived_from_constant() || ap2->is_derived_from_constant())
		return ConstantValue::make(0);




	if(ap1 > ap2) {
		AccessPath* t = ap1;
		ap1 = ap2;
		ap2 = t;
	}




	if(ap1->get_base() == NULL || ap2->get_base() == NULL)
		return ConstantValue::make(0);

	AccessPath* b1 = ap1;
	AccessPath* b2 = ap2;

	if(b1->is_structured_ptr_arithmetic()) {
		b1 = ((ArithmeticValue*)ap1)->get_base_ptr();
	}
	if(b2->is_structured_ptr_arithmetic()) {
		b2 =  ((ArithmeticValue*)ap2)->get_base_ptr();
	}

	//cout << "Base 1: " << b1->to_string() << " base 2: " << b2->to_string() << endl;

	if(b1 == b2) return ConstantValue::make(1);
	//return ConstantValue::make(0);


	if(is_locally_allocated(b1) || is_locally_allocated(b2)) {
		return ConstantValue::make(0);
	}


	/*AccessPath* inner_b1 = b1->get_base();
	AccessPath* inner_b2 = b2->get_base();

	if(inner_b1 == NULL || inner_b2 == NULL) {
		return ConstantValue::make(0);
	}





	if(inner_b1->get_ap_type() != AP_VARIABLE ||
			inner_b2->get_ap_type() != AP_VARIABLE)
		ConstantValue::make(0);


	Variable* v1 = (Variable*) inner_b1;
	Variable* v2 = (Variable*) inner_b2;





	if(!v1->is_argument() && !v1->is_global_var())
		return ConstantValue::make(0);
	if(!v2->is_argument() && !v2->is_global_var())
		return ConstantValue::make(0); */

	vector<AccessPath*> args;
	args.push_back(b1);
	args.push_back(b2);


	int arg_id;
	if(shared_bases.count(make_pair(b1, b2)) > 0) {
		arg_id = shared_bases[make_pair(b1, b2)];
	}
	else {
		arg_id = shared_bases.size();
		shared_bases[make_pair(b1, b2)] = arg_id;
	}

	vector<Term*> arg_terms;
	arg_terms.push_back(ConstantValue::make(arg_id));

	FunctionValue* fv = new FunctionValue(FN_SHARED_BASELOC,
			"same_base", args, arg_terms,
			il::get_integer_type(),
			false, ATTRIB_NOATTRIB);

	return (FunctionValue*) FunctionTerm::get_term(fv);



}


AccessPath* FunctionValue::make_is_nil(AccessPath* ap)
{



	AccessPath* adt_base = ap;
	if(ap->get_ap_type() == AP_ARITHMETIC)
	{
		if(ap->get_inner() != NULL) {
			adt_base = ap->get_inner();
		}
	}
	adt_base = adt_base->find_representative();
	if(adt_base->strip_deref()->get_ap_type() == AP_ADT ||
			(ap->get_base()->get_ap_type() == AP_VARIABLE &&
			((Variable*) ap->get_base())->is_summary_temp()))
	{
		FunctionValue* fv = new FunctionValue(FN_IS_NIL, "", ap,
				il::get_integer_type(), false, ATTRIB_IS_NIL_FN);
		return (FunctionValue*)FunctionTerm::get_term(fv);
	}


	if(ap->get_ap_type() == AP_NIL) return ConstantValue::make(1);
	return ConstantValue::make(0);
}


bool FunctionValue::is_length()
{
	return fn_type == FN_LENGTH;
}
bool FunctionValue::is_nil_function()
{
	return fn_type == FN_IS_NIL;
}


bool FunctionValue::is_disjoint()
{
	return fn_type == FN_DISJOINT;
}
bool FunctionValue::is_target()
{
	return fn_type == FN_TARGET;
}

bool FunctionValue::is_imprecise()
{
	return fn_type == FN_UNMODELED;
}

bool FunctionValue::is_adt_pos_selector()
{
	return fn_type == FN_ADT_POS_SELECTOR;
}

bool FunctionValue::is_multiplication()
{
	return fn_type == FN_TIMES;
}
bool FunctionValue::is_division()
{
	return fn_type == FN_DIVIDE;
}

bool FunctionValue::is_subtype_predicate()
{
	return fn_type == FN_SUBTYPE;
}

bool FunctionValue::is_shared_base_predicate()
{
	return fn_type == FN_SHARED_BASELOC;
}
FunctionValue::FunctionValue(ap_fun_type apt, const string&  id,
		AccessPath* arg1, AccessPath* arg2, il::type* t, bool invertible,
		int attribute):
	FunctionTerm(CNode::get_varmap().get_id(fun_type_to_prefix(apt) + id),
			arg1->to_term(), arg2->to_term(), invertible, attribute)
{
	this->apt = AP_FUNCTION;
	this->t = t;
	this->specialization_type = AP_FUNCTION;
	this->has_index_var = arg1->contains_index_var() ||
						  arg2->contains_index_var();
	arguments.push_back(arg1);
	arguments.push_back(arg2);
	this->fn_type = apt;
	add_signedness_attribute();
}
FunctionValue::FunctionValue(ap_fun_type apt, const string& id, AccessPath* arg,
		il::type* t, bool invertible, int attribute):
	FunctionTerm(CNode::get_varmap().get_id(fun_type_to_prefix(apt)+id),
			arg->to_term(), invertible, attribute)
{
	this->apt = AP_FUNCTION;
	this->t = t;
	this->specialization_type = AP_FUNCTION;
	this->has_index_var = arg->contains_index_var();
	arguments.push_back(arg);
	this->fn_type = apt;
	add_signedness_attribute();
}


FunctionValue::FunctionValue(ap_fun_type apt, const string& id,
		const vector<AccessPath*>& args, il::type* t, bool invertible,
		int attribute):
	FunctionTerm(CNode::get_varmap().get_id(fun_type_to_prefix(apt)+id),
			this->args_to_terms(args), invertible, attribute)
{
	this->apt = AP_FUNCTION;
	this->t = t;
	this->specialization_type = AP_FUNCTION;
	arguments = args;
	this->fn_type = apt;
	has_index_var = false;

	for(unsigned int i=0; i<args.size(); i++)
	{
		if(args[i]->contains_index_var()){
			has_index_var = true;
			break;
		}
	}
	add_signedness_attribute();
}

FunctionValue::FunctionValue(ap_fun_type apt, const string& id,
			const vector<AccessPath*>& args,
			const vector<Term*>& term_args, il::type* t,
			bool invertible, int attribute):
			FunctionTerm(CNode::get_varmap().get_id(fun_type_to_prefix(apt)+id),
					term_args, invertible, attribute)
{
	this->apt = AP_FUNCTION;
	this->t = t;
	this->specialization_type = AP_FUNCTION;
	arguments = args;
	this->fn_type = apt;
	has_index_var = false;

	for(unsigned int i=0; i<args.size(); i++)
	{
		if(args[i]->contains_index_var()){
			has_index_var = true;
			break;
		}
	}
	add_signedness_attribute();
}

FunctionValue::FunctionValue(ap_fun_type apt, int id,
		const vector<AccessPath*>& args, il::type* t, bool invertible,
		int attribute):
	FunctionTerm(id,this->args_to_terms(args), invertible, attribute)
{
	this->apt = AP_FUNCTION;
	this->t = t;
	this->specialization_type = AP_FUNCTION;
	arguments = args;
	this->fn_type = apt;
	has_index_var = false;

	for(unsigned int i=0; i<args.size(); i++)
	{
		if(args[i]->contains_index_var()){
			has_index_var = true;
			break;
		}
	}
	add_signedness_attribute();
}

ap_fun_type FunctionValue::get_fun_type()
{
	return fn_type;
}


string FunctionValue::fun_type_to_prefix(ap_fun_type t)
{
	switch(t)
	{
	case FN_BITWISE_NOT: return "bit_not";
	case FN_BITWISE_AND: return "bit_and";
	case FN_BITWISE_OR: return "bit_or";
	case FN_BITWISE_XOR: return "bit_xor";
	case FN_LEFT_SHIFT: return "bit_lshift";
	case FN_RIGHT_SHIFT: return "bit_rshift";
	case FN_MOD: return "%";
	case FN_DISJOINT: return "d";
	case FN_TARGET: return "t";
	case FN_UNMODELED: return "im";
	case FN_LENGTH: return "length";
	case FN_ADT_POS_SELECTOR: return "pos";
	case FN_IS_NIL: return "IsNil";
	case FN_TIMES: return "times";
	case FN_DIVIDE: return "divide";
	case FN_SUBTYPE: return "subtype";
	case FN_SHARED_BASELOC: return "base_eq";
	case FN_UNINTERPRETED: return "";
	default: c_assert(false);
	}
}

string FunctionValue::fun_type_to_string(ap_fun_type t)
{


	switch(t)
	{
	case FN_BITWISE_NOT: return "~";
	case FN_BITWISE_AND: return "&";
	case FN_BITWISE_OR: return "|";
	case FN_BITWISE_XOR: return "^";
	case FN_LEFT_SHIFT: return "<<";
	case FN_RIGHT_SHIFT: return ">>";
	case FN_MOD: return "%";
	case FN_DISJOINT: return "d";
	case FN_TARGET: return "t";
	case FN_UNMODELED: return "im";
	case FN_LENGTH: return "length";
	case FN_ADT_POS_SELECTOR: return "pos";
	case FN_IS_NIL: return "IsNil";
	case FN_TIMES: return "*";
	case FN_DIVIDE: return "/";
	case FN_SUBTYPE: return "<:";
	case FN_SHARED_BASELOC: return "base_eq";
	case FN_UNINTERPRETED: return "";
	default: c_assert(false);
	}

}


string FunctionValue::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	if(fn_type == 	FN_UNINTERPRETED)
		return FunctionTerm::to_string();
	switch(this->fn_type)
	{
	case FN_BITWISE_NOT:
		return "~"+arguments[0]->to_string();
	case FN_BITWISE_AND:
		return arguments[0]->to_string()+"&"+arguments[1]->to_string();
	case FN_BITWISE_OR:
		return arguments[0]->to_string()+"|"+arguments[1]->to_string();
	case FN_BITWISE_XOR:
		return arguments[0]->to_string()+"^"+arguments[1]->to_string();
	case FN_LEFT_SHIFT:
		return  arguments[0]->to_string()+"<<"+arguments[1]->to_string();
	case FN_RIGHT_SHIFT:
		 return arguments[0]->to_string()+">>"+arguments[1]->to_string();
	case FN_MOD:
			 return arguments[0]->to_string()+"%"+arguments[1]->to_string();
	case FN_DISJOINT:
	case FN_TARGET:
	case FN_UNMODELED:
	case FN_LENGTH:
		return FunctionTerm::to_string();
	case FN_ADT_POS_SELECTOR:
		return "pos(" + arguments[0]->to_string()+","+arguments[1]->to_string()
			+")";
	case FN_IS_NIL:
		return "IsNil("+ arguments[0]->to_string() + ")";
	case FN_TIMES:
		return arguments[0]->to_string() + "*" + arguments[1]->to_string();
	case FN_DIVIDE:
		return arguments[0]->to_string() + "/" + arguments[1]->to_string();
	case FN_SUBTYPE:
		return  arguments[0]->to_string() + " <: " + arguments[1]->to_string();
	case FN_SHARED_BASELOC:
		return "base_eq(" + arguments[0]->to_string() + ", " +
		arguments[1]->to_string() + ")";
	default: c_assert(false);
	}
}

string FunctionValue::get_function_name()
{
	if(fn_type == 	FN_UNINTERPRETED)
		return this->get_name();
	return fun_type_to_string(fn_type);
}
vector<AccessPath*>& FunctionValue::get_arguments()
{
	return arguments;
}

vector<Term*> FunctionValue::args_to_terms(const vector<AccessPath*>& args)
{
	vector<Term*> term_args;
	for(unsigned int i=0; i<args.size(); i++)
	{
		AccessPath* arg = args[i];
		Term* tt = arg->to_term();
		term_args.push_back(tt);
	}
	return term_args;
}

Term* FunctionValue::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	bool changed = false;
	vector<AccessPath*> new_args;
	for(unsigned int i=0; i<arguments.size(); i++)
	{
		AccessPath* arg = arguments[i];
		Term* arg_t = arg->to_term()->substitute(subs);
		AccessPath* new_arg = AccessPath::to_ap(arg_t);
		if(new_arg != arg) changed = true;
		new_args.push_back(new_arg);
	}
	if(!changed) return this;
	return FunctionValue::make(this->fn_type, this->get_id(), new_args,
			t ,attribute)->to_term();
}

Term* FunctionValue::substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	bool changed = false;
	vector<AccessPath*> new_args;
	for(unsigned int i=0; i<arguments.size(); i++)
	{
		AccessPath* arg = arguments[i];
		Term* arg_t = arg->to_term()->substitute(sub_func, my_data);
		AccessPath* new_arg = AccessPath::to_ap(arg_t);
		if(new_arg != arg) changed = true;
		new_args.push_back(new_arg);
	}
	if(!changed) return this;
	return FunctionValue::make(this->fn_type, this->get_id(), new_args,
			t, attribute)->to_term();
}

void FunctionValue::clear()
{
	imprecise_id = 0;
	shared_bases.clear();
}

FunctionValue::~FunctionValue()
{

}
