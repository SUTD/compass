/*
 * Function.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */


#include "Function.h"
#include "Symbol.h"
#include "Binop.h"
#include "BasicBlock.h"
#include "Unop.h"
#include "Constant.h"
#include "Instruction.h"
#include "Assembly.h"
#include "Label.h"
#include "Jump.h"
#include "Branch.h"
#include "Assignment.h"
#include "FieldRefWrite.h"
#include "FieldRefRead.h"
#include "ArrayRefWrite.h"
#include "ArrayRefRead.h"
#include "FunctionCall.h"
#include "FunctionPointerCall.h"
#include "Variable.h"
#include "Store.h"
#include "Load.h"
#include "AddressVar.h"
#include "AddressLabel.h"
#include "util.h"
#include "Cast.h"
#include "Cfg.h"
#include "TranslationUnit.h"
#include "AddressString.h"
#include "DropVariable.h"
#include "StaticAssert.h"
#include "Assume.h"
#include "AssumeSize.h"


#include "function_declaration.h"
#include "expression.h"
#include "function_call_expression.h"
#include "integer_const_exp.h"
#include "function_pointer_call_expression.h"
#include "deref_expression.h"
#include "cast_expression.h"
#include "expr_list_expression.h"
#include "field_ref_expression.h"
#include "array_ref_expression.h"
#include "addressof_expression.h"
#include "modify_expression.h"
#include "function_address_expression.h"
#include "builtin_expression.h"
#include "variable_expression.h"
#include "block_expression.h"
#include "statement.h"
#include "block.h"
#include "case_label.h"
#include "label.h"
#include "expression_instruction.h"
#include "binop_expression.h"
#include "unop_expression.h"
#include "conditional_expression.h"
#include "goto_statement.h"
#include "if_statement.h"
#include "switch_statement.h"
#include "return_statement.h"
#include "set_instruction.h"
#include "initializer_list_expression.h"
#include "variable_declaration.h"
#include "file.h"
#include "type.h"
#include "namespace_context.h"
#include "try_catch_statement.h"
#include "try_finally_statement.h"

#include "fileops.h"

#define DEBUG false

#define INIT_FUNCTION_NAME "$init"

/*
 * Name of g++ function used to lock static initializations.
 */
#define CXA_GUARD_ACQUIRE "__cxa_guard_acquire"
#define CXA_GUARD_RELEASE "__cxa_guard_release"


using namespace il;

namespace sail {



/*
 * Constructor for the special init functions to initialize globals.
 */
Function::Function(vector<il::variable_declaration*>& global_decls,
		string file_name, bool cpp, vector<Variable*> & zvg_vars)
{

	this->cpp = cpp;
	language_type lt = (cpp ? LANG_CPP: LANG_C);
	this->has_return = false;
	FileIdentifier fi(file_name);
	this->id = Identifier(FileIdentifier(file_name), fi.to_string('$')+
			INIT_FUNCTION_NAME, il::namespace_context(),
			il::get_void_function_type(), lt);

	this->return_label = NULL;
	this->original = NULL;
	this->cur_line = -1;
	this->first_line = -1;
	this->label_counter = 0;
	this->default_counter = 0;
	this->body = new vector<Instruction*>();
	vector<il::variable_declaration*>::iterator it = global_decls.begin();
	for(; it!=global_decls.end(); it++)
	{
		il::variable_declaration* vd = *it;

		assert(vd->has_initializer());

		il::variable* v = vd->get_variable();
		location l;
		il::variable_expression* ve = new il::variable_expression(v, l);
		Variable* sail_v =  new Variable(ve, ve->get_type());
		Symbol* init_s = process_expression(vd->get_initializer());
		Assignment* inst = new Assignment(sail_v, init_s, NULL, cur_line);
		body->push_back(inst);
		add_drop_temp(init_s);
	}

	/*
	 * Now, set all C++ zvg vars to 0.
	 */
	vector<Variable*>::iterator it2 = zvg_vars.begin();
	for(; it2 != zvg_vars.end(); it2++)
	{
		Variable* v = *it2;
		Symbol* zero = new Constant(0, false, 4);
		Assignment* inst = new Assignment(v, zero, NULL, cur_line);
		body->push_back(inst);

	}

	cfg = new Cfg(this);
	this->is_method_flag = false;
	this->is_virtual_method_flag = false;
	this->is_abstract_virtual_method_flag = false;
	this->is_constructor_flag = false;
	this->is_destructor_flag = false;
}

//haiyan changed the prototype 5.28
Function::Function(il::function_declaration* original, file* f,
	bool cpp)
{

	this->cpp = cpp;
	language_type lt = (cpp ? LANG_CPP : LANG_C);


	this->id = Identifier(FileIdentifier(f->get_name()), original->get_name(),
			original->get_namespace(), original->get_signature(), lt);

	this->return_label = NULL;
	this->original = original;
	this->cur_line = -1;
	this->first_line = -1;
	this->label_counter = 0;
	this->default_counter = 0;
	this->body = new vector<Instruction*>();
	if(DEBUG){
		cout << "*******ORIGINAL IL:******"<<endl;
		cout << original->to_string() << endl;
		cout << "*********END IL********" << endl;
	}

	make_function_body();
	 if(DEBUG){
		cout << "************** SAIL original :" << endl;
		cout << this->to_string(false) << endl;
		cout << "SAIL END ******* " << endl;

	 }

	 /*
	  * This optimization unfortunately interacts badly with
	  * pretty-printing. For example,
	  * int x;
	  * x = a? foo(): 2;
	  * will not pretty-print correctly.
	  * DO NOT COMMENT BACK IN
	  */
	 //remove_redundant_temporaries();


	 remove_redundant_labels();
	 remove_null_instructions();

	 label_eqv_map.clear();



	 if(DEBUG){
		cout << "************** SAIL :" << endl;
		cout << this->to_string(false) << endl;
		cout << "SAIL END ******* " << endl;

	 }

	 cfg = new Cfg(this);
	 Variable::clear_maps();
	 this->is_method_flag = (original != NULL && original->is_method());
	 this->is_virtual_method_flag =
			 (original != NULL && original->is_virtual_method());
	 this->is_abstract_virtual_method_flag =
			 (original != NULL && original->is_abstract_virtual_method());
	 this->is_constructor_flag =
			 (original != NULL && original->is_constructor());
	 this->is_destructor_flag =
			 (original != NULL && original->is_destructor());

}





/*
 * Constructor used by dex-parser
 */
Function::Function(const string& name,
		const string & file,
		il::namespace_context ns,
		il::function_type* signature, vector<Instruction*>* instruction,
		bool is_virtual_method, bool is_abstract_method,
		bool is_constructor, bool is_destructor)
{

	this->cpp = true;
	language_type lt = LANG_JAVA;
	//cout << "file: " << file << endl;
	string path = FileIdentifier("").to_string('/');

	string res;
	find_path_for_file(path, file, res);

	string full_file = (res == "" ? file : (res + "/" + file));

	FileIdentifier fi(full_file);
	//cout << "FI: " << fi << "  name: " << fi.get_filename() << endl;

	//assert(false);

	this->id = Identifier(FileIdentifier(full_file), name,
			ns, signature, lt);

	this->return_label = NULL;
	this->original = NULL;
	this->cur_line = -1;
	this->first_line = -1;
	this->label_counter = 0;
	this->default_counter = 0;
	this->body = instruction;


	
	if (0) {
		cout << "************** SAIL before moving labels :" << endl;
		cout << this->to_string(false) << endl;
		cout << "SAIL before moving labels END ******* " << endl;
	}
	
	remove_redundant_labels();
	remove_null_instructions();
	label_eqv_map.clear();
	 if(1){
		cout << "************** SAIL :" << endl;
		cout << this->to_string(false) << endl;
		cout << "SAIL END ******* " << endl;

	 }
	 if(0){
		cout << "************** SAIL PP :" << endl;
		cout << this->to_string(true) << endl;
		cout << "SAIL PP END ******* " << endl;

	 }

	 cfg = new Cfg(this);
	 Variable::clear_maps();
	 this->is_method_flag = true;
	 this->is_virtual_method_flag =is_virtual_method;
	 this->is_abstract_virtual_method_flag = is_abstract_method;
	 this->is_constructor_flag = is_constructor;
	 this->is_destructor_flag = is_destructor;

}
int Function::get_first_line()
{
	return first_line;
}

string  Function::get_file()
{
	return id.get_file_id().to_string();
}

BasicBlock* Function::get_entry_block()
{
	return cfg->get_entry_block();
}
BasicBlock* Function::get_exit_block()
{
	return cfg->get_exit_block();
}

BasicBlock* Function::get_exception_block()
{
	return cfg->get_exception_block();
}

expression* Function::strip_casts(expression* e)
{
	if(e->node_type_id != CAST_EXP) return e;
	cast_expression* ce = (cast_expression*) e;
	return strip_casts(ce->get_inner_expression());
}

void Function::get_instructions(vector<Instruction*>& instructions)
{
	set<BasicBlock*>& blocks = cfg->get_basic_blocks();
	set<BasicBlock*>::iterator it = blocks.begin();
	for(; it!= blocks.end(); it++){
		BasicBlock* b = *it;
		vector<Instruction*>& statements = b->get_statements();
		for(int i=0; i<(int)statements.size(); i++){
			instructions.push_back(statements[i]);
		}
	}
}

Identifier Function::get_identifier()
{
	return id;
}

il::namespace_context  Function::get_namespace()
{
	if(original == NULL) return il::namespace_context();
	return original->get_namespace();
}

bool Function::is_init_function()
{
	return original == NULL && this->id.get_language()!=LANG_JAVA;
}



bool Function::is_method()
{

	return this->is_method_flag;
}

il::record_type* Function::get_defining_class()
{
	cout << "in function: " << this->get_identifier().to_string() << endl;
	if (!is_method())
	{
		cout << "not method" << endl;
		return NULL;
	}
	il::function_type* ft = this->get_signature();
	cout << "ft: " << ft->to_string() << endl;
	if(ft->get_arg_types().size() == 0) return NULL;
	il::type* first_arg = ft->get_arg_types()[0];
	if(!first_arg->is_pointer_type()) return NULL;
	first_arg = first_arg->get_inner_type();
	if(!first_arg->is_record_type()) return NULL;
	return static_cast<il::record_type*>(first_arg);

}

bool Function::is_virtual_method()
{
	return this->is_virtual_method_flag;
}


bool Function::is_abstract_virtual_method()
{
	return this->is_abstract_virtual_method_flag;
}


bool Function::is_constructor()
{
	return this->is_constructor_flag;
}

bool Function::is_destructor()
{
	return this->is_destructor_flag;
}


il::type* Function::get_return_t(){
	return id.get_function_signature()->get_return_type();
}
/**
 * Gives the return type of this function
 */
il::type* Function::get_return_type()
{
	if(original == NULL) return il::get_void_type();
	else return original->get_ret_type();
}




il::function_type* Function::get_signature()
{
	return id.get_function_signature();
}

Cfg* Function::get_cfg()
{
	return cfg;
}



string Function::to_string() const
{
	string res = id.to_string() + " {\n";
	for(int i=0 ; i<(int)body->size(); i++)
	{

		res += "\t" + escape_string((*body)[i]->to_string()) + "; \n";
	}
	res += "}";
	return res;
}

string Function::to_string(bool pretty_print) const
{
	string res = id.to_string() + " {\n";
	for(int i=0 ; i<(int)body->size(); i++)
	{
		if(pretty_print && (*body)[i]->is_removable()) continue;
		res += "\t" + escape_string((*body)[i]->to_string(pretty_print)) + "; \n";
	}
	res += "}";
	return res;
}

il::function_declaration* Function::get_original_declaration()
{
	return original;
}

vector<Instruction*>* Function::get_body()
{
	return body;
}


void Function::add_annotation(Annotation* a)
{
	this->annotations.push_back(a);
}

//add by haiyan
void Function::add_annotations(vector<Annotation *>& a){
	this->annotations = a;
}

const vector<Annotation*> & Function::get_annotations()
{
	return this->annotations;
}



// --------------------------------------------

void Function::make_function_body()
{
	has_return = false;
	process_statement(original->get_body(), false);
	if(has_return)
		body->push_back(get_return_label());

}

const list<sail::Variable*>& Function::get_local_vars()
{
	return local_vars;
}

/*
 * The return value is the value the statement evaluates to where
 * relevant. The return value can be NULL if the statement
 * does not produce a value.
 */
Symbol* Function::process_statement(il::statement* stmt, bool return_needed)
{
	Symbol* res = NULL;
	if((int)stmt->get_location().line_start > 0){
		this->cur_line = stmt->get_location().line_start;
		if(this->first_line < 0){
			first_line = cur_line;
		}
	}
	switch(stmt->node_type_id)
	{
	case ASSEMBLY:
	{
		Assembly* inst = new Assembly((il::assembly*) stmt, cur_line);
		body->push_back(inst);
		res = NULL;
		break;
	}
	case BLOCK:
	{
		il::block* b = (il::block*) stmt;



		const vector<il::variable_declaration*> & var_decls  =
				b->get_var_declarations();
		vector<il::variable_declaration*>::const_iterator it = var_decls.begin();

		vector<Variable*> locals;
		for(; it != var_decls.end(); it++)
		{
			il::variable_declaration* vd = *it;
			//cout << "var decl: " << vd->to_string() << endl;
			il::type* t = vd->get_variable()->get_type();

			il::variable* var = vd->get_variable();
			il::location loc = vd->get_location();
			il::variable_expression* var_exp =
				new il::variable_expression(var, loc);
			sail::Symbol* local_var = process_expression(var_exp);
			assert(local_var->is_variable());
			sail::Variable* lv = (Variable*) local_var;
			locals.push_back(lv);

			/*
			 * Make a fake AssumeSize instruction to set the
			 * correct size of a vararray.
			 */
			if(t->is_vararray_type()) {
				il::vararray_type* vt = (il::vararray_type*) t;
					il::expression* size_exp = vt->get_num_elems();
					Symbol* size = process_expression(size_exp);
				/*	il::variable* var = vd->get_variable();
					il::location loc = vd->get_location();
					il::variable_expression* var_exp =
						new il::variable_expression(var, loc);*/
					il::addressof_expression* addr_var = new
							il::addressof_expression(var_exp,
								il::pointer_type::make(vt, ""),  loc);
					Symbol* addr_symbol = process_expression(addr_var);

					AssumeSize* set_size = new AssumeSize(addr_symbol,
							size, vd, loc.line_start);

					body->push_back(set_size);


			}



			il::expression* init = vd->get_initializer();
			if(!cpp || init == NULL) continue;

			/*
			 * Process variable declarations that have initializers.
			 * This is only necessary in C++, in C the initializers
			 * are repeated later.
			 */
			il::variable* declared_var = vd->get_variable();

			il::variable_expression* ve = new
					il::variable_expression(declared_var, init->get_location());
			il::set_instruction* si = new il::set_instruction(ve, init,
					init->get_location(), true);
			Symbol* cur_res = process_statement(si, return_needed);
			if(cur_res != NULL) res = cur_res;

		}

		vector<Variable*>::reverse_iterator loc_it = locals.rbegin();
		for(; loc_it!= locals.rend(); loc_it++) {
			Variable* v = *loc_it;
			this->local_vars.push_front(v);
		}






		/*
		 * Now, process statements
		 */
		vector<il::statement*> & stmts = b->get_statements();
		for(unsigned int i=0; i< stmts.size(); i++) {
			Symbol* cur_res = process_statement(stmts[i], return_needed);
			if(cur_res != NULL) res = cur_res;
		}
		vector<il::variable_declaration*> & decls = b->get_var_declarations();
		for(unsigned int i=0; i < decls.size(); i++) {
			il::variable_declaration* decl = decls[i];
			il::variable* v = decl->get_variable();
			location l;
			il::variable_expression* ve = new il::variable_expression(v, l);
			Variable* sail_v =  new Variable(ve, ve->get_type());
			add_drop_ref(sail_v);
		}

		break;
	}
	case EXPR_INSTRUCTION:
	{
		il::expression_instruction* exp_inst =
			(il::expression_instruction*) stmt;
		res = process_expression(exp_inst->get_expression());
		break;
	}
	case GOTO_STMT:
	{
		il::goto_statement* goto_stmt = (il::goto_statement*) stmt;
		Label* l = find_or_create_label(goto_stmt->get_label());
		Jump* j = new Jump(l, stmt);
		body->push_back(j);
		res = NULL;
		break;
	}
	case IF_STMT:
	{
		process_if_statement(stmt);
		res = NULL;
		break;
	}
	case LABEL:
	{
		il::label* il_label = (il::label*) stmt;
		Label* sail_label = find_or_create_label(il_label);
		body->push_back(sail_label);
		res = NULL;
		break;
	}
	case CASE_LABEL:
	{
		il::case_label* il_label = (il::case_label*) stmt;
		if(!case_label_exists(il_label))
			break;
		Label* sail_label = find_or_create_label(il_label);
		body->push_back(sail_label);
		res = NULL;
		break;
	}
	case NOOP_INSTRUCTION: {
		res = NULL;
		break;
	}
	case RETURN_STMT:
	{
		process_return_statement(stmt);
		res = NULL;
		break;
	}

	case SET_INSTRUCTION:
	{
		res = process_set_instruction(stmt, return_needed);
		break;
	}

	case SWITCH_STMT:
	{
		process_switch_statement(stmt);
		res = NULL;
		break;
	}

	/*
	 * FIXME: For now, we pretend exceptions don't happen
	 */
	case TRY_CATCH_STMT:
	{
		il::try_catch_statement* tc = static_cast<il::try_catch_statement*>(stmt);
		il::statement* try_stmt = tc->get_try_statement();
		res = process_statement(try_stmt, return_needed);
		break;
	}

	case TRY_FINALLY_STMT:
	{
		il::try_finally_statement* tf = static_cast<il::try_finally_statement*>(stmt);
		il::statement* try_stmt = tf->get_try_statement();
		process_statement(try_stmt, false);
		il::statement* final_stmt = tf->get_exit_statement();
		res = process_statement(final_stmt, return_needed);
		break;
	}

	default:
	{
		cout << "Unexpected statement: " << stmt->to_string()  << " node id: "
				<< stmt->node_type_id << endl;
		assert(false);
	}


	}
	return res;
}

void Function::remove_redundant_temporaries()
{
	for(int i=1; i<(int) body->size(); i++)
	{

		Instruction* cur = (*body)[i];
		if(cur->get_instruction_id() != ASSIGNMENT) continue;
		Assignment* assign = (Assignment*) cur;
		Symbol* rhs = assign->get_rhs();
		if(rhs->is_constant()) continue;
		Variable* rhs_var = (Variable*) rhs;
		cout << "********* RHS VAR:" << rhs_var->to_string(false) << endl;
		cout << "IsTemp? " << rhs_var->is_temp() << endl;
		if(!rhs_var->is_temp()) continue;

		cout << "GOT TO 1" << endl;
		Variable* lhs = assign->get_lhs();
		Instruction* prev = (*body)[i-1];

		if(prev==NULL || !prev->is_save_instruction()) continue;
		cout << "GOT TO 2" << endl;
		SaveInstruction* save = (SaveInstruction*) prev;
		Variable* prev_lhs = save->get_lhs();
		if(prev_lhs == NULL) continue;
		cout << "GOT TO 3" << endl;
		if(prev_lhs != rhs_var) continue;
		cout << "GOT TO 4" << endl;

		save->set_lhs(lhs);
		(*body)[i] = NULL;
		delete prev_lhs;
		delete assign;
	}
}

void Function::remove_redundant_labels()
{
	vector<Label*> to_delete;
	compute_label_equivalences();
	for(int i=0; i<(int)body->size(); i++)
	{
		Instruction* cur_inst = (*body)[i];
		if(cur_inst == NULL) continue;
		if(cur_inst->get_instruction_id() == JUMP)
		{
			Jump* j = (Jump*) cur_inst;
			Label* l = j->get_label();
			Label* rep = label_eqv_map[l];
			if(rep==NULL){
				cout << l->get_label_name() << "    cause the problem!!" <<endl;
				cerr << "ASSERT FAILED: " <<
					this->original->get_name() <<
					this->original->get_location().to_string() << endl <<
					l->get_label_name() << endl;
					cerr << "Label address:" << (long int) l->get_original_node() << endl;;

				for(int i=0; i<(int) body->size(); i++)
					{
						Instruction* cur_inst = (*body)[i];
						if(cur_inst == NULL) continue;
						cerr << cur_inst->to_string(false) << endl;
					}

				assert(false);
			}
			if(l != rep) j->set_label(rep);
			continue;
		}

		if(cur_inst->get_instruction_id()== SAIL_LABEL)
		{
			Label* l = (Label*) cur_inst;
			Label* rep = label_eqv_map[l];
			assert(rep!=NULL);
			if(l != rep) {
				(*body)[i] = NULL;
				to_delete.push_back(l);
			}
			continue;
		}

		if(cur_inst->get_instruction_id() == BRANCH)
		{
			Branch* b = (Branch*) cur_inst;
			vector<pair<Symbol*, Label*> > * targets = b->get_targets();
			for(int i=0; i<(int) targets->size(); i++)
			{
				Label* l = (*targets)[i].second;
				Symbol* s = (*targets)[i].first;
				Label* rep = label_eqv_map[l];
				assert(rep!=NULL);
				if(l!=rep){
					(*targets)[i] = pair<Symbol*, Label*>(s, rep);
				}

			}
		}
	}

	for(int i=0; i<(int)to_delete.size(); i++){
		delete to_delete[i];
	}
}

void Function::compute_label_equivalences()
{
	Label* cur_representative = NULL;
	for(int i=0; i<(int) body->size(); i++)
	{
		Instruction* cur_inst = (*body)[i];
		if(cur_inst == NULL) continue;
		if(cur_inst->get_instruction_id() != SAIL_LABEL){
			cur_representative = NULL;
			continue;
		}

		Label* cur_label = (Label*) cur_inst;

		if(cur_representative == NULL)
		{
			cur_representative = cur_label;

		}
		//cout << "cur label: " << (long int) cur_label->get_original_node() <<  " " << cur_label->get_label_name()
			//	<<  "rep: " << cur_representative->get_label_name() << endl;
		label_eqv_map[cur_label] = cur_representative;
		assert(label_eqv_map[cur_label] != NULL);
	}
}

void Function::remove_null_instructions()
{
	vector<Instruction*>* new_body = new vector<Instruction*>();
	for(int i=0; i<(int) body->size(); i++)
	{
		if((*body)[i] != NULL)
			new_body->push_back((*body)[i]);
	}

	delete body;
	body = new_body;
}

Symbol* Function::generate_condition_from_ranges(vector<pair<long, long> >*
		ranges, Symbol *switch_var, type* t)
{
	set<Symbol*> to_drop;
	vector<Variable*> to_or;
	for(int i =0; i < (int)ranges->size(); i++)
	{
		long low = (*ranges)[i].first;
		long high = (*ranges)[i].second;
		Variable *v = NULL;
		if(low == high)
		{
			Constant *c = new Constant(low, t->is_signed_type(), t->get_size());
			string v_alias_name = switch_var->to_string(true)
				+ "==" + c->to_string();
			v = new Variable(v_alias_name, get_integer_type());
			to_drop.insert(v);
			Binop *b = new Binop(v, switch_var, c, il::_EQ, NULL, cur_line);
			body->push_back(b);

		}
		else
		{
			Constant* low_const = new Constant(low,
					t->is_signed_type(), t->get_size());
			Constant* high_const = new Constant(high,
					t->is_signed_type(), t->get_size());
			string v1_alias_name =
				low_const->to_string() + "<=" + switch_var->to_string(true);
			Variable* v1 = new Variable(v1_alias_name, get_integer_type());
			Binop* binop1 = new Binop(v1, low_const, switch_var, il::_LEQ,
					NULL, cur_line);
			body->push_back(binop1);

			string v2_alias_name =
				switch_var->to_string(true)+ "<=" + high_const->to_string();
			Variable* v2 = new Variable(v2_alias_name, get_integer_type());
			Binop* binop2 = new Binop(v2, switch_var, high_const, il::_LEQ,
					NULL, cur_line);
			body->push_back(binop2);

			string v_alias_name = low_const->to_string() + "<=" +
				switch_var->to_string(true) + "<=" + high_const->to_string();
			v = new Variable(v_alias_name, get_integer_type());
			Binop* binop3 = new Binop(v, v1, v2, il::_LOGICAL_AND,
					NULL, cur_line);
			body->push_back(binop3);
			to_drop.insert(v1);
			to_drop.insert(v2);
			to_drop.insert(v);

		}
		to_or.push_back(v);
	}

	/*
	 * The size of this vector can be 0 if switch has only a default
	 */
	if(to_or.size() == 0)
		return new Constant(0);
	if(to_or.size() == 1)
		return to_or[0];

	Variable *prev_res = to_or[0];
	string prev_alias_name = to_or[0]->to_string(true);
	for(int i=1; i< (int)to_or.size(); i++)
	{
		string cur_alias_name = prev_alias_name + " || "
									+ to_or[i]->to_string(true);
		Variable *cur_res = new Variable(cur_alias_name, get_integer_type());
		if(i!=(int)to_or.size() - 1) to_drop.insert(cur_res);
		Binop* b = new Binop(cur_res, prev_res, to_or[i], il::_LOGICAL_OR,
				NULL, cur_line);
		body->push_back(b);
		prev_res = cur_res;
		prev_alias_name = cur_alias_name;
	}
	set<Symbol*>::iterator it = to_drop.begin();
	for(; it!= to_drop.end(); it++) add_drop_temp(*it);
	return prev_res;


}


void Function::process_switch_statement(il::statement* stmt)
{
	set<Symbol*> to_drop;
	switch_statement* il_switch = (switch_statement*) stmt;
	expression* cond = il_switch->get_cond();
	type* t = cond->get_type();

	Symbol* switch_var = process_expression(cond);
	to_drop.insert(switch_var);

	map<case_label*, vector<case_label*>* >* label_eqs =
		il_switch->get_label_equivalence_classes();

	map<case_label*, vector<pair<long, long> > * > & eq_class_ranges =
		il_switch->get_equivalence_class_ranges();

	map<case_label*, vector<case_label*>* >::iterator it =
		label_eqs->begin();

	vector<pair<Symbol*, Label*> > *targets =
		new vector<pair<Symbol*, Label*> >();

	bool has_default = false;
	for(; it!= label_eqs->end(); it++)
	{
		case_label *rep = it->first;
		Symbol *cond_var;
		if(rep->is_default_label())
		{
			has_default = true;
			vector<pair<long, long> > & ranges = il_switch->get_case_ranges();
			Symbol *cond_var_not = generate_condition_from_ranges(&ranges, switch_var,
							t);
			string cond_var_name = "!(" + cond_var_not->to_string(true) +")";
			cond_var = new Variable(cond_var_name, get_integer_type());
			Unop* u = new Unop((Variable*)cond_var, cond_var_not, il::_LOGICAL_NOT,
					NULL, cur_line);
			body->push_back(u);
			to_drop.insert(cond_var_not);
			to_drop.insert(cond_var);
		}
		else{
			vector<pair<long, long> > *ranges = eq_class_ranges[rep];
			cond_var = generate_condition_from_ranges(ranges, switch_var, t);
			to_drop.insert(cond_var);


		}

		Label *cond_target = find_or_create_label(rep);
		targets->push_back(pair<Symbol*, Label*>(cond_var, cond_target));
	}

	Label *default_target = NULL;
	if(!has_default) {
		vector<pair<long, long> > & ranges = il_switch->get_case_ranges();
		Symbol *cond_var_not = generate_condition_from_ranges(&ranges,
				switch_var, t);
		to_drop.insert(cond_var_not);
		string cond_var_name = "!(" + cond_var_not->to_string(true) +")";
		Variable* cond_var = new Variable(cond_var_name, get_integer_type());
		to_drop.insert(cond_var);
		Unop* u = new Unop(cond_var, cond_var_not, il::_LOGICAL_NOT,
				NULL, cur_line);
		body->push_back(u);
		default_target = create_default_label();
		targets->push_back(pair<Symbol*, Label*>(cond_var, default_target));
	}
	body->push_back(new Branch(targets, stmt));
	process_statement(il_switch->get_body(), false);
	if(default_target != NULL) body->push_back(default_target);
	set<Symbol*>::iterator drop_it = to_drop.begin();
	for(; drop_it!= to_drop.end(); drop_it++) add_drop_temp(*drop_it);

}

/**
 * The boolean flag return_needed should be set to true if the set instruction
 * needs to evaluate to something, e.g. part of a modify expression, and to
 * false otherwise. If it's set to false, this function always returns NULL.
 */
Symbol* Function::process_set_instruction(il::statement* stmt, bool return_needed,
		il::node* original)
{
	if(original == NULL) original = stmt;


	set_instruction* set_inst = (set_instruction*) stmt;

	if(set_inst->get_lvalue()->to_string().find("_vptr")!= string::npos) return NULL;


	Symbol* rhs = process_expression(set_inst->get_rhs());
	expression* lhs = set_inst->get_lvalue();


	Instruction* inst;
	Symbol* return_symbol = NULL;

	switch(lhs->node_type_id)
	{
	case VARIABLE_EXP:
	{
		Symbol* sail_lhs = process_expression(lhs);
		assert(sail_lhs->is_variable());
		Variable* lhs_var = (Variable*) sail_lhs;
		inst = new Assignment(lhs_var, rhs, original, cur_line);
		body->push_back(inst);
		add_drop_temp(rhs);
		if(return_needed) return_symbol = lhs_var;
		else add_drop_temp(lhs_var);
		break;
	}
	case DEREF_EXP:
	{
		deref_expression* de = (deref_expression*) lhs;
		il::expression* inner_lhs = de->get_inner_expression();
		int offset = 0;
		string name = "";
		Symbol* sail_inner_lhs = process_expression(inner_lhs);
		inst = new Store(sail_inner_lhs, rhs, offset, name, original, cur_line);
		body->push_back(inst);
		if(return_needed){
			string alias_name = "*" + sail_inner_lhs->to_string(true);
			return_symbol = new Variable(de, alias_name, de->get_type());
			Load* l = new Load((Variable*)return_symbol, sail_inner_lhs,
					offset, name, de, cur_line);
			body->push_back(l);
		}
		else add_drop_temp(sail_inner_lhs);
		add_drop_temp(rhs);
		break;
	}
	case FIELD_REF_EXP:
	{
		field_ref_expression* fre = (field_ref_expression*) lhs;
		expression* inner_lhs = fre->get_inner_expression();
		record_info* field = fre->get_field_info();
		int offset = field->offset;
		string name = field->fname;
		while(inner_lhs->node_type_id == FIELD_REF_EXP)
		{
			field_ref_expression* inner_f = (field_ref_expression*)inner_lhs;
			offset+=inner_f->get_field_info()->offset;
			name = inner_f->get_field_info()->fname+"."+name;
			inner_lhs = inner_f->get_inner_expression();
		}
		Symbol* sail_inner_lhs;
		Variable* lhs_var;
		string prefix;

		if(inner_lhs->node_type_id == DEREF_EXP)
		{
			deref_expression* de = (deref_expression*) inner_lhs;
			inner_lhs = de->get_inner_expression();
			sail_inner_lhs = process_expression(inner_lhs);
			assert(sail_inner_lhs->is_variable());
			lhs_var = (Variable*) sail_inner_lhs;
			if(ALLOW_OFFSETS)
			{
				inst = new Store(sail_inner_lhs, rhs, offset, name, original, cur_line);
				prefix = sail_inner_lhs->to_string(true);
				body->push_back(inst);
				if(return_needed){
					string alias_name = prefix + "." +fre->get_field_info()->fname;
					return_symbol = new Variable(fre, alias_name, fre->get_type());
					Load* fread = new Load((Variable*)return_symbol,
							lhs_var, offset, name, fre, cur_line);
					body->push_back(fread);
				}
				else add_drop_temp(sail_inner_lhs);
			}
			else
			{
				return_symbol  = process_complex_store(lhs, rhs, stmt, return_needed);
			}
		}
		else if(inner_lhs->node_type_id==ARRAY_REF_EXP)
		{
			return_symbol  = process_complex_store(lhs, rhs, stmt, return_needed);
		}
		else
		{
			sail_inner_lhs = process_expression(inner_lhs);
			assert(sail_inner_lhs->is_variable());
			lhs_var = (Variable*) sail_inner_lhs;
			inst = new FieldRefWrite(lhs_var, rhs, name, offset, original, cur_line);
			prefix = sail_inner_lhs->to_string(true);
			body->push_back(inst);
			if(return_needed){
				string alias_name = prefix + "." +fre->get_field_info()->fname;
				return_symbol = new Variable(fre, alias_name, fre->get_type());
				FieldRefRead* fread = new FieldRefRead((Variable*)return_symbol,
						lhs_var, fre->get_field_info(), fre, cur_line);
				body->push_back(fread);
			}
			else add_drop_temp(sail_inner_lhs);

		}



		add_drop_temp(rhs);
		break;
	}
	case ARRAY_REF_EXP:
	{
		array_ref_expression* are = (array_ref_expression*) lhs;
		expression* il_array_exp = are->get_array_expression();

		if(il_array_exp->node_type_id != ARRAY_REF_EXP
				&& il_array_exp->node_type_id != FIELD_REF_EXP)

		{
			Symbol* sail_array = process_expression(il_array_exp);
			assert(sail_array->is_variable());
			Variable* array_var = (Variable*) sail_array;
			expression* il_index = are->get_index_expression();
			Symbol* sail_index = process_expression(il_index);
			inst = new ArrayRefWrite(array_var, rhs, sail_index,
					0, "", original, cur_line);
			body->push_back(inst);
			if(return_needed){
				string alias_name = array_var->to_string(true) + "[" +
					sail_index->to_string(true) + "]";
				return_symbol = new Variable(are, alias_name, are->get_type());
				ArrayRefRead* arrayRead = new ArrayRefRead((Variable*)return_symbol,
						array_var, sail_index, 0, "", are, cur_line);
				body->push_back(arrayRead);
			}
			add_drop_temp(sail_array);
			add_drop_temp(sail_index);

		}

		else {
			return_symbol = process_complex_store(are, rhs, stmt, return_needed);

		}
		break;
	}
	default:
	{
		cout << "ASSERTION FAILURE: " <<
			"Unexpected expression in set_instruction with node id: " <<
			lhs->node_type_id << endl;
		assert(false);
	}

	}


	return return_symbol;

}


Symbol* Function::calculate_complex_address(expression* lhs_exp,
		il::node* stmt)
{
	vector<expression*> expressions;
	expression* cur = lhs_exp;
	while(true)
	{
		if(cur->node_type_id != ARRAY_REF_EXP
					&& cur->node_type_id != FIELD_REF_EXP) break;

		if(cur->node_type_id == FIELD_REF_EXP){
			expressions.push_back(cur);
			field_ref_expression* fre = (field_ref_expression*)cur;
			cur = fre->get_inner_expression();
		}
		else {
			expressions.push_back(cur);
			array_ref_expression* are = (array_ref_expression*) cur;
			cur = are->get_array_expression();
		}
	}
	pointer_type* ptr_type = pointer_type::make(cur->get_type(), "");
	addressof_expression * adr = new addressof_expression(cur, ptr_type,
			cur->get_location() );

	Symbol* cur_symbol = process_expression(adr);
	for(int i = expressions.size()-1; i>=0; i--) {
		expression* cur = expressions[i];
		Symbol *offset;
		if(cur->node_type_id == FIELD_REF_EXP)
		{
			field_ref_expression* fre = (field_ref_expression*)cur;
			type* field_type = fre->get_field_info()->t;
			ptr_type = pointer_type::make(field_type, "");
			offset = new Constant(fre->get_field_info()->offset/8, true, 4);
		}
		else if(cur->node_type_id == ARRAY_REF_EXP)
		{
			array_ref_expression* are = (array_ref_expression*)cur;
			type* t = are->get_array_expression()->get_type();
			assert(t->is_array_type());
			Symbol* s = process_expression(are->get_index_expression());

			array_type* at = (array_type*) t;
			int elem_size = at->get_elem_type()->get_size()/8;

			Constant* size = new Constant(elem_size, true, 4);
			string alias_name = int_to_string(elem_size) + "*" +
										s->to_string(true);
			offset = new Variable(alias_name,
					il::get_integer_type());
			Binop* bin = new Binop((Variable*)offset, s, size,
					_MULTIPLY, stmt, cur_line);
			body->push_back(bin);
			add_drop_temp(s);

		}
		else assert(false);

		string alias_name = "&(" + lhs_exp->to_string() +")";
		Variable* new_cur_symbol = new Variable(alias_name, ptr_type);
		Binop *b = new Binop(new_cur_symbol, cur_symbol, offset,
				_PLUS, stmt, cur_line);
		body->push_back(b);
		add_drop_temp(cur_symbol);
		add_drop_temp(offset);
		cur_symbol = new_cur_symbol;
	}
	return cur_symbol;
}

/*
 * A complex store expression is one where we have multiple array references
 * or array references mixed with field accesses, e.g. a[i].f.b[j]
 * In this case, we compute a sum of offsets needed to store into this location
 * and convert it to *(&a + offset_exp) = rhs
 */
Symbol* Function::process_complex_store(expression* lhs_exp, Symbol* rhs,
		il::statement* stmt, bool return_needed)
{

	Symbol *cur_symbol = calculate_complex_address(lhs_exp,  stmt);

	Store* st = new Store(cur_symbol, rhs, 0, "", stmt, cur_line );
	body->push_back(st);

	Symbol* return_symbol = NULL;
	if(return_needed){
		return_symbol = rhs;
	}
	else add_drop_temp(cur_symbol);
	return return_symbol;

}

void Function::process_return_statement(il::statement* stmt)
{
	has_return = true;
	return_statement* ret_stmt = (return_statement*) stmt;
	if(ret_stmt->has_return_value()){
		expression* ret_exp = ret_stmt->get_ret_expression();
		type* ret_type = ret_exp->get_type();
		Symbol* sail_ret = process_expression(ret_exp);
		Variable* return_var = Variable::get_return_variable(ret_type);
		Assignment* ret_assign = new Assignment(return_var, sail_ret,
				stmt, cur_line);
		body->push_back(ret_assign);
		add_drop_temp(sail_ret);
	}
	Jump* end_jump = new Jump(get_return_label(), NULL);
	body->push_back(end_jump);
}

void Function::add_drop_temp(Symbol* s)
{
	if(!ADD_DROP_TEMP_INSTRUCTIONS) return;
	if(s== NULL || !s->is_variable()) return;
	Variable* v = (Variable*) s;
	if(v->is_temp()) {
		DropVariable* dt = new DropVariable(v, true);
		body->push_back(dt);
	}
}

void Function::add_drop_ref(Symbol* s)
{
	if(!ADD_DROP_TEMP_INSTRUCTIONS) return;
	if(s== NULL || !s->is_variable()) return;
	Variable* v = (Variable*) s;
	DropVariable* dt = new DropVariable(v, false);
	body->push_back(dt);

}

//---------------------------------
// G++ hacks helper functions



/*
 * Subtracts 8 from the allocation size requested by
 * operator new[].
 */
il::node* subtract_operator_new_size(il::node* e)
{
	if(e->node_type_id != FUNCTION_CALL_EXP) return NULL;
	il::function_call_expression* fc =
			(function_call_expression*)e;
	if(!fc->is_operator_new()) return NULL;
	if(fc->get_function().find("[]") == string::npos) return NULL;



	assert(fc->get_arguments().size() == 1);
	il::expression* arg = fc->get_arguments()[0];
	arg = new il::binop_expression(arg, new il::integer_const_exp(8,
			il::get_integer_type(), fc->get_location()), _MINUS,
			il::get_integer_type(), fc->get_location());

	vector<expression*> new_args;
	new_args.push_back(arg);



	il::node* res =  new function_call_expression(fc->get_function(), fc->get_namespace(),
		fc->get_signature(), new_args, fc->get_type(), fc->is_exit_function(),
		fc->is_allocator(), fc->is_operator_new(), fc->is_deallocator(),
		fc->is_constructor(), fc->is_destructor(), fc->get_vtable_lookup_expr(),
		fc->get_vtable_index(), fc->get_location());


	return res;



}

vector<il::variable_expression*> var_exps;

il::node* find_all_var_exps(il::node* e)
{
	if(e->node_type_id == VARIABLE_EXP)
	{
		il::variable_expression* ve = (il::variable_expression*) e;
		var_exps.push_back(ve);
	}
	if(e->node_type_id == VARIABLE_DECL)
	{
		il::variable_declaration* vd = (il::variable_declaration*) e;
		var_exps.push_back(new variable_expression(vd->get_variable(),
				vd->get_location()));
	}
	return NULL;
}

il::function_call_expression* destructor_call = NULL;

il::node* find_destructor(il::node* e)
{
	if(e->node_type_id == FUNCTION_CALL_EXP)
	{
		il::function_call_expression* fce =
				(il::function_call_expression*)e;
		if(fce->is_destructor())
			destructor_call = fce;
	}
	return NULL;
}


static int label_num = 0;
static int var_num = 0;


statement* make_conditional_jump_to_loop_end(expression* cond,  label* end_label,
		location loc)
{
	if(cond == NULL) {
		return NULL;
	}


	// 1) Make binop for cond == 0
	il::integer_const_exp* zero_exp =
			new il::integer_const_exp(0, il::get_integer_type(), loc);

	il::binop_expression* binop = new il::binop_expression(cond, zero_exp,
				_EQ, il::get_integer_type(), loc);

	// 2) Make goto END
	il::goto_statement* goto_end = new goto_statement(end_label,
			end_label->get_label(), loc);

	// 3) Make if statement for conditional jump to end
	if_statement* if_stmt = new il::if_statement(binop, goto_end, NULL, loc);

	return if_stmt;
}

statement* make_for_statement(statement* init, expression* cont_cond ,
		expression* inc_exp, statement* body , int line, int column)
{
	il::location loc (line, column);


	vector<statement*> block_body;
	if(init != NULL) block_body.push_back(init);

	// Make begin label
	il::label* begin_label =
			new label("$DELETE_LABEL_BEGIN_"+int_to_string(label_num), loc);
	block_body.push_back(begin_label);

	// Make end label

	il::label* end_label =
			new label("$DELETE_LABEL_END_"+int_to_string(label_num++), loc);

	if(cont_cond != NULL)
	{
		statement* cond_jump = make_conditional_jump_to_loop_end(cont_cond,
				end_label, loc);
		block_body.push_back(cond_jump);
	}

	if(body != NULL) block_body.push_back(body);


	if(inc_exp != NULL) {
		il::expression_instruction* inc_inst =
				new il::expression_instruction(inc_exp);
		block_body.push_back(inc_inst);
	}

	// Make a "goto BEGIN" instruction
	il::goto_statement* goto_begin = new il::goto_statement(begin_label,
			begin_label->get_label(), loc);
	block_body.push_back(goto_begin);


	// Push End label
	block_body.push_back(end_label);

	// Finally, make the block
	vector<variable_declaration*> decls;
	il::block* b = new il::block(decls, block_body);

	return b;


}

map<string, Variable*>& Function::get_cxx_zvg_vars()
{
	return cxx_zgv_vars;
}


Symbol* Function::process_expression(il::expression* exp)
{



	exp->assert_expression();
	if((int)exp->get_location().line_start > 0){
		//this->cur_line = exp->get_location().line_start;
		if(this->first_line < 0){
			first_line = exp->get_location().line_start;;
		}
	}
	int exp_id = exp->node_type_id;
	Symbol* return_symbol = NULL;
	switch(exp_id)
	{
	case ADDRESSOF_EXP:
	{
		addressof_expression* adr_exp = (addressof_expression*) exp;
		type* exp_type = exp->get_type();
		expression* inner_addr = adr_exp->get_inner_expression();
		switch(inner_addr->node_type_id)
		{




		case VARIABLE_EXP:
		{
			Variable* var_symbol =
					new Variable(inner_addr, inner_addr->get_type());
			/*
			 * Special g++ guard variables. According
			 * to C++ ABI 3.0 any variable starting with
			 * _ZGV is assumed to start at 0, so we
			 * need to add this to the global
			 * vars to be initialized.
			 */
			if(var_symbol->get_var_name().find("_ZGV") == 0)
			{
				cxx_zgv_vars[var_symbol->get_var_name()] = var_symbol;
			}

			string alias_name = "&" + var_symbol->to_string(true);
			return_symbol = new Variable(exp, alias_name, exp_type);
			AddressVar* inst =
					new AddressVar((Variable*)return_symbol, var_symbol,
							exp, cur_line);
			body->push_back(inst);
			add_drop_temp(var_symbol);
			break;
		}
		case DEREF_EXP:
		{
			deref_expression* de = (deref_expression*)inner_addr;
			return_symbol = process_expression(de->get_inner_expression());
			break;
		}
		case FIELD_REF_EXP:
		case ARRAY_REF_EXP:
		{
			return_symbol = calculate_complex_address(inner_addr,  exp);
			break;
		}
		case STRING_CONST_EXP:
		{
			return_symbol = new Variable(exp, exp->to_string(), exp_type);
			string_const_exp* sc = (string_const_exp*)inner_addr;
			AddressString* as = new AddressString((Variable*)return_symbol,
					sc, exp, cur_line);
			body->push_back(as);
			break;
		}
		case BLOCK_EXP:
		{
			Symbol* var_symbol = process_expression(inner_addr);
			assert(var_symbol != NULL);
			string alias_name = "&" + var_symbol->to_string(true);
			return_symbol = new Variable(exp, alias_name, exp_type);
			AddressVar* inst =
					new AddressVar((Variable*)return_symbol, var_symbol,
							exp, cur_line);
			body->push_back(inst);
			add_drop_temp(var_symbol);
			break;

		}
		default:
		{
			cout << "WEIRD ADDRESS CASE: " << exp->to_string() << endl;
			Symbol* c = this->process_expression(inner_addr);
			string var_str = c->to_string(true);
			Variable* temp = new Variable(var_str, inner_addr->get_type());
			Assignment* assign = new Assignment(temp, c, inner_addr, cur_line);
			body->push_back(assign);
			string alias_name = "&" + var_str;
			return_symbol = new Variable(exp, alias_name, exp_type);
			AddressVar* inst =
					new AddressVar((Variable*)return_symbol, temp,
							exp, cur_line);
			body->push_back(inst);
			add_drop_temp(temp);
			break;
		}

		}

		break;
	}

	case ARRAY_REF_EXP:
	{
		array_ref_expression* are = (array_ref_expression*) exp;
		Symbol* index_exp = process_expression(are->get_index_expression());
		while(are->get_array_expression()->node_type_id == ARRAY_REF_EXP){
			array_ref_expression* inner =
				(array_ref_expression*)are->get_array_expression();
			Symbol* new_index = process_expression(inner->get_index_expression());
			assert(inner->get_type()->is_array_type());
			array_type* at = (array_type*)inner->get_type();

			int elem_size = at->get_size()/
			at->get_elem_type()->get_size();
			Constant* c = new Constant(elem_size, 4, true);
			Variable* mult_res = make_multiply_binop(new_index, c);
			index_exp = make_add_binop(mult_res, index_exp);
			are = inner;
		}

		Symbol* array_exp = process_expression(are->get_array_expression());
		assert(array_exp->is_variable());
		string alias_name = array_exp->to_string(true) + "[" +
			index_exp->to_string(true) + "]";
		return_symbol = new Variable(exp, alias_name, exp->get_type());
		ArrayRefRead* inst = new ArrayRefRead((Variable*)return_symbol,
				(Variable*) array_exp, index_exp, 0, "", exp, cur_line);
		body->push_back(inst);
		add_drop_temp(array_exp);
		add_drop_temp(index_exp);
		break;
	}



	case BINOP_EXP:
	{

		binop_expression* bin = (binop_expression*) exp;
		if(bin->get_operator() == il::_LOGICAL_AND)
		{
			Symbol* op1 = process_expression(bin->get_first_operand());

			if(op1->is_constant())
			{
				Constant *c = (Constant*)op1;
				if(c->get_integer() == 0){
					return_symbol = c;
					break;
				}
				return_symbol = process_expression(bin->get_second_operand());
				break;
			}
			Variable* v1 = (Variable*)op1;
			string alias_name = "!" + v1->to_string(true);
			Variable *not_v1 = new Variable(alias_name, v1->get_type());
			Unop* u = new Unop(not_v1, v1, _LOGICAL_NOT, NULL, cur_line);
			body->push_back(u);
			Label* then_label = get_fresh_label();
			Label* end_label = get_fresh_label();
			Branch* b = new Branch(v1, not_v1, then_label, end_label, NULL);
			body->push_back(b);
			body->push_back(then_label);
			Symbol *op2 = process_expression(bin->get_second_operand());
			Assignment* a = new Assignment(v1, op2, NULL, cur_line);
			body->push_back(a);
			body->push_back(end_label);
			add_drop_temp(op2);
			add_drop_temp(not_v1);
			v1->set_original(NULL);
			return_symbol = v1;
			break;
		}
		else if(bin->get_operator() == il::_LOGICAL_OR)
		{
			Symbol* op1 = process_expression(bin->get_first_operand());

			if(op1->is_constant())
			{
				Constant *c = (Constant*)op1;
				if(c->get_integer() != 0){
					return_symbol = c;
					break;
				}
				return_symbol = process_expression(bin->get_second_operand());
				break;
			}
			Variable* v1 = (Variable*)op1;
			string alias_name = "!" + v1->to_string(true);
			Variable *not_v1 = new Variable(alias_name, v1->get_type());
			Unop* u = new Unop(not_v1, v1, _LOGICAL_NOT, NULL, cur_line);
			body->push_back(u);
			Label* then_label = get_fresh_label();
			Label* end_label = get_fresh_label();
			Branch* b = new Branch(not_v1, v1, then_label, end_label, NULL);
			body->push_back(b);
			body->push_back(then_label);
			Symbol *op2 = process_expression(bin->get_second_operand());
			Assignment* a = new Assignment(v1, op2, NULL, cur_line);
			body->push_back(a);
			body->push_back(end_label);
			add_drop_temp(op2);
			add_drop_temp(not_v1);
			v1->set_original(NULL);
			return_symbol = v1;
			break;
		}
		else
		{
			il::binop_type bt = bin->get_operator();
			Symbol* mod_res = convert_to_mod_expression(bin);
			if(mod_res != NULL) return mod_res;

			Symbol* op1 = process_expression(bin->get_first_operand());
			Symbol* op2 = process_expression(bin->get_second_operand());


			string alias_name = op1->to_string(true) +
					binop_expression::binop_to_string(bt) + op2->to_string(true);

			return_symbol = new Variable(exp, alias_name, exp->get_type());
			Binop* inst = new Binop((Variable*)return_symbol, op1, op2,
					bt, exp, cur_line);
			body->push_back(inst);
			add_drop_temp(op1);
			add_drop_temp(op2);
		}
		break;
	}
	case UNOP_EXP:
	{
		unop_expression* unop = (unop_expression*) exp;
		if(unop->has_side_effect()){
			return_symbol = process_unop_side_effect(unop);
			break;
		}
		Symbol* op = process_expression(unop->get_operand());
		il::unop_type ut = unop->get_operator();

		string alias_name = unop_expression::unop_to_string(ut, NULL) +
				op->to_string(true);
		return_symbol = new Variable(exp, alias_name, exp->get_type());
		Unop* inst = new Unop((Variable*)return_symbol, op, ut, exp, cur_line);
		body->push_back(inst);
		add_drop_temp(op);
		break;
	}

	case BLOCK_EXP:
	{
		block_expression* be = (block_expression*) exp;



		Symbol* res = process_statement(be->get_block(), true);
		if(res == NULL) {
			res = new Variable(new il::variable_expression(
					il::variable::get_unmodeled_var(), be->get_location()),
					il::get_integer_type());
		}
		return_symbol = res;


		break;
	}

	case BUILTIN_EXP:
	{
		builtin_expression* be = (builtin_expression*) exp;
		/*
		 * For now we only handle VA_ARG as a builtin expression.
		 */
		assert(be->get_builtin_type() == _VA_ARG);
		type* arg_type = be->get_type(); //e.g. va_arg(_, int)
		vector<expression*>& args = be->get_arguments();
		Symbol* arg1 = process_expression(args[0]);
		Constant* arg2 = new Constant(arg_type->get_size(), false, 4);
		vector<Symbol*>* sail_args = new vector<Symbol*>();
		sail_args->push_back(arg1);
		sail_args->push_back(arg2);

		string alias_name = "va_arg(" + arg1->to_string(true) + ", "
				+ arg2->to_string() +")";
		return_symbol = new Variable(exp, alias_name, be->get_type());
		FunctionCall* inst = new FunctionCall((Variable*) return_symbol,
				"va_arg", il::namespace_context(),
				NULL, sail_args, false, false, false, false, exp, cur_line);
		body->push_back(inst);
		add_drop_temp(arg1);
		break;
	}

	case CAST_EXP:
	{
		cast_expression* ce = (cast_expression*) exp;
		Symbol* inner = process_expression(ce->get_inner_expression());
		/*
		 * Can only happen if inner has no value but exits (exit fn)
		 */
		if(inner == NULL)
		{
			inner = new Constant(0);
		}
		type* cast_type = ce->get_type();
		string alias_name = "("+ cast_type->to_string() + ")" +
				inner->to_string(true);
		return_symbol = new Variable(exp, alias_name, exp->get_type());
		Cast* inst = new Cast((Variable*) return_symbol, inner,cast_type,
				exp, cur_line);
		body->push_back(inst);
		add_drop_temp(inner);
		break;
	}

	case COMPLEX_CONST_EXP:
	case FIXED_CONST_EXP:
	case REAL_CONST_EXP:
	case INTEGER_CONST_EXP:
	case VECTOR_CONST_EXP:
	case STRING_CONST_EXP:
	{
		return_symbol = new Constant((const_expression*)exp);
		break;
	}

	case CONDITIONAL_EXP:
	{
		return_symbol = process_conditional_expression(exp);
		break;
	}

	case VARIABLE_EXP:
	{

		Variable* v = new Variable(exp, exp->get_type());
		/*
		 * Special g++ guard variables. According
		 * to C++ ABI 3.0 any variable starting with
		 * _ZGV is assumed to start at 0, so we
		 * need to add this to the global
		 * vars to be initialized.
		 */
		if(v->get_var_name().find("_ZGV") == 0)
		{
			cxx_zgv_vars[v->get_var_name()] = v;
		}
		return_symbol = v;
		break;
	}

	case DEREF_EXP:
	{
		deref_expression* de = (deref_expression*) exp;
		Symbol* inner = process_expression(de->get_inner_expression());

		string alias_name = "*" + inner->to_string(true);
		return_symbol = new Variable(exp, alias_name, exp->get_type());
		Load* inst = new Load((Variable*)return_symbol, inner, 0, "",
				exp, cur_line);
		body->push_back(inst);
		add_drop_temp(inner);
		break;
	}
	case EXPR_LIST_EXP:
	{
		/**
		 * Expression lists evaluate their nested expressions in
		 * left-to-right order, but the result of their own evaluation
		 * is the rightmost one.
		 */
		expr_list_expression* list = (expr_list_expression*) exp;
		vector<expression*> & exprs = list->get_inner_expressions();


		/*
		 * G++ hack: In g++, Foo* f = new foo[3] will
		 * allocate 8 extra bytes and store the size of the
		 * array in the first 8 bytes of the buffer. We really do
		 * not want to see this in SAIL, so we match on this special form
		 * of esxp list here and change it back.
		 */
		if(exprs.size() >=2 &&
			exprs[0]->to_string().find("$anonymous") != string::npos &&
			exprs[0]->to_string().find("operator new []") != string::npos &&
			exprs[1]->to_string().find("long unsigned int*") != string::npos)
		{

			bool found_new = false;
			if(exprs[0]->node_type_id == BLOCK_EXP){
				il::block_expression* b = (il::block_expression* )exprs[0];
				il::block* bb = b->get_block();
				//cout << "eeee 1" << b->to_string() << endl;

				if(bb->get_var_declarations().size() == 1)
				{
					//cout << "eeee 2" << endl;
					il::expression* e = bb->get_var_declarations()[0]->get_initializer();
					if(e != NULL)
					{
						if(e->node_type_id == CAST_EXP)
						{
							e = ((il::cast_expression*)e)->get_inner_expression();
						}
						//cout << "node type: " << e->node_type_id << endl;
						if(e->node_type_id == FUNCTION_CALL_EXP)
						{
							il::function_call_expression* fe =
									(il::function_call_expression*)e;
							if(fe->is_operator_new() &&
									fe->to_string().find("[]")!= string::npos)
							{
								found_new =true;
							}
						}
					}
				}

			}
			if(found_new)
			{
				//cout << "**##########******** FOUND BAD new EXP " << list->to_string()
				//		<< endl;

				list = (expr_list_expression* )
						list->substitute(subtract_operator_new_size);

				exprs = list->get_inner_expressions();

				//cout << "**##########******** MODIFIED BAD new EXP " << list->to_string()
				//					<< endl;


				/*
				 * Now, remove the second entry from this list -- this
				 * is the one that actually stores the size. Thanks, g++!
				 */
				vector<expression*> new_exps;
				for(unsigned int i=0; i < exprs.size(); i++)
				{
					if(i == 1){
						var_exps.clear();
						expression* e = exprs[i];
						e->substitute(find_all_var_exps);

						assert(var_exps.size() == 2);
						set_instruction* new_s = new il::set_instruction(var_exps[0],
								var_exps[1], e->get_location(), false);


						expression* new_e = new il::modify_expression(new_s, false,
								e->get_location());
						new_exps.push_back(new_e);

						var_exps.clear();


						continue;
					}
					new_exps.push_back(exprs[i]);
				}
				list = new expr_list_expression(new_exps, list->get_type(),
						list->get_location());
			}


		}

		if(exprs.size()>2)
		{
			il::expression* last = exprs[exprs.size()-1];
			if(last->node_type_id == FUNCTION_CALL_EXP)
			{
				il::function_call_expression* fc =
						(il::function_call_expression*)last;
				if(fc->is_deallocator() &&
						fc->to_string().find("[]") != string::npos)
				{
					//cout << "**** BAD delete[] expression: " << endl;
					//cout << list->to_string() << endl;
					//cout << "**** BAD delete[] expression END : " << endl;

					expression* first = exprs[0];
					assert(first->node_type_id == MODIFY_EXP);
					il::set_instruction* si =
							((il::modify_expression*)first)->get_set_instruction();


					expression* rhs =si->get_rhs();
					assert(rhs->node_type_id == BINOP_EXP);
					il::binop_expression* be = (il::binop_expression*)rhs;
					expression* ptr = be->get_first_operand();



					vector<il::type*> arg_t;
					arg_t.push_back(ptr->get_type());
					il::type* sig = il::function_type::make(il::get_integer_type(),
							arg_t, false);
					vector<il::expression*> args;
					args.push_back(ptr);
					il::function_call_expression* new_rhs =
							new il::function_call_expression("buffer_size",
									namespace_context(), sig, args,
									il::get_integer_type(), false,
									false, false, false, false, false, NULL,
									0, ptr->get_location());


					location loc(-1, -1);
					string i_name = "$delete_" + int_to_string(var_num++);

					il::variable* v = new il::variable(i_name, namespace_context(),
							NULL,
							LOCAL, NONE, NULL, il::get_integer_type());
					il::variable_declaration* vd =
							new il::variable_declaration(v, namespace_context(),
									NULL, loc);
					v->set_declaration(vd);

					il::variable_expression* ve =
							new il::variable_expression(v, loc);
					il::set_instruction* init_stmt = new set_instruction(ve,
							new il::integer_const_exp(0, il::get_integer_type(),
									loc), loc, false);



					destructor_call = NULL;
					exprs[1]->substitute(find_destructor);
					assert(destructor_call != NULL);
					assert(ptr->get_type()->is_pointer_type());
					il::type* obj_type = ptr->get_type()->get_inner_type();
					while(obj_type->is_array_type())
						obj_type = obj_type->get_inner_type();


					il::expression* div_rhs = new il::binop_expression(new_rhs,
							new il::integer_const_exp(obj_type->get_size()/8,
									il::get_integer_type(),loc), _DIV,
									il::get_integer_type(), loc
					);


					il::expression* cond_cont = new il::binop_expression(ve,
							div_rhs, _LT, il::get_integer_type(), loc);



					il::expression * cur = new binop_expression(ve,
						new il::integer_const_exp(obj_type->get_size()/8,
								il::get_integer_type(),loc), _MULTIPLY,
								il::get_integer_type(),loc
					);
					cur = new binop_expression(ptr, cur, _POINTER_PLUS,
							ptr->get_type(), loc);
					//cout << "** CUR: " << cur->to_string() << endl;

					vector<il::expression*> d_args;
					d_args.push_back(cur);


					il::function_call_expression* d_call =
							new il::function_call_expression(
									destructor_call->get_function(),
									destructor_call->get_namespace(),
									destructor_call->get_signature(),
									d_args, destructor_call->get_type(),
									destructor_call->is_exit_function(),
									false, false, false, false, true,
									destructor_call->get_vtable_lookup_expr(),
											destructor_call->get_vtable_index(),
											loc);
					il::expression_instruction* body =
							new il::expression_instruction(d_call);


					il::expression* inc_exp =
							new il::unop_expression(ve, _POSTINCREMENT,
									il::get_integer_type(), loc);





					//cout << "**INIT: " << init_stmt->to_string() << endl;
					//cout << "** COND: " << cond_cont->to_string() << endl;
					//cout << "*** BODY: " << body->to_string() << endl;
					//cout << "*** INC: " << inc_exp->to_string() << endl;

					statement* res =
							make_for_statement(init_stmt, cond_cont ,
							inc_exp, body , -1, -1);
					vector<expression*> new_exps;
					{
						vector<variable_declaration*> vds;
						vector<statement*>  sss;
						sss.push_back(res);

						vector<il::expression*> delete_args;
						delete_args.push_back(ptr);

						il::block* my_block = new il::block(vds, sss);
						il::block_expression* be =
								new il::block_expression(my_block, NULL,
								list->get_type(), list->get_location());
						new_exps.push_back(be);
						il::function_call_expression* delete_call =
							new il::function_call_expression(
							"operator delete []",namespace_context(),
							destructor_call->get_signature(),
							delete_args, il::get_void_type(), false, false,
							false, true, false, false, NULL, 0,	loc);
						new_exps.push_back(delete_call);


					}

					list = new expr_list_expression(new_exps, list->get_type(),
								list->get_location());
					//cout << "New list" << list->to_string() << endl;




				}

			}


		}

		exprs = list->get_inner_expressions();



		for(int i=0; i<(int)exprs.size(); i++)
		{
			expression* cur_exp = exprs[i];
			Symbol* res = process_expression(cur_exp);
			if(i==(int)exprs.size()-1) return_symbol = res;
			else add_drop_temp(res);
		}
		break;
	}

	case FIELD_REF_EXP:
	{
		field_ref_expression* fre = (field_ref_expression*) exp;
		expression* inner_rhs = fre->get_inner_expression();
		record_info* field = fre->get_field_info();
		int offset = field->offset;
		string name = field->fname;
		while(inner_rhs->node_type_id == FIELD_REF_EXP)
		{
			field_ref_expression* inner_f = (field_ref_expression*)inner_rhs;
			offset+=inner_f->get_field_info()->offset;
			name = inner_f->get_field_info()->fname+"."+name;
			inner_rhs = inner_f->get_inner_expression();
		}
		Symbol* sail_inner_rhs = NULL;
		Variable* rhs_var = NULL;
		Instruction* inst = NULL;
		if(ALLOW_OFFSETS && inner_rhs->node_type_id == DEREF_EXP)
		{
			deref_expression* de = (deref_expression*) inner_rhs;
			inner_rhs = de->get_inner_expression();
			sail_inner_rhs = process_expression(inner_rhs);
			assert(sail_inner_rhs->is_variable());
			rhs_var = (Variable*) sail_inner_rhs;
			string alias_name = "*" + rhs_var->to_string(true) + "."+name;
			return_symbol = new Variable(exp, alias_name, exp->get_type());
			inst = new Load((Variable*) return_symbol, sail_inner_rhs,
					offset, name, exp, cur_line);
			body->push_back(inst);
			add_drop_temp(sail_inner_rhs);
		}
		else if(ALLOW_OFFSETS && inner_rhs->node_type_id == ARRAY_REF_EXP)
		{

			array_ref_expression* are = (array_ref_expression*) inner_rhs;
			expression* array_exp = are->get_array_expression();
			expression* index_exp = are->get_index_expression();
			Symbol* index_sym = process_expression(index_exp);

			while(array_exp->node_type_id == ARRAY_REF_EXP){
				array_ref_expression* inner =
					(array_ref_expression*)array_exp;

				Symbol* new_index = process_expression(inner->get_index_expression());
				assert(inner->get_type()->is_array_type());
				array_type* at = (array_type*)inner->get_type();

				int elem_size = at->get_size()/
				at->get_elem_type()->get_size();
				Constant* c = new Constant(elem_size, 4, true);
				Variable* mult_res = make_multiply_binop(new_index, c);
				index_sym = make_add_binop(mult_res, index_sym);
				array_exp = inner->get_array_expression();
			}

			Symbol* array_sym = process_expression(array_exp);

			assert(array_sym->is_variable());
			Variable* array_var = (Variable*) array_sym;
			string alias_name = "*" + array_var->to_string(true) +
				"[" + index_sym->to_string(true) + "]"+"."+name;
			return_symbol = new Variable(exp, alias_name, exp->get_type());
			inst = new ArrayRefRead((Variable*) return_symbol, array_var,
					index_sym, offset, name, exp, cur_line);
			body->push_back(inst);
			add_drop_temp(array_var);
			add_drop_temp(index_sym);
		}
		else
		{
			sail_inner_rhs = process_expression(inner_rhs);
			assert(sail_inner_rhs->is_variable());
			rhs_var = (Variable*) sail_inner_rhs;
			string alias_name = rhs_var->to_string(true) + "."+name;
			return_symbol = new Variable(exp, alias_name, exp->get_type());
			inst = new FieldRefRead((Variable*)return_symbol,
					rhs_var, offset, name, exp, cur_line);
			body->push_back(inst);
			add_drop_temp(sail_inner_rhs);
		}


		break;


	}

	case FUNCTION_ADDRESS_EXP:
	{
		function_address_expression* fae = (function_address_expression*) exp;
		return_symbol = new Variable(exp, exp->get_type());
		AddressLabel* inst = new AddressLabel((Variable*)return_symbol,
				fae->get_function_name(), true, exp, cur_line);
		body->push_back(inst);
		break;
	}

	case FUNCTION_CALL_EXP:
	{
		return_symbol = process_function_call(exp);
		break;
	}

	case FUNCTION_PTR_CALL_EXP:
	{
		return_symbol = process_function_pointer_call(exp);
		break;
	}

	case INITIALIZER_LIST_EXP:
	{
		return_symbol = process_initializer_list(exp);
		break;
	}

	case MODIFY_EXP:
	{
		modify_expression* modify = (modify_expression*) exp;
		return_symbol = process_set_instruction(modify->get_set_instruction()
				, true);
		break;
	}

	default:
		cout << "UNEXPECTED EXPRESSION: " << exp->to_string() <<
		" expression id: " << exp_id << endl;
		assert(false);
	}



	return return_symbol;

}



Symbol* Function::convert_to_mod_expression(il::binop_expression* exp)
{

	if(exp->get_operator() != _EQ) {
		return NULL;
	}
	make_first_op_constant(exp);
	il::expression* op1 = exp->get_first_operand();
	il::expression* op2 = exp->get_second_operand();

	if(op1->node_type_id!= INTEGER_CONST_EXP) {
		return NULL;
	}

	il::integer_const_exp* c = (il::integer_const_exp*) op1;
	if(c->get_integer() != 0) return NULL;
	il::expression* op2_stripped = strip_casts(op2);
	if(op2_stripped == NULL) return NULL;
	if(op2_stripped->node_type_id != BINOP_EXP ||
			((il::binop_expression*) op2_stripped)->
			get_operator() != il::_BITWISE_AND) {
		return NULL;
	}
	il::binop_expression* bin = (il::binop_expression*) op2_stripped;
	make_first_op_constant(bin);

	il::expression* and_op1 = bin->get_first_operand();
	il::expression* and_op2 = bin->get_second_operand();
	if(and_op1->node_type_id != INTEGER_CONST_EXP) {
		return NULL;
	}
	il::integer_const_exp* c_exp = (il::integer_const_exp*) and_op1;
	int val = c_exp->get_integer();
	// not of the form 2^c -1, so can't play bitwise and trick.
	if(((val+1) & val)!=0) {
		return NULL;
	}

	Symbol* mod_op1 = process_expression(and_op2);
	Constant* mod_const = new Constant(val+1, true, 4);
	string mod_alias_name = mod_op1->to_string(true) +
			"%" + mod_const->to_string(true);
	Symbol* mod_res = new Variable(op2, mod_alias_name, op2->get_type());
	Binop* inst = new Binop((Variable*)mod_res, mod_op1, mod_const,
			_MOD, op2, cur_line);
	body->push_back(inst);
	add_drop_temp(mod_op1);

	/*
	 * Now make up the mod == 0 expression
	 */
	Constant* zero = new Constant(0, true, 4);
	string eq_alias_name = op2->to_string() + "=" + zero->to_string();
	Symbol* res = new Variable(exp, eq_alias_name, exp->get_type());
	Binop* eq_inst = new Binop((Variable*)res, mod_res, zero,
			_EQ, exp, cur_line);
	body->push_back(eq_inst);
	add_drop_temp(mod_res);
	return res;






}

void Function::make_first_op_constant(il::binop_expression* exp)
{
	il::expression* op1 = exp->get_first_operand();
	il::expression* op2 = exp->get_second_operand();
	if(op1->node_type_id != INTEGER_CONST_EXP)
	{
		il::expression* t = op1;
		op1 = op2;
		op2 = t;
	}
	exp->set_first_operand(op1);
	exp->set_second_operand(op2);

}

Variable* Function::process_initializer_list(il::expression* exp)
{
	Variable* return_symbol = NULL;
	initializer_list_expression* ile = (initializer_list_expression*) exp;
	type* t = ile->get_type();
	if(t->is_record_type()) {
		return_symbol = process_record_initializer_list(ile);
	}
	else{
		assert(t->is_constarray_type() || t->is_vararray_type());
		return_symbol = process_array_initializer_list(ile);
	}
	return return_symbol;
}

Variable* Function::process_record_initializer_list
		(il::initializer_list_expression* init_exp)
{
	type* t = init_exp->get_type();
	assert(t->is_record_type());
	record_type* rt = (record_type*) t;
	vector<record_info*>& fields = rt->get_fields();
	vector<expression*>& init_list = init_exp->get_init_list();
	Variable* v = new Variable(NULL, init_exp->get_type());
	for(int i= 0; i<(int)init_list.size(); i++)
	{
		/*
		 * gcc lets this compile; but this should never happen!
		 * TODO: Figure out what the actual semantics of such invalid
		 * initialization lists are and report error if necessary.
		 */
		if(i>=(int)fields.size()) break;
		Symbol* cur_exp = process_expression(init_list[i]);
		record_info* cur_field = fields[i];

		FieldRefWrite* frw = new FieldRefWrite(v, cur_exp, cur_field,
				NULL, cur_line);
		body->push_back(frw);
		add_drop_temp(cur_exp);

	}
	return v;
}
Variable* Function::process_array_initializer_list
		(il::initializer_list_expression* init_exp)
{
	type* t = init_exp->get_type();
	assert(t->is_constarray_type() || t->is_vararray_type());
	vector<expression*>& init_list = init_exp->get_init_list();
	Variable* v = new Variable(NULL, t);
	for(int i=0; i<(int)init_list.size(); i++)
	{
		Symbol* cur_exp = process_expression(init_list[i]);
		Constant* c = new Constant(i, true, 4);
		ArrayRefWrite* arw = new ArrayRefWrite(v, cur_exp, c, 0, "",
				NULL, cur_line);
		body->push_back(arw);
		add_drop_temp(cur_exp);
	}
	return v;
}

Variable* Function::process_function_pointer_call(il::expression* exp)
{
	Variable* return_symbol = NULL;
	function_pointer_call_expression* fce =
					(function_pointer_call_expression*) exp;
	Symbol* fun_ptr = process_expression(fce->get_function_pointer());
	assert(fun_ptr->is_variable());
	vector<expression*>& args = fce->get_arguments();
	vector<Symbol*>* sail_args = new vector<Symbol*>();
	for(int i=0; i<(int) args.size(); i++)
	{
		Symbol* cur_arg = process_expression(args[i]);
		sail_args->push_back(cur_arg);
	}
	if(fce->has_return()){
		return_symbol = new Variable(exp, exp->get_type());
		Variable* ret_var = ((Variable*)return_symbol);
		ret_var->set_alias_name(ret_var->to_string());
	}
	else{
		return_symbol = NULL;
	}
	FunctionPointerCall* inst = new FunctionPointerCall((Variable*)
			return_symbol, (Variable*)fun_ptr, sail_args, exp, cur_line);
	body->push_back(inst);
	add_drop_temp(fun_ptr);
	for(int i=0; i<(int) sail_args->size(); i++)
	{
		add_drop_temp((*sail_args)[i]);
	}
	return return_symbol;
}

Variable* Function::process_function_call(il::expression* exp)
{
	Variable* return_symbol = NULL;
	function_call_expression* fce = (function_call_expression*) exp;
	string name = fce->get_function();
	if(name == STATIC_ASSERT_ID && fce->get_arguments().size()==1){
		expression* arg = fce->get_arguments()[0];
		Symbol* assertion = process_expression(arg);
		StaticAssert* sa = new StaticAssert(assertion, exp, cur_line);
		body->push_back(sa);
		add_drop_temp(assertion);
		return NULL;
	}
	if(name == ASSUME_ID && fce->get_arguments().size()==1){
		expression* arg = fce->get_arguments()[0];
		Symbol* assumption = process_expression(arg);
		Assume* sa = new Assume(assumption, exp, cur_line);
		body->push_back(sa);
		add_drop_temp(assumption);
		return NULL;
	}
	if(name == ASSUME_SIZE_ID && fce->get_arguments().size()==2){
		expression* buffer_exp = fce->get_arguments()[0];
		expression* size_exp = fce->get_arguments()[1];
		Symbol* buffer = process_expression(buffer_exp);
		Symbol* size = process_expression(size_exp);
		AssumeSize* as = new AssumeSize(buffer, size, exp, cur_line);
		body->push_back(as);
		add_drop_temp(buffer);
		add_drop_temp(size);
		return NULL;
	}

	/*
	 * We always return 1 for the g++ function
	 * CXA_GUARD_ACQUIRE. This function is used to acquire a lock
	 * before initializing static variables.
	 */
	if(name == CXA_GUARD_ACQUIRE)
	{
		Symbol* one = new Constant(1, false, 4);
		Variable* res = new Variable(NULL, il::get_integer_type());
		Assignment* a = new Assignment(res, one, NULL, cur_line);
		body->push_back(a);
		return res;

	}
	/*
	 * This function has the effect of *arg0 = 1;
	 */
	if(name == CXA_GUARD_RELEASE)
	{
		vector<expression*>& args = fce->get_arguments();
		assert(args.size() >= 1);
		Symbol* arg = process_expression(args[0]);
		Symbol* one = new Constant(1, false, 4);
		Store* s = new Store(arg, one, 0, "", NULL, -1);
		body->push_back(s);

		return NULL;

	}


	type* sig = fce->get_signature();
	vector<expression*>& args = fce->get_arguments();
	vector<Symbol*>* sail_args = new vector<Symbol*>();
	for(int i=0; i<(int) args.size(); i++)
	{
		Symbol* cur_arg = process_expression(args[i]);
		sail_args->push_back(cur_arg);
	}
	if(fce->has_return()){
		return_symbol = new Variable(exp, exp->get_type());
		Variable* ret_var = ((Variable*)return_symbol);
		ret_var->set_alias_name(ret_var->to_string());
	}
	else{
		return_symbol = NULL;
	}

	FunctionCall* inst = new FunctionCall((Variable*)return_symbol,
			name, fce->get_namespace(),
			sig, sail_args, fce->is_call_to_virtual(),
			fce->is_allocator(), fce->is_constructor(),
			false,
			exp, cur_line);
	body->push_back(inst);
	for(int i=0; i<(int) sail_args->size(); i++)
	{
		add_drop_temp((*sail_args)[i]);
	}
	return return_symbol;
}

Symbol* Function::process_unop_side_effect(il::unop_expression* unop)
{
	Symbol* return_symbol = NULL;
	switch(unop->get_operator())
	{
	case _PREDECREMENT:
	case _PREINCREMENT:
	{
		return_symbol = process_pre_unop(unop);
		break;
	}
	case _POSTDECREMENT:
	case _POSTINCREMENT:
	{
		return_symbol = process_post_unop(unop);
		break;
	}

	default:
		assert(false);

	}
	return return_symbol;
}

Symbol* Function::process_pre_unop(unop_expression* unop)
{
	binop_type op = (unop->get_operator()== _PREDECREMENT) ? _MINUS : _PLUS;
	expression* operand = unop->get_operand();
	il::integer_const_exp* one;
	if(op == _MINUS && unop->get_type()->is_pointer_type())
	{
		op = _POINTER_PLUS;


		pointer_type *pt = (pointer_type*) unop->get_type();
		one = new il::integer_const_exp(-pt->get_deref_type()->get_size()/8,
				get_integer_type(), unop->get_location());


	}
	else{
		if(unop->get_type()->is_pointer_type()){
			pointer_type *pt = (pointer_type*) unop->get_type();
			one = new il::integer_const_exp(pt->get_deref_type()->get_size()/8,
					get_integer_type(), unop->get_location());
		}

		else one = new il::integer_const_exp(1, get_integer_type(),
				unop->get_location());
	}
	binop_expression* binop = new binop_expression(operand, one, op,
			unop->get_type(), unop->get_location());
	set_instruction* si = new set_instruction(operand, binop,
			unop->get_location(), false);
	Symbol* s = process_set_instruction(si, true, unop);
	return s;

}

Symbol* Function::process_post_unop(unop_expression* unop)
{
	binop_type op = (unop->get_operator()== _POSTDECREMENT) ? _MINUS : _PLUS;
	expression* operand = unop->get_operand();

	Symbol* sail_exp = process_expression(operand);
	Variable* res = new Variable(NULL, operand->get_type());
	Assignment* assign = new Assignment(res, sail_exp, NULL, cur_line);
	body->push_back(assign);

	il::integer_const_exp* one;
	if(op == _MINUS && unop->get_type()->is_pointer_type())
	{
		op = _POINTER_PLUS;
		pointer_type *pt = (pointer_type*) unop->get_type();
		one = new il::integer_const_exp(-pt->get_deref_type()->get_size()/8,
						get_integer_type(), unop->get_location());
	}
	else{

		if(unop->get_type()->is_pointer_type()){
			pointer_type *pt = (pointer_type*) unop->get_type();
			one = new il::integer_const_exp(pt->get_deref_type()->get_size()/8,
					get_integer_type(), unop->get_location());
		}

		else one = new il::integer_const_exp(1, get_integer_type(),
				unop->get_location());
	}

	binop_expression* binop = new binop_expression(operand, one, op,
			unop->get_type(), unop->get_location());
	set_instruction* si = new set_instruction(operand, binop,
			unop->get_location(), false);
	process_set_instruction(si, false, unop);
	add_drop_temp(sail_exp);
	return res;
}

Symbol* Function::process_conditional_expression(expression* exp)
{
	conditional_expression* ce = (conditional_expression*) exp;

	/*
	 * Hack to figure out artificial conditionals introduced by g++
	 * to avoid unnecessary copying.
	 */
	{
		il::expression* then_clause = ce->get_then_clause();
		il::expression* else_clause = ce->get_else_clause();

		if(then_clause->to_string().find("__builtin_memcpy")!=string::npos &&
			else_clause->to_string().find("__builtin_memcpy")==string::npos)
		{
			return process_expression(then_clause);
		}

		if(else_clause->to_string().find("__builtin_memcpy")!=string::npos &&
			then_clause->to_string().find("__builtin_memcpy")==string::npos)
		{
			return process_expression(else_clause);
		}
	}


	Symbol* cond_symbol = process_expression(ce->get_conditional());
	string not_cond_name = "!(" + cond_symbol->to_string(true) +")";
	type* cond_type = ce->get_conditional()->get_type();

	il::node* not_orig = NULL;
	if(cond_symbol->get_original_node()!=NULL)
	{
		il::node* orig = cond_symbol->get_original_node();
		il::expression* orig_e = (il::expression*) orig;
		orig_e->assert_expression();
		not_orig = new il::unop_expression(orig_e, il::_LOGICAL_NOT,
				orig_e->get_type(), orig_e->get_location());
	}
	Variable* not_cond_symbol;
	if(not_orig == NULL)
		not_cond_symbol= new Variable(not_cond_name, cond_type);
	else not_cond_symbol= new Variable(not_orig, cond_type);
	Unop* unop = new Unop(not_cond_symbol, cond_symbol,
			il::_LOGICAL_NOT, NULL, cur_line);
	body->push_back(unop);
	Label* then_label = get_fresh_label();
	Label* else_label = get_fresh_label();
	Label* end_label = get_fresh_label();



	Branch* b = new Branch(cond_symbol, not_cond_symbol,
			then_label, else_label, exp);
	body->push_back(b);
	body->push_back(then_label);
	il::expression* then_exp = ce->get_then_clause();
	il::expression* else_exp = ce->get_else_clause();
	assert(then_exp!=NULL);
	assert(else_exp != NULL);

	Symbol* then_result = process_expression(ce->get_then_clause());

	/*
	 * If either the then result or the else result is NULL,
	 * then this is an expression without a value. This is only
	 * legal for exit functions, in which case the result does not
	 * matter.
	 */
	if(then_result == NULL)
	{
		then_result = new Constant(0);
	}

	string alias_name = cond_symbol->to_string(true) + " ? " +
			then_exp->to_string() + " : "
			+ else_exp->to_string();
	Variable* return_symbol = new Variable(exp, alias_name,
				ce->get_then_clause()->get_type());
	Assignment* assign = new Assignment(return_symbol,
			then_result, exp, cur_line);
	body->push_back(assign);
	body->push_back(new Jump(end_label, NULL));
	body->push_back(else_label);

	Symbol* else_result = process_expression(ce->get_else_clause());


	if(else_result == NULL)
	{
		else_result = new Constant(0);
	}

	Assignment* assign2 = new Assignment((Variable*)return_symbol,
	else_result, exp, cur_line);
	body->push_back(assign2);
	body->push_back(new Jump(end_label, NULL));
	body->push_back(end_label);
	add_drop_temp(then_result);
	add_drop_temp(else_result);
	add_drop_temp(not_cond_symbol);
	return return_symbol;
}

Label* Function::find_or_create_label(il::label* l)
{
	if(label_map[l]!=NULL) return label_map[l];
	Label* sail_label = new Label(l->get_label(), l);
	label_map[l] = sail_label;
	return sail_label;
}

Label* Function::create_default_label()
{
	string name  = "__default" + int_to_string(++default_counter);
	Label* default_label = new Label(name, NULL);
	return default_label;
}


Label* Function::find_or_create_label(il::case_label* l)
{
	if(case_label_map[l]!=NULL) return case_label_map[l];
	Label* sail_label = new Label(l->to_string(), l);
	case_label_map[l] = sail_label;
	return sail_label;
}

bool Function::case_label_exists(il::case_label* l)
{
	return case_label_map[l]!=NULL;
}

Label* Function::get_fresh_label()
{
	string lname = "__label";
	lname += int_to_string(++label_counter);
	Label* fresh_label = new Label(lname, NULL);
	return fresh_label;
}

Label* Function::get_return_label()
{
	if(return_label != NULL) return return_label;
	return_label = new Label("__return_label", NULL);
	return return_label;
}


void Function::process_if_statement(il::statement* stmt)
{
	if_statement* if_stmt = (if_statement*) stmt;
	expression* cond = if_stmt->get_test();
	Symbol* test_var = process_expression(cond);
	string else_var_name = "!(" + test_var->to_string(true) +")";

	il::node* not_orig = NULL;
	if(test_var->get_original_node()!=NULL)
	{
		il::node* orig = test_var->get_original_node();
		il::expression* orig_e = (il::expression*) orig;
		orig_e->assert_expression();
		not_orig = new il::unop_expression(orig_e, il::_LOGICAL_NOT,
				orig_e->get_type(), orig_e->get_location());
	}
	Variable* else_var;
	if(not_orig == NULL)
		else_var = new Variable(else_var_name, cond->get_type());
	else
		else_var = new Variable(not_orig, cond->get_type());

	Unop *uno = new Unop(else_var, test_var, il::_LOGICAL_NOT, NULL, cur_line);
	body->push_back(uno);
	Label* then_label = get_fresh_label();
	Label* else_label = get_fresh_label();
	Label* end_label;
	if(if_stmt->get_else_statement() == NULL)
		end_label = else_label;
	else end_label = get_fresh_label();
	Branch* b = new Branch(test_var, else_var, then_label, else_label, stmt);
	body->push_back(b);
	body->push_back(then_label);
	process_statement(if_stmt->get_if_statement(), false);
	Jump* end_jump = new Jump(end_label, NULL);
	body->push_back(end_jump);

	if(if_stmt->get_else_statement() != NULL){
		body->push_back(else_label);
		process_statement(if_stmt->get_else_statement(), false);
		body->push_back(end_jump);
	}
	body->push_back(end_label);
	add_drop_temp(test_var);
	add_drop_temp(else_var);
}

Variable* Function::make_multiply_binop(Symbol* s1, Symbol* s2)
{
	string alias_name = s1->to_string(true) + "*" + s2->to_string(true);
	Variable* res = new Variable(alias_name, il::get_integer_type());
	Binop* bin = new Binop(res, s1, s2, _MULTIPLY, NULL, cur_line);
	body->push_back(bin);
	add_drop_temp(s1);
	add_drop_temp(s2);
	return res;
}
Variable* Function::make_add_binop(Symbol* s1, Symbol* s2)
{
	string alias_name = s1->to_string(true) + "+" + s2->to_string(true);
	Variable* res = new Variable(alias_name, il::get_integer_type());
	Binop* bin = new Binop(res, s1, s2, _PLUS, NULL, cur_line);
	body->push_back(bin);
	add_drop_temp(s1);
	add_drop_temp(s2);
	return res;
}



Function::~Function()
{
	delete body;
}

/*
 * Summary-unit related functions.
 */
bool Function::is_function()
{
	return true;
}
bool Function::is_superblock()
{
	return false;
}



}


