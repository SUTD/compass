#include "node.h"
#include "il.h"

namespace il
{

node::node()
{
}

node::~node()
{
}

location node::get_location()
{
	return this->loc;
}

void node::assert_expression()
{
	assert(is_expression());
}

bool node::is_expression()
{
	return (this->node_type_id == ADDRESSOF_EXP ||
	this->node_type_id == ARRAY_REF_EXP ||
	this->node_type_id == BINOP_EXP ||
	this->node_type_id == BLOCK_EXP ||
	this->node_type_id == BUILTIN_EXP ||
	this->node_type_id == CAST_EXP ||
	this->node_type_id == COMPLEX_CONST_EXP ||
	this->node_type_id == CONDITIONAL_EXP ||
	this->node_type_id == DEREF_EXP ||
	this->node_type_id == EXPR_LIST_EXP ||
	this->node_type_id == FIELD_REF_EXP ||
	this->node_type_id == FIXED_CONST_EXP ||
	this->node_type_id == FUNCTION_ADDRESS_EXP ||
	this->node_type_id == FUNCTION_CALL_EXP ||
	this->node_type_id == FUNCTION_PTR_CALL_EXP ||
	this->node_type_id == INITIALIZER_LIST_EXP ||
	this->node_type_id == INTEGER_CONST_EXP ||
	this->node_type_id == MODIFY_EXP ||
	this->node_type_id == REAL_CONST_EXP ||
	this->node_type_id == STRING_CONST_EXP ||
	       this->node_type_id == THROW_EXP ||
	this->node_type_id == UNOP_EXP ||
	this->node_type_id == VARIABLE_EXP ||
	this->node_type_id == VECTOR_CONST_EXP);
}

node* node::substitute(node* (*sub_func)(node*))
{
	if(this == NULL) return NULL;
	node* res = sub_func(this);
	if(res != NULL) return res;
	switch(node_type_id)
	{
	case ADDRESSOF_EXP:
	{
		il::addressof_expression* ad = (il::addressof_expression*) this;
		expression* new_inner = (expression*)ad->get_inner_expression()->substitute(sub_func);
		if(new_inner != ad->get_inner_expression())
			return new il::addressof_expression(new_inner,
				ad->get_type(), ad->get_location());
		return ad;
	}
	case ARRAY_REF_EXP:
	{
		il::array_ref_expression* ar = (il::array_ref_expression*) this;
		expression* new_array = (expression*)ar->get_array_expression()->substitute(sub_func);
		expression* new_index = (expression*)ar->get_index_expression()->substitute(sub_func);
		if(new_array == ar->get_array_expression() &&
				new_index == ar->get_index_expression())
			return ar;
		return new il::array_ref_expression(new_array, new_index, ar->get_type(),
				ar->get_location());
	}
	case ASSEMBLY:
		return this;
	case BINOP_EXP:
	{
		il::binop_expression* bi = (binop_expression*)this;
		expression* new_one = (expression*)bi->get_first_operand()->substitute(sub_func);
		expression* new_two = (expression*)bi->get_second_operand()->substitute(sub_func);
		if(new_one == bi->get_first_operand() && new_two == bi->get_second_operand())
			return this;
		return new il::binop_expression(new_one, new_two, bi->get_operator(),
				bi->get_type(),
				bi->get_location());
	}
	case BLOCK:
	{
		il::block* b = (block*) this;
		vector<variable_declaration*> new_decls;
		bool changed = false;
		for(unsigned int i = 0; i < b->get_var_declarations().size(); i++)
		{
			il::variable_declaration* vd = b->get_var_declarations()[i];
			il::variable_declaration* new_vd = (il::variable_declaration*)
					vd->substitute(sub_func);
			new_decls.push_back(new_vd);
			if(new_vd != vd) changed = true;
		}
		vector<statement*> new_stmts;
		for(unsigned int i = 0; i < b->get_statements().size(); i++)
		{
			il::statement* s = b->get_statements()[i];
			il::statement* new_s = (statement*) s->substitute(sub_func);
			new_stmts.push_back(new_s);
			if(s != new_s) changed = true;
		}
		if(!changed) return this;
		return new il::block(new_decls, new_stmts);

	}
	case BLOCK_EXP:
	{
		il::block_expression* be = (il::block_expression*)this;
		il::variable_declaration* new_vd = NULL;
		if(be->get_vardecl() != NULL){
			new_vd = (il::variable_declaration*)
				be->get_vardecl()->substitute(sub_func);

		}
		il::block* new_b = (il::block*)be->get_block()->substitute(sub_func);
		if(new_vd == be->get_vardecl() && new_b == be->get_block())
			return this;
		node* res =  new il::block_expression(new_b, new_vd, be->get_type(),
				be->get_location());
		return res;
	}
	case BREAK_STMT:
	case BUILTIN_EXP:
	case CASE_LABEL:
	case COMPLEX_CONST_EXP:
	case CONTINUE_STMT:
	case FIXED_CONST_EXP:
	case FOR_LOOP:
	case DO_WHILE_LOOP:
		return this;
	case CAST_EXP:
	{
		il::cast_expression* ce = (cast_expression*) this;
		il::expression* new_inner = (expression*)
				ce->get_inner_expression()->substitute(sub_func);
		if(new_inner == ce->get_inner_expression()) return this;
		node* res =
				new cast_expression(new_inner, ce->get_type(), ce->get_location());
		return res;
	}
	case CONDITIONAL_EXP:
	{
		il::conditional_expression* ce = (il::conditional_expression*) this;
		il::expression* new_cond = (expression*)
				ce->get_conditional()->substitute(sub_func);

		il::expression* new_then = (expression*)
					ce->get_then_clause()->substitute(sub_func);

		il::expression* new_else = (expression*)
						ce->get_else_clause()->substitute(sub_func);

		if(new_cond == ce->get_conditional() && new_then == ce->get_then_clause()
				&& new_else == ce->get_else_clause()) return this;
		return new conditional_expression(new_cond, new_then, new_else,
				ce->get_type(), ce->get_location());
	}
	case DEREF_EXP:
	{
		il::deref_expression* de = (deref_expression*)this;
		il::expression* new_inner = (il::expression*)
				de->get_inner_expression()->substitute(sub_func);
		if(new_inner == de->get_inner_expression()) return this;
		return new deref_expression(new_inner, de->get_type(), de->get_location());
	}
	case EXPR_LIST_EXP:
	{
		il::expr_list_expression* el = (expr_list_expression*)this;

		vector<expression*>  new_exps;
		bool changed = false;
		for(unsigned int i=0; i < el->get_inner_expressions().size(); i++)
		{
			expression* new_exp = (expression*)
					el->get_inner_expressions()[i]->substitute(sub_func);
			new_exps.push_back(new_exp);
			if(new_exp != el->get_inner_expressions()[i])
				changed = true;
		}
		if(!changed) return this;
		return new il::expr_list_expression(new_exps, el->get_type(),
				el->get_location());
	}
	case EXPR_INSTRUCTION:
	{
		il::expression_instruction* ei = (expression_instruction*)this;
		expression* new_e = (expression*)ei->get_expression()->substitute(sub_func);
		if(new_e == ei->get_expression()) return this;
		return new expression_instruction(new_e);

	}
	case FIELD_REF_EXP:
	{
		il::field_ref_expression* fi = (field_ref_expression*)this;
		il::expression* new_e = (expression*)
				fi->get_inner_expression()->substitute(sub_func);
		if(new_e == fi->get_inner_expression()) return fi;
		return new field_ref_expression(new_e, fi->get_field_info(),
				fi->get_type(), fi->get_location());
	}
	case FILE_NODE:
	{
		assert(false);
	}
	case FUNCTION_ADDRESS_EXP:
	{
		return this;

	}
	case FUNCTION_CALL_EXP:
	{
		il::function_call_expression* fc = (function_call_expression*) this;
		vector<expression*> new_args;
		bool changed = false;
		for(unsigned int i=0; i < fc->get_arguments().size(); i++)
		{
			expression* new_e = (expression*)
					fc->get_arguments()[i]->substitute(sub_func);
			new_args.push_back(new_e);
			if(new_e != fc->get_arguments()[i])
				changed = true;
		}
		if(!changed) return this;
		return new function_call_expression(fc->get_function(), fc->get_namespace(),
				fc->get_signature(), new_args, fc->get_type(), fc->is_exit_function(),
				fc->is_allocator(), fc->is_operator_new(), fc->is_deallocator(),
				fc->is_constructor(), fc->is_destructor(), fc->get_vtable_lookup_expr(),
				fc->get_vtable_index(), fc->get_location());

	}
	case FUNCTION_DECL:
	{
		assert(false);
	}
	case FUNCTION_PTR_CALL_EXP:
	{
		il::function_pointer_call_expression* fc =
				(il::function_pointer_call_expression*)this;
		vector<expression*> new_args;
		bool changed = false;
		for(unsigned int i=0; i < fc->get_arguments().size(); i++)
		{
			expression* new_e = (expression*)
					fc->get_arguments()[i]->substitute(sub_func);
			new_args.push_back(new_e);
			if(new_e != fc->get_arguments()[i])
				changed = true;
		}
		if(!changed) return this;
		return new function_pointer_call_expression(fc->get_function_pointer(),
				new_args, fc->get_type(), fc->get_location());
	}
	case GOTO_STMT:
		return this;
	case IF_STMT:
	{
		il::if_statement* i = (if_statement*)this;
		expression* new_t = (expression*)i->get_test()->substitute(sub_func);
		il::statement* new_i = (statement*)
				i->get_if_statement()->substitute(sub_func);
		il::statement* new_e = (statement*)
				i->get_else_statement()->substitute(sub_func);
		if(new_t == i->get_test() && new_i == i->get_if_statement() &&
				new_e == i->get_else_statement()) return this;
		return new if_statement(new_t, new_i, new_e, i->get_location());
	}
	case INITIALIZER_LIST_EXP:
	{
		il::initializer_list_expression* e = (initializer_list_expression*)this;
		vector<expression*> new_init;
		bool changed = false;
		for(unsigned int i=0; i < e->get_init_list().size(); i++)
		{
			expression* new_e = (expression*)
					e->get_init_list()[i]->substitute(sub_func);
			new_init.push_back(new_e);
			if(new_e != e->get_init_list()[i] )
				changed = true;
		}
		if(!changed) return this;
		return new initializer_list_expression(new_init, e->get_type(),
				e->get_location());

	}
	case INTEGER_CONST_EXP:
	case LABEL:
		return this;
	case MODIFY_EXP:
	{
		il::modify_expression* m = (il::modify_expression*) this;
		set_instruction* new_s = (set_instruction*)
				m->get_set_instruction()->substitute(sub_func);
		if(new_s == m->get_set_instruction()) return this;
		return new modify_expression(new_s, m->is_initalizing_exp(), m->get_location());

	}
	case NOOP_INSTRUCTION:
	case REAL_CONST_EXP:
		return this;
	case RETURN_STMT:
	{
		il::return_statement* r = (return_statement*)this;
		expression* new_e = (expression*)
				r->get_ret_expression()->substitute(sub_func);
		if(new_e == r->get_ret_expression()) return this;
		return new return_statement(new_e, r->get_location());

	}
	case SET_INSTRUCTION:
	{
		il::set_instruction* s = (set_instruction*)this;
		expression* n_lvalue = (expression*)s->get_lvalue()->substitute(sub_func);
		expression* n_rhs = (expression*)s->get_rhs()->substitute(sub_func);
		if(n_lvalue == s->get_lvalue() && n_rhs == s->get_rhs()) return this;
		return new set_instruction(n_lvalue, n_rhs, s->get_location(),
				s->is_declaration_inst());

	}
	case STRING_CONST_EXP:
		return this;
	case SWITCH_STMT:
	{
		assert(false);

	}
	case UNOP_EXP:
	{
		il::unop_expression* u = (unop_expression*)this;
		expression* new_inner = (expression*)
				u->get_operand()->substitute(sub_func);
		if(new_inner == u->get_operand()) return this;
		return new unop_expression(new_inner, u->get_operator(),
				u->get_type(), u->get_location());

	}
	case VARIABLE_DECL:
	{
		il::variable_declaration* vd = (variable_declaration*)this;
		il::expression* new_init = vd->get_initializer();
		if(new_init != NULL) new_init =
				(expression*)new_init->substitute(sub_func);
		if(new_init == vd->get_initializer()) return this;
		return new variable_declaration(vd->get_variable(), vd->get_namespace(),
				new_init, vd->get_location());


	}
	case VARIABLE_EXP:
	case VECTOR_CONST_EXP:
	case WHILE_LOOP:
		return this;
	case TRY_CATCH_STMT:
	{
		il::try_catch_statement* tc = (try_catch_statement*)this;
		statement* new_t =
				(statement*)tc->get_try_statement()->substitute(sub_func);
		vector<statement *> new_catch_bodies;
		bool changed = (new_t != tc->get_try_statement());
		for(unsigned int i =0; i < tc->get_catch_bodies().size(); i++)
		{
			statement* new_s = (statement*)
					tc->get_catch_bodies()[i]->substitute(sub_func);
			if(new_s != tc->get_catch_bodies()[i])
				changed = true;
		}
		if(!changed) return this;
		return new try_catch_statement(new_t, tc->get_location(),
				tc->get_catch_types(), new_catch_bodies);

	}
	case TRY_FINALLY_STMT:
	{
		il::try_finally_statement* tf = (try_finally_statement*) this;
		statement* new_t = (statement*)
				tf->get_try_statement()->substitute(sub_func);
		statement* new_e = (statement*)
				tf->get_exit_statement()->substitute(sub_func);
		if(new_t == tf->get_try_statement() && new_e == tf->get_exit_statement())
			return this;
		return new try_finally_statement(new_t, new_e, tf->get_location());

	}
	case THROW_EXP:
	{
		il::throw_expression* t = (throw_expression*)this;
		expression* new_e = (expression*)
				t->get_full_throw_expression()->substitute(sub_func);
		if(new_e == t->get_full_throw_expression()) return this;
		return new throw_expression(t->get_thrown_type(), new_e, t->get_type(),
				t->get_location());
	}


	}
	assert(false);

}


}
