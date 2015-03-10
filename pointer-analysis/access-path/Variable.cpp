/*
 * Variable.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "Variable.h"
#include "sail/Variable.h"
#include "util.h"


#define DISJOINT_VAR_PREFIX "d"
#define TARGET_VAR_PREFIX "t"
#define TEMP_VAR_PREFIX "$"

#define DEBUG true

#define SUM_TEMP_PREFIX "__t_sum"

int Variable::counter = 0;
int Variable::cur_su_id = 0;
int Variable::e_counter = 0;

Variable* Variable::make(const string& name, il::type* t)
{
	Variable* v = new Variable(name, t, ATTRIB_NOATTRIB);
	return (Variable*) VariableTerm::get_term(v);
}

Variable* Variable::make_program_var(sail::Variable* var)
{
	ap_attribute attrib = ATTRIB_PROGRAM_VAR;
	if(var->is_loop_exit_var()) attrib = ATTRIB_SAIL_EXIT_VAR;
	Variable* v = new Variable(var, var->get_type(), attrib);
	return (Variable*)VariableTerm::get_term(v);

}

Variable* Variable::make_program_var(sail::Variable* var, il::type* t)
{
	ap_attribute attrib = ATTRIB_PROGRAM_VAR;
	if(var->is_loop_exit_var()) attrib = ATTRIB_SAIL_EXIT_VAR;
	Variable* v = new Variable(var, t, attrib);
	return (Variable*) VariableTerm::get_term(v);
}

Variable* Variable::make_string_const_var(const string& id)
{
	il::type* char_type = il::get_char_type();
	il::type* t = il::pointer_type::make(char_type, "");
	Variable* v = new Variable(id, t, ATTRIB_STRING_CONST);
	return (Variable*) VariableTerm::get_term(v);
}

Variable* Variable::make_temp(const string & name)
{
	Variable* v = new Variable(name, il::get_integer_type(), ATTRIB_TEMP_VAR);
	return (Variable*) VariableTerm::get_term(v);
}

Variable* Variable::make_client_var(const string& name, il::type* t)
{
	Variable* v = new Variable(name, t, ATTRIB_MISTRAL_VAR);
	return (Variable*) VariableTerm::get_term(v);
}

Variable* Variable::make_temp(il::type* t)
{
	Variable* v = new Variable(int_to_string(++counter),
			t, ATTRIB_TEMP_VAR);
	return (Variable*) VariableTerm::get_term(v);
}


Variable* Variable::make_loop_error_temp(const string & sum_id)
{
	string name = "e" + int_to_string(e_counter++) + "_" + sum_id;
	Variable* v = new Variable(name, il::get_unsigned_integer_type(),
			ATTRIB_LOOP_ERROR_TEMP);
	return (Variable*) VariableTerm::get_term(v);
}




Variable* Variable::make_disjointness_var(int id, il::type* t)
{
	Variable* v = new Variable(int_to_string(id), t, ATTRIB_DISJOINT);
	return (Variable*) VariableTerm::get_term(v);

}

Variable* Variable::make_target_var(int id)
{
	Variable* v = new Variable(int_to_string(id), il::get_integer_type(),
			ATTRIB_TARGET);
	return (Variable*) VariableTerm::get_term(v);

}



Variable* Variable::make_sum_temp(il::type* t)
{
	string name = int_to_string(++counter);
	Variable* v = new Variable(name, t, ATTRIB_SUMMARY_TEMP);
	return (Variable*) VariableTerm::get_term(v);
}


Variable::Variable(string name, il::type* t, ap_attribute attrib):
	VariableTerm(CNode::get_varmap().get_id("var_"+name), attrib)
{
	this->name = name;
	this->t = t;
	this->sail_var = NULL;
	this->specialization_type = AP_VARIABLE;
	this->apt = AP_VARIABLE;
	this->has_index_var = false;
	this->su_id = cur_su_id;
	if(is_global_var()) this->su_id = 0;
	add_signedness_attribute();
}

bool Variable::is_disjointness_var()
{
	return this->get_id_attribute() == ATTRIB_DISJOINT;
}

bool Variable::is_string_const()
{
	return this->get_id_attribute() == ATTRIB_STRING_CONST;
}

bool Variable::is_temporary()
{
	return this->get_id_attribute() == ATTRIB_TEMP_VAR;
}


Variable::Variable(sail::Variable* var, il::type* t, ap_attribute attrib):
	VariableTerm(CNode::get_varmap().get_id("var_"+
			(var->get_namespace().to_string() + var->get_var_name())),
			attrib)
{
	this->name = var->get_namespace().to_string() + var->get_var_name();
	this->sail_var = var->clone();
	this->t = t;
	this->specialization_type = AP_VARIABLE;
	this->apt = AP_VARIABLE;
	this->has_index_var = false;
	this->su_id = cur_su_id;
	if(is_global_var()){
		this->su_id = 0;
		name += "(" + t->to_string() + ")";

	}



	add_signedness_attribute();
	//cout << "this sail var 2" << sail_var->to_string() << endl;
}

bool Variable::is_background_target_var()
{
	return this->get_id_attribute() == ATTRIB_TARGET;
}

bool Variable::is_loop_exit_var()
{
	return this->get_id_attribute() == ATTRIB_SAIL_EXIT_VAR;
}

bool Variable::is_anonymous()
{
	return to_string().find("anonymous") != string::npos;
}

bool Variable::is_loop_error_temp()
{
	return this->get_id_attribute() == ATTRIB_LOOP_ERROR_TEMP;
}
string Variable::to_string()
{
	string res;
	if(get_id_attribute() == ATTRIB_DISJOINT)
		res += DISJOINT_VAR_PREFIX;
	else if(get_id_attribute() == ATTRIB_SUMMARY_TEMP)
		res += SUM_TEMP_PREFIX;
	else if(get_id_attribute() == ATTRIB_TARGET)
		res += TARGET_VAR_PREFIX;
	else if(get_id_attribute() == ATTRIB_TEMP_VAR)
		res += TEMP_VAR_PREFIX;
	string short_name = name;
	if(sail_var != NULL)
	{
		short_name = sail_var->get_var_name();
	}


	res += short_name;
	return res;

}

string Variable::get_name()
{
	return name;
}


bool Variable::is_local_var()
{
	if(sail_var == NULL) return false;
	return sail_var->is_local();
}
bool Variable::is_global_var()
{
	if(sail_var == NULL) return false;
	return sail_var->is_global();
}
bool Variable::is_argument()
{
	if(sail_var == NULL) return false;
	return sail_var->is_argument();
}
bool Variable::is_return_var()
{
	if(sail_var == NULL) return false;
	return sail_var->is_return_variable();
}
int Variable::get_arg_number()
{
	if(sail_var == NULL) return -1;
	return sail_var->get_arg_number();
}

sail::Variable* Variable::get_original_var()
{
	return sail_var;
}


bool Variable::is_synthetic()
{

	return get_id_attribute() != ATTRIB_PROGRAM_VAR;
}




Term* Variable::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) >0) return subs[this];
	return this;
}

Term* Variable::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}

/*
 * Is this variable introduced to help with instantiating access paths?
 */
bool Variable::is_summary_temp()
{
	return get_id_attribute() == ATTRIB_SUMMARY_TEMP;
}

void Variable::clear()
{
	counter = 0;
	e_counter = 0;
}



bool Variable::operator==(const Term& __other)
{



	Term& other = (Term&) __other;
	if(other.get_specialization() != AP_VARIABLE) {
		return false;
	}
	Variable * o = (Variable*)AccessPath::to_ap(&other);
	if(get_var_id() != o->get_var_id()){
		return false;
	}
	if(name != o->name)
	{
		return false;
	}
	if(get_id_attribute() != o->get_id_attribute()){

		return false;
	}
	if(is_local_var() != o->is_local_var()){
		return false;
	}
	if(is_global_var() != o->is_global_var()){
		return false;
	}
	if(is_return_var() != o->is_return_var()) {
		return false;
	}
	if(get_arg_number() != o->get_arg_number()){
		return false;
	}
	bool is_program_var = this->sail_var != NULL;
	if(is_program_var && su_id != o->su_id){
		return false;
	}
	if(!is_program_var && !(*o->t == *t)) {
		return false;
	}
	return true;


}

Variable::~Variable() {
}
