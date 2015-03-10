
#include <iostream>
#include <unordered_map>
#include <assert.h>
#include <unordered_set>
#include <string.h>

#include <dirent.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>

#include "./sail/il/expression.h"
#include "./sail/il/variable_expression.h"
#include "./sail/il/type.h"
#include "./sail/il/file.h"
#include "./sail/il/translation_unit.h"
#include "./sail/il/block.h"
#include "./sail/il/variable.h"
#include "./sail/il/variable_declaration.h"
#include "./sail/il/set_instruction.h"
#include "./sail/il/assembly.h"
#include "./sail/il/if_statement.h"
#include "./sail/il/namespace_context.h"
#include "./sail/il/noop_instruction.h"
#include "./sail/il/for_loop.h"
#include "./sail/il/goto_statement.h"
#include "./sail/il/label.h"
#include "./sail/il/return_statement.h"
#include "./sail/il/switch_statement.h"
#include "./sail/il/try_catch_statement.h"
#include "./sail/il/try_finally_statement.h"
#include "./sail/il/throw_expression.h"
#include "./sail/il/expression_instruction.h"
#include "./sail/il/integer_const_exp.h"
#include "./sail/il/string_const_exp.h"
#include "./sail/il/real_const_exp.h"
#include "./sail/il/fixed_const_exp.h"
#include "./sail/il/complex_const_exp.h"
#include "./sail/il/vector_const_exp.h"
#include "./sail/il/cast_expression.h"
#include "./sail/il/deref_expression.h"
#include "./sail/il/addressof_expression.h"
#include "./sail/il/expr_list_expression.h"
#include "./sail/il/field_ref_expression.h"
#include "./sail/il/array_ref_expression.h"
#include "./sail/il/binop_expression.h"
#include "./sail/il/unop_expression.h"
#include "./sail/il/function_call_expression.h"
#include "./sail/il/function_pointer_call_expression.h"
#include "./sail/il/conditional_expression.h"
#include "./sail/il/modify_expression.h"
#include "./sail/il/case_label.h"
#include "./sail/il/initializer_list_expression.h"
#include "./sail/il/function_address_expression.h"
#include "./sail/il/block_expression.h"
#include "./sail/il/builtin_expression.h"
#include "./sail/il/function_declaration.h"
#include "./sail/sail/TranslationUnit.h"
#include "./sail/sail/Function.h"
#include "./sail/sail/Serializer.h"
#include "./sail/FileIdentifier.h"
#include "language.h"


#include "./sail/il/node.h"
using namespace std;
using namespace il;

typedef uint64 tree;

map<string, vector<variable_declaration*> > global_decls;

language_type cur_language = LANG_UNSUPPORTED;

static int label_counter = 0;


struct label_info
{
	label* begin_label; // label to mark beginning of loop
	label* end_label; // label to mark end of loop or switch statement
	label* continue_label; // label to mark  where to continue if continue
							// expression is encountered

	label_info(label* begin, label* end, label* cont)
	{
		this->begin_label = begin;
		this->end_label = end;
		this->continue_label = cont;
	}
};


/*
 * A stack of begin and end labels.
 * This is used for keeping track of which label is associated with
 * a break statement.
 */
static vector<label_info> label_stack;



#define DISABLED true

#define BEGIN_LABEL_PREFIX "$BEGIN"
#define END_LABEL_PREFIX "$END"
#define CONTINUE_LABEL_PREFIX "$CONTINUE"



map<case_label*, vector<case_label*>* >*
get_label_to_equivalence_classes(tree switch_id);



static char* sail_output_dir = "";

extern "C" void push_label_context(int line, int column)
{
	int cur_label_id = label_counter++;
	location loc(line, column);
	string begin_label_name =  BEGIN_LABEL_PREFIX + int_to_string(cur_label_id);
	il::label* begin_label = new il::label(begin_label_name, loc);
	string end_label_name =  END_LABEL_PREFIX + int_to_string(cur_label_id);
	il::label* end_label = new il::label(end_label_name, loc);
	string cont_label_name = CONTINUE_LABEL_PREFIX + int_to_string(cur_label_id);
	il::label* continue_label = new il::label(cont_label_name, loc);
	label_info li(begin_label, end_label, continue_label);
	label_stack.push_back(li);
}
extern "C" void pop_label_context()
{
	label_stack.pop_back();
}

label* get_current_begin_label()
{
	assert(label_stack.size() > 0);
	return label_stack[label_stack.size()-1].begin_label;
}


label* get_current_end_label()
{
	return label_stack[label_stack.size()-1].end_label;
}

label* get_current_continue_label()
{
	return label_stack[label_stack.size()-1].continue_label;
}

extern "C" void set_language(language_type lang)
{
	cur_language = lang;
}

extern "C" void set_sail_output_directory(char* dir)
{
	sail_output_dir = dir;
}


extern "C" void cpp_test(void *t)
{
	long a = long(t);
}

extern "C" void assert_vardecl(void* vt)
{
	variable_declaration *v = (variable_declaration *)vt;
	assert(v->node_type_id  == VARIABLE_DECL);
}



extern "C" void* hashmap_create()
{
	unordered_map<unsigned long int, bool> * hm;
	hm = new unordered_map<unsigned long int, bool>();
	return hm;
}


extern "C" void hashmap_delete(void * _hm)
{
	unordered_map<unsigned long int, bool> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	delete hm;
}

extern "C" void hashmap_insert(void * _hm, unsigned long int key)
{
	unordered_map<unsigned long int, bool> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	(*hm)[key] = true;
}

extern "C" bool hashmap_contains(void * _hm, unsigned long int key)
{
	unordered_map<unsigned long int, bool> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	return (hm->count(key) > 0);
}

extern "C" void* hashmap_create2()
{
	unordered_map<unsigned long int, char*> * hm;
	hm = new unordered_map<unsigned long int, char*>();
	return hm;
}


extern "C" void hashmap_insert2(void * _hm, unsigned long int key, char *s)
{
	unordered_map<unsigned long int, char*> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	(*hm)[key] = s;
}

extern "C" bool hashmap_contains2(void * _hm, unsigned long int key)
{
	unordered_map<unsigned long int, char*> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	return (hm->count(key) > 0);
}

extern "C" char* hashmap_find2(void * _hm, unsigned long int key)
{
	unordered_map<unsigned long int, char*> * hm;
	memcpy(&hm, &_hm, sizeof(void*));
	return (*hm)[key];
}


typedef void * namespace_t;

extern "C" namespace_t namespace_make(char *name, namespace_t outer_namespace)
{
	return namespace_context(name,
			(il::_namespace_context*) outer_namespace).get_ns();
}

extern "C" const char *namespace_tostring(namespace_t ns)
{
	if (ns){

		namespace_context nn(static_cast<_namespace_context *>(ns));
		return nn.to_string().c_str();
	}
	else
		return "";
}

/*
 * Compass connectors in C++
 */

static unordered_map<unsigned long int, unsigned long int> parent_map;




static unordered_map<unsigned long int, type*> id_to_type;



unordered_map<unsigned long int, vector<type **> > unresolved_links;


void resolve_links(tree id, type* t)
{
	unordered_map<unsigned long int, vector<type **> >::iterator it =
				unresolved_links.find(id);

	if(it ==  unresolved_links.end())
		return;

	vector<type **> & elems = it->second;

	for(unsigned int i = 0; i < elems.size(); i++)
	{
		*(elems[i]) = t;
	}
	unresolved_links.erase(it);

}

void add_to_unresolved(tree id, type ** ref)
{
	if(ref == NULL) return;
	unordered_map<unsigned long int, vector<type **> >::iterator it =
					unresolved_links.find(id);

	if(it ==  unresolved_links.end())
	{
		vector<type**> v;
		v.push_back(ref);
		unresolved_links[id] = v;
		return;
	}

	vector<type**> & v = unresolved_links[id];
	v.push_back(ref);

}

extern "C" bool type_exists(tree gcc_id)
{

	bool res =  id_to_type.count(gcc_id) > 0;

	return res;
}

extern "C" bool type_exists_as_incomplete_struct(tree gcc_id)
{

	bool res =  id_to_type.count(gcc_id) > 0;
	if(res == false) return false;
	type* t = id_to_type[gcc_id];
	if(t->is_record_type())
	{
		il::record_type* rt = (il::record_type*)t;
		if(rt->get_size() == -1) return true;
	}
	return false;
}

extern "C" void print_type(tree gcc_id)
{
	if(id_to_type.count(gcc_id) == 0)
		cout << "type not there " << endl;
	else
	{
		type* t = id_to_type[gcc_id];
		cout << "type: " << t->to_string() << endl;
		if(t->is_record_type())
		{
			il::record_type* rt = (il::record_type*)t;
			cout << "num fileds: " << rt->get_fields().size() << endl;
		}
	}
}




extern "C" void _make_compass_basetype(char* name, int type_size,
		int type_align_size, tree gcc_id, value_type v, bool is_signed,
		char* typedef_name)
{

	string sname(name);
	string stypedef_name(typedef_name);

	//check if already in map
	if(type_exists(gcc_id))
		return;

	base_type *bt = base_type::make(sname, type_size, type_align_size, is_signed,
			v, stypedef_name);
	id_to_type[gcc_id] = bt;

	//check if we can resolve links
	resolve_links(gcc_id, bt);

}



extern "C" void _make_compass_pointertype(tree gcc_id, tree elem_id,
		char* typedef_name)
{

	//check if already in map
	if(type_exists(gcc_id))
		return;

	type* elem_type = NULL;
	bool resolved = false;
	if(type_exists(elem_id))
	{
		elem_type = id_to_type[elem_id];
		resolved = true;
	}

	pointer_type *pt = pointer_type::make(elem_type, typedef_name);
	id_to_type[gcc_id] = pt;

	//check if we can resolve links
	resolve_links(gcc_id, pt);

	//pointers should always process their inner type before themselves.
	assert(resolved);

}


extern "C" void _make_compass_constarraytype(long int num_elems,
		tree gcc_id, tree elem_id, char* typedef_name)
{

	//check if already in map
	if(type_exists(gcc_id))
		return;

	type* elem_type = NULL;
	bool resolved = false;
	if(type_exists(elem_id))
	{
		elem_type = id_to_type[elem_id];
		resolved = true;
	}


	constarray_type *ct = constarray_type::make(elem_type, num_elems, typedef_name);

	id_to_type[gcc_id] = ct;

	//check if we can resolve links
	resolve_links(gcc_id, ct);

	//constarray type should always process inner type first.
	assert(resolved);

}






struct enum_elem{
	long int value;
	char *name;

};

extern "C" void _make_compass_enumtype(char* name, namespace_t ctx, int type_size,
		int type_align_size, tree gcc_id, bool is_signed,
		enum_elem ** elems, int elem_size, char* typedef_name)
{
	//check if already in map
	if(type_exists(gcc_id))
		return;


	/*
	 * Convert the enum info into C++ vector
	 */
	vector<enum_info> info;
	for(int i=0; i < elem_size; i++)
	{
		enum_info einfo;
		enum_elem * cur = elems[i];
		einfo.fname = string(cur->name);
		einfo.value = cur->value;
		info.push_back(einfo);
	}

	enum_type *et = enum_type::make(name,
	                                (_namespace_context*)(ctx),
	                                type_size,
	                                type_align_size,
	                                is_signed,
	                                info,
	                                typedef_name);

	id_to_type[gcc_id] = et;

	//check if we can resolve links
	resolve_links(gcc_id, et);
}


struct field_elem {
	/* Number of bits from beginning of struct/union */
	int bit_offset;
	char* name;

	/* GCC id of declared type */
	tree gcc_type;
};


struct base_class {
	void *type;
	long int offset;
};


/*
 * Careful with changes! The resolve field type logic is a little
 * tricky.
 */
extern "C" void _make_compass_struct_or_uniontype(char *_name,
                                                  namespace_t ctx,
                                                  int type_size,
                                                  int type_align_size,
                                                  tree gcc_id,
                                                  bool is_union,
                                                  struct field_elem **elems,
                                                  int elem_size,
                                                  struct base_class **bases,
                                                  int bases_size,
                                                  char *typedef_name,
                                                  bool is_abstract)
{
	bool updating_exisiting_abstract = false;
	//check if already in map
	if(type_exists(gcc_id))
	{
		type* e_t = id_to_type[gcc_id];
		if(e_t->is_record_type() && ((record_type*)e_t)->get_size() == -1)
			updating_exisiting_abstract = true;
		else return;
	}

	string name = _name;
	/*
	 * Since structs are allowed to have empty string as their name, we make
	 * up a unique name for structs that don't have a name.
	 */
	if(name == "") {
		name = "$" + int_to_string((long int) gcc_id);
	}


	/*
	 * Convert the record info into C++ vector
	 */
	vector<record_info*> info;
	vector<int> unresolved_index;
	for(int i=0; i < elem_size; i++)
	{
		record_info* rinfo = new record_info();
		field_elem * cur = elems[i];
		rinfo->fname = string(cur->name);
		rinfo->offset = cur->bit_offset;

		type *elem_type = NULL;
		if(type_exists(cur->gcc_type))
		{
			elem_type = id_to_type[cur->gcc_type];
		}
		rinfo->t = elem_type;
		if(elem_type == NULL)
			unresolved_index.push_back(i);

		info.push_back(rinfo);
	}

	assert(info.size() == elem_size);

	record_type *rt = NULL;
	if(updating_exisiting_abstract)
	{
		rt = (record_type*)id_to_type[gcc_id];
		assert(rt != NULL);
		rt->update_incomplete_record(type_size, type_align_size,
				info, is_abstract);


	}
	else
	{
		rt = record_type::make(name,
			 namespace_context(static_cast<_namespace_context *>(ctx)),
			  type_size, type_align_size, is_union, false, info,
			  typedef_name, is_abstract);
	}

	/*
	 * For every unresolved field, enqueue it for resolving
	 */


	assert(rt != NULL);
	for(int j=0; j < unresolved_index.size(); j++)
	{
		int i = unresolved_index[j];
		assert(i < elem_size);
		add_to_unresolved(elems[i]->gcc_type, rt->get_typeref_from_index(i));
	}

	id_to_type[gcc_id] = rt;

	//check if we can resolve links
	resolve_links(gcc_id, rt);

	/* Link together base and derived class */
	
	for (int i = 0; i < bases_size; ++i) {
		record_type *base_rt = static_cast<record_type *>(bases[i]->type);
		rt->add_base(base_rt, bases[i]->offset);
		base_rt->add_derivative(rt);
	}
}

extern "C" void _make_compass_vectortype(tree gcc_id, tree elem_id,
		int num_elems)
{
	//check if already in map
	if(type_exists(gcc_id))
		return;

	type* elem_type = NULL;
	bool resolved = false;
	if(type_exists(elem_id))
	{
		elem_type = id_to_type[elem_id];
		resolved = true;
	}



	vector_type *vt = vector_type::make(elem_type, num_elems);
	id_to_type[gcc_id] = vt;

	//check if we can resolve links
	resolve_links(gcc_id, vt);

	//vector_type should have processed its inner type first.
	assert(resolved);
}



extern "C" void _make_compass_complextype(tree gcc_id, tree elem_id)
{
	//check if already in map
	if(type_exists(gcc_id))
		return;

	type* elem_type = NULL;
	bool resolved = false;
	if(type_exists(elem_id))
	{
		elem_type = id_to_type[elem_id];
		resolved = true;
	}



	complex_type *ct = complex_type::make(elem_type);
	id_to_type[gcc_id] = ct;

	//check if we can resolve links
	resolve_links(gcc_id, ct);

	//complex_type should have processed its inner type first.
	assert(resolved);

}

/*
 * Return type can be NULL to indicate void
 */
extern "C" void _make_compass_functiontype(tree gcc_id, tree ret_id,
		tree* arg_types, int arg_size, bool is_vararg)
{
	//check if already in map
	if(type_exists(gcc_id))
		return;


	/*
	 * Process return value
	 */
	type* ret_type = NULL;
	bool resolved = false;
	if(ret_id == 0)
		resolved = true;
	else if(type_exists(ret_id))
	{
		ret_type = id_to_type[ret_id];
		resolved = true;
	}



	//------------

	/*
	 * Convert the arg types into C++ vector
	 */
	vector<type*> args;

	for(int i=0; i < arg_size; i++)
	{

		type *elem_type = NULL;
		if(type_exists(arg_types[i]))
		{
			elem_type = id_to_type[arg_types[i]];
		}



		//function_type arg should have processed its inner type first.
		assert(elem_type != NULL);

		args.push_back(elem_type);
	}

	/*
	 * For every unresolved field, enqueue it for resolving
	 */
	function_type *ft = function_type::make(ret_type, args, is_vararg);




	id_to_type[gcc_id] = ft;

	//function_type return should have processed its inner type first.
	assert(resolved);

	//check if we can resolve links
	resolve_links(gcc_id, ft);
}





extern "C" void _assert_no_unresolved_types()
{
	assert(unresolved_links.size() == 0);

}

















static unordered_map<unsigned long int, node*> id_to_node;



unordered_map<unsigned long int, vector<node **> > unresolved_nodes;


void resolve_nodes(tree id, node* t)
{
	unordered_map<unsigned long int, vector<node **> >::iterator it =
				unresolved_nodes.find(id);

	if(it ==  unresolved_nodes.end())
		return;

	vector<node **> & elems = it->second;

	for(unsigned int i = 0; i < elems.size(); i++)
	{
		*(elems[i]) = t;
	}
	unresolved_nodes.erase(it);

}

void add_to_unresolved_nodes(tree id, node ** ref)
{
	unordered_map<unsigned long int, vector<node **> >::iterator it =
					unresolved_nodes.find(id);

	if(it ==  unresolved_nodes.end())
	{
		vector<node**> v;
		v.push_back(ref);
		unresolved_nodes[id] = v;
		return;
	}

	vector<node**> & v = it->second;//unresolved_nodes[id];
	v.push_back(ref);
}

extern "C" bool node_exists(tree gcc_id)
{
	return id_to_node.count(gcc_id) > 0;
}




static file *cur_file = NULL;

static translation_unit* tu = NULL;

//enum scope_type {GLOBAL, ARGUMENT, LOCAL};
//typedef enum scope_type scope_type;

static scope_type st = GLOBAL;

block *cur_block = NULL;




string get_absolute_path_name(string file)
{
	FileIdentifier fi(file);
	return fi.to_string();

}

extern "C" void _make_compass_file(unsigned long int id, char* _name)
{

	if(tu == NULL)
		tu = new translation_unit();
	string name = get_absolute_path_name(_name);



	cur_file = tu->get_files()[name];
	if(cur_file == NULL)
	{
		cur_file = new file(name.c_str());
		tu->get_files()[name] = cur_file;
	}
	st = GLOBAL;

}

static set<unsigned long int> node_id_in_function;

extern "C" void *_make_compass_vardecl(char* name, namespace_t name_ctx,
		tree parent, tree cur_id,
		void * init_exp, int line, int column, tree type_id, qualifier_type qt,
		bool global_decl)
{
	if(node_exists(cur_id)){
		variable_declaration *ttt = (variable_declaration *)id_to_node[cur_id];
		assert(ttt->node_type_id == VARIABLE_DECL);
		return id_to_node[cur_id];
	}

	assert(type_exists(type_id));

	type *decl_type = id_to_type[type_id];

	namespace_context ns= static_cast<_namespace_context *>(name_ctx);

	//First make variable
	string sname = name;
	variable *v = new variable(sname, ns, cur_block, st, qt, NULL, decl_type);


	//make vardecl
	location loc(line, column);

	expression * init = (expression*) init_exp;
	if(init!=NULL) init->assert_expression();



	variable_declaration *vd = new variable_declaration(v, ns, init, loc);

	v->set_declaration(vd);

	id_to_node[cur_id] = vd;
	if(!global_decl)
		node_id_in_function.insert(cur_id);

	resolve_nodes(cur_id, vd);


	return vd;

}

extern "C" void _set_init_exp_vardecl(void* var_decl, void* init_exp)
{
	variable_declaration *vd = (variable_declaration *) var_decl;
	vd->set_initializer((expression*)init_exp);
}

extern "C" void vardecl_has_record_type(void *_vd)
{
	variable_declaration *vd = (variable_declaration*)_vd;
	if(!vd->get_variable()->get_type()->is_record_type())
	{
		*((int*)0) = 5;
	}
}

struct arg
{
	char *name;
	namespace_t ns;
	tree type;
	qualifier_type qt;
	tree gcc_id;
};

extern "C" bool is_namespace_std(namespace_t name_ctx)
{
	 namespace_context ns= static_cast<_namespace_context *>(name_ctx);
	 namespace_context top_level = ns.get_toplevel_namespace();
	 return top_level.get_name().find("std") == 0;
}

extern "C" void* _make_compass_fundecl(char *name, namespace_t name_ctx,
		tree declaration, tree parent,
		unsigned int qt, tree return_type, bool is_allocator,
		bool is_dealloc, bool is_constructor, bool is_destructor,
		bool is_inline, bool is_vararg, void *owning_class,
		bool is_static_member_function,
		bool is_virtual, bool is_abstract, arg ** args, int num_args,
		tree body, tree funtype, int line, int column)
{


	assert(type_exists(funtype));
	type* _fun_type = id_to_type[funtype];
	function_type* fun_type = (function_type*) _fun_type;
	assert(fun_type->is_function_type());
	assert(return_type == 0 || type_exists(return_type));
	type* ret_type = NULL;

	if(return_type != 0)
		ret_type = id_to_type[return_type];

	vector<variable_declaration* > arguments;
	for(int i=0; i<num_args; i++)
	{
		arg* cur_arg = args[i];
		assert(type_exists(cur_arg->type));
		type* cur_arg_type = id_to_type[cur_arg->type];
		namespace_context cur_arg_ns=
				static_cast<_namespace_context *>(cur_arg->ns);

		variable *cur_var = new variable(cur_arg->name, cur_arg_ns,  NULL, ARGUMENT,
				cur_arg->qt, NULL, cur_arg_type, i);

		location loc(line, column);
		variable_declaration *vd = new variable_declaration(cur_var,cur_arg_ns,
				NULL, loc);
		cur_var->set_declaration(vd);

		id_to_node[cur_arg->gcc_id] = vd;
		node_id_in_function.insert(cur_arg->gcc_id);
		resolve_nodes(cur_arg->gcc_id, vd);

		arguments.push_back(vd);
	}

	block* fun_body = NULL;

	namespace_context ns= static_cast<_namespace_context *>(name_ctx);


	function_declaration *fd
	    = new function_declaration(name,
	                               static_cast<_namespace_context *>(name_ctx),
	                               static_cast<qualifier_type>(qt),
	                               ret_type,
	                               arguments,
	                               fun_body,
	                               is_allocator,
	                               is_dealloc,
	                               is_constructor,
	                               is_destructor,
	                               is_inline,
	                               is_vararg,
	                               (owning_class ? true : false),
	                               is_static_member_function,
	                               is_virtual, is_abstract,
	                               fun_type);

	if (owning_class)
		static_cast<record_type *>(owning_class)->add_member_function(
				fd->get_name(),fd->get_namespace(), fd->get_signature(),
				fd->is_virtual_method(), fd->is_static_method(),
				fd->is_abstract_virtual_method());

	resolve_nodes(declaration, fd);
	return fd;

}

extern "C" void _set_fundecl_body(void *f, void *body)
{
	function_declaration *fd = static_cast<function_declaration *>(f);
	assert(fd->node_type_id == FUNCTION_DECL);
	statement* s = (statement*) body;
	s->assert_statement();
	block* b;
	if(s->node_type_id == BLOCK) b = static_cast<block*>(s);
	else {
		vector<variable_declaration*>  var_decls;
		vector<statement*> stmts;
		stmts.push_back(s);
		b = new block(var_decls, stmts);

	}
	fd->set_body(b);
	//cout << "********************** \n" <<
	//	fd->to_string() << " \n ******************** " << endl;
}

extern "C" void _make_compass_vararraytype(void* _num_elems, tree gcc_id,
		tree elem_id, char* typedef_name)
{
	//check if already in map
	if(type_exists(gcc_id))
		return;

	type* elem_type = NULL;
	bool resolved = false;
	if(type_exists(elem_id))
	{
		elem_type = id_to_type[elem_id];
		resolved = true;
	}

	location l(-1, -1);

	expression* num_elems = (expression*) _num_elems;
	num_elems->assert_expression();

	vararray_type *ct = vararray_type::make(elem_type, num_elems, typedef_name);

	id_to_type[gcc_id] = ct;

	//check if we can resolve links
	resolve_links(gcc_id, ct);

	//vararray_type return should have processed its inner type first.
	assert(resolved);

}




extern "C" void* _make_compass_var_exp(void *var, int line, int column)
{
	location loc(line, column);
	expression* ve =  new variable_expression((variable*)var, loc);
	return ve;
}


extern "C" void *_make_compass_block(void** decls, int num_decls, void** stmts,
		int num_stmts)
{
	vector<variable_declaration*> declarations;
	vector<statement*> statements;
	for(int i=0; i < num_decls; i++)
	{
		variable_declaration *vd = (variable_declaration*)decls[i];
		assert(vd->node_type_id  == VARIABLE_DECL);
		declarations.push_back(vd);
	}

	for(int i=0; i < num_stmts; i++)
	{
		statement *s = (statement*)stmts[i];
		if(s==NULL) continue;

		s->assert_statement();
		statements.push_back(s);
	}
	block *b = new block(declarations, statements);
	return b;


}

extern "C" void* _make_compass_break_stmt(int line, int column)
{
	location loc(line, column);
	label* l = get_current_end_label();
	il::goto_statement* goto_end = new il::goto_statement(l, l->get_label(), loc);
	return goto_end;
}

extern "C" void* _make_compass_continue_stmt(int line, int column)
{
	location loc(line, column);
	label* l = get_current_continue_label();
	il::goto_statement* goto_begin = new il::goto_statement(l, l->get_label(),
			loc);
	return goto_begin;
}




// Helper function for loops constructs
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

extern "C" void* _make_compass_exit_exp_statement(void* _cond, int line, int column)
{
	location loc(line, column);
	expression* cond = (expression*)_cond;
	assert(cond->is_expression());
	label* end_label = get_current_end_label();
	statement* cond_jump = make_conditional_jump_to_loop_end(cond,
				end_label, loc);
	return cond_jump;
}

extern "C" void* _make_compass_do_stmt(void* cond_exp, void* do_body,
		int line, int column)
{
	expression* cond = (expression*) cond_exp;
	statement* body = (statement*) do_body;

	vector<statement*> stmts;

	location loc(line, column);

	// Make begin label
	label* begin_label = get_current_begin_label();
	stmts.push_back(begin_label);

	// Add body to stmts
	if(body!= NULL) stmts.push_back(body);

	// Add continue label right before performing test
	label* continue_label = get_current_continue_label();
	stmts.push_back(continue_label);

	// Make end label
	label* end_label = get_current_end_label();

	// Make test: if(cond == 0) goto END
	if(cond != NULL) {
		statement* cond_jump =
				make_conditional_jump_to_loop_end(cond, end_label, loc);
		stmts.push_back(cond_jump);
	}

	// Add goto BEGIN
	il::goto_statement* goto_begin = new goto_statement(begin_label,
			begin_label->get_label(), loc);
	stmts.push_back(goto_begin);

	// Add END label
	stmts.push_back(end_label);

	// Make and return block
	vector<variable_declaration*> decls;
	il::block* res = new il::block(decls, stmts);

	return res;



}

extern "C" void* _make_compass_while_stmt(void* cond_exp, void* while_body,
		int line, int column)
{
	expression* cond = (expression*) cond_exp;
	statement* body = (statement*) while_body;

	vector<statement*> stmts;

	location loc(line, column);

	// Make begin label
	label* begin_label = get_current_begin_label();
	stmts.push_back(begin_label);

	// For a while loop, continue label is at the same place as begin label
	label* continue_label = get_current_continue_label();
	stmts.push_back(continue_label);



	// Make test: if(cond == 0) goto END
	label* end_label = get_current_end_label();
	if(cond != NULL)
	{
		statement* cond_jump = make_conditional_jump_to_loop_end(cond,
				end_label, loc);
		stmts.push_back(cond_jump);


	}

	// Add body of while loop
	if(body!=NULL) stmts.push_back(body);

	// Make goto BEGIN
	il::goto_statement* goto_begin = new goto_statement(begin_label,
			begin_label->get_label(), loc);
	stmts.push_back(goto_begin);

	// Add END label
	stmts.push_back(end_label);



	// Make the block
	vector<variable_declaration*> decls;
	il::block* res = new il::block(decls, stmts);

	return res;


}



extern "C" void *_make_compass_for_stmt(void* init_stmt, void* continuation_cond,
		void* _inc_exp, void* loop_body, int line, int column)
{
	statement* init = (statement*) init_stmt;
	expression* cont_cond = (expression*) continuation_cond;
	expression* inc_exp = (expression*) _inc_exp;
	statement* body = (statement*) loop_body;


	vector<statement*> block_body;
	if(init != NULL) block_body.push_back(init);

	// Make begin label
	il::label* begin_label = get_current_begin_label();
	block_body.push_back(begin_label);

	// Make end label
	location loc(line, column);
	il::label* end_label = get_current_end_label();
	il::goto_statement* goto_stmt = new goto_statement(end_label,
			end_label->get_label(), loc);

	if(cont_cond != NULL)
	{
		statement* cond_jump = make_conditional_jump_to_loop_end(cont_cond,
				end_label, loc);
		block_body.push_back(cond_jump);
	}

	if(body != NULL) block_body.push_back(body);

	// For a for loop, continue point is right before the increment
	il::label* continue_label = get_current_continue_label();
	block_body.push_back(continue_label);

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

extern "C" void *_make_compass_set_instruction(void* exp_lval, void *exp_rhs,
		int line, int column, bool is_decl)
{
	expression *lval = (expression*) exp_lval;
	expression *rhs = (expression*) exp_rhs;
	lval->assert_expression();
	rhs->assert_expression();
	location loc(line, column);
	set_instruction *si = new set_instruction(lval, rhs, loc, is_decl);
	return si;
}


extern "C" void add_to_global_decls(void* var_decl, void* init_exp,
		int line, int column, char* filename)
{


	string name = get_absolute_path_name(filename);
	variable* var = (variable*) var_decl;
	expression* init = (expression*) init_exp;

	location loc(line, column);

	variable_declaration* decl = new il::variable_declaration(var, var->get_namespace(),
			init, loc);



	global_decls[name].push_back(decl);

}

extern "C" bool is_expression(void *node)
{
	if(node == NULL) return false;
	il::node* n = (il::node*) node;
	return n->is_expression();
}


extern "C" void *_make_compass_assembly_instruction(char *a_string,
		void ** exp_outs,int num_outs, void **exp_ins, int num_ins,
		void **char_clobbers, int num_clobbers, int line, int column)
{
	vector<expression*> outs;
	for(int i=0; i < num_outs; i++)
	{
		expression *co = (expression*) exp_outs[i];
		co->assert_expression();
		outs.push_back(co);
	}

	vector<expression*> ins;
	for(int i=0; i < num_ins; i++)
	{
		expression *ci = (expression*) exp_ins[i];
		ci->assert_expression();
		ins.push_back(ci);
	}

	vector<string> clobber;
	for(int i=0; i < num_clobbers; i++)
	{
		string s = (char*) char_clobbers[i];
		clobber.push_back(s);
	}
	location loc(line, column);
	assembly* ai = new assembly(a_string,
			outs, ins, clobber, loc);
	return ai;

}

extern "C" void *_make_compass_if_statement(void *exp, void *then_stmt, void *else_stmt,
		int line, int column)
{
	expression *ce = (expression*) exp;
	ce->assert_expression();
	statement *ts = (statement*) then_stmt;
	ts->assert_statement();
	statement *es = (statement*) else_stmt;
	if(es!=NULL) es->assert_statement();
	location loc(line, column);

	if_statement * is = new if_statement(ce, ts, es, loc);
	return is;
}


extern "C" void *_make_compass_for_loop(void *init_exp, void *continue_exp,
		void *incr_exp, void* body_stmt,
		int line, int column)
{
	expression* init = (expression*) init_exp;
	if(init!=NULL) init->assert_expression();
	expression* continue_cond = (expression*) continue_exp;
	if(continue_cond!=NULL) continue_cond->assert_expression();
	expression* incr = (expression*) incr_exp;
	if(incr!=NULL) incr->assert_expression();
	statement* body = (statement*) body_stmt;
	if(body!=NULL) body->assert_statement();
	location loc(line, column);
	for_loop* loop = new for_loop(init, continue_cond, incr, body, loc);
	return loop;
}

noop_instruction noop;

extern "C" void *_make_compass_noop_instruction()
{
	return (void*) &noop;
}


map<tree, label*> label_mapping;
map<tree, vector<label**> > label_update_map;

extern "C" void *_make_compass_goto_statement(char* l, tree label_id,
		int line, int column)
{
	location loc(line, column);
	label *label_st = label_mapping[label_id];
	goto_statement *gs = new goto_statement(label_st, l, loc);


	if(label_st == NULL)
	{
		if(label_update_map.count(label_id)>0)
		{
			label_update_map[label_id].push_back(gs->get_label_ref());
		}
		else
		{
			vector<label**> vec;
			vec.push_back(gs->get_label_ref());
			label_update_map[label_id] = vec;
		}
	}

	return gs;
}

extern "C" void *_make_compass_label_statement(char* l, tree label_id,
		int line, int column)
{
	location loc(line, column);
	label *label_st = new label(l, loc);
	label_mapping[label_id] = label_st;
	if(label_update_map.count(label_id)>0)
	{
		vector<label**> & to_update = label_update_map[label_id];
		for(int i=0; i < (int)to_update.size(); i++)
		{
			*to_update[i] = label_st;
		}
		label_update_map.erase(label_id);
	}
	return label_st;
}

extern "C" void *_make_compass_case_label_statement(long low,
		bool has_low, long high, bool has_high, int line, int column)
{
	location loc(line, column);
	case_label* cl = NULL;
	if(has_low && has_high){
		cl = new case_label(low, high, loc);
	}
	else if(has_low){
		cl = new case_label(low, loc);
	}
	else{
		cl = new case_label(loc);
	}
	return cl;
}

extern "C" void *_make_compass_return_statement(void* ret_exp, int line,
				int column)
{
	location loc(line, column);
	expression* return_exp = (expression*) ret_exp;
	if(return_exp!= NULL) return_exp->assert_expression();
	return_statement* rs = new return_statement(return_exp, loc);
	return rs;
}

extern "C" void *_make_compass_switch_statement(void* cond_exp, void* body_stmt,
			void* case_label_vector, tree switch_id, int line, int column)
{
	location loc(line, column);
	expression* compass_cond = (expression*) cond_exp;
	compass_cond->assert_expression();
	statement* compass_body = (statement*) body_stmt;
	compass_body->assert_statement();
	vector<case_label*>* compass_case_labels =
				(vector<case_label*>*) case_label_vector;
	if(compass_case_labels == NULL){
		compass_case_labels = new vector<case_label*>();
	}

	map<case_label*, vector<case_label*>* >* eqs =
	get_label_to_equivalence_classes(switch_id);

	switch_statement* sws =
			new switch_statement(compass_cond, compass_body,
					compass_case_labels, eqs, loc);

	/*
	 * If in C++, we need to put an end label.
	 */
	if(cur_language == LANG_C) {
		cout << "PROCESSING C!" << endl;
		return sws;



	}
	else {
		il::label* l = get_current_end_label();
		vector<variable_declaration*> decls;
		vector<statement*> stmts;
		stmts.push_back(sws);
		stmts.push_back(l);
		il::block* b = new il::block(decls, stmts);
		return b;

	}
}

struct catch_handler {
	void *type;
	void *body;
};

extern "C" void *_make_compass_try_catch_statement(void *try_stmt,
                                                   struct catch_handler **handlers,
                                                   int num_handlers,
                                                   int line,
                                                   int column)
{
	location loc(line, column);
	try_catch_statement *tcs = new try_catch_statement(static_cast<statement *>(try_stmt), loc);
	
	for (int i = 0; i < num_handlers; ++i) {
		type *catch_type = static_cast<type *>(handlers[i]->type);
		statement *catch_body = static_cast<statement *>(handlers[i]->body);
		tcs->add_handler(catch_type, catch_body);
	}
	
	return tcs;
}

extern "C" void *_make_compass_try_finally_statement(void *try_stmt,
                                          void *exit_stmt,
                                          int line,
                                          int column)
{
	location loc(line, column);
	return new try_finally_statement(static_cast<statement *>(try_stmt),
	                                 static_cast<statement *>(exit_stmt),
	                                 loc);
}

extern "C" void* _make_compass_expression_instruction(void* expr)
{
	expression* compass_expr = (expression*) expr;
	compass_expr->assert_expression();
	expression_instruction* expr_inst =
		new expression_instruction(compass_expr);
	return expr_inst;
}


extern "C" void* _make_compass_integer_const(long int int_const,
		void* compass_type, int line, int column)
{
	type* t = (type*) compass_type;
	assert(t!=NULL);
	location loc(line, column);
	integer_const_exp* ice = new integer_const_exp(int_const, t, loc);
	return ice;
}

extern "C" void* _make_compass_string_const(char* string_const,
		void* compass_type, int line, int column)
{
	type* t = (type*) compass_type;
	location loc(line, column);
	string_const_exp* sce = new string_const_exp(string_const, t, loc);
	return sce;
}

extern "C" void* _make_compass_real_const(void* compass_type,
		int line, int column)
{
	type* t = (type*) compass_type;
	location loc(line, column);
	real_const_exp* rce = new real_const_exp(t, loc);
	return rce;
}

extern "C" void* _make_compass_fixed_const(void* compass_type,
		int line, int column)
{
	type* t = (type*) compass_type;
	location loc(line, column);
	fixed_const_exp* fce = new fixed_const_exp(t, loc);
	return fce;
}

extern "C" void* _make_compass_complex_const(void* compass_type,
		int line, int column)
{
	type* t = (type*) compass_type;
	location loc(line, column);
	complex_const_exp* cce = new complex_const_exp(t, loc);
	return cce;
}

extern "C" void* _make_compass_vector_const(void* compass_type,
		int line, int column)
{
	type* t = (type*) compass_type;
	location loc(line, column);
	vector_const_exp* vce = new vector_const_exp(t, loc);
	return vce;
}

extern "C" void* _make_compass_cast_expression(void* inner_exp,
		void* compass_type, int line, int column)
{
	expression* e = (expression*) inner_exp;
	e->assert_expression();
	type* t = (type*) compass_type;
	/*
	 * g++ adds fantasy void casts; we just ignore these.
	 */
	if(t->is_void_type()) return e;

	/*
	 * Don't introduce redundant casts.
	 */
	if(e->get_type() == t) return e;
	location loc(line, column);
	cast_expression* ce = new cast_expression(e, t, loc);
	return ce;
}
extern "C" void* _make_compass_deref_expression(void* inner_exp,
		void* compass_type, int line, int column)
{
	expression* e = (expression*) inner_exp;
	e->assert_expression();
	type* t = (type*) compass_type;
	location loc(line, column);
	deref_expression* drf = new deref_expression(e, t, loc);
	return drf;
}

extern "C" void* _make_compass_addressof_expression(void* inner_exp,
		void* compass_type, int line, int column)
{
	expression* e = (expression*) inner_exp;
	e->assert_expression();
	type* t = (type*) compass_type;
	location loc(line, column);
	addressof_expression* addr_exp = new addressof_expression(e, t, loc);
	return addr_exp;
}

extern "C" void* _make_compass_expression_list(void* exprs, int size, void* compass_type,
		int line, int column)
{
	vector<expression*> v;
	for(int i=0; i<size; i++){
		expression* e = ((expression**)(exprs))[i];
		e->assert_expression();
		v.push_back(e);
	}
	type* t = (type*) compass_type;
	location loc(line, column);
	expr_list_expression* exp_list = new expr_list_expression(v, t, loc);
	return exp_list;
}

extern "C" void* _make_compass_field_ref_expression(void *inner_exp,
                                                    char *field_info,
                                                    void *compass_type,
                                                    int line,
                                                    int column)
{

	expression* e = (expression*) inner_exp;
	e->assert_expression();
	type* et = e->get_type();
	record_type *rt = (record_type*) et;
	record_info* f = rt->get_field_from_name(field_info);
	/*
	 * This should only be NULL if GCC aggregated variables into
	 * anonymous dummy "structs". In this case, we just care about the
	 * inner expression.
	 */
	if(f == NULL){
		return inner_exp;
	}
	type* t = (type*) compass_type;
	location loc(line, column);
	field_ref_expression* field_ref = new field_ref_expression(e, f, t, loc);
	return field_ref;
}

extern "C" void* _make_compass_array_ref_expression(void* array_exp,
		void* index_exp, void* compass_type, int line, int column)
{
	expression* ae = (expression*) array_exp;
	expression* ie = (expression*) index_exp;
	ae->assert_expression();
	ie->assert_expression();
	type* t= (type*) compass_type;
	location loc(line, column);
	array_ref_expression* are = new array_ref_expression(ae, ie, t, loc);
	return are;
}

il::expression* strip_void_casts(il::expression* e)
{
	while(e->node_type_id == CAST_EXP &&
			((il::cast_expression*)e)->get_type()->is_void_star_type())
	{
		il::cast_expression* ce = (il::cast_expression*) e;
		e = ce->get_inner_expression();
	}
	return e;
}

extern "C" void* _make_compass_binop_expression(void* exp1, void* exp2,
		binop_type binop, void* compass_type, int line, int column)
{
	expression* e1 = (expression*) exp1;
	expression* e2 = (expression*) exp2;
	e1->assert_expression();
	e2->assert_expression();
	e1 = strip_void_casts(e1);
	type* t = (type*) compass_type;
	location loc(line, column);
	binop_expression* bine = new  binop_expression(e1, e2, binop, t, loc);
	return bine;
}

extern "C" void* _make_compass_unop_expression(void* exp,  unop_type unop,
		void* compass_type, int line, int column)
{
	expression* e = (expression*) exp;
	e->assert_expression();
	type* t = (type*) compass_type;
	location loc(line, column);
	unop_expression* une = new unop_expression(e, unop, t, loc);
	return une;
}

extern "C" void* _make_compass_function_call_expression(char* fn_name,
														namespace_t ns,
                                                        void* fn_signature,
                                                        void* args,
                                                        int num_args,
                                                        void* compass_type,
                                                        bool is_no_return,
                                                        bool is_alloc,
                                                        bool is_operator_new,
                                                        bool is_dealloc,
                                                        bool is_constructor,
                                                        bool is_destructor,
                                                        void* vtbl_lookup_expr,
                                                        long int vtbl_idx,
                                                        int line,
                                                         int column)
{
	type* sig = (type*) fn_signature;
	vector<expression*> compass_args;
	for(int i = 0; i < num_args; i++){
		expression* cur_arg = ((expression**)args)[i];
		cur_arg->assert_expression();
		compass_args.push_back(cur_arg);
	}
	type* t = (type*) compass_type;
	location loc(line, column);
	function_call_expression *fn_call
		= new function_call_expression(fn_name,
				  static_cast<_namespace_context*>(ns),
		          sig, compass_args, t, is_no_return, is_alloc, is_operator_new,
		          is_dealloc, is_constructor, is_destructor,
		          static_cast<expression *>(vtbl_lookup_expr),
		          vtbl_idx, loc);
	return fn_call;
}

extern "C" void* _make_compass_function_pointer_call_expression(void* fn_ptr,
		void* args, int num_args, void* compass_type, int line, int column)
{
	expression* compass_fn_ptr = (expression*) fn_ptr;
	compass_fn_ptr->assert_expression();
	vector<expression*> compass_args;
	for(int i=0; i<num_args; i++){
		expression* cur_arg = ((expression**)args)[i];
		cur_arg->assert_expression();
		compass_args.push_back(cur_arg);
	}
	type* t = (type*) compass_type;
	location loc(line, column);
	function_pointer_call_expression* fptr_call =
		new function_pointer_call_expression(compass_fn_ptr, compass_args,
				t, loc);
	return fptr_call;
}

extern "C" void* _make_compass_conditional_expression(void* cond_exp,
		void* then_clause, void* else_clause, void* compass_type,
		int line, int column)

{
	expression* cond_e = (expression*) cond_exp;
	expression* then_e = (expression*) then_clause;
	expression* else_e = (expression*) else_clause;
	cond_e->assert_expression();
	then_e->assert_expression();
	else_e->assert_expression();
	type* t= (type*) compass_type;
	location loc(line, column);
	conditional_expression* ce = new conditional_expression(cond_e, then_e,
			else_e, t, loc);
	return ce;
}

extern "C" void* _make_compass_modify_expression(void *set_inst,
                                                 bool is_init,
                                                 void *compass_type,
                                                 int line,
                                                 int column)
{
	set_instruction* si = (set_instruction*) set_inst;
	assert(si->node_type_id == SET_INSTRUCTION);
	location loc(line, column);
	modify_expression* me = new modify_expression(si, is_init, loc);
	return me;
}

extern "C" void* _make_compass_init_list_expression(void* init_list,
		int num_elems, void* compass_type, int line, int column)
{
	vector<expression*> elems;
	for(int i=0; i<num_elems; i++){
		expression* e = ((expression**)init_list)[i];
		e->assert_expression();
		elems.push_back(e);
	}

	type* t = (type*) compass_type;
	location loc(line, column);
	initializer_list_expression* ile = new initializer_list_expression(
			elems, t, loc);
	return ile;
}

extern "C" void* _make_compass_function_address_expression(char* fn_name,
		void* fn_sig, void* compass_type, int line, int column)
{
	type* sig = (type*) fn_sig;
	type* t = (type*) compass_type;
	location loc(line, column);
	function_address_expression* fae =
		new function_address_expression(fn_name, sig, t, loc);
	return fae;
}


extern "C" void* _make_compass_block_expression(void* compass_block, void* var_decl,
		void* compass_type, int line, int column)
{
	block* b = (block*) compass_block;
	variable_declaration* v = (variable_declaration*) var_decl;
	type* t = (type*) compass_type;
	location loc(line, column);
	block_expression* be = new block_expression(b, v, t, loc);
	return be;
}


extern "C" void* _make_compass_builtin_expression(compass_builtin_type bt,
		void* args, int num_args, void* compass_type, int line, int column)
{
	vector<expression*> compass_args;
	for(int i=0; i<num_args; i++){
		expression* cur = ((expression**)args)[i];
		cur->assert_expression();
		compass_args.push_back(cur);
	}

	type* t = (type*) compass_type;
	location loc(line, column);
	builtin_expression* be = new builtin_expression(bt, compass_args, t, loc);
	return be;
}

extern "C" void *_make_compass_throw_expression(void *throw_exp_type,
                                                void *full_throw_exp,
                                                void *compass_type,
                                                int line,
                                                int column)
{
	location loc(line, column);
	return new throw_expression(static_cast<type *>(throw_exp_type),
	                            static_cast<expression *>(full_throw_exp),
	                            static_cast<type *>(compass_type),
	                            loc);
}

extern "C" void* _get_int_type()
{
	return get_integer_type();
}

extern "C" void* _get_compass_node(tree gcc_id)
{


	if(!(node_exists(gcc_id)))
	{
		cout << "Cannot find vardecl tree id: " << gcc_id << endl;
		assert(false);
	}


	return id_to_node[gcc_id];
}


extern "C" void* _get_variable_from_variable_decl(void *v)
{
	variable_declaration *vd = (variable_declaration*)v;
	assert(vd->node_type_id == VARIABLE_DECL);
	return vd->get_variable();
}

extern "C" void _compass_set_variable_scope(void *vd, void *b)
{
	variable_declaration *vardecl = (variable_declaration*) vd;
	block *bb = (block*) b;

	vardecl->get_variable()->set_scope(bb);

}



struct loop_info
{
	int line;
	int column;
	bool is_for;
	bool cond_first;
	tree init_exp;
	tree cond;
	tree incr;
	tree body;
};

unordered_set<unsigned long> nodes_to_ignore;

unordered_map<unsigned long, loop_info*> node_to_loop_info;


extern "C" bool _ignore_statement(tree stmt)
{
	return nodes_to_ignore.find(stmt) != nodes_to_ignore.end();
}

extern "C" loop_info* _get_loop_from_node(tree node)
{
	return node_to_loop_info[node];
}


void process_loop_node(tree & first_node, tree exp, loop_info *li)
{
	if(exp != 0)
	{
		if(first_node == 0)
		{
			first_node = exp;
			assert(node_to_loop_info[exp] == NULL);
			node_to_loop_info[exp] = li;
		}
		else nodes_to_ignore.insert(exp);
	}
}


extern "C" void* _get_compass_type(tree gcc_id)
{
	if (!type_exists(gcc_id)) {
		fprintf(stderr, "++++++++++++++++++ Type not found: %i\n", gcc_id);
		assert(false);
	}
	return id_to_type[gcc_id];
}


extern "C" void _add_loop_mapping(int line, int column, tree init_exp,
		tree cond,
		tree incr, tree body, tree break_label, tree continue_label,
		bool cond_is_first, bool is_for, tree t, tree top, tree c_label,
		tree b_label, tree entry, tree exit)
{

	loop_info *li = new loop_info();
	li->line = line;
	li->column = column;
	li->is_for = is_for;
	li->cond_first = cond_is_first;
	li->init_exp = init_exp;
	li->cond = cond;
	li->incr = incr;
	li->body = body;

	tree first_node = 0;


	process_loop_node(first_node, init_exp, li);
	process_loop_node(first_node, cond, li);
	process_loop_node(first_node, incr, li);
	process_loop_node(first_node, body, li);
	process_loop_node(first_node, break_label, li);
	process_loop_node(first_node, continue_label, li);
	process_loop_node(first_node, t, li);
	process_loop_node(first_node, top, li);
	process_loop_node(first_node, c_label, li);
	process_loop_node(first_node, b_label, li);
	process_loop_node(first_node, entry, li);
	process_loop_node(first_node, exit, li);

	assert(first_node != 0);

}



extern "C" void _compass_add_to_translation_unit(void *c_decl, char *ff)
{
	if(tu == NULL)
			tu = new translation_unit();
	string name = get_absolute_path_name(ff);
	file *f = tu->get_file_from_name(name);
	if(f == NULL)
	{
		f = new file(name.c_str());
		tu->get_files()[name] = f;
	}
	f->add_declaration((declaration*)c_decl);
}


extern "C" void _compass_translation_unit_finished(char *asm_name)
{

   assert(label_update_map.size() == 0);
   sail::TranslationUnit* sail_tu = new  sail::TranslationUnit(tu, asm_name);
   map<string,file*>::iterator it = tu->get_files().begin();

   map<string, vector<sail::Variable*> > zgv_vars;
   for(; it!=tu->get_files().end(); it++)
   {
       file* f = it->second;
       vector<declaration*> &functions = f->get_declarations();
       for(int i=0; i<(int)functions.size(); i++)
       {
           declaration* cur_decl = functions[i];
           if(cur_decl->is_fun_decl()){
               function_declaration* cur_fundecl =
                   (function_declaration*) cur_decl;
               sail::Function* sail_fn =
                   new sail::Function(cur_fundecl, f, cur_language == LANG_CPP);
               sail_tu->add_function(sail_fn);

               map<string, sail::Variable*> & cxx_zgv_vars =
            		   sail_fn->get_cxx_zvg_vars();
               map<string, sail::Variable*>::iterator it = cxx_zgv_vars.begin();
               for(; it != cxx_zgv_vars.end(); it++)
               {
            	   sail::Variable* v = it->second;
            	   zgv_vars[f->get_name()].push_back(v);
               }

           }
           if(cur_decl->is_var_decl())
           {
               variable_declaration* vd = (variable_declaration*) cur_decl;
               if(!vd->has_initializer()) continue;
               global_decls[f->get_name()].push_back(vd);
           }
       }
   }
   /*
    * If there are any global variable initializations, make up a special init
    * function that initializes these globals.
    */
   map<string, vector<variable_declaration*> >::iterator it2 =
           global_decls.begin();
   for(; it2!= global_decls.end(); it2++) {
       string file_name = it2->first;
       vector<sail::Variable*> & zgvs = zgv_vars[file_name];
       sail::Function* init_fn = new sail::Function(it2->second,
               file_name, cur_language == LANG_CPP, zgvs);
       sail_tu->add_function(init_fn);
   }



   sail::Serializer s(sail_tu, sail_output_dir);




}


/********************************************************************/
map<tree, vector<case_label*>*> case_label_map;

extern "C" void _add_case_label(tree switch_id, void* il_label)
{
	assert(switch_id!=0);
	if(case_label_map.count(switch_id) == 0){
		vector<case_label*>* labels = new vector<case_label*>();
		case_label_map[switch_id] = labels;
	}

	vector<case_label*>* case_labels = case_label_map[switch_id];
	case_label* compass_label = (case_label*)il_label;
	assert(compass_label->node_type_id == CASE_LABEL);
	case_labels->push_back(compass_label);
}

/* returns a C++ vector* */
extern "C" void* _get_case_labels(tree switch_id)
{
	return case_label_map[switch_id];
}

map<tree, map<case_label*, vector<case_label*>* >*> switch_to_equivalences;

extern "C" void _add_label_to_equivalence_class(tree switch_id,
		void *_representative, void* _l)
{
	map<case_label*, vector<case_label*>* >* equivalence_classes =
		switch_to_equivalences[switch_id];

	if(equivalence_classes == NULL)
	{
		equivalence_classes = new map<case_label*, vector<case_label*>* >();
		switch_to_equivalences[switch_id] = equivalence_classes;

	}

	case_label *l = (case_label*) _l;
	case_label *representative = (case_label*) _representative;
	if(representative == NULL)
	{
		vector<case_label*>* vec = new vector<case_label*>();
		vec->push_back(l);
		(*equivalence_classes)[l] = vec;
		return;
	}
	else if(l->is_default_label())
	{
		vector<case_label*>* vec = (*equivalence_classes)[representative];
		equivalence_classes->erase(representative);
		vec->push_back(l);
		(*equivalence_classes)[l] = vec;
		return;

	}
	vector<case_label*>* vec = (*equivalence_classes)[representative];
	assert(vec != NULL);
	vec->push_back(l);
}

map<case_label*, vector<case_label*>* >*
get_label_to_equivalence_classes(tree switch_id)
{
	map<case_label*, vector<case_label*>* >* equivalence_classes =
			switch_to_equivalences[switch_id];
	assert(equivalence_classes != NULL);
	return equivalence_classes;
}



extern "C" void start_compass_function()
{
	assert(unresolved_nodes.size()==0);
	set<unsigned long int>::iterator it = node_id_in_function.begin();
	for(; it != node_id_in_function.end(); it++)
	{
		unsigned long int k = *it;
		id_to_node.erase(k);
	}
	node_id_in_function.clear();


	label_mapping.clear();
	case_label_map.clear();
	assert(label_update_map.size() == 0);


}

extern "C" void _compass_enable_regressions()
{
	cout << "REGRESSIONS ENABLED " << endl;
}





