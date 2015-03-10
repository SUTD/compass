/*
 * Variable.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#include "Variable.h"
#include <sstream>
#include "util.h"
#include "node.h"
#include "block.h"
#include "variable_expression.h"

#define TEMP_PREFIX "__temp"
#define RETVAR_STR  "__return"
using namespace il;

namespace sail {

/*
 * Temporary variable counter to give a fresh temporary name.
 */
static int temp_counter = 0;
static int block_counter = 0;
static Variable* return_var = NULL;
static map<string, pair<il::scope_type, il::block*> > name_to_block_map;
static map<il::block*, int> block_id_map;

Variable::Variable(il::node* original, type* var_type)
{
	initialize(original, var_type);
}

Variable::Variable(il::node* original, string alias_name, type* var_type)
{
	initialize(original, var_type);
	this->alias_name = alias_name;
}

Variable::Variable(string alias_name, type* var_type)
{
	assert(var_type != NULL);
	this->original = NULL;
	this->name = get_temp_name();
	this->isTemp = true;
	this->isRemovableTemp = true;
	this->alias_name = alias_name;
	this->is_retvar = false;
	this->is_loop_exit = false;
	//assert(var_type != NULL);
	this->var_type = var_type;
	set_local_status();
	set_arg_status();
	set_global_status();
	set_is_static();
	//added by haiyan
	is_point_to_alloc = false;
	//ended by haiyan
}

Variable::Variable(const sail::Variable& other){
	this->original = other.original;
	this->name = other.name;
	this->isTemp = other.isTemp;
	this->isRemovableTemp = other.isRemovableTemp;
	this->alias_name = other.alias_name;
	this->is_retvar = other.is_retvar;
	this->var_type = other.var_type;
	this->is_loop_exit = other.is_loop_exit;
	set_local_status();
	set_arg_status();
	set_global_status();
	set_is_static();

	//added by haiyan
	is_point_to_alloc = false;
	//ended by haiyan
}

Variable::Variable(string special_temp_name, type* var_type, bool unused)
{
	assert(var_type != NULL);
	this->original = NULL;
	this->name = special_temp_name;
	this->isTemp = true;
	this->isRemovableTemp = false;
	this->alias_name = special_temp_name;
	this->is_retvar = false;
	this->var_type = var_type;
	this->is_loop_exit = true;
	set_local_status();
	set_arg_status();
	set_global_status();
	set_is_static();

	//added by haiyan
	is_point_to_alloc = false;
	//ended by haiyan
}

Variable::Variable(string special_temp_name, il::type* var_type, bool mark_temp,
		bool is_arg, int num_arg, bool is_global)
{
	assert(var_type != NULL);
	this->original = NULL;
	this->name = special_temp_name;
	this->isTemp = mark_temp;
	this->isRemovableTemp = false;
	this->alias_name = special_temp_name;
	this->is_retvar = false;
	this->var_type = var_type;
	this->is_loop_exit = false;
	this->local = !is_arg;
	this->arg = is_arg;
	this->arg_num = num_arg;
	this->global = is_global;
	this->is_static_var = is_static_var;

	//added by haiyan
	is_point_to_alloc = false;
	//ended by haiyan
}

Variable* Variable::clone()
{
	Variable* new_v = new Variable();
	new_v->original = original;
	new_v->name = name;
	new_v->ns = ns;
	new_v->alias_name = alias_name;
	new_v->isTemp = isTemp;
	new_v->isRemovableTemp = isRemovableTemp;
	new_v->is_retvar = is_retvar;
	new_v->var_type= var_type;
	new_v->is_loop_exit = is_loop_exit;
	new_v->local = local;
	new_v->arg = arg;
	new_v->arg_num = arg_num;
	new_v->global = global;
	new_v->is_static_var = is_static_var;

	//added by haiyan
	new_v->is_point_to_alloc = is_point_to_alloc;
	//ended by haiyan
	return new_v;
}



bool Variable::operator==(const sail::Variable& _other)
{
	Variable& other= (Variable&) _other;
	if(name != other.name) return false;
	if(ns != other.ns) return false;
	if(is_retvar != other.is_retvar) return false;
	if(local != other.local) return false;
	if(arg_num != other.arg_num) return false;
	if(global != other.global) return false;
	if(var_type != other.var_type) return false;
	//added by haiyan
	if(is_point_to_alloc != other.is_point_to_alloc) return false;
	//ended by haiyan
	return true;

}

string Variable::get_unique_id()
{
	if(isTemp) return name;
	if(is_argument()) return "arg_"+name;
	if(is_global()) return "glob_"+name;
	il::variable_expression* ve = (il::variable_expression*) original;
	il::variable* v = ve->get_variable();
	string res = ns.to_string();
	if(v->get_scope() == NULL) {
		res += name;
	}
	else {
		il::block* b = v->get_scope();
		res += int_to_string((long int)b) + "_"+name;
	}
	return res;

}

void Variable::set_local_status()
{
	if(isTemp){
		local = true;
		return;
	}
	assert(original!=NULL);
	assert(original->node_type_id == VARIABLE_EXP);
	variable_expression* ve = (variable_expression*) original;
	local = ve->get_variable()->is_local();
}

void Variable::set_arg_status()
{
	if(isTemp) {
		arg = false;
		arg_num = -1;
		return;
	}
	assert(original!=NULL);
	assert(original->node_type_id == VARIABLE_EXP);
	variable_expression* ve = (variable_expression*) original;
	arg = ve->get_variable()->is_argument();
	arg_num = ve->get_variable()->get_arg_num();
}

void Variable::set_global_status()
{
	if(isTemp){
		global = false;
		return;
	}
	assert(original!=NULL);
	assert(original->node_type_id == VARIABLE_EXP);
	variable_expression* ve = (variable_expression*) original;
	global= ve->get_variable()->is_global();
}

bool Variable::is_local()
{
	if(this->is_synthesised_var()) return true;
	if(name.find("anonymous")!= string::npos) return true;
	return local&& !is_static_var;
}

bool Variable::is_anonymous()
{
	if(name.find("anonymous")!= string::npos) return true;
	return false;
}

bool Variable::is_argument()
{
	return arg;
}
void Variable::set_as_local(){
	arg = false;
	arg_num = -1;
}
int Variable::get_arg_number()
{
	return arg_num;
}

bool Variable::is_synthesised_var()
{
	return name.find("__") == 0;
}
bool Variable::is_global()
{
	if(this->is_synthesised_var()) return false;

	return global || (is_static_var);
}

bool Variable::is_return_variable()
{
	return is_retvar;
}

bool Variable::is_static()
{
	return is_static_var;

}

void Variable::set_original(il::node* orig)
{
	this->original = orig;
}

il::variable_declaration* Variable::get_declaration()
{

	if(original == NULL) {
		return NULL;
	}
	if(original->node_type_id != VARIABLE_EXP) {
		return NULL;
	}
	il::variable_expression* ve = (il::variable_expression*) original;
	return ve->get_variable()->get_declaration();
}

void Variable::initialize(il::node* original, type* var_type)
{
	assert(var_type != NULL);
	this->original  = original;
	this->is_loop_exit = false;
	//assert(var_type != NULL);
	this->var_type = var_type;
	if(original != NULL && this->original->node_type_id == VARIABLE_EXP){
		il::variable_expression* ve =
			(il::variable_expression*) this->original;
		il::variable* v = ve->get_variable();
		il::block* b = ve->get_variable()->get_scope();
		il::scope_type st = ve->get_variable()->get_scope_type();
		add_block(b);
		this->name = get_name(v->get_name(), b, st);
		this->ns = v->get_namespace();
		this->isTemp = false;
		this->isRemovableTemp = false;
		this->alias_name = name;

	}
	else{
		this->name = get_temp_name();
		il::namespace_context global_ns;
		this->ns = global_ns;
		this->isTemp = true;

		if(original != NULL){
			this->isRemovableTemp = true;
			alias_name = original->to_string();
		}
		else{
			this->isRemovableTemp = false;
			alias_name = name;
		}

	}
	this->is_retvar = false;
	set_local_status();
	set_arg_status();
	set_global_status();
	set_is_static();

}

void Variable::set_is_static()
{
	is_static_var = false;
	if(original!= NULL && original->node_type_id == VARIABLE_EXP){
		il::variable_expression* ve = (il::variable_expression*) original;
		if(ve->get_variable()->is_static())
			is_static_var = true;

	}
}

/***
 * added by haiyan
 */
void Variable::set_is_point_to_alloc(){
	this->is_point_to_alloc = true;
}

bool Variable::has_point_to_alloc(){
	return this->is_point_to_alloc;
}
/**
 * Private constructor to make return variables.
 */
Variable::Variable(type* var_type)
{
	this->original = NULL;
	this->name = RETVAR_STR;
	this->isTemp = true;
	this->isRemovableTemp = false;
	this->is_retvar = true;
	this->alias_name = name;
	this->is_loop_exit = false;
	//assert(var_type!=NULL);
	this->var_type = var_type;
	this->arg = false;
	this->arg_num = -1;
	this->local = false;
	this->global = false;
	this->is_static_var = false;

}

void Variable::replace_type(il::type* t)
{
	this->var_type = t;
}

void Variable::set_alias_name(string s)
{
	this->alias_name = s;
}
////
//hzhu added 5.25
void Variable::set_type(il::type* type)
{
    this->var_type = type;
}
void Variable::set_name(string name)
{
    this->name = name;
}
//hzhu end 5.25
////
void Variable::set_removable(bool removable)
{
	this->isRemovableTemp = removable;
}

/**
 * Factory method to make return variables.
 */
Variable* Variable::get_return_variable(type* var_type)
{
	if(return_var != NULL)
		return return_var;
	return_var = new Variable(var_type);
	return return_var;

}

type* Variable::get_type()
{
	return this->var_type;
}

string Variable::to_string() const
{
	string res = ns.to_string() + name;
	//added by haiyan to check the type
	res += "(" + var_type->to_string() + ")";
	if(is_retvar)
		res += "[RET_VAR]";
	if(arg_num != -1){
		res += "[ARG : ";
		res += int_to_string(arg_num);
		res += "]";
	}
//	}else{
//		res += "[LOCAL]";
//	}
//	res += "{addr::";
//	res += int_to_string((long)this);
//	res +=	" }";
	//ended by haiyan to check the type

	return res;
}

string Variable::get_var_name()
{
	return name;
}

const il::namespace_context& Variable::get_namespace() const
{
	return this->ns;
}

string Variable::to_string(bool print_readable) const
{
	if(print_readable) return alias_name;

	return to_string();

}
il::node* Variable::get_original_node()
{
	return original;

}
bool Variable::is_temp()
{
	return isTemp;
}

bool Variable::is_removable_temp()
{
	return isRemovableTemp;
}

bool Variable::is_constant()
{
	return false;
}

bool Variable::is_variable()
{
	return true;
}

string Variable::get_temp_name()
{
	string res  = TEMP_PREFIX;
	stringstream stream_count;
	stream_count << ++temp_counter;
	string str_count;
	stream_count >> str_count;
	res += str_count;
	return res;


}

bool Variable::is_loop_exit_var()
{
	return is_loop_exit;
}

string Variable::get_name(string orig_name, il::block* b, il::scope_type st)
{
	pair<scope_type, block*> block_qualifier = pair<scope_type, block*>(st, b);
	if(name_to_block_map.count(orig_name) == 0) {
		name_to_block_map[orig_name] = block_qualifier;
		return orig_name;
	}
	pair<scope_type, block*> existing_block = name_to_block_map[orig_name];
	if(block_qualifier == existing_block) return orig_name;


	if(st == GLOBAL) return orig_name + "_glob";
	if(st == ARGUMENT) return orig_name + "_arg";
	assert(block_id_map.count(b) > 0);
	int block_id = block_id_map[b];
	return orig_name+"_" + int_to_string(block_id);
}


void Variable::add_block(il::block* b)
{
	if(b == NULL) return;
	if(block_id_map.count(b) > 0) return;
	block_id_map[b] = ++block_counter;
}

void Variable::clear_maps()
{
	block_counter = 0;
	temp_counter = 0;
	block_id_map.clear();
	name_to_block_map.clear();
	return_var = NULL;
}



Variable::~Variable() {
}

}
