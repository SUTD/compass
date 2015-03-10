#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "input.h"
#include "tm.h"
#include "intl.h"
#include "tree.h" 
#include "tree-inline.h"
#include "rtl.h"
#include "flags.h"
#include "function.h"
#include "output.h"
//#include "c-tree.h"
#include "toplev.h"
#include "ggc.h"
#include "tm_p.h"
#include "cpplib.h"
#include "target.h"
#include "debug.h"
#include "opts.h"
#include "timevar.h"
#include "c-common.h"
#include "c-pragma.h"
#include "langhooks.h"
#include "tree-mudflap.h"
#include "tree-gimple.h"
#include "diagnostic.h"
#include "tree-dump.h"
#include "cgraph.h"
#include "hashtab.h" 
#include "libfuncs.h"
#include "except.h"
#include "cp/cp-tree.h"

#include "langhooks-def.h"
#include "pointer-set.h"

#include "il-main.h"
#include "il-c.h"

tree cp_global_trees[CPTI_MAX];

/* Nonzero if NODE is the std namespace.  */
#define COMPASS_DECL_NAMESPACE_STD_P(NODE)			\
  (TREE_CODE (NODE) == NAMESPACE_DECL			\
   && DECL_NAME (NODE) == std_identifier)



/**
 * General comments:
 * -------------------
 * This file performs a recursive post-order traversal (process node
 * after its children) of all nodes in the GCC AST, with
 * the starting points being the process_translation_unit and
 * process_fundecl_body (both are called from our modified gcc).
 * The return value of each process_xxx() function is a void*, representing
 * the corresponding il representation of the GCC representation.
 * To get a void* corresponding to the il class, one needs to call the
 * corresponding C++ function implemented in il-main.cpp.
 *
 */
#define DEBUG false

extern const struct lang_hooks lang_hooks;

/*
 * Flag to disable all compass work from build -- sometimes necessary
 * for bootstrapping gcc to succeed.
 */
#define DISABLE_COMPASS false


/*
 * This is set to true while we are analyzing a function body.
 * We use this flag only to skip over initializations of static variable
 * declared in functions because their initialization is already done
 * by a special init function.
 */
bool processing_function_body = false;

/* Utility implementations */

struct _vec {
	unsigned int size;
	unsigned int alloc;
	void **data;
};

typedef struct _vec _vec;

typedef _vec* vector_t;
vector_t vector_create(int alloc);
void vector_free(vector_t v);
void *vector_ith(vector_t v, int i);
void vector_append(vector_t v, void *elem);
int vector_size(vector_t v);

void free_vector_transitive(vector_t v);



static int enable_regressions = 0;

namespace_t get_namespace_ctx(tree t);

static int anonymous_counter = 0;

char* get_anonymous_name();

/*-------------------------------------*/



typedef enum language_type sail_lang_type;

sail_lang_type get_current_lang()
{
  const char* lang_name = lang_hooks.name;
  if(strcmp(lang_name, "GNU C++") == 0) return LANG_CPP;
  else if(strcmp(lang_name, "GNU C") == 0) return LANG_C;
  else return LANG_UNSUPPORTED;
}



/**
 * If you change this, make sure you also change type in
 * builtin_expression!!!!
 */
enum compass_builtin_type { _VA_ARG };

/*
 * If you change anything in this enum type, you also have to change the
 * definition in il/binop_expression.h!!!
 */
enum binop_type {
		_PLUS, _MINUS, _MULTIPLY,_DIV, _MOD,
		_POINTER_PLUS,
		_LT, _LEQ, _GT, _GEQ, /* <, <=, >, >= */
		_EQ, _NEQ, /* =, != */
		_REAL_DIV,
		_LEFT_SHIFT, _RIGHT_SHIFT, /* <<, >>*/
		_BITWISE_OR, _BITWISE_AND, _BITWISE_XOR, /* |, &, ^ */
		_LOGICAL_AND, _LOGICAL_OR, /* &&, || (shortcircuit semantics) */
		_LOGICAL_AND_NO_SHORTCIRCUIT, _LOGICAL_OR_NO_SHORTCIRCUIT,
		_VEC_BINOP /* For now, we don't reason about vector binops */
	};


/*
 * If you change this enum type, you also have to change the definition
 * in il/unop_expression.h!!!!
 */
enum unop_type {
	_NEGATE, /* e.g. -a*/
	_CONJUGATE, /* complex conjugate */
	_PREDECREMENT, _PREINCREMENT,  /* --i, ++i */
	_POSTDECREMENT, _POSTINCREMENT, /* i--, i++ */
	_BITWISE_NOT, /* ~x */
	_LOGICAL_NOT, /* !x */
	_VEC_UNOP /* For now, we don't reason about vector unops */
	};

struct _btree;

typedef struct _btree _btree;

struct _btree {

	char *key;
	void *data;

	_btree *left;
	_btree * right;
};

typedef _btree * btree_t;

btree_t btree_create(void);
void btree_insert(btree_t t, char* key, void* value);
void *btree_find(btree_t t, char* key);
void btree_keys(btree_t t, vector_t v);


struct full_name {
	char *name;
	namespace_t ctx;
};


typedef unsigned int c_id;
/*
 enum value_type {INTEGER, IEEE_FLOAT, VOID, COMPLEX, BOOL};
 typedef enum value_type value_type;
 */

enum qualifier_type {NONE = 0, EXTERN = 1, VOLATILE = 2, STATIC = 4,
	REGISTER = 8, CONSTANT = 16, PRIVATE = 32, PUBLIC=64, PROTECTED=128};
typedef enum qualifier_type qualifier_type;


const char* file_name = NULL;

/*
 * Macros
 */

#define assert(E) if (!(E)) {printf("Assert FAILED at line %i in file %s\
		 while analyzing %s\n",\
__LINE__,__FILE__, file_name);*((int*)(NULL))=2;}

#define CST_P(T) (TREE_CODE(T)== INTEGER_CST ||\
	TREE_CODE(T)== REAL_CST ||\
	TREE_CODE(T)== FIXED_CST ||\
	TREE_CODE(T)== COMPLEX_CST ||\
	TREE_CODE(T)== VECTOR_CST ||\
	TREE_CODE(T)== STRING_CST)

#define UNDEFINED_SIZE ( (unsigned long)(~(0L)))

/*
 * Convenience typedefs
 */

typedef enum tree_code_class tree_code_class;
typedef enum tree_code tree_code;

tree cur_id = 0;

tree cur_switch_id = 0;

/*
 * Prototypes
 */
void il_test(const_tree tree);
void process_translation_unit(tree tu);
void process_file(char* file, vector_t declarations);
void *process_var_declaration(tree declaration, tree parent);
void *process_var_declaration_with_compass_init(tree declaration,
		tree parent, void* compass_init_exp);
void process_function_declaration(tree declaration, tree parent);


/* This is probably one of the most central function here. It adds
 * the type info into COMPASS and associates a compass type id with
 * the GCC internal type id.
 */
void process_type(tree type);
bool process_function_type(tree type);
void process_array_type(tree type);
void process_pointer_type(tree type);
void process_base_type(tree type);
void process_base_type_integral(tree type, char *name, namespace_t ctx);
void process_base_type_enum(tree type, char *name, namespace_t ctx);
void process_base_type_struct_or_union(tree type, char *name, namespace_t ctx);
void process_vector_type(tree type);
void process_complex_type(tree type);

/*
 * Testing & helper functions
 */

void print_location(tree t);
void print_tree_node(tree t);
bool type_processed(tree t);

long int get_integer_constant(tree t);

tree get_type_id(tree t);

static int last_line = -1;
static int last_col = -1;
static char* filename = NULL;

expanded_location get_exp_location(tree exp)
{
	expanded_location el;
	el.line = -1;
	el.column = -1;
	

	if (!exp)
		return el;
	


	if (EXPR_P(exp) && EXPR_HAS_LOCATION(exp)) {
		location_t loc = EXPR_LOCATION(exp);
		expanded_location el1 = expand_location(loc);
		last_line = el1.line;
		last_col = el1.column;
		if(EXPR_FILENAME(exp) != NULL) {
			filename =  EXPR_FILENAME(exp);
		}
		return el1;
	}
	
	if (DECL_P(exp)) {
		location_t loc = DECL_SOURCE_LOCATION(exp);
		expanded_location el1 = expand_location(loc);
		last_line = el1.line;
		last_col = el1.column;
		if(DECL_SOURCE_FILE(exp)!=NULL) {
			filename = DECL_SOURCE_FILE(exp);
		}
		return el1;
	}
	el.line = last_line;
	el.column = last_col;
	return el;

}

void set_sail_output_dir(char* dir)
{
	set_sail_output_directory(dir);
}


/*
 * COMPASS connectors
 */
void make_compass_file(tree id, char* name);
void make_compass_basetype(char* name, int size, int asize, tree
gcc_id, value_type v, bool is_signed, char* typedef_name);
void make_compass_enumtype(char *name, namespace_t ctx, int type_size, int type_align_size,
		tree gcc_id, bool is_signed, vector_t elems);

void make_compass_struct_or_uniontype(char *name,
                                      namespace_t ctx,
                                      int type_size,
                                      int type_align_size,
                                      tree gcc_id,
                                      bool is_union,
                                      vector_t fields,
                                      vector_t bases,
                                      bool is_abstract);

void make_compass_constarray(long int num_elems, tree gcc_id, tree elem_id);

void make_compass_vararray(void* num_elems, tree gcc_id, tree elem_id);

void make_compass_pointertype(tree gcc_id, tree elem_id);

void make_compass_vectortype(tree type, tree elem_type, int num_elems);

void make_compass_complextype(tree type, tree elem_type);

void make_compass_functiontype(tree type, tree return_type, vector_t args,
		bool is_vararg);

void* make_compass_vardecl(char *name, namespace_t ns, tree declaration, tree parent,
		qualifier_type qt, tree type, void * c_exp);

void* make_compass_fundecl(char *name,
                           namespace_t name_ctx,
                           tree declaration,
                           tree parent,
                           qualifier_type qt,
                           tree *return_type,
                           bool is_allocator,
                           bool is_dealloc,
                           bool is_constructor,
                           bool is_destructor,
                           bool is_inline,
                           bool is_vararg,
                           void *owning_class,
                           bool is_static_member_function,
                           bool is_virtual,
                           bool is_abstract,
                           vector_t args,
                           tree body,
                           tree funtype);

/*
 * Function body processing
 */

void *process_function_body(tree body);
void *process_expression(tree init_expression, tree parent_exp,
		bool return_null_ifn_exp);
void* process_binop_expression(tree exp, binop_type binop);
void* process_unop_expression(tree exp, unop_type unop);
void process_compound_expression(tree compound_exp, vector_t leaf_exps,
		tree parent_exp);
void* process_call_expression(tree exp);
void *process_aggr_init_expression(tree exp);
vector_t get_args(tree exp); //Just a helper function

void *make_compass_dummy_exp(init_exp);

/* Tells compass all nodes for one file have been added.
 * This is when the C++ code will use all the id's to
 * reconnect the parse tree. Returns false if something goes wrong.
 */
bool end_compass_file(void);

char *function_name = "";

void *process_bind_expression(tree bind);

void* process_statement(tree exp, bool check_ignore);

void* process_decl_statement(tree exp);
void* process_set_statement(tree exp);
void* process_asm_statement(tree exp);
void* process_for_statement(tree for_stmt);
void* process_while_statement(tree while_stmt);
void* process_do_statement(tree do_stmt);
void * process_continue_statement(tree continue_stmt);

void *process_if_statement(tree exp);


void compass_add_to_translation_unit(void *compass_declaration);




//----------------------------------

int label_id = 1;

hashmap_t label_map= NULL;

tree funbody_gcc = NULL;
int last_stmt_loc = -1;

//-----------------------------------


/**
 * -------------------------------------------------------
 * Little tools.
 * -------------------------------------------------------
 */

bool subtree_contains(tree node, int code)
{
	if (!node)
		return false;
	
	if (TREE_CODE(node) == code)
		return true;
	
	int i, len = TREE_OPERAND_LENGTH(node);
	for (i = 0; i < len; ++i)
		if (subtree_contains(TREE_OPERAND(node, i), code))
			return true;
	
	return false;
}

const char *const get_tree_code_name(tree node)
{
	return tree_code_name[(int) TREE_CODE(node)];
}

/**
 * Find the leftmost (first in left-to-right post-order DFS) or rightmost
 * (first in right-to-left post-order DFS) AST node with the given tree code.
 * 
 * Useful when we have a hierarchy of compound expressions and want to find
 * the first or the last.
 * 
 * Returns NULL_TREE if no such node is found anywhere in the AST rooted at
 * the node given.
 */
tree find_leftmost_expression(tree node, int code)
{
	if (!node)
		return NULL_TREE;
	
	int i, len = TREE_OPERAND_LENGTH(node);
	
	for (i = 0; i < len; ++i) {
		tree cand = find_leftmost_expression(TREE_OPERAND(node, i), code);
		if (cand)
			return cand;
	}
	
	if (TREE_CODE(node) == code)
		return node;
	
	return NULL_TREE;
}

tree find_rightmost_expression(tree node, int code)
{
	if (!node)
		return NULL_TREE;
	
	int i, len = TREE_OPERAND_LENGTH(node);
	
	for (i = len - 1; i >= 0; --i) {
		tree cand = find_rightmost_expression(TREE_OPERAND(node, i), code);
		if (cand)
			return cand;
	}
	
	if (TREE_CODE(node) == code)
		return node;
	
	return NULL_TREE;
}


static const char *k_base_field_prefix = "$base_";
static const int k_base_field_prefix_length = 6;

/**
 * GCC makes, in every type that inherits from multiple base types, a nameless
 * field declaration corresponding to each base type (e.g. so that pointer casts
 * to base types are translated as shifting a pointer to point to the basetype's
 * field within the pointee object).  We require that all fields in an object have
 * names.  This is the function used to obtain a string that should be used as the
 * field name corresponding to the given base type.   
 *    
 * NB: Caller must free() the result.
 */
char *make_field_name_for_base_class_as_field(tree basetype)
{
	static int counter = 0;
	char *basename;
	if (!TYPE_NAME(basetype) || !DECL_NAME(TYPE_NAME(basetype))) {
		basename = xmalloc(7);
		sprintf(basename, "%d", counter++);
	}
	else basename = IDENTIFIER_POINTER(DECL_NAME(TYPE_NAME(basetype)));
	char *field_name = xmalloc(k_base_field_prefix_length + strlen(basename) + 1);
	
	strcpy(field_name, k_base_field_prefix);
	strcpy(&field_name[k_base_field_prefix_length], basename);
	
	return field_name;
}

/**
 * -------------------------------------------------------
 * AST-processing functions.
 * ------------------------------------------------------- 
 */

void* process_decl_statement(tree exp) {

	tree gcc_lval= DECL_EXPR_DECL(exp);
	tree gcc_rhs= DECL_INITIAL(gcc_lval);

	bool static_init =  processing_function_body && TREE_STATIC(gcc_lval);


	if(get_current_lang() == LANG_CPP && static_init){
		return  _make_compass_noop_instruction();
	}

	/* Return noop if decl has no init expr */
	if (gcc_rhs == 0)
		return _make_compass_noop_instruction();

	expanded_location el = get_exp_location(exp);

	expanded_location lval_el = get_exp_location(gcc_lval);;

	assert(TREE_CODE(gcc_lval) == VAR_DECL);

	void *variable_declaration = NULL;
	if(!node_exists(gcc_lval))
		variable_declaration = process_var_declaration(gcc_lval, 0);
	else variable_declaration = _get_compass_node(gcc_lval);

	void *lval_variable =
			_get_variable_from_variable_decl(variable_declaration);

	void *lval_var_exp = _make_compass_var_exp(lval_variable, lval_el.line,
			lval_el.column);

	void *rhs_expression = process_expression(gcc_rhs, exp, false);

	assert(lval_var_exp != NULL);
	assert(rhs_expression!= NULL);



	if(get_current_lang() == LANG_C && static_init) {
		assert(filename != NULL);
		add_to_global_decls(lval_variable, rhs_expression, el.line, el.column,
				filename);
		return  _make_compass_noop_instruction();
	}



	void *set_inst = _make_compass_set_instruction(lval_var_exp,
			rhs_expression, el.line, el.column, true);

	return set_inst;

}

void* process_break_statement(tree break_stmt)
{
	expanded_location el = get_exp_location(break_stmt);
	return _make_compass_break_stmt(el.line, el.column);
}

void * process_continue_statement(tree continue_stmt)
{
	expanded_location el = get_exp_location(continue_stmt);
	return _make_compass_continue_stmt(el.line, el.column);
}


void* process_while_statement(tree while_stmt)
{
	tree cond = WHILE_COND(while_stmt);
	tree body = WHILE_BODY(while_stmt);
	expanded_location el = get_exp_location(while_stmt);

	push_label_context(el.line, el.column);


	void* compass_cond = process_expression(cond, while_stmt, false);
	void* compass_body = process_statement(body, true);

	void* res = _make_compass_while_stmt(compass_cond, compass_body,
			el.line, el.column);

	pop_label_context();
	return res;
}

void* process_do_statement(tree do_stmt)
{
	tree cond = DO_COND(do_stmt);
	tree body = DO_BODY(do_stmt);
	expanded_location el = get_exp_location(do_stmt);

	push_label_context(el.line, el.column);

	void* compass_cond = process_expression(cond, do_stmt, false);
	void* compass_body = process_statement(body, true);

	void* res = _make_compass_do_stmt(compass_cond, compass_body,
			el.line, el.column);

	pop_label_context();
	return res;
}

void* process_for_statement(tree for_stmt)
{




	tree init = FOR_INIT_STMT(for_stmt);




	tree cond = FOR_COND(for_stmt);




	tree inc_exp = FOR_EXPR(for_stmt);


	tree body = FOR_BODY(for_stmt);



	expanded_location el = get_exp_location(init);



	push_label_context(el.line, el.column);

	void* compass_init_stmt = process_statement(init, true);
	void* compass_cond_exp = process_expression(cond, for_stmt, false);
	void* compass_inc_exp = process_expression(inc_exp, for_stmt, false);
	void* compass_loop_body = process_statement(body, true);

	void* res= _make_compass_for_stmt(compass_init_stmt, compass_cond_exp,
			compass_inc_exp, compass_loop_body, el.line, el.column);

	pop_label_context();
	return res;



}

void* process_set_statement(tree exp) {






	expanded_location el = get_exp_location(exp);

	tree lval= GENERIC_TREE_OPERAND (exp, 0);
	tree rhs= GENERIC_TREE_OPERAND (exp, 1);



	void *lval_exp = process_expression(lval, exp, false);

	void *rhs_exp = process_expression(rhs, exp, false);

	assert(lval_exp != NULL);
	assert(rhs_exp != NULL);

	void *set_inst = _make_compass_set_instruction(lval_exp, rhs_exp, el.line,
			el.column, false);

	return set_inst;

}

void* process_asm_statement(tree exp) {
	tree asm_string= ASM_STRING(exp); //string cnst
	tree asm_out= ASM_OUTPUTS(exp); //exp treelist
	tree asm_in= ASM_INPUTS(exp); //exp treelist
	tree asm_clobber= ASM_CLOBBERS(exp); //string treelist


	char* asm_instruction= TREE_STRING_POINTER(asm_string);

	vector_t outs = vector_create(4);
	tree cur;
	for (cur = asm_out; cur; cur = TREE_CHAIN (cur)) {
		tree out_gcc= TREE_VALUE(cur);
		void *out_exp = process_expression(out_gcc, exp, false);
		vector_append(outs, out_exp);
	}

	vector_t ins = vector_create(4);
	for (cur = asm_in; cur; cur = TREE_CHAIN (cur)) {
		tree in_gcc= TREE_VALUE(cur);
		void *in_exp = process_expression(in_gcc, exp, false);
		vector_append(ins, in_exp);
	}

	vector_t clobbers = vector_create(4);
	for (cur = asm_clobber; cur; cur = TREE_CHAIN (cur)) {
		tree clobber_gcc= TREE_VALUE(cur);
		char *c= TREE_STRING_POINTER(clobber_gcc);
		vector_append(clobbers, c);
	}

	expanded_location el = get_exp_location(exp);

	return _make_compass_assembly_instruction(asm_instruction, outs->data,
			vector_size(outs), ins->data, vector_size(ins), clobbers->data,
			vector_size(clobbers), el.line, el.column);

}

int code(tree id) {
	if (id == 0)
		return 0;
	return TREE_CODE(id);
}

void *process_if_statement(tree exp) {

	//printf("Processing if statement!\n");
	//debug_tree(exp);

	assert(TREE_CODE(exp) == COND_EXPR || TREE_CODE(exp) == IF_STMT);

	tree cond_exp = TREE_OPERAND(exp, 0);
	tree then_stmt = TREE_OPERAND(exp, 1);
	tree else_stmt = TREE_OPERAND(exp, 2);


	assert(cond_exp!= 0);
	assert(then_stmt!=0);

	void* compass_cond_exp = process_expression(cond_exp, exp, false);
	void* compass_then_stmt = process_statement(then_stmt, true);
	void* compass_else_stmt= NULL;
	if (else_stmt != 0)
		compass_else_stmt = process_statement(else_stmt, true);

	assert(compass_cond_exp != NULL);
	assert(compass_then_stmt != NULL);

	expanded_location el = get_exp_location(exp);

	void *if_stmt = _make_compass_if_statement(compass_cond_exp,
			compass_then_stmt, compass_else_stmt, el.line, el.column);

	return if_stmt;

}

bool is_lval_expr(tree exp)
{
	switch(TREE_CODE(exp))
	{
	case VAR_DECL:
	case PARM_DECL:
	case COMPONENT_REF:
	case ARRAY_REF:
	case ARRAY_RANGE_REF:
	case ALIGN_INDIRECT_REF:
	case MISALIGNED_INDIRECT_REF:
	case INDIRECT_REF:
		return true;
	default:
		return false;
	}
}

void *process_expression(tree exp, tree parent_exp, bool return_null_ifn_exp) {


	if (exp == 0) return NULL;

	//printf("******EXPRESSSION BEGIN**********\n");
	//debug_tree(exp);
	//printf("******EXPRESSSION END**********\n");

	expanded_location el = get_exp_location(exp);
	
	if (DEBUG)
		fprintf(stderr, ">> processing expression %p: %s\n", exp,
				get_tree_code_name(exp));

	tree elem_type = get_type_id(TREE_TYPE(exp));
	
	if (!elem_type) {
		if (DEBUG)
			fprintf(stderr, "    expression has 0 type\n", exp);
		
		return NULL;
	}
	
	process_type(elem_type);
	void *compass_exp_type = _get_compass_type(elem_type);

	void *compass_exp = NULL;

	int a;
	switch (code(exp)) {

	case INTEGER_CST: {
		long int_const = get_integer_constant(exp);
		compass_exp = _make_compass_integer_const(int_const, compass_exp_type,
				el.line, el.column);
		break;
	}

	case STRING_CST: {
		char* str_const= TREE_STRING_POINTER(exp);
		compass_exp = _make_compass_string_const(str_const, compass_exp_type,
				el.line, el.column);
		break;
	}

	case REAL_CST: {
		compass_exp = _make_compass_real_const(compass_exp_type, el.line,
				el.column);
		break;
	}

	case FIXED_CST:
		compass_exp = _make_compass_fixed_const(compass_exp_type, el.line,
				el.column);
		break;

	case COMPLEX_CST:
	case COMPLEX_EXPR:
		compass_exp = _make_compass_complex_const(compass_exp_type, el.line,
				el.column);
		break;

	case VECTOR_CST:
		compass_exp = _make_compass_vector_const(compass_exp_type, el.line,
				el.column);
		break;

	case FLOAT_EXPR:
	case FIX_TRUNC_EXPR:
	case FIXED_CONVERT_EXPR:
	case CONVERT_EXPR:
	case NOP_EXPR: {
		tree inner_exp= TREE_OPERAND(exp, 0);
		assert(inner_exp != 0);
		void* compass_inner_exp = process_expression(inner_exp, exp, true);
		if(compass_inner_exp == NULL) {

			void* stmt = process_statement(inner_exp, true);
			void* compass_block = _make_compass_block(NULL, 0, &stmt, 1);
			compass_exp = _make_compass_block_expression(compass_block,
			        NULL, compass_exp_type, el.line, el.column);
		}
		else {
			compass_exp = _make_compass_cast_expression(compass_inner_exp,
				compass_exp_type, el.line, el.column);
		}
		break;
	}

	case PARM_DECL:
	case VAR_DECL: {
		void *variable_declaration = NULL;
		
		if (!node_exists(exp))
			variable_declaration = process_var_declaration(exp, 0);
		else
		{
			variable_declaration = _get_compass_node(exp);
		}
		void *variable = _get_variable_from_variable_decl(variable_declaration);
		compass_exp = _make_compass_var_exp(variable, el.line, el.column);
		break;
	}
	case ALIGN_INDIRECT_REF:
	case MISALIGNED_INDIRECT_REF:
	case INDIRECT_REF: {
		tree inner_exp= TREE_OPERAND(exp, 0);
		assert(inner_exp != 0);
		void* compass_inner_exp = process_expression(inner_exp, exp, false);
		compass_exp = _make_compass_deref_expression(compass_inner_exp,
				compass_exp_type, el.line, el.column);
		break;
	}

	case ADDR_EXPR: {
		tree inner_exp = TREE_OPERAND(exp, 0);
		assert(inner_exp != 0);

		// If inner expression is a fun_decl, then we treat it specially
		if (TREE_CODE(inner_exp) == FUNCTION_DECL) {
			tree fn_type = get_type_id(TREE_TYPE(inner_exp));
			process_type(fn_type);
			void *compass_fn_type = _get_compass_type(fn_type);
			char *name = IDENTIFIER_POINTER (DECL_NAME (inner_exp));
			compass_exp = _make_compass_function_address_expression(name,
					compass_fn_type, compass_exp_type, el.line, el.column);

		}
		else if(TREE_CODE(inner_exp) ==  TARGET_EXPR &&
				is_lval_expr( TREE_OPERAND(inner_exp, 1)))
		{
			exp = TREE_OPERAND(exp, 0);
			tree var = TREE_OPERAND(exp, 0);
			assert(code(var) == VAR_DECL);

			tree initializer = TREE_OPERAND(exp, 1);

			void *compass_initializer = process_expression(initializer, exp, false);
			void *compass_var_decl = process_var_declaration_with_compass_init(var,
					exp, compass_initializer);



			void* add_exp = _make_compass_addressof_expression(compass_initializer,
					compass_exp_type, el.line, el.column);
			void* exp_inst = _make_compass_expression_instruction(add_exp);


			void *compass_block = _make_compass_block(&compass_var_decl, 1,
					&exp_inst, 1);


			compass_exp = _make_compass_block_expression(compass_block,
			                                             NULL,
			                                             compass_exp_type,
			                                             el.line,
			                                             el.column);
			break;
		}

		else {
			void *compass_inner_exp = process_expression(inner_exp, exp, false);
			compass_exp = _make_compass_addressof_expression(compass_inner_exp,
					compass_exp_type, el.line, el.column);
		}
		break;
	}

	case COMPOUND_EXPR: {
		vector_t inner_exps = vector_create(10);
		process_compound_expression(exp, inner_exps, parent_exp);
		compass_exp = _make_compass_expression_list(inner_exps->data,
				vector_size(inner_exps), compass_exp_type, el.line, el.column);
		vector_free(inner_exps);
		break;
	}

	case COMPONENT_REF: {
		tree inner_exp = TREE_OPERAND(exp, 0);
		assert(inner_exp !=0);
		void *compass_inner_exp = process_expression(inner_exp, exp, false);

		tree field = TREE_OPERAND(exp, 1);
		assert(code(field) == FIELD_DECL);
		
		/**
		 * We create our own field name here if the field has no name, since we have
		 * no way of looking it up given current infrastructure.  See also the comments
		 * in process_base_type_struct_or_union() and in
		 * make_field_name_for_base_class_as_field().
		 * 
		 * This comes up when a class A inherits from B and C, the base ctors are called
		 * with |this| set to point to nameless fields that correspond to the base classes
		 * B and C.
		 * 
		 * For instance:
		 * 
		 * stmt <cleanup_point_expr 0x7fc8cb369380 type <void_type 0x7fc8cc49f840 void>
		 *  side-effects tree_1
		 *  arg 0 <expr_stmt 0x7fc8cb369340 type <void_type 0x7fc8cc49f840 void>
		 *      side-effects
		 *      arg 0 <call_expr 0x7fc8cc481af0 type <void_type 0x7fc8cc49f840 void>
		 *          side-effects nothrow tree_4
		 *          fn <addr_expr 0x7fc8cb369300 type <pointer_type 0x7fc8cb365cc0>
		 *              constant invariant arg 0 <function_decl 0x7fc8cb363750 __base_ctor >>
		 *          arg 0 <addr_expr 0x7fc8cb361e00 type <pointer_type 0x7fc8cb3539c0>
		 *              arg 0 <component_ref 0x7fc8cc4817d0 type <record_type 0x7fc8cb353600 B>
		 *                  arg 0 <indirect_ref 0x7fc8cb361bc0 type <record_type 0x7fc8cb362600 A>
		 *                      arg 0 <nop_expr 0x7fc8cb361b80 type <pointer_type 0x7fc8cb3629c0>
		 *                          readonly arg 0 <parm_decl 0x7fc8cc482e10 this>>>
		 *              arg 1 <field_decl 0x7fc8cb35d960 D.2066>>>>
		 *      virtual.cpp:20:7>
		 *  virtual.cpp:20:7>
		 * 
		 * So we actually do make COMPONENT_REFs to these fields, and hence we synthesize a name
		 * for them in process_base_type_struct_or_union().
		 */
		
		
		char *field_name;

		bool has_name = DECL_NAME(field) ? true : false;
		//bool has_name = IDENTIFIER_POINTER(DECL_NAME(field)) != NULL;

		if (has_name) {
			field_name = IDENTIFIER_POINTER(DECL_NAME(field));

		} else {
			tree basetype = get_type_id(TREE_TYPE(field));
			assert(basetype);
			field_name = make_field_name_for_base_class_as_field(basetype);
		}
		assert(field_name != NULL);

		compass_exp = _make_compass_field_ref_expression(compass_inner_exp,
		                                                 field_name,
		                                                 compass_exp_type,
		                                                 el.line,
		                                                 el.column);
		
		if (!has_name)
			free(field_name);
		
		break;
	}

	case ARRAY_RANGE_REF:
	case ARRAY_REF:
	{
		tree array_exp = TREE_OPERAND(exp, 0);
		tree index_exp = TREE_OPERAND(exp, 1);
		void* compass_array_exp = process_expression(array_exp, exp, false);
		void* compass_index_exp = process_expression(index_exp, exp, false);
		compass_exp = _make_compass_array_ref_expression(compass_array_exp,
				compass_index_exp, compass_exp_type, el.line, el.column);
		break;
	}

	case PLUS_EXPR:
		compass_exp = process_binop_expression(exp, _PLUS);
		break;
	case MINUS_EXPR:
		compass_exp = process_binop_expression(exp, _MINUS);
		break;
	case MULT_EXPR:
		compass_exp = process_binop_expression(exp, _MULTIPLY);
		break;
	case TRUNC_DIV_EXPR:
	case EXACT_DIV_EXPR:
		compass_exp = process_binop_expression(exp, _DIV);
		break;
	case TRUNC_MOD_EXPR:
		compass_exp = process_binop_expression(exp, _MOD);
		break;
	case POINTER_PLUS_EXPR:
		compass_exp = process_binop_expression(exp, _POINTER_PLUS);
		break;
	case LT_EXPR:
		compass_exp = process_binop_expression(exp, _LT);
		break;
	case LE_EXPR:
		compass_exp = process_binop_expression(exp, _LEQ);
		break;
	case GT_EXPR:
		compass_exp = process_binop_expression(exp, _GT);
		break;
	case GE_EXPR:
		compass_exp = process_binop_expression(exp, _GEQ);
		break;
	case EQ_EXPR:
		compass_exp = process_binop_expression(exp, _EQ);
		break;
	case NE_EXPR:
		compass_exp = process_binop_expression(exp, _NEQ);
		break;
	case RDIV_EXPR:
		compass_exp = process_binop_expression(exp, _REAL_DIV);
		break;
	case LSHIFT_EXPR:
		compass_exp = process_binop_expression(exp, _LEFT_SHIFT);
		break;
	case RSHIFT_EXPR:
		compass_exp = process_binop_expression(exp, _RIGHT_SHIFT);
		break;
	case BIT_IOR_EXPR:
		compass_exp = process_binop_expression(exp, _BITWISE_OR);
		break;
	case BIT_XOR_EXPR:
		compass_exp = process_binop_expression(exp, _BITWISE_XOR);
		break;
	case BIT_AND_EXPR:
		compass_exp = process_binop_expression(exp, _BITWISE_AND);
		break;
	case TRUTH_ANDIF_EXPR:
		compass_exp = process_binop_expression(exp, _LOGICAL_AND);
		break;
	case TRUTH_ORIF_EXPR:
		compass_exp = process_binop_expression(exp, _LOGICAL_OR);
		break;
	case TRUTH_AND_EXPR:
		compass_exp = process_binop_expression(exp, _LOGICAL_AND_NO_SHORTCIRCUIT);
		break;
	case TRUTH_OR_EXPR:
		compass_exp = process_binop_expression(exp, _LOGICAL_OR_NO_SHORTCIRCUIT);
		break;
	case TRUTH_XOR_EXPR:
	{
		tree exp1 = TREE_OPERAND(exp, 0);
		tree exp2 = TREE_OPERAND(exp, 1);
		void* compass_exp1 = process_expression(exp1, exp, false);
		void* compass_exp2 = process_expression(exp2, exp, false);
		void* compass_exp1_negated = _make_compass_unop_expression(
				compass_exp1, _LOGICAL_NOT,
				_get_int_type(), el.line, el.column);
		void* compass_exp2_negated = _make_compass_unop_expression(
				compass_exp2, _LOGICAL_NOT,
				_get_int_type(), el.line, el.column);
		void* op1 = _make_compass_binop_expression(compass_exp1,
				compass_exp2_negated,  _LOGICAL_AND_NO_SHORTCIRCUIT,
				_get_int_type(), el.line, el.column);
		void* op2 = _make_compass_binop_expression(compass_exp1_negated,
				compass_exp2,  _LOGICAL_AND_NO_SHORTCIRCUIT,
				_get_int_type(), el.line, el.column);
		compass_exp = _make_compass_binop_expression(op1, op2,
				  _LOGICAL_OR_NO_SHORTCIRCUIT,
				_get_int_type(), el.line, el.column);

		break;
	}
	case VEC_LSHIFT_EXPR:
	case VEC_RSHIFT_EXPR:
	case VEC_PACK_TRUNC_EXPR:
	case VEC_PACK_SAT_EXPR:
	case VEC_PACK_FIX_TRUNC_EXPR:
		compass_exp = process_binop_expression(exp, _VEC_BINOP);
		break;
	case NEGATE_EXPR:
		compass_exp = process_unop_expression(exp, _NEGATE);
		break;
	case CONJ_EXPR:
		compass_exp = process_unop_expression(exp, _CONJUGATE);
		break;
	case PREDECREMENT_EXPR:
		compass_exp = process_unop_expression(exp, _PREDECREMENT);
		break;
	case POSTDECREMENT_EXPR:
		compass_exp = process_unop_expression(exp, _POSTDECREMENT);
		break;
	case PREINCREMENT_EXPR:
		compass_exp = process_unop_expression(exp, _PREINCREMENT);
		break;
	case POSTINCREMENT_EXPR:
		compass_exp = process_unop_expression(exp, _POSTINCREMENT);
		break;
	case BIT_NOT_EXPR:
		compass_exp = process_unop_expression(exp, _BITWISE_NOT);
		break;
	case TRUTH_NOT_EXPR:
		compass_exp = process_unop_expression(exp, _LOGICAL_NOT);
		break;
	case VEC_UNPACK_HI_EXPR:
	case VEC_UNPACK_LO_EXPR:
	case VEC_UNPACK_FLOAT_HI_EXPR:
	case VEC_UNPACK_FLOAT_LO_EXPR:
	case VEC_EXTRACT_EVEN_EXPR:
	case VEC_EXTRACT_ODD_EXPR:
	case VEC_INTERLEAVE_HIGH_EXPR:
	case VEC_INTERLEAVE_LOW_EXPR:
		compass_exp = process_unop_expression(exp, _VEC_UNOP);
		break;


	case CALL_EXPR:
	{
		compass_exp = process_call_expression(exp);
		break;
	}
	case AGGR_INIT_EXPR:
	{
		compass_exp = process_aggr_init_expression(exp);
		break;
	}
	case COND_EXPR:
	{
		tree cond_exp = TREE_OPERAND(exp, 0);
		tree then_exp = TREE_OPERAND(exp, 1);
		tree else_exp = TREE_OPERAND(exp, 2);

		void* compass_cond = process_expression(cond_exp, exp, false);
		void* compass_then = process_expression(then_exp, exp, false);
		void* compass_else = process_expression(else_exp, exp, false);

		compass_exp = _make_compass_conditional_expression(compass_cond,
				compass_then, compass_else, compass_exp_type,
				el.line, el.column);
		break;
	}

	/**
	 * We believe these to be noops for our purposes.
	 * 
	 * FIXME SAVE_EXPR actually means "evaluate the inner expression and 
	 * put in a temporary the first time it is encountered; read the
	 * temporary on every subsequent encounter".  It is used for side-effecting
	 * expressions that are referenced in multiple places in the AST.  It's not
	 * really a no-op, but this is an OK over-approximation for now.
	 */
	case MUST_NOT_THROW_EXPR:
	case NON_LVALUE_EXPR:
	case SAVE_EXPR:
	{
		tree inner_exp = TREE_OPERAND(exp, 0);
		compass_exp = process_expression(inner_exp, exp, false);
		break;
	}

	case INIT_EXPR:
	case MODIFY_EXPR: {
		void* set_instruction = process_set_statement(exp);
		compass_exp = _make_compass_modify_expression(set_instruction,
		                                              TREE_CODE(exp) == INIT_EXPR,
		                                              compass_exp_type,
		                                              el.line,
		                                              el.column);
		break;
	}

	case CONSTRUCTOR:
	{
		unsigned HOST_WIDE_INT ix;
		tree value;
		vector_t init_list = vector_create(10);
		VEC(constructor_elt,gc)* elts = CONSTRUCTOR_ELTS (exp);
		FOR_EACH_CONSTRUCTOR_VALUE (elts, ix, value)
		{
			 void* compass_exp = process_expression(value, exp, false);
			 vector_append(init_list, compass_exp);
		 // printf("ELEMENT: %i \n", code(value));
		}
		compass_exp = _make_compass_init_list_expression(
				init_list->data, vector_size(init_list), compass_exp_type,
				el.line, el.column);
		vector_free(init_list);
		break;
	}

	case MIN_EXPR:
	case MAX_EXPR:
	{
		binop_type binop;
		if(TREE_CODE(exp) == MIN_EXPR) binop = _LT;
		else binop = _GT;
		tree exp1 = TREE_OPERAND(exp, 0);
		tree exp2 = TREE_OPERAND(exp, 1);
		void* compass_exp1 = process_expression(exp1, exp, false);
		void* compass_exp2 = process_expression(exp2, exp, false);
		// type of cond is always int in min_expr
		void* cond_type = _get_int_type();
		assert(cond_type != NULL);
		void* cond = _make_compass_binop_expression(compass_exp1, compass_exp2,
				binop,cond_type, el.line, el.column);
		compass_exp = _make_compass_conditional_expression(cond, compass_exp1,
				compass_exp2, compass_exp_type, el.line, el.column);
		break;
	}
	case ABS_EXPR:
	{
		tree exp1 = TREE_OPERAND(exp, 0);
		void* compass_exp1 = process_expression(exp1, exp, false);
		void* cond_type = _get_int_type();
		void* compass_exp2 = _make_compass_integer_const(0,
				cond_type, el.line, el.column);

		void* neg_compass_exp1 =  _make_compass_unop_expression(compass_exp1,
				_NEGATE, cond_type, el.line, el.column);

		void* cond = _make_compass_binop_expression(compass_exp1, compass_exp2,
						_LT,cond_type, el.line, el.column);
		compass_exp = _make_compass_conditional_expression(cond,
				neg_compass_exp1, compass_exp1, compass_exp_type, el.line,
				el.column);
		break;
	}

	case TARGET_EXPR:
	{
		tree var = TREE_OPERAND(exp, 0);
		assert(code(var) == VAR_DECL);

		tree initializer = TREE_OPERAND(exp, 1);

		void *compass_initializer = process_expression(initializer, exp, false);
		void *compass_var_decl = process_var_declaration_with_compass_init(var,
				exp, compass_initializer);

		void *compass_block = _make_compass_block(&compass_var_decl, 1,
				NULL, 0);

		// These can also have a cleanup expression,
		// but we don't handle them until we see one.
		tree cleanup = TREE_OPERAND(exp, 2);

		compass_exp = _make_compass_block_expression(compass_block,
		                                             NULL,
		                                             compass_exp_type,
		                                             el.line,
		                                             el.column);
		break;
	}

	case VA_ARG_EXPR:
	{
		tree arg = TREE_OPERAND(exp, 0);
		assert(arg!=0);
		void* compass_arg = process_expression(arg, exp, false);
		compass_exp = _make_compass_builtin_expression(_VA_ARG, &compass_arg,
				1, compass_exp_type, el.line, el.column);
		break;
	}

	case EXPR_STMT: {
		compass_exp = process_expression(EXPR_STMT_EXPR(exp), parent_exp, true);
		if(compass_exp == NULL) {

			void* stmt = process_statement(EXPR_STMT_EXPR(exp), true);
			void* compass_block = _make_compass_block(NULL, 0, &stmt, 1);
			compass_exp = _make_compass_block_expression(compass_block,
					  NULL, compass_exp_type, el.line, el.column);
		}
		break;
	}

	case CLEANUP_POINT_EXPR: {
		/* Attempt to extract the sub-AST wrapped inside the
		   CLEANUP_POINT_EXPR node under the assumption that there aren't
		   actually any cleanups.  Parent expression is current parent,
		   because we're being sneaky. */
		
		if (subtree_contains(exp, WITH_CLEANUP_EXPR)) {
			fprintf(stderr,
					"CLEANUP_POINT_EXPR actually has cleanups"
					"inside.  Not handled.\n");
			assert(false);
		}
		exp = TREE_OPERAND(exp, 0);
		compass_exp = process_expression(exp, parent_exp, true);
		if(compass_exp == NULL) {

			void* stmt = process_statement(exp, true);
			void* compass_block = _make_compass_block(NULL, 0, &stmt, 1);
			compass_exp = _make_compass_block_expression(compass_block,
					  NULL, compass_exp_type, el.line, el.column);
		}
		break;
	}
	
	case THROW_EXPR: {
		/**
		 * So far as we know, all throw expressions wrap a compound expression
		 * where the first of the two operands is itself a tree of compound
		 * expressions whose rightmost expression is the one that has the actual
		 * expression we are throwing (and hence its type).  This is what we're
		 * really interested in, so we traverse here to find it.
		 * 
		 * FIXME Verify these assumptions in a more complicated setting
		 */
		
		tree inner_exp = TREE_OPERAND(exp, 0);
		assert(TREE_CODE(inner_exp) == COMPOUND_EXPR);
		
		tree must_not_throw_exp = find_rightmost_expression(inner_exp,
				MUST_NOT_THROW_EXPR);

		if(must_not_throw_exp == NULL)
		{
			compass_exp = _make_compass_integer_const(0,
							_get_int_type(), el.line, el.column);
			break;
		}
		assert(must_not_throw_exp);
		
		tree init_exp = find_rightmost_expression(must_not_throw_exp, INIT_EXPR);



		
		if(!init_exp)
		{


			compass_exp = _make_compass_integer_const(0,
				_get_int_type(), el.line, el.column);
			break;
		}
		

		assert(init_exp);
		
		tree throw_exp_type = get_type_id(TREE_TYPE(init_exp));
		process_type(throw_exp_type);
		void *compass_throw_exp_type = _get_compass_type(throw_exp_type);
		
		void *compass_full_throw_exp = process_expression(inner_exp, exp, false);
		
		compass_exp = _make_compass_throw_expression(compass_throw_exp_type,
		                                             compass_full_throw_exp,
		                                             compass_exp_type,
		                                             el.line,
		                                             el.column);		
		break;
	}

	case TRY_CATCH_EXPR:
	{

		/*
		 * FIXME: This is currently not correctly handled;
		 * we just process whatever is in the try block, but ignore the
		 * catch. This requires introducing a try_catch_expression.
		 */

		tree inner_exp = TREE_OPERAND(exp, 0);
		compass_exp = process_expression(inner_exp, exp, false);




		break;
	}

	/*
	 * Same as while(1) {LOOP_BODY_EXPR(exp);}
	 */
	case LOOP_EXPR:
	{

		//printf("****** $$$$ **** LOOP\n");
		//debug_tree(exp);
		//printf("****** $$$$ **** LOOP END\n");
		push_label_context(el.line, el.column);

		tree inner_exp = LOOP_EXPR_BODY(exp);
		compass_exp = process_expression(inner_exp, exp, false);

		void* compas_loop_body =
				_make_compass_expression_instruction(compass_exp);

		void* stmt = _make_compass_while_stmt(NULL, compas_loop_body,
					el.line, el.column);

		pop_label_context();

		void* compass_block = _make_compass_block(NULL, 0, &stmt, 1);
		compass_exp = _make_compass_block_expression(compass_block,
			NULL, compass_exp_type, el.line, el.column);







		break;
	}

	case EXIT_EXPR:
	{
		tree inner_exp = EXIT_EXPR_COND(exp);
		void* cond = process_expression(inner_exp, exp, false);
		void* stmt = _make_compass_exit_exp_statement(cond, el.line, el.column);
		void* compass_block = _make_compass_block(NULL, 0, &stmt, 1);
		compass_exp = _make_compass_block_expression(compass_block,
			NULL, compass_exp_type, el.line, el.column);
		break;
	}

	default: {

		if(return_null_ifn_exp) return NULL;

		compass_exp = _make_compass_integer_const(0,
				_get_int_type(), el.line, el.column);

		expanded_location pel = get_exp_location(parent_exp);
		
		if (!DEBUG)
			break;
		
		fprintf(stderr,
				"UNEXPECTED EXPRESSION! GCC code: %i, parent: %i\n",
				code(exp),
				code(parent_exp));
		fprintf(stderr, "  Expression name: %s\n", get_tree_code_name(exp));
		fprintf(stderr,	"  Found on line (line %i)\n", last_stmt_loc);
		fprintf(stderr, "  Parent expression %p: %s\n",
				parent_exp,
				parent_exp ? get_tree_code_name(parent_exp) : "");
		fprintf(stderr, "  AST Subtree:\n");
		debug_tree(parent_exp);
		//assert(false);

		break;
	}
	}

	return compass_exp;
}

/**
 * Adapted from gcc-4.3.4/gcc/cp/error.c
 * 
 * We've gotten an indirect REFERENCE (an OBJ_TYPE_REF) to a virtual                                                                                                                                                                        
 * function.  Resolve it to a close relative -- in the sense of static                                                                                                                                                                      
 * type -- variant being overridden.  That is close to what was written in                                                                                                                                                                  
 * the source code.
 */
static tree resolve_virtual_fun_from_obj_type_ref(tree ref)
{
	tree obj_type = TREE_TYPE(OBJ_TYPE_REF_OBJECT(ref));
	long int index = get_integer_constant(OBJ_TYPE_REF_TOKEN(ref));
	tree fun = BINFO_VIRTUALS(TYPE_BINFO(TREE_TYPE (obj_type)));
	while (index) {
		fun = TREE_CHAIN(fun);
		index -= (TARGET_VTABLE_USES_DESCRIPTORS ? TARGET_VTABLE_USES_DESCRIPTORS : 1);
	}

	return BV_FN(fun);
}

bool is_operator_new(tree fn_decl)
{
	assert(code(fn_decl) == FUNCTION_DECL);
	char *name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
	return(strcmp(name, "operator new")==0);
}

bool is_operator_new_array(tree fn_decl)
{
	assert(code(fn_decl) == FUNCTION_DECL);
	char *name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
	return(strcmp(name, "operator new []")==0);
}

bool is_deallocator(tree fn_decl)
{
	assert(code(fn_decl) == FUNCTION_DECL);
	char *name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
	if(strcmp(name, "operator delete")==0) return true;
	if(strcmp(name, "operator delete []")==0) return true;
	return (strcmp(name, "free")==0);

}

#define CONSTRUCTOR_SUFFIX "_$constructor"
#define DESTRUCTOR_SUFFIX "_$destructor"

char* get_name_from_fun_decl(tree fn_decl)
{
	assert(code(fn_decl) == FUNCTION_DECL);
	char *name = IDENTIFIER_POINTER(DECL_NAME(fn_decl));
	if(get_current_lang() == LANG_C) {
		char* res = xmalloc(strlen(name)+1);
		strcpy(res, name);
		return res;
	}


	if(DECL_CONSTRUCTOR_P(fn_decl) || DECL_DESTRUCTOR_P(fn_decl)) {


		/* First arg should exist  */
		tree cur_arg = DECL_ARGUMENTS(fn_decl);
		assert(cur_arg);

		assert(TREE_CODE(cur_arg) == PARM_DECL);
		tree arg_type = DECL_ARG_TYPE(cur_arg);

	    struct full_name fullname;

	    get_name_of_type(&fullname, arg_type);
	    char* type_name= fullname.name;


		assert(type_name != NULL);


		char* res;
		if(DECL_CONSTRUCTOR_P(fn_decl)) {
			res = xmalloc(strlen(type_name) + strlen(CONSTRUCTOR_SUFFIX) +1 );
			strcpy(res, type_name);
			strcpy(res+strlen(type_name), CONSTRUCTOR_SUFFIX);
		}
		else {
			res = xmalloc(strlen(type_name) + strlen(DESTRUCTOR_SUFFIX) +1 );
			strcpy(res, type_name);
			strcpy(res+strlen(type_name), DESTRUCTOR_SUFFIX);
		}

		return res;

	}

	return name;


}


void *process_call_expression(tree exp)
{


	expanded_location el = get_exp_location(exp);

	tree elem_type = get_type_id(TREE_TYPE(exp));
	process_type(elem_type);
	void *compass_exp_type = _get_compass_type(elem_type);

	void *compass_call_exp;

	tree fn_exp = CALL_EXPR_FN(exp);
	vector_t compass_args = get_args(exp);
	int num_args = vector_size(compass_args);



	if(code(fn_exp) == ADDR_EXPR || code(fn_exp) == OBJ_TYPE_REF) {

		tree fn_decl;
		long int vtbl_index = -1;
		void *compass_fn_addr_expr = NULL;

		if (code(fn_exp) == ADDR_EXPR) {
			// non-function-pointer case
			fn_decl = TREE_OPERAND(fn_exp, 0);
		} else {
			// virtual call case (function-pointer, but with more information)
			fn_decl = resolve_virtual_fun_from_obj_type_ref(fn_exp);
			vtbl_index = get_integer_constant(OBJ_TYPE_REF_TOKEN(fn_exp));
			compass_fn_addr_expr = process_expression(OBJ_TYPE_REF_EXPR(fn_exp),
					fn_exp, false);
		}


		assert(code(fn_decl) == FUNCTION_DECL);


		bool is_no_return = TREE_THIS_VOLATILE(fn_decl);
		char *name = get_name_from_fun_decl(fn_decl); //IDENTIFIER_POINTER(DECL_NAME(fn_decl));
		tree fn_type = get_type_id(TREE_TYPE(fn_decl));
		bool is_constructor = (get_current_lang() == LANG_CPP &&
				DECL_CONSTRUCTOR_P(fn_decl));
		bool is_destructor = (get_current_lang() == LANG_CPP &&
				DECL_DESTRUCTOR_P(fn_decl));




		process_type(fn_type);
		bool operator_new = (is_operator_new(fn_decl)
				|| is_operator_new_array(fn_decl));
		bool is_alloc = (DECL_IS_MALLOC(fn_decl) || operator_new);
		bool is_dealloc = is_deallocator(fn_decl);


		namespace_t ns = get_namespace_ctx(fn_decl);

		//debug_tree(fn_decl);
		//printf("Fud decl: %s NEW: %d\n", name, DECL_IS_OPERATOR_NEW(fn_decl));


		void *compass_fn_type = _get_compass_type(fn_type);
		compass_call_exp = _make_compass_function_call_expression(name, ns,
		                                                          compass_fn_type,
		                                                          compass_args->data,
		                                                          num_args,
		                                                          compass_exp_type,
		                                                          is_no_return,
		                                                          is_alloc,
		                                                          operator_new,
		                                                          is_dealloc,
		                                                          is_constructor,
		                                                          is_destructor,
		                                                          compass_fn_addr_expr,
		                                                          vtbl_index,
		                                                          el.line,
		                                                           el.column);

	} else {
		// function-pointer case
		void *compass_fnptr_exp = process_expression(fn_exp, exp, false);
		compass_call_exp = _make_compass_function_pointer_call_expression(
				compass_fnptr_exp,
				  compass_args->data,
				  num_args,
				  compass_exp_type,
				  el.line,
				  el.column);
	}

	vector_free(compass_args);

	return compass_call_exp;
}


void *process_aggr_init_expression(tree exp)
{



	expanded_location el = get_exp_location(exp);

	tree elem_type = get_type_id(TREE_TYPE(exp));
	process_type(elem_type);
	void *compass_exp_type = _get_compass_type(elem_type);

	void *compass_call_exp;

	tree fn_exp = AGGR_INIT_EXPR_FN(exp);

	//---------
	tree object = ((AGGR_INIT_VIA_CTOR_P (exp)
			  ? AGGR_INIT_EXPR_SLOT (exp)
			  : AGGR_INIT_EXPR_ARG (exp, 0)));

	while (object != NULL && TREE_CODE (object) == NOP_EXPR)
	  object = TREE_OPERAND (object, 0);

	if(object == NULL)
	{
		return  _make_compass_integer_const(0,
						_get_int_type(), el.line, el.column);
	}


	void* compass_object = process_expression(object, exp, true);
	assert(compass_object != NULL);


	//-----------

	vector_t compass_args = vector_create(10);
	tree arg;
	call_expr_arg_iterator iter;


	bool first = true;
	FOR_EACH_AGGR_INIT_EXPR_ARG (arg, iter, exp)
	{

		if(first && (AGGR_INIT_VIA_CTOR_P (exp)))
		{
			first = false;
			void* arg =_make_compass_addressof_expression(compass_object,
								NULL, -1, -1);
			vector_append(compass_args, arg);
			continue;

		}
		void* compass_arg = process_expression(arg, exp, false);
		assert(compass_arg!=NULL);
		vector_append(compass_args, compass_arg);
	}



	int num_args = vector_size(compass_args);



	if(code(fn_exp) == ADDR_EXPR || code(fn_exp) == OBJ_TYPE_REF) {

		tree fn_decl;
		long int vtbl_index = -1;
		void *compass_fn_addr_expr = NULL;

		if (code(fn_exp) == ADDR_EXPR) {
			// non-function-pointer case
			fn_decl = TREE_OPERAND(fn_exp, 0);
		} else {
			//assert(false);
			// virtual call case (function-pointer, but with more information)
			fn_decl = resolve_virtual_fun_from_obj_type_ref(fn_exp);
			vtbl_index = get_integer_constant(OBJ_TYPE_REF_TOKEN(fn_exp));
			compass_fn_addr_expr = process_expression(OBJ_TYPE_REF_EXPR(fn_exp),
					fn_exp, false);
		}


		assert(code(fn_decl) == FUNCTION_DECL);


		bool is_no_return = TREE_THIS_VOLATILE(fn_decl);
		char *name = get_name_from_fun_decl(fn_decl); //IDENTIFIER_POINTER(DECL_NAME(fn_decl));
		tree fn_type = get_type_id(TREE_TYPE(fn_decl));
		bool is_constructor = (get_current_lang() == LANG_CPP &&
				DECL_CONSTRUCTOR_P(fn_decl));
		bool is_destructor = (get_current_lang() == LANG_CPP &&
				DECL_DESTRUCTOR_P(fn_decl));




		process_type(fn_type);
		bool operator_new = (is_operator_new(fn_decl)
				|| is_operator_new_array(fn_decl));
		bool is_alloc = (DECL_IS_MALLOC(fn_decl) || operator_new);
		bool is_dealloc = is_deallocator(fn_decl);


		namespace_t ns = get_namespace_ctx(fn_decl);

		//debug_tree(fn_decl);
		//printf("Fud decl: %s NEW: %d\n", name, DECL_IS_OPERATOR_NEW(fn_decl));


		void *compass_fn_type = _get_compass_type(fn_type);
		compass_call_exp = _make_compass_function_call_expression(name, ns,
		                                                          compass_fn_type,
		                                                          compass_args->data,
		                                                          num_args,
		                                                          compass_exp_type,
		                                                          is_no_return,
		                                                          is_alloc,
		                                                          operator_new,
		                                                          is_dealloc,
		                                                          is_constructor,
		                                                          is_destructor,
		                                                          compass_fn_addr_expr,
		                                                          vtbl_index,
		                                                          el.line,
		                                                          el.column);

	} else {
		assert(false);
	}

	vector_free(compass_args);

	if(!AGGR_INIT_VIA_CTOR_P (exp))
		return compass_call_exp;
	vector_t inner_exps = vector_create(2);
	vector_append(inner_exps, compass_call_exp);
	vector_append(inner_exps, compass_object);
	void* compass_exp = _make_compass_expression_list(inner_exps->data,
				vector_size(inner_exps), NULL, -1, -1);
	vector_free(inner_exps);

	return compass_exp;

}

/*
 * Returns a vector of compass expressions from a node corresponding
 * to a call expression.
 *
 * The vector returned by this function must be freed by whoever calls it!!
 */
vector_t get_args(tree exp)
{
	vector_t compass_args = vector_create(10);
	tree arg;
	call_expr_arg_iterator iter;
	FOR_EACH_CALL_EXPR_ARG (arg, iter, exp)
	{
		void* compass_arg = process_expression(arg, exp, false);
		assert(compass_arg!=NULL);
		vector_append(compass_args, compass_arg);
	}
	return compass_args;
}

void* process_binop_expression(tree exp, binop_type binop)
{
	expanded_location el = get_exp_location(exp);

	tree elem_type = get_type_id(TREE_TYPE(exp));
	process_type(elem_type);
	void* compass_exp_type = _get_compass_type(elem_type);

	tree exp1 = TREE_OPERAND(exp, 0);
	tree exp2 = TREE_OPERAND(exp, 1);
	void* compass_exp1 = process_expression(exp1, exp, false);
	void* compass_exp2 = process_expression(exp2, exp, false);

	void* compass_binop_exp = _make_compass_binop_expression(compass_exp1,
			compass_exp2, binop, compass_exp_type, el.line, el.column);
	return compass_binop_exp;
}

void* process_unop_expression(tree exp, unop_type unop)
{
	expanded_location el = get_exp_location(exp);

	tree elem_type = get_type_id(TREE_TYPE(exp));
	process_type(elem_type);
	void* compass_exp_type = _get_compass_type(elem_type);

	tree inner_exp = TREE_OPERAND(exp, 0);
	assert(inner_exp != 0);
	void* compass_inner_exp = process_expression(inner_exp, exp, false);
	void* compass_exp = _make_compass_unop_expression(compass_inner_exp,
			unop, compass_exp_type, el.line, el.column);
	return compass_exp;
}


void process_compound_expression(tree exp, vector_t leaf_exps, tree parent_exp) {
	if (TREE_CODE(exp) != COMPOUND_EXPR) {
		void* compass_exp = process_expression(exp, parent_exp, false);
		vector_append(leaf_exps, compass_exp);
		return;
	}

	tree first= TREE_OPERAND (exp, 0);
	tree second= TREE_OPERAND(exp, 1);

	process_compound_expression(first, leaf_exps, exp);
	process_compound_expression(second, leaf_exps, exp);
}

struct loop_info {
	int line;
	int column;
	bool is_for;
	bool cond_first;
	tree init_exp;
	tree cond;
	tree incr;
	tree body;
};

typedef struct loop_info loop_info;

void *process_loop(loop_info *li);
void *process_for_loop(loop_info *li);
void *process_do_while_loop(loop_info *li);
void *process_while_loop(loop_info *li);

void *process_stmt_list(tree exp, bool force_block);

void *process_for_loop(loop_info *li) {
	void* compass_init_exp = process_expression(li->init_exp, NULL, false);
	void* compass_continue_exp = process_expression(li->cond, NULL, false);
	void* compass_incr_exp = process_expression(li->incr, NULL, false);

	void* compass_body_stmt = process_statement(li->body, false);

	//assert(compass_body_stmt != _make_compass_noop_instruction());

	void *for_loop = _make_compass_for_loop(compass_init_exp,
			compass_continue_exp, compass_incr_exp, compass_body_stmt,
			li->line, li->column);
	return for_loop;
}

void *process_do_while_loop(loop_info *li) {
	return _make_compass_noop_instruction();
}

void *process_while_loop(loop_info *li) {
	return _make_compass_noop_instruction();
}

//returns statement*
void* process_loop(loop_info *li) {


	if (li->is_for)
		return process_for_loop(li);
	if (li->cond_first)
		return process_do_while_loop(li);

	return process_while_loop(li);

}

void *process_stmt_list(tree exp, bool force_block) {

	vector_t real_stmts = vector_create(10);
	void* res_stmt = NULL;

	tree_stmt_iterator cur;
	for (cur = tsi_start(exp); !tsi_end_p(cur); tsi_next(&cur)) {
		tree stmt = tsi_stmt(cur);
		void *statement = process_statement(stmt, true);
		if (statement == _make_compass_noop_instruction())
			continue;

		vector_append(real_stmts, statement);
	}
	if (vector_size(real_stmts) == 0 && !force_block)
		res_stmt = _make_compass_noop_instruction();
	else if (vector_size(real_stmts) == 1 && !force_block)
		res_stmt = vector_ith(real_stmts, 0);
	else {
		res_stmt = _make_compass_block(NULL, 0, real_stmts->data,
                                               vector_size(real_stmts));
	}
	vector_free(real_stmts);
	return res_stmt;
}

char *get_name_from_label_decl(tree ld) {
	if (label_map == NULL)
		label_map = hashmap_create2();
	tree name=  DECL_NAME (ld);
	if (name!=0)
		return IDENTIFIER_POINTER (name);
	if (hashmap_contains2(label_map, ld))
		return hashmap_find2(label_map, ld);
	char* res = xmalloc(30);
	memset(res, 0, 30);
	char* num = xmalloc(7);
	//itoa(label_id++, num, 10);

	sprintf(num, "%d", label_id++);
	strcat(res, "__label");
	strcat(res, num);
	hashmap_insert2(label_map, ld, res);
	return res;

}

void *process_goto_statement(tree exp) {
	tree dest= GOTO_DESTINATION (exp);
	char* label_name = get_name_from_label_decl(dest);

	expanded_location el = get_exp_location(exp);
	return _make_compass_goto_statement(label_name, dest, el.line, el.column);

}

void* process_label_statement(tree exp) {
	tree label= TREE_OPERAND (exp, 0);
	char* label_name = get_name_from_label_decl(label);
	expanded_location el = get_exp_location(exp);
	return _make_compass_label_statement(label_name, label, el.line, el.column);
}

void* case_label_representative = NULL;

void* process_case_label_statement(tree exp)
{
	tree low = TREE_OPERAND(exp, 0);
	long low_int;
	bool has_low = false;
	if(low!=0){
		low_int = get_integer_constant(low);
		has_low = true;
	}

	tree high = TREE_OPERAND(exp, 1); // can be null
	long high_int;
	bool has_high = false;
	if(high!=0){
		high_int = get_integer_constant(high);
		has_high = true;
	}

	bool is_default = !has_low && !has_high;

	expanded_location el = get_exp_location(exp);

	void* compass_case_label = _make_compass_case_label_statement(low_int,
			has_low, high_int, has_high, el.line, el.column);
	_add_case_label(cur_switch_id, compass_case_label);

	_add_label_to_equivalence_class(cur_switch_id, case_label_representative,
			compass_case_label);

	if(case_label_representative == NULL || is_default)
			case_label_representative = compass_case_label;

	return compass_case_label;

}

void* process_return_statement(tree ret_stmt) {

	expanded_location el = get_exp_location(ret_stmt);
	tree expr= TREE_OPERAND(ret_stmt, 0);

	if (expr == NULL)
		return _make_compass_return_statement(NULL, el.line, el.column);


	tree return_expr= TREE_OPERAND(expr, 1);
	if(return_expr == 0)
	{
		return _make_compass_return_statement(NULL, el.line, el.column);
	}
	void* compass_return_expr = process_expression(return_expr, ret_stmt, false);
	return _make_compass_return_statement(compass_return_expr, el.line,
			el.column);

}


void* process_switch_statement(tree switch_stmt) {


	expanded_location el =get_exp_location(switch_stmt);
	push_label_context(el.line, el.column);

	tree old_switch_id = cur_switch_id;
	cur_switch_id = switch_stmt;

	void* compass_switch_body= NULL;
	tree cond= TREE_OPERAND(switch_stmt, 0);///SWITCH_COND(switch_stmt);
	assert(cond != 0); /* switch stmt must always have a condition*/
	void* compass_cond_expr = process_expression(cond, switch_stmt, false);

	tree body= TREE_OPERAND(switch_stmt, 1); //SWITCH_BODY(switch_stmt);
	if (body==0) {
		compass_switch_body = _make_compass_noop_instruction();
	} else {
		compass_switch_body = process_statement(body, true);
	}



	void* compass_case_labels = _get_case_labels(switch_stmt);

	void* compass_switch_stmt = _make_compass_switch_statement(
			compass_cond_expr, compass_switch_body, compass_case_labels,
			cur_switch_id, el.line, el.column);

	cur_switch_id = old_switch_id;

	pop_label_context();

	return compass_switch_stmt;
}

struct catch_handler *process_handler(tree handler) {

	
	struct catch_handler *h = xmalloc(sizeof(struct catch_handler));
	
	tree type = get_type_id(HANDLER_TYPE(handler));
	
	if (type) {
		process_type(type);
		h->type = _get_compass_type(type);
	} else {
		h->type = NULL;
	}
	
	h->body = process_statement(HANDLER_BODY(handler), true);
	
	return h;
}

void *process_try_catch_statement(tree try_block)
{
	expanded_location el = get_exp_location(try_block);
	
	tree try_stmt = TRY_STMTS(try_block);
	void *compass_try_stmt = process_statement(try_stmt, true);
	
	vector_t compass_handlers = vector_create(10);
	tree handlers = TRY_HANDLERS(try_block);
	
	if (TREE_CODE(handlers) == HANDLER || TREE_CODE(handlers) == BIND_EXPR) {
          /* HANDLER is the regular case, but sometimes we see a BIND_EXPR if the
             try-catch is synthesized in a silly way by something (happens for
             instance when we see delete[]). */
		vector_append(compass_handlers, process_handler(handlers));
	} else {
		assert(TREE_CODE(handlers) == STATEMENT_LIST);
		tree_stmt_iterator cur;
		
		for (cur = tsi_start(handlers); !tsi_end_p(cur); tsi_next(&cur)) {
			tree handler = tsi_stmt(cur);
			assert(TREE_CODE(handler) == HANDLER);
			vector_append(compass_handlers, process_handler(handler));
		}
	}
	
	void *tcs = _make_compass_try_catch_statement(compass_try_stmt,
	                                              compass_handlers->data,
	                                              vector_size(compass_handlers),
	                                              el.line,
	                                              el.column);
	free_vector_transitive(compass_handlers);
	
	return tcs;
}

void *process_try_finally_statement(tree try_finally)
{
	expanded_location el = get_exp_location(try_finally);

	tree try_stmt  = TREE_OPERAND(try_finally, 0);
	tree exit_stmt = TREE_OPERAND(try_finally, 1);
	
	return _make_compass_try_finally_statement(process_statement(try_stmt, true),
	                                           process_statement(exit_stmt, true),
	                                           el.line,
	                                           el.column);
}


void *process_expression_statement(tree expr_stmt)
{
	void *compass_expr = process_expression(expr_stmt, expr_stmt, false);
	return _make_compass_expression_instruction(compass_expr);
}


//returns statement*
void *process_statement(tree exp, bool check_ignore) {

	//printf("TREE exp stmt ***********\n");
	//debug_tree(exp);

	if(!exp) return NULL;

	int tree_code= TREE_CODE(exp);
	void *statement= NULL;

	if(tree_code != CASE_LABEL_EXPR)
		case_label_representative = 0;

	// Just for debugging for now
	expanded_location el = get_exp_location(exp);
	last_stmt_loc = el.line;
	// End debug

	switch (tree_code) {
	/* This only handles lists with one non-ignored (not noop) statement */
	case STATEMENT_LIST:
		statement = process_stmt_list(exp, false);
		break;
		/* Instructions */
	case DECL_EXPR:
		statement = process_decl_statement(exp);
		break;
	case MODIFY_EXPR:
		statement = process_set_statement(exp);
		break;
	case ASM_EXPR:
		statement = process_asm_statement(exp);
		break;
	case BIND_EXPR:
		statement = process_bind_expression(exp);
		break;

		/* Control statements */
	case COND_EXPR:
	case IF_STMT:
		statement = process_if_statement(exp);
		break;

	case GOTO_EXPR:
		statement = process_goto_statement(exp);
		break;

	case LABEL_EXPR:
		statement = process_label_statement(exp);
		break;

	case CASE_LABEL_EXPR:
		statement = process_case_label_statement(exp);
		break;

	case RETURN_EXPR:
		statement = process_return_statement(exp);
		break;

	case SWITCH_EXPR:
	case SWITCH_STMT:
	{
		statement = process_switch_statement(exp);
		break;
	}

	case TRY_BLOCK:
		statement = process_try_catch_statement(exp);
		break;
	
	case TRY_FINALLY_EXPR:
		statement = process_try_finally_statement(exp);
		break;
	case FOR_STMT:
	{
		statement= process_for_statement(exp);
		break;
	}
	case BREAK_STMT:
	{
		statement = process_break_statement(exp);
		break;
	}
	case WHILE_STMT:
	{
		statement = process_while_statement(exp);
		break;
	}
	case DO_STMT:
	{
		statement = process_do_statement(exp);
		break;
	}

	case CONTINUE_STMT:
	{
		statement = process_continue_statement(exp);
		break;
	}

	default:

		statement = process_expression_statement(exp);

	}

	return statement;

}



/*
 * Returns a block*
 */
void *process_bind_expression(tree bind) {
	assert(TREE_CODE(bind) == BIND_EXPR);

	/*
	 * Contains id of variable_declaration*
	 */
	vector_t decls = vector_create(10);

	tree vars= BIND_EXPR_VARS(bind);

	tree cur;


	for (cur = vars; cur; cur = TREE_CHAIN (cur)) {
		if (TREE_CODE(cur) != VAR_DECL)
			continue;
		void *vd = process_var_declaration(cur, 0);
		assert_vardecl(vd);
		vector_append(decls, vd);

	}

	/*
	 * Retrieve statements from body
	 */
	tree body= BIND_EXPR_BODY(bind);

	/*
	 * statement * vector
	 */
	vector_t statements;

	if (TREE_CODE(body) == STATEMENT_LIST) {
		statements = vector_create(20);

		tree_stmt_iterator cur;
		for (cur = tsi_start(body); !tsi_end_p(cur); tsi_next(&cur)) {
			tree stmt = tsi_stmt(cur);
			void *statement = process_statement(stmt, true);

			assert(statement != NULL);
			vector_append(statements, statement);
		}
	} else {
		statements = vector_create(1);
		void *statement = process_statement(body, true);

		/* Some Gcc statements turn into noops */
		assert(statement != NULL);
		vector_append(statements, statement);
	}
	void *block = _make_compass_block(decls->data, vector_size(decls),
			statements->data, vector_size(statements));

	int i;
	for (i=0; i < vector_size(decls); i++) {
		void *vd = vector_ith(decls, i);
		_compass_set_variable_scope(vd, block);

	}
	return block;

}


// can return any instruction
void *process_function_body(tree body)
{
    void *block;
    funbody_gcc = body;


    processing_function_body = true;



    if (TREE_CODE(body) == BIND_EXPR) {
        block = process_bind_expression(body);
    } else {
        if (TREE_CODE(body) == CLEANUP_POINT_EXPR) {
            /* Attempt to extract the real function body from within the
               CLEANUP_POINT_EXPR node */
            if (subtree_contains(body, WITH_CLEANUP_EXPR)) {
                fprintf(stderr,
                        "Function body is a CLEANUP_POINT_EXPR that actually"
                        "has cleanups inside.  Not handled.\n");
                assert(false);
            }
            body = TREE_OPERAND(body, 0);
        }

        block = process_statement(body, true);
    }

    processing_function_body = false;
    return block;
}

//----------------------------------
/*
void *make_compass_dummy_exp(tree exp) {
	return _make_compass_dummy_exp(exp);
}
*/

//------------------------------------


void il_test(const_tree tree) {
	cpp_test(tree);
}

/*
 * Recursively lookup declaration scope and chain together the
 * names of the enclosing classes and namespaces.
 */
namespace_t get_namespace_ctx(tree t)
{
	char *name;
	namespace_t ctx;

    /* if TYPE_P (t) passes then we have a class, otherwise a declaration */
	tree scope = TYPE_P(t) ? TYPE_CONTEXT(t) : DECL_CONTEXT(t);
	
	/* null scope means we've hit global scope */
	if (scope && get_current_lang() == LANG_CPP) {
		char *name;
		if (TYPE_P(scope))
			name = IDENTIFIER_POINTER(DECL_NAME(TYPE_NAME(scope)));
		else{
			if(DECL_NAME(scope) == NULL) return NULL;
			assert(DECL_NAME(scope) != NULL);
			name = IDENTIFIER_POINTER(DECL_NAME(scope));
		}
		
		return namespace_make(name, get_namespace_ctx(scope));
	} else {
		return NULL;
	}
}

/*
 * Returns the name of a non-pointer type
 */
void get_name_of_basetype(struct full_name *result, tree type)
{


	if(TREE_CODE(type) == INTEGER_CST || TREE_CODE(type) == 212)
	{
		//long int get_integer_constant(tree t);
		 result->name = "<invalid>";
		 result->ctx = NULL;
		 return;

	}

	//printf("Tree code: %d\n", TREE_CODE(type));

    assert(TYPE_NAME(type) != NULL);
    assert(!POINTER_TYPE_P(type));



    if (TREE_CODE (TYPE_NAME (type)) == IDENTIFIER_NODE) {
        result->name = IDENTIFIER_POINTER (TYPE_NAME (type));
        result->ctx = NULL;
    } else {

    	if(!TYPE_P (type))
    	{
    		 result->name = "<invalid>";
    		 result->ctx = NULL;
    		 return;
    	}
    	assert(TYPE_P (type));

        assert(TREE_CODE (TYPE_NAME (type)) == TYPE_DECL);
        assert(DECL_NAME (TYPE_NAME (type)));

        result->name = IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type)));
        result->ctx = get_namespace_ctx(type); 
    }

    assert(result->name != NULL);
}

/*
 * We have our own type routines to prepare for the C++ references
 * and vtable pointers down the road.
 */
bool is_pointer_type(tree t)
{
	return POINTER_TYPE_P(t);
}

bool is_array_type(tree t)
{
	return TREE_CODE (t) == ARRAY_TYPE;
}



/*
 * Includes any *'s for dereferences.
 */
void get_name_of_actual_type(struct full_name* fn, tree type)
{
	int num_stars = 0;

	tree d = type;
	while(is_pointer_type(d))
	{
		d = TREE_TYPE(d);
		num_stars++;
		if(d == 0) break;

	}

	get_name_of_type(fn, type);

	char* temp = xmalloc(strlen(fn->name) + num_stars + 1);
	strcpy(temp, fn->name);
	char* t = temp+strlen(fn->name);
	while(num_stars-->0) {
		strcpy(t++, "*");
	}
	fn->name = temp;

}


char* get_name_of_template_type(tree t)
{
	if(TREE_CODE(t) != TREE_VEC)
	{
		struct full_name fn;
		get_name_of_actual_type(&fn, t);

		char* ns = namespace_tostring(fn.ctx);
		char* res = xmalloc(strlen(ns) + strlen(fn.name) + 1);
		char* final = res;
		strcpy(res, ns);
		res+=strlen(ns);
		strcpy(res, fn.name);

		return final;
	}
	char* res = xmalloc(10000);
	char* final = res;
	strcpy(res, "<");
	res++;
	int i;
	for (i = 0; i < TREE_VEC_LENGTH (t); i++)
	{
		tree inner = TREE_VEC_ELT (t, i);
		char* cur = get_name_of_template_type(inner);
		strcpy(res, cur);
		res+=strlen(cur);
		free(cur);
		if(i!=TREE_VEC_LENGTH (t)-1){
			strcpy(res, ", ");
			res +=2;
		}


	}
	strcpy(res, ">");
	return final;


}



void get_name_of_type(struct full_name *result, tree t)
{

    if (!t) {
        result->name = "";
        result->ctx = NULL;
        return;
    }

    /* Get pointee type of any pointer types */
    while (TYPE_NAME(t) == NULL) {
        t = TREE_TYPE(t);
        if (!t) {
            result->name = "";
            result->ctx = NULL;
            return;
        }
    }

    get_name_of_basetype(result, t);
    if(get_current_lang() == LANG_CPP &&
    		TREE_CODE (t) == RECORD_TYPE && CLASSTYPE_TEMPLATE_INFO(t))
    {
		tree temp = TI_ARGS(CLASSTYPE_TEMPLATE_INFO(t));
		char* res = get_name_of_template_type(temp);
		char* new_name = xmalloc(strlen(result->name) + strlen(res) + 1);
		strcpy(new_name, result->name);
		strcpy(new_name+strlen(result->name), res);
		free(res);
		result->name = new_name;
    }

}

hashmap_t processed_types= NULL;


bool is_complete_struct_type(tree type)
{
	if(TREE_CODE (type) != RECORD_TYPE && TREE_CODE (type) != UNION_TYPE)
		return false;

	return  TYPE_SIZE(type);
}

/*
 * Process the GCC type and add it to COMPASS.
 */
void process_type(tree type)
{

	if (!type) {
        return;
	}

	type = get_type_id(type);

	if(is_complete_struct_type(type) && type_exists_as_incomplete_struct(type))
	{
		process_base_type(type);
	}

	if (type_exists(type)) {
        return;
    }



    if (is_array_type(type)) {
    	process_array_type(type);

    }
    else if (is_pointer_type(type))  {
    	process_pointer_type(type);
    }
    else if (TREE_CODE(type) == FUNCTION_TYPE) {
    	process_function_type(type);
    }
    else if (TREE_CODE(type) == METHOD_TYPE) {
    	process_function_type(type);
    }
    else if (TREE_CODE(type) == VECTOR_TYPE) {
    	process_vector_type(type);

    }
    else if (TREE_CODE(type) == COMPLEX_TYPE) {
    	process_complex_type(type);

    }
    else {
        process_base_type(type);
    }



}

/*
 * A vector type is an oversized register that
 * can hold num_elems things of type elem_type
 * (like 4 ints next to each other) and perform
 * operations on them in parallel (like the old
 * vector computers).
 * This is basically a way to export special purpose assembly
 * from recent IA32 and IA64 instructions (MMX, SSEx, ...) in an
 * architecture-independant way. Unfortunately, the standard library
 * uses this...
 * Clearly, this has to be removed ASAP in IL.
 */
void process_vector_type(tree type) {
	assert(TREE_CODE(type) == VECTOR_TYPE);

	tree elem_type= TREE_TYPE(type);
	process_type(elem_type);

	int num_elems= TYPE_VECTOR_SUBPARTS(type);

	make_compass_vectortype(type, elem_type, num_elems);
}

/*
 * Complex type is a built-in type with real/img parts
 * of type elem_type and special (optimized) functions in
 * complex.h to operate on.
 */
void process_complex_type(tree type) {
	assert(TREE_CODE(type) == COMPLEX_TYPE);
	tree elem_type= TREE_TYPE(type);
	process_type(elem_type);

	make_compass_complextype(type, elem_type);

}

/**
 * FIXME vararg discovery here appears to be broken.  All functions
 * are being marked as vararg.
 */
bool process_function_type(tree type)
{

	tree return_type = TREE_TYPE(type);
	/* GCC is inconsistent: some functions w/ type qualifiers
	 * and void return have VOID_TYPE instead of NULL
	 * as return type. We make sure that return_type
	 * is only set if a function actually returns something
	 */
	if (VOID_TYPE_P(return_type))
		return_type = NULL;

	process_type(return_type);

	bool is_vararg = false;
	tree cur;
	vector_t args = vector_create(10);

	for (cur = TYPE_ARG_TYPES(type); cur; cur = TREE_CHAIN(cur)) {
		tree arg_type = TREE_VALUE(cur);
		if(arg_type == void_type_node) {
			is_vararg = true;
			break;
		}

		process_type(arg_type);
		vector_append(args, arg_type);
	}

	make_compass_functiontype(type, return_type, args, is_vararg);
	return is_vararg;

}
/*
 Used to represent the type of non-member functions and of static member
 functions. The TREE_TYPE gives the return type of the function.
 The TYPE_ARG_TYPES are a TREE_LIST of the argument types. The
 TREE_VALUE of each node in this list is the type of the corresponding
 argument; the TREE_PURPOSE is an expression for the default argument
 value, if any. If the last node in the list is void_list_node (a TREE_LIST
 node whose TREE_VALUE is the void_type_node), then functions of this
 type do not take variable arguments. Otherwise, they do take a
 variable number of arguments.

 Note that in C (but not in C++) a function declared like void f()
 is an unprototyped function taking a variable number of
 arguments; the TYPE_ARG_TYPES of such a function will be NULL.

 */
void process_array_type(tree type) {

	tree at= TYPE_DOMAIN(type);
	tree elem_type= TREE_TYPE(type);
	process_type(elem_type);
	assert (elem_type != NULL);
	if (at == NULL) /* Incomplete array type decl */
	{
		make_compass_constarray(UNDEFINED_SIZE, type, elem_type);
		return;
	}
	assert(at != NULL);
	tree maxval= TYPE_MAX_VALUE (at);
	/* Does the array have constant size? */
	if (maxval == NULL || TREE_CODE(maxval) == INTEGER_CST) {

		long int array_size;
		if (maxval == NULL) { /* Declaration of int a[] in struct, undefined
		 length */
			array_size = UNDEFINED_SIZE;
		} else {
			assert(TREE_CODE(maxval) == INTEGER_CST);
			/* Add one since GCC will give us the last index (why??) */
			array_size = get_integer_constant(maxval) + 1;
		}

		make_compass_constarray(array_size, type, elem_type);
	} else /* C90 vararry declaration */
	{
		assert(EXPR_P(maxval));

		void* compass_size_exp = process_expression(maxval, type, false);

		make_compass_vararray(compass_size_exp, type, elem_type);
	}


	//tree maxval = TYPE_MAX_VALUE (TYPE_DOMAIN (t));


}

void process_pointer_type(tree type) {
	/*int i = 0;
	 while(is_pointer_type(type))
	 {
	 i++;
	 type = TREE_TYPE(type);
	 }*/
	tree elem_type = get_type_id(TREE_TYPE(type));
	assert(elem_type);

	process_type(elem_type);
	make_compass_pointertype(type, elem_type);

}

struct enum_elem {
    long int value;
    char *name;
};

typedef struct enum_elem enum_elem;

/*
 * Union, struct, complex, C builtin
 */
void process_base_type(tree type) {
    /* Word to the wise: Enum is an integral type in C,
     * but aggregate in C++. Therefore, we use slightly strange
     * conditionals to keep these things here language-agnostic.
     */
    struct full_name fullname;

    get_name_of_type(&fullname, type);
    if (!AGGREGATE_TYPE_P(type) && TREE_CODE (type) != ENUMERAL_TYPE) {
        process_base_type_integral(type, fullname.name, fullname.ctx);
    } else if (TREE_CODE (type) == ENUMERAL_TYPE) {
        process_base_type_enum(type, fullname.name, fullname.ctx);
    } else if (TREE_CODE (type) == RECORD_TYPE ||
               TREE_CODE (type) == UNION_TYPE) {
        process_base_type_struct_or_union(type, fullname.name, fullname.ctx);
    } else {
        /* there should be no other base types */
        assert(false);
    }
}

/*
 * Integral (non-enum) types
 */
void process_base_type_integral(tree type, char *name, namespace_t ctx)
{

    /* Number of bits used to represent this type
     * This can be NULL if the type does not have a size,
     * i.e. typedef void my_type; extern my_type t;
     * */
    int type_size = 0;

    if (TYPE_SIZE(type) != NULL)
        type_size = TREE_INT_CST_LOW(TYPE_SIZE(type));

    /* Number of bits used for alignment of this type:
     * It is possible that an architecture has 24-bit
     * integers but requires 32 bit alignment.
     */
    int type_align_size= TYPE_ALIGN(type);

    bool is_signed = !TYPE_UNSIGNED(type);

    value_type v;
    if (VOID_TYPE_P(type))
        v = VOID;
    else if (TREE_CODE (type) == COMPLEX_TYPE)
        v = COMPLEX;
    else if (TREE_CODE (type) == INTEGER_TYPE)
        v = INTEGER;
    else if (TREE_CODE (type) == OFFSET_TYPE)
    	v = INTEGER;
    else if (TREE_CODE (type) == REAL_TYPE)
        v = IEEE_FLOAT;
    else if (TREE_CODE (type) == BOOLEAN_TYPE)
        v = BOOL;
    else {
    	v = INTEGER;
       // printf("~~~~~~~~ Unknown tree code %i\n", TREE_CODE(type));
        //return;
       // assert(false); /* This shold be an exhaustive list */
    }

    /* TODO: Typedef not added. Perhaps add ref to decl_node if
     * this should ever be desired
     */
    make_compass_basetype(name, type_size, type_align_size, type, v, is_signed, "");
}

void process_base_type_enum(tree type, char *name, namespace_t ctx)
{
    /* Number of bits used to represent this type:
     * This is *not* alwasy 32. If a larger value is
     * manually specified, the type will "upgrade" to
     * 64 bits, but not more.
     */
    int type_size = TREE_INT_CST_LOW(TYPE_SIZE(type));

    /* Number of bits used for alignment of this type:
     * It is possible that an architecture has 24-bit
     * integers but requires 32 bit alignment. However,
     * we believe that on the IA32 and IA64 type_size ==
     * type_align_size.
     */
    int type_align_size= TYPE_ALIGN(type);

    bool is_signed = !TYPE_UNSIGNED(type);
    tree vals = TYPE_VALUES(type);

    vector_t elems = vector_create(20);

    tree cur;
    for (cur = vals; cur; cur = TREE_CHAIN (cur)) {
        tree purpose = TREE_PURPOSE(cur);
        assert(TREE_CODE(purpose) == IDENTIFIER_NODE);

        tree value = TREE_VALUE(cur);
        assert(TREE_CODE(value) == INTEGER_CST);

        char* enum_name = IDENTIFIER_POINTER(purpose);

        /* This can be a signed or unsigned
         * (up to) 64 bit value. To interpret correctly,
         * check is_signed
         */
        assert(TREE_CODE(value) == INTEGER_CST);
        long int enum_value = get_integer_constant(value);

        enum_elem *ev = xmalloc(sizeof(enum_elem));
        ev->value = enum_value;
        ev->name = enum_name;
        vector_append(elems, ev);
    }

    make_compass_enumtype(name, ctx, type_size, type_align_size, type, is_signed,
                          elems);
}

long int get_integer_constant(tree t)
{
    assert(TREE_CODE(t) == INTEGER_CST);
    return TREE_INT_CST_LOW(t) | (TREE_INT_CST_HIGH(t) << 32);
}

struct field_elem {
    /* Number of bits from beginning of struct/union */
    int bit_offset;
    char* name;

    /* GCC id of declared type */
    tree gcc_type;
};

typedef struct field_elem field_elem;

void process_base_type_struct_or_union(tree type, char *name, namespace_t ctx)
{
    vector_t vfields = vector_create(20);
    vector_t bases = vector_create(5);
    
    bool is_union = (TREE_CODE(type) == UNION_TYPE);
    

    bool is_abstract = false;

    if(get_current_lang() == LANG_CPP) {
    	is_abstract = CLASSTYPE_PURE_VIRTUALS(type)!=0;
    }


    /* Incomplete type */
    if (!TYPE_SIZE(type)) {
    	//printf("***** incomplete type: \n");
    	//debug_tree(type);



        make_compass_struct_or_uniontype(name,
                                         ctx,
                                         UNDEFINED_SIZE,
                                         UNDEFINED_SIZE,
                                         type,
                                         is_union,
                                         vfields,
                                         bases,
                                         is_abstract);
        return;
    }



    /* Number of bits used to represent this type:
     * This is *not* always 32. If a larger value is
     * manually specified, the type will "upgrade" to
     * 64 bits, but not more.
     */
    int type_size = TREE_INT_CST_LOW(TYPE_SIZE(type));

    /* Number of bits used for alignment of this type:
     * It is possible that an architecture has 24-bit
     * integers but requires 32 bit alignment.
     */
    int type_align_size = TYPE_ALIGN(type);

    bool is_signed = !TYPE_UNSIGNED(type);
    
    /* Process fields */

    vector_t elems = vector_create(20);
    tree fields = TYPE_FIELDS(type);


    vector_t elem_types = vector_create(20);

	if(fields != NULL)
	{

        tree cur;
        for (cur = fields; cur; cur = TREE_CHAIN(cur)) {
            /* We may get an artificial TYPE_DECL here, corresponding
               to the containing struct / class, when parsing C++.  This
               is because the C++

                   struct S {};

               is more or less equivalent to the C

                   struct S {};
                   typedef struct S S;

               (Note that we may see some TYPE_DECLs other than this one
               as well.) -Roy

               We can also see a VAR_DECL for fields declared as static.
               */

         /*   if(TREE_CODE(cur) != FIELD_DECL && TREE_CODE(cur) != TYPE_DECL &&
            		 TREE_CODE(cur) != VAR_DECL )
            {
            	printf("Unexpected field in struct %s!\n", name);
            	debug_tree(cur);

            	printf("*********************\n");
            	debug_tree(type);
            	printf("*********************\n");
            	assert(false);
            } */


            if (TREE_CODE(cur) == FIELD_DECL) {
                /* Note: Safe to use only lower part,
                 * cannot imagine a field larger then 4GB :)
                 */
                int field_offset = 0;
                tree o = DECL_FIELD_OFFSET(cur);
                if(o != NULL)
                	field_offset = TREE_INT_CST_LOW(o);
                int field_bit_offset = 0;
                o = DECL_FIELD_BIT_OFFSET(cur);
                if(o != NULL)
                	field_bit_offset = TREE_INT_CST_LOW(o);
                int bit_offset = field_offset*8 + field_bit_offset;

                /* Some fields have no name.  For instance, g++ makes a field_decl for
                   every non-virtual base of this class (unless the class is empty, cf
                   gcc/cp/class.c).  These field declarations have no name but have
                   type corresponding to the base class.  However, we require a name since
                   we map to a record's field information by field name (cf record_type in
                   type.h), so we make up an unlikely name if one isn't provided. */
                
                field_elem *fe = xmalloc(sizeof(struct field_elem));

                fe->bit_offset = bit_offset;
				fe->gcc_type = TREE_TYPE(cur);

                if (DECL_NAME(cur)) {
					fe->name = IDENTIFIER_POINTER(DECL_NAME(cur));
                } else {
                	tree basetype = get_type_id(TREE_TYPE(cur));
                	assert(basetype);
                	fe->name = make_field_name_for_base_class_as_field(basetype);
                }

                /**
                 * FIXME Fields without names whose type also has no name do exist (e.g.
                 * in throw expressions), but it doesn't look like we care about them at
                 * all. 
                 */
                if (fe->name)
                	vector_append(vfields, fe);
            }

            /* Process the field's or TYPE_DECL's type (if not yet encountered). */
            vector_append(elem_types, get_type_id(TREE_TYPE(cur)));
           // process_type(TREE_TYPE(cur));
        }
    }

    /* Process base classes for inheritance graph update */

    int i;
    tree base_binfo, binfo = TYPE_BINFO(type);
    
    if (binfo) {
		for (i = 0; BINFO_BASE_ITERATE(binfo, i, base_binfo); ++i) {
			tree basetype = get_type_id(BINFO_TYPE(base_binfo));
			
			process_type(basetype);
			
			struct base_class *bc = xmalloc(sizeof(struct base_class));
			
			bc->type = _get_compass_type(basetype);
			bc->offset = get_integer_constant(BINFO_OFFSET(base_binfo));
			
			vector_append(bases, bc);
		}
    }
    


    make_compass_struct_or_uniontype(name,
                                     ctx,
                                     type_size,
                                     type_align_size,
                                     type,
                                     is_union,
                                     vfields,
                                     bases, is_abstract);
    

    for(i=0; i < vector_size(elem_types); i++)
    {
    	tree t = vector_ith(elem_types, i);
    	process_type(t);
    }

    free_vector_transitive(bases);
}



/*
 * One TU is the fundamental unit in our framework.
 * However, we want "files", not all the random crap
 * that happens to be included (from some god-awful .h
 * mess) all in one file. Therefore, we look at all the
 * definitions and sort them by file.
 */
void process_translation_unit(tree tu) {
	if(DISABLE_COMPASS)
		return;

	if(errorcount) return;


	/*
	 * Register used language with C++
	 */
	set_language(get_current_lang());

	if(enable_regressions)
		_compass_enable_regressions();
	btree_t files = btree_create();

	tree cur;
	for (cur = tu; cur; cur = TREE_CHAIN (cur)) {
		expanded_location el = get_exp_location(cur);
		char *file_name = el.file;

		vector_t vec = btree_find(files, file_name);
		if (vec == NULL) {
			vec = vector_create(25);
			btree_insert(files, file_name, vec);
		}
		vector_append(vec, cur);
	}

	vector_t all_keys = vector_create(25);
	btree_keys(files, all_keys);
	int i;
	for (i=0; i< vector_size(all_keys); i++) {
		char* k = vector_ith(all_keys, i);
		vector_t values = btree_find(files, k);
		assert(values != NULL);

		process_file(k, values);

	}


	_compass_translation_unit_finished(asm_file_name);

}



bool type_processed(tree t)
{
    if (processed_types == NULL)
	processed_types = hashmap_create();

    assert(t != NULL);

    tree ct = TYPE_CANONICAL(t);
    if (ct == NULL)
        ct = t;

    if (hashmap_contains(processed_types, ct))
	return true;

    hashmap_insert(processed_types, ct);

    return false;
}

/*
 * Process all declarations for one file with
 * given name. Note that this includes the
 * the gcc builtin functions in a separate
 * file as well.
 */

void process_file(char* file, vector_t declarations) {

	/* First, add this file to compass */
	make_compass_file(cur_id, file);

	/* Now, iterate through all decls and emit */
	int i;
	c_id our_id = cur_id;
	for (i=0; i < vector_size(declarations); i++) {
		++cur_id;
		process_declaration(vector_ith(declarations, i), 0);
	}

	end_compass_file();
	//hashmap_delete(processed_types);
	//processed_types = NULL;

}

/*
 * Switching function: Determines the concrete type of decl
 * and calls the appropriate process_ function (if necessary).
 */
void process_declaration(tree declaration, tree parent) {

	if(errorcount) return;

	if (declaration == NULL)
		return;

	if(!processing_function_body)
	{
		/*
		 * Don't analyze function bodies in system headers.
		 */
		if(DECL_IN_SYSTEM_HEADER(declaration)) {
			return;
		}

		namespace_t ns = get_namespace_ctx(declaration);

		if(is_namespace_std(ns)) {
			return;
		}
	}


	assert(DECL_P(declaration));
	tree_code code= TREE_CODE(declaration);
	switch (code) {
	case FUNCTION_DECL:
		if (DECL_SAVED_TREE(declaration) == NULL)
			break;
		/* Body here will always be NULL */
		assert(false);
		break;

	case LABEL_DECL:

		break;
	case FIELD_DECL:
		/* Should get this indirectly from type info */
		break;

	case VAR_DECL:
	{
		void * global_decl = process_var_declaration(declaration, parent);
		expanded_location el = get_exp_location(declaration);
		file_name = el.file;
		_compass_add_to_translation_unit(global_decl, file_name);
		break;
	}
		/* An enum */
	case CONST_DECL:
		/* Should get this indirectly from type info */
		break;

	case PARM_DECL:
		/* Should be handled in fundecl */
		assert(false)
		;
		break;

	case TYPE_DECL:
		/* Should get this indirectly from type info */
		break;

	case RESULT_DECL:
		/* Return value decl, should be hadled by fundecl */
		assert(false)
		;

		break;
	case NAMESPACE_DECL:
	{
		break;
	}
	default:
		printf("Unexpected decl: %d", code);
		debug_tree(declaration);
		assert(false);
	}
}

struct arg {
	char *name;
	namespace_t ns;
	tree type;
	qualifier_type qt;
	tree gcc_id;

};

typedef struct arg arg;


/*
 * Process one function declaration. A fundec has
 * location
 * name
 * return type
 * list (arg name, arg type)
 * type qualifiers
 * body
 * is_allocator
 */
void process_function_declaration(tree declaration, tree parent_id) {
	assert(declaration != NULL);



	tree_code code = TREE_CODE(declaration);
	assert(code == FUNCTION_DECL);

	expanded_location el = get_exp_location(declaration);
	file_name = el.file;



	/* Process the type of the function.
	 * Incidentally, this is also the only way to find out if
	 * this is a vararg function
	 */
	tree function_type = TREE_TYPE(declaration);

	bool is_vararg = process_function_type(function_type);

	char *name = get_name_from_fun_decl(declaration); //IDENTIFIER_POINTER(DECL_NAME(declaration));

	/* for debuggging only */
	function_name = name;

	tree parent = DECL_CONTEXT(declaration);

	/* Make sure that parent is null except for nested functions and
	 * member functions in classes/stucts
	 */
	if (parent != NULL && TREE_CODE(parent) == TRANSLATION_UNIT_DECL)
		parent = NULL;
	
	namespace_t name_ctx = get_namespace_ctx(declaration);

	/* Get all (applicable) type qualifiers */
	qualifier_type qt = NONE;
	if (TREE_STATIC(declaration))
		qt |= STATIC;
	if (DECL_EXTERNAL(declaration))
		qt |= EXTERN;

	/* Does GCC know that this is an allocator? */
	bool is_allocator = (DECL_IS_MALLOC(declaration) ||
				is_operator_new(declaration));

	bool is_dealloc = is_deallocator(declaration);

	bool is_constructor = (get_current_lang() == LANG_CPP &&
			DECL_CONSTRUCTOR_P(declaration));
	bool is_destructor = (get_current_lang() == LANG_CPP &&
			DECL_DESTRUCTOR_P(declaration));

	/* INLINE */
	bool is_inline = DECL_DECLARED_INLINE_P(declaration);

	/* NULL for void return decls */
	tree return_decl = DECL_RESULT(declaration);
	tree return_type = NULL;

	if (return_decl != NULL) {
		assert(TREE_CODE(return_decl) == RESULT_DECL);

		/* Find the type of declaration & process */
		return_type = TREE_TYPE(return_decl);

		process_type(return_type);

		/* GCC is inconsistent: some functions w/ type qualifiers
		 * and void return have VOID_TYPE instead of NULL
		 * as return type. We make sure that return_type
		 * is only set if a function actually returns something
		 */
		if (VOID_TYPE_P(return_type))
			return_type = NULL;
	}

	/* Process PARM_DECL's */

	/* Can be null */
	tree parms = DECL_ARGUMENTS(declaration);
	tree cur_arg;



	vector_t args = vector_create(10);
	int arg_num = 0;
	for (cur_arg = parms; cur_arg; cur_arg = TREE_CHAIN(cur_arg), arg_num++) {

		assert(cur_arg != void_list_node);

		assert(TREE_CODE(cur_arg) == PARM_DECL);
		namespace_t ns = get_namespace_ctx(cur_arg);
		tree arg_type = DECL_ARG_TYPE(cur_arg);
		bool is_const = TYPE_READONLY(arg_type);
		process_type(arg_type);
		bool has_name = DECL_NAME(cur_arg) ? true: false;

		char *arg_name;
		if(!has_name){
			char buffer[50];
			sprintf( buffer, "%d", arg_num);
			char* prefix = "$__arg";
			arg_name = xmalloc(strlen(buffer) + strlen(prefix)+1);
			strcpy(arg_name, prefix);
			strcpy(arg_name + strlen(prefix), buffer);

		}
		else arg_name = IDENTIFIER_POINTER(DECL_NAME(cur_arg));

		arg *ca = xmalloc(sizeof(arg));
		ca->name = arg_name;
		ca->ns = ns;
		ca->type = arg_type;
		ca->gcc_id = cur_arg;
		if (!is_const)
			ca->qt = NONE;
		else
			ca->qt = CONST;
		
		vector_append(args, ca);
	}



	
	
	bool is_static_member_function = false;
	bool is_virtual = false;
	bool is_abstract = false;
	void *owning_class = NULL;
	
	if(get_current_lang() == LANG_CPP)
	{
		is_static_member_function = DECL_STATIC_FUNCTION_P(declaration);
		is_virtual = DECL_VIRTUAL_P(declaration);
		is_abstract = DECL_PURE_VIRTUAL_P(declaration);

		if (DECL_FUNCTION_MEMBER_P(declaration)) {
			tree parent_type = get_type_id(parent);
			process_type(parent_type);
			owning_class = _get_compass_type(parent_type);
		}
	}
	

	

	/* Get the body ptr */
	tree body = DECL_SAVED_TREE(declaration);
	assert(body != NULL);

	void* fd = make_compass_fundecl(name,
	                                name_ctx,
	                                declaration,
	                                parent,
	                                qt,
	                                return_type,
	                                is_allocator,
	                                is_dealloc,
	                                is_constructor,
	                                is_destructor,
	                                is_inline,
	                                is_vararg,
	                                owning_class,
	                                is_static_member_function,
	                                is_virtual,
	                                is_abstract,
	                                args,
	                                0,
	                                function_type);


	void *compass_body = process_function_body(body);
	_set_fundecl_body(fd, compass_body);
	_compass_add_to_translation_unit(fd, file_name);
}

//Checks if you start with "/usr/"
bool is_system_file(char* c)
{
	if(strlen(c)<5) return false;

	char f[6];
	memcpy(f, c, 5);
	f[5] = '\0';
	if(strcmp(f, "/usr/") == 0) return true;
	return false;
}


void process_fundecl_body(tree fundecl) {
	if(errorcount) return;

	if(DISABLE_COMPASS)
		return;

	char *name = get_name_from_fun_decl(fundecl);



	/*
	 * Don't analyze function bodies in system headers.
	 */
	if(DECL_IN_SYSTEM_HEADER(fundecl)) {
		return;
	}




	/*
	 * Don't analyze function bodies coming from namespace std.
	 */
	if(COMPASS_DECL_NAMESPACE_STD_P(fundecl)) {
		return;
	}

	namespace_t ns = get_namespace_ctx(fundecl);

	if(is_namespace_std(ns)) {
		return;
	}



	expanded_location el = get_exp_location(fundecl);
	if(is_system_file(el.file)) {
		return;
	}


	start_compass_function();
	tree body= DECL_SAVED_TREE(fundecl);

	process_function_declaration(fundecl, 0);

}

char* get_anonymous_name()
{

	char* anon = "$anonymous_";
	int cur_id = anonymous_counter++;

	char* a = xmalloc(strlen(anon) + 20);
	strcpy(a, anon);
	sprintf(a+strlen(anon), "%d", cur_id);
	return a;



}

/*
 * Process one var declaration
 */

void *process_var_declaration_with_compass_init(tree declaration,
		tree parent, void* compass_init_exp)
{
	tree_code code= TREE_CODE(declaration);
	assert(code == VAR_DECL);


	/* Is this variable declared static? */
	bool is_static = (TREE_STATIC(declaration)!=0);


	/* Is an extern declaration */
	bool is_extern = (TREE_PUBLIC(declaration)!=0);

	/* All global decls should have user-given names */
	//assert(DECL_NAME (declaration)!= NULL);
	char* name;
	if(DECL_NAME (declaration)!= NULL)
		name= IDENTIFIER_POINTER (DECL_NAME (declaration));
	else name = get_anonymous_name();

	/* Find the type of declaration & process */
	tree type= TREE_TYPE(declaration);
	process_type(type);

	/* Needs: EXTERN, VOLITILE, STATIC, REGISTER
	 * name
	 * Type
	 * init_exp
	 * location
	 */
	qualifier_type qt = NONE;
	if (TREE_STATIC(declaration))
		qt |= STATIC;
	if (DECL_EXTERNAL(declaration))
		qt |= EXTERN;
	if (TYPE_READONLY(type))
		qt |= CONSTANT;
	if (TYPE_VOLATILE(type))
		qt |= VOLATILE;

	namespace_t ns = get_namespace_ctx(declaration);


	/*
	 * Here, we need to make the declaration before we process the init
	 * expression to prevent infinite recursion when the variable being
	 * declared is used as part of its initializer expression,
	 * apparently legal in C!
	 */
	void* decl = make_compass_vardecl(name, ns, declaration, parent, qt, type,
			NULL);


	if(is_static && processing_function_body)
		return decl;

	if(compass_init_exp == NULL)  {
		tree init_exp= DECL_INITIAL(declaration);
		void* compass_init = process_expression(init_exp, declaration, false);
		_set_init_exp_vardecl(decl, compass_init);
	}

	else {
		_set_init_exp_vardecl(decl, compass_init_exp);
	}





	return decl;
}
void *process_var_declaration(tree declaration, tree parent) {

	return process_var_declaration_with_compass_init(declaration, parent, NULL);

}

/* -------------------------------------------- */

void print_exp_location(tree t) {
	expanded_location el = get_exp_location(t);
	printf("\tEXP Location: (at %s, %i, %i) \n", el.file, el.line, el.column);
}

void print_location(tree t) {
	location_t loc= DECL_SOURCE_LOCATION(t);
	expanded_location el = expand_location(loc);
	printf("\tLocation: (at %s, %i, %i) \n", el.file, el.line, el.column);
}

void print_tree_node(tree t) {
	tree cur;
	vector_t v = vector_create(5);

	for (cur = t; cur; cur = TREE_CHAIN (cur)) {
		vector_append(v, cur);
	}

	int i;
	for (i=0; i < vector_size(v); i++) {
		tree cur = vector_ith(v, i);
		tree_code code= TREE_CODE(cur);

		printf("Node %s\n", tree_code_name[code]);
		print_location(cur);
	}

}

/* ------------------------------------------------- */
/*
 * The following always holds: If node i has n children,
 * the order is determined by the id OF THE CHILDREN nodes.
 * Ex: If node 0 has children 3, 6, 7, <3,6,7> is the
 * correct (original) order
 */

/*
 * id: the id of the node created
 */
void make_compass_file(tree id, char* name) {
	_make_compass_file(id, name);
}

/* -------------------------------------------------- */

tree get_type_id(tree t) {
	if (t == NULL)
		return t;
	tree id= TYPE_CANONICAL(t);
	if (id == NULL)
		return t;
	return id;

}

char* get_typename(tree type)
{
	char *name = NULL;
	if (TYPE_NAME(type) == NULL)
		return "";

	if (TREE_CODE (TYPE_NAME (type)) == IDENTIFIER_NODE) {
		name = IDENTIFIER_POINTER (TYPE_NAME (type));
	} else if (TREE_CODE (TYPE_NAME (type)) == TYPE_DECL
	         && DECL_NAME (TYPE_NAME (type)))
	{
		name = IDENTIFIER_POINTER (DECL_NAME (TYPE_NAME (type)));
	}

	assert(name != NULL);

	return name;
}

void make_compass_constarray(long int num_elems, tree gcc_id, tree elem_id) {

	if (DEBUG) {
		printf("*** Const Array Type added ***\n");
		printf("\tNum Elems: %lld\n", num_elems);
		printf("\tGCC id: %lld -> elem id: %lld\n", gcc_id, elem_id);
	}
	tree id = get_type_id(gcc_id);
	tree eid = get_type_id(elem_id);
	process_type(eid);
	_make_compass_constarraytype(num_elems, id, eid, get_typename(gcc_id));
}

void make_compass_vararray(void* num_elems, tree gcc_id, tree elem_id) {

	if (DEBUG) {
		printf("*** Var Array Type added ***\n");
		printf("\tNum Elems exp: %lld\n", num_elems);
		printf("\tGCC id: %lld -> elem id: %lld\n", gcc_id, elem_id,
				get_typename(gcc_id));
	}
	tree id = get_type_id(gcc_id);
	tree eid = get_type_id(elem_id);
	process_type(eid);
	_make_compass_vararraytype(num_elems, id, eid, get_typename(gcc_id));
}

void make_compass_pointertype(tree gcc_id, tree elem_id) {
	if (DEBUG) {
		printf("*** Pointer Type added ***\n");
		printf("\tGCC id: %lld -> elem id: %lld\n", gcc_id, elem_id);
	}
	tree id = get_type_id(gcc_id);
	tree eid = get_type_id(elem_id);
	process_type(eid);
	_make_compass_pointertype(id, eid, get_typename(gcc_id));

}

void make_compass_basetype(char *name,
                           int type_size,
                           int type_align_size,
                           tree gcc_id,
                           value_type v,
                           bool is_signed,
                           char* typedef_name)
{
    tree id = get_type_id(gcc_id);
    if (DEBUG) {
        printf("*** Base Type added ***:\n\tName: %s\n", name);
        printf("\tSize %i, ASize: %i %s\n", type_size, type_align_size,
               is_signed ? "Signed" : "Unsigned");

        printf("\tType: ");
        if (v==VOID)
            printf("void");
        else if (v==COMPLEX)
            printf("complex");
        else if (v==INTEGER)
            printf("int");
        else if (v==IEEE_FLOAT)
            printf("float");
        else if (v==BOOL)
            printf("Bool");
        printf("\n\tGCC id: %i\n", gcc_id);
    }
    _make_compass_basetype(name, type_size, type_align_size, id, v, is_signed,
                           typedef_name);

}

/*
 * Responsible for freeing elems as well as contained struct's
 */
void make_compass_enumtype(char *name, namespace_t ctx, int type_size, int type_align_size,
		tree gcc_id, bool is_signed, vector_t elems) {
	if (DEBUG) {
		printf("*** Enum Type added ***:\n\tName: %s\n", name);
		printf("\tSize %i, ASize: %i %s\n", type_size, type_align_size,
				is_signed ? "Signed" : "Unsigned");

		int i;
		for (i=0; i < vector_size(elems); i++) {
			enum_elem *ee = vector_ith(elems, i);
			printf("\tName: %s, Value: %lld\n", ee->name, ee->value);
		}
		printf("\tGCC id: %i\n", gcc_id);
	}

	tree id = get_type_id(gcc_id);

	_make_compass_enumtype(name, ctx, type_size, type_align_size, id, is_signed,
			elems->data, vector_size(elems), get_typename(gcc_id));

	/* ------------- */
	free_vector_transitive(elems);
}

/*
 * Responsible for freeing elems as well as contained struct's
 */
void make_compass_struct_or_uniontype(char *name,
                                      namespace_t ctx,
                                      int type_size,
                                      int type_align_size,
                                      tree gcc_id,
                                      bool is_union,
                                      vector_t fields,
                                      vector_t bases,
                                      bool is_abstract)
{
	int i;
	if (DEBUG) {
		printf("*** Struct/Union Type added ***:\n"
		       "\tName: %s\n"
		       "\tNamespace: %s\n",
		       name,
		       namespace_tostring(ctx));
		
		printf("\tSize %i, ASize: %i %s\n", type_size, type_align_size,
				is_union ? "union" : "struct");

		for (i=0; i < vector_size(fields); i++) {
			field_elem *fe = vector_ith(fields, i);
			printf("\tName: %s, Offset:%i Type ID %lld\n", fe->name,
					fe->bit_offset, fe->gcc_type);
		}
		printf("\tGCC id: %lld\n", gcc_id);
	}

	tree id = get_type_id(gcc_id);

	/* Canonicalize all field types
	 */
	for (i=0; i < vector_size(fields); i++) {
		field_elem *fe = vector_ith(fields, i);
		fe->gcc_type = get_type_id(fe->gcc_type);
	}



	_make_compass_struct_or_uniontype(name,
                                      ctx,
                                      type_size,
                                      type_align_size,
                                      id,
                                      is_union,
                                      fields->data,
                                      vector_size(fields),
                                      bases->data,
                                      vector_size(bases),
                                      get_typename(gcc_id),
                                      is_abstract);

	/* ------------- */
	free_vector_transitive(fields);
}

void qt_print(qualifier_type qt) {
	if (qt & EXTERN)
		printf("extern ");
	if (qt & VOLATILE)
		printf("volatile ");
	if (qt & STATIC)
		printf("static ");
	if (qt & REGISTER)
		printf("register ");
	if (qt & CONSTANT)
		printf("const ");
	if (qt & PRIVATE)
		printf("private ");
	if (qt & PUBLIC)
		printf("public ");
	if (qt & PROTECTED)
		printf("protected ");
}

void make_compass_vectortype(tree type, tree elem_type, int num_elems) {
	if (DEBUG) {
		printf("*** Vector Type added ***\n");
		printf("\tGCC id: %lld -> elem id: %lld (%i)\n", type, elem_type,
				num_elems);
	}

	tree id = get_type_id(type);
	tree eid = get_type_id(elem_type);
	process_type(eid);
	_make_compass_vectortype(id, eid, num_elems);

}

void make_compass_complextype(tree type, tree elem_type) {
	if (DEBUG) {
		printf("*** Complex Type added ***\n");
		printf("\tGCC id: %lld -> elem id: %lld\n", type, elem_type);
	}

	tree id = get_type_id(type);
	tree eid = get_type_id(elem_type);
	process_type(eid);
	_make_compass_complextype(id, eid);

}

void make_compass_functiontype(tree type, tree return_type, vector_t args,
		bool is_vararg) {
	int i;
	if (DEBUG) {
		printf("*** Function Type added ***\n");
		printf("\tGCC id: %lld -> Return type: %lld\n", type, return_type);

		for (i=0; i < vector_size(args); i++) {
			tree at = vector_ith(args, i);
			printf("\t\t Arg Type: %lld\n", at);
		}
	}

	tree id = get_type_id(type);
	tree ret_id = return_type == NULL ? NULL : get_type_id(return_type);
	process_type(ret_id);
	tree* arg_types = xmalloc(sizeof(tree) * vector_size(args));

	for (i=0; i < vector_size(args); i++) {
		tree at = vector_ith(args, i);
		arg_types[i] = get_type_id(at);
		process_type(arg_types[i]);
	}

	_make_compass_functiontype(id, ret_id, arg_types, vector_size(args),
			is_vararg);

	free(arg_types);

	vector_free(args);
}

/* --------------------------------------------------------*/
void *make_compass_vardecl(char *name, namespace_t ns, tree declaration,
		tree parent, qualifier_type qt, tree type, void* compass_init_exp) {

	tree type_id = get_type_id(type);

	expanded_location el = get_exp_location(declaration);

	bool is_global = false;
	tree context = DECL_CONTEXT(declaration);
	if((context == NULL_TREE ||  TREE_CODE(context) == TRANSLATION_UNIT_DECL))
		is_global = true;

	void *t =  _make_compass_vardecl(name, ns, parent, declaration, compass_init_exp,
			el.line, el.column, type_id, qt, is_global);
	//if(TREE_CODE (type_id) == RECORD_TYPE)
	//	vardecl_has_record_type(t);

	return t;


}

void* make_compass_fundecl(char *name,
                           namespace_t name_ctx,
                           tree declaration,
                           tree parent,
                           qualifier_type qt,
                           tree *return_type,
                           bool is_allocator,
                           bool is_dealloc,
                           bool is_constructor,
                           bool is_destructor,
                           bool is_inline,
                           bool is_vararg,
                           void *owning_class,
                           bool is_static_member_function,
                           bool is_virtual,
                           bool is_abstract,
                           vector_t args,
                           tree body,
                           tree funtype)
{
	expanded_location el = get_exp_location(declaration);

	tree fun_id = get_type_id(funtype);

	int i;
	for (i=0; i < vector_size(args); i++) {
		arg *a = (arg*) vector_ith(args, i);
		a->type = get_type_id(a->type);
	}

	unsigned int temp = qt;

	return _make_compass_fundecl(name,
	                             name_ctx,
	                             declaration,
	                             parent,
	                             temp,
	                             get_type_id(return_type),
	                             is_allocator,
	                             is_dealloc,
	                             is_constructor,
	                             is_destructor,
	                             is_inline,
	                             is_vararg,
	                             owning_class,
	                             is_static_member_function,
	                             is_virtual,
	                             is_abstract,
	                             args->data,
	                             vector_size(args),
	                             body,
	                             fun_id,
	                             el.line,
	                             el.column);

	free_vector_transitive(args);
}

/* ----------------------------------------------------*/

bool end_compass_file(void)
{
	_assert_no_unresolved_types();
	cur_id = 0;
	return true;
}

/* -------------------------------------------------- */

/*
 * Frees all elements in v as well as v
 */
void free_vector_transitive(vector_t v) {
	int i;
	for (i=0; i < vector_size(v); i++) {
		void *p = vector_ith(v, i);
		free(p);
	}
	vector_free(v);
}

/* Utility implementations */

/*
 * small c vector
 */
vector_t vector_create(int alloc) {
	if (alloc<1)
		alloc = 10;
	vector_t res = xmalloc(sizeof(_vec));
	res->alloc = alloc;
	res->size = 0;
	res->data = xmalloc(sizeof(void*) * alloc);
	return res;
}

void vector_free(vector_t v) {
	free(v->data);
	free(v);
}

void *vector_ith(vector_t v, int i) {
	assert(i < (int)v->size);
	return v->data[i];
}

void vector_append(vector_t v, void *elem) {
	assert(v->alloc>= v->size);

	if (v->alloc == v->size) {
		void *tmp = xmalloc(sizeof(void*)*2*v->alloc);
		memcpy(tmp, v->data, sizeof(void*)*v->alloc);
		v->alloc = 2 * v->alloc;
		free(v->data);
		v->data = tmp;
	}
	v->data[v->size++] = elem;
}

int vector_size(vector_t v) {
	return (int)v->size;
}

/* ------------------------------------------------*/

/*
 * A primitive (non-balancing b-tree)
 */

btree_t btree_create(void) {
	btree_t res = xmalloc(sizeof(_btree));
	res->key = res->data = res->left = res->right = NULL;
	return res;
}

void btree_insert(btree_t t, char* key, void* value) {
	assert(t!= NULL);
	if (t->key == NULL) {
		t->key = key;
		t->data = value;
		return;
	}

	int cmp = strcmp(key, t->key);
	if (cmp == 0) {
		t->key = key;
		t->data = value;
		return;
	} else if (cmp < 0) {
		if (t->left == NULL) {
			t->left = btree_create();
			t->left->key = key;
			t->left->data = value;
			return;
		} else
			btree_insert(t->left, key, value);
	} else {
		if (t->right == NULL) {
			t->right = btree_create();
			t->right->key = key;
			t->right->data = value;
			return;
		} else
			btree_insert(t->right, key, value);
	}
}

void *btree_find(btree_t t, char* key) {
	assert(t!= NULL);
	if (t->key == NULL)
		return NULL;

	int cmp = strcmp(key, t->key);
	if (cmp == 0) {
		return t->data;
	} else if (cmp < 0) {
		if (t->left == NULL)
			return NULL;
		else
			return btree_find(t->left, key);
	} else {
		if (t->right == NULL)
			return NULL;
		else
			return btree_find(t->right, key);
	}
}

/*
 * Fills a vector with all keys
 */
void btree_keys(btree_t t, vector_t v) {
	if (t == NULL || t->key == NULL)
		return;
	btree_keys(t->left, v);
	vector_append(v, t->key);
	btree_keys(t->right, v);
}

/***********************************************/
extern void add_loop_mapping(location_t loc, tree init_exp, tree cond,
		tree incr, tree body, tree break_label, tree continue_label,
		bool cond_is_first, bool is_for, tree t, tree top, tree c_label,
		tree b_label, tree entry, tree exit) {
	expanded_location el = expand_location(loc);

	_add_loop_mapping(el.line, el.column, init_exp, cond, incr, body,
			break_label, continue_label, cond_is_first, is_for, t, top,
			c_label, b_label, entry, exit);

}

extern void enable_regression_output()
{
	enable_regressions = 1;
}

