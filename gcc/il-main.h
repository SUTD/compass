#ifndef _IL_MAIN_H
#define _IL_MAIN_H

enum language_type {
	LANG_C = 0,
	LANG_CPP = 1,
	LANG_UNSUPPORTED = 2
};
typedef enum language_type language_type;




void set_language(enum language_type lang);


enum value_type {INTEGER, IEEE_FLOAT, VOID, COMPLEX, BOOL};
typedef enum value_type value_type;
void cpp_test(const_tree tree);

enum binop_type;
typedef enum binop_type binop_type;

enum unop_type;
typedef enum binop_type unop_type;

enum compass_builtin_type;
typedef enum compass_builtin_type compass_builtin_type;

void vardecl_has_record_type(void *_vd);

/*
 * Sets the desired sail output directory.
 */
void set_sail_output_directory(char* dir);

void push_label_context(int line, int column);
void pop_label_context();


typedef void * namespace_t;

namespace_t namespace_make(char *name, namespace_t outer_context);
const char *namespace_tostring(namespace_t ns);

bool is_namespace_std(namespace_t ns);

struct catch_handler {
	void *type;
	void *body;
};


/*
 * COMPASS connectors
 * ------------------
 * Each of these functions is called from GCC to add the appropriate
 * kind of node to COMPASS IL.
 */


void assert_vardecl(void* vt);


void _make_compass_file(unsigned long int id, char* name);

void *_make_compass_vardecl(char* name, namespace_t ns,
		tree parent, tree cur_id, void* init_exp,
		int line, int column, tree type_id, int qt, bool global_decl);

void _set_init_exp_vardecl(void* var_decl, void* init_exp);

void _set_fundecl_body(void *f, void *b);


void _compass_add_to_translation_unit(void *c_decl, char *file);

void _compass_translation_unit_finished(char* asm_name);

/* ---------------------------------------------- */


bool is_expression(void *node);

void _make_compass_basetype(char* name, int type_size,
		int type_align_size, tree gcc_id, value_type v, bool is_signed,
		char* typedef_name);

void _make_compass_pointertype(tree gcc_id, tree elem_id, char* typedef_name);

void _make_compass_constarraytype(long int num_elems, tree gcc_id,
		tree elem_id, char* typedef_name);

void _make_compass_vararraytype(void* num_elems, tree gcc_id,
		tree elem_id, char* typedef_name);



struct enum_elem;


void _make_compass_enumtype(char* name, namespace_t ctx, int type_size,
		int type_align_size, tree gcc_id, bool is_signed,
		struct enum_elem ** elems, int elem_size,
		char* typedef_name);

struct field_elem;

void _make_compass_struct_or_uniontype(char *name,
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
                                       bool is_abstract);


void _make_compass_vectortype(tree gcc_id, tree elem_id, int num_elems);

void _make_compass_complextype(tree gcc_id, tree elem_id);

void _make_compass_functiontype(tree gcc_id, tree ret_id, tree* arg_types,
		int arg_size, bool is_vararg);


struct arg;




void *_make_compass_fundecl(char *name, namespace_t name_ctx,
		tree declaration, tree parent,
		unsigned int qt, tree return_type, bool is_allocator,
		bool is_dealloc, bool is_constructor, bool is_destructor,
		bool is_inline, bool is_vararg, void *owning_class,
		bool is_static_member_function,
		bool is_virtual, bool is_abstract, struct arg** args, int num_args,
		tree body, tree funtype, int line, int column);




void *_make_compass_block(void** decls, int num_decls, void** stmts,
		int num_stmts);

void *_make_compass_set_instruction(void* exp_lval, void *exp_rhs, int line,
		int column, bool is_decl);

void add_to_global_decls(void* var_decl, void* init_exp, int line, int column,
		char* filename);


void *_make_compass_for_loop(void* init_stmt, void* continuation_cond,
		void* inc_exp, void* loop_body, int line, int column);


void *_make_compass_assembly_instruction(char *a_string, void ** exp_outs,
		int num_outs, void **exp_ins, int num_ins, void **char_clobbers,
		int num_clobbers, int line, int column);



void *_make_compass_if_statement(void *exp, void *then_stmt, void *else_stmt,
		int line, int column);

void *_make_compass_for_stmt(void *init_exp, void *continue_exp,
		void *incr_exp, void* body_stmt,
		int line, int column);

void* _make_compass_while_stmt(void* cond_exp, void* while_body, int line,
		int column);

void* _make_compass_do_stmt(void* cond_exp, void* do_body, int line, int column);

void* _make_compass_break_stmt(int line, int column);
void* _make_compass_continue_stmt(int line, int column);

void* _make_compass_exit_exp_statement(void* cond, int line, int column);


void *_make_compass_goto_statement(char* l, tree label_id, int line,
		int column);
void *_make_compass_label_statement(char* l, tree label_id, int line,
		int column);
void *_make_compass_case_label_statement(long low, bool has_low, long high,
		bool has_high, int line, int column);
void *_make_compass_return_statement(void* ret_exp, int line, int column);
void *_make_compass_switch_statement(void* cond_exp, void* body_stmt,
			void* case_label_vector, tree switch_id, int line, int column);

void *_make_compass_try_catch_statement(void *try_stmt,
                                        struct catch_handler **handlers,
                                        int num_handlers,
                                        int line,
                                        int column);

void *_make_compass_try_finally_statement(void *try_stmt,
                                          void *exit_stmt,
                                          int line,
                                          int column);

void* _make_compass_expression_instruction(void* expr);

void *_make_compass_noop_instruction();

void* _make_compass_integer_const(long int int_const,
		void* compass_type, int line, int column);

void* _make_compass_string_const(char* string_const,
		void* compass_type, int line, int column);

void* _make_compass_real_const(
		void* compass_type, int line, int column);

void* _make_compass_fixed_const(void* compass_type, int line, int column);

void* _make_compass_complex_const(void* compass_type, int line, int column);

void* _make_compass_vector_const(void* compass_type, int line, int column);

void* _make_compass_cast_expression(void* inner_exp, void* compass_type,
		int line, int column);

void* _make_compass_deref_expression(void* inner_exp, void* compass_type,
		int line, int column);

void* _make_compass_addressof_expression(void* inner_exp, void* compass_type,
		int line, int column);

void* _make_compass_expression_list(void* exprs, int size, void* compass_type,
		int line, int column);

void* _make_compass_field_ref_expression(void* inner_exp, char* field_name,
		void* compass_type, int line, int column);

void* _make_compass_array_ref_expression(void* array_exp, void* index_exp,
		void* compass_type, int line, int column);

void* _make_compass_binop_expression(void* exp1, void* exp2, binop_type binop,
		void* compass_type, int line, int column);

void* _make_compass_unop_expression(void* exp, unop_type unop,
		void* compass_type, int line, int column);

void* _make_compass_function_call_expression(char* fn_name,
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
                                             int column);

void* _make_compass_function_pointer_call_expression(void* fn_ptr, void* args,
		int num_args, void* compass_type, int line, int column);

void* _make_compass_conditional_expression(void* cond_exp,
		void* then_clause, void* else_clause, void* compass_type,
		int line, int column);

void* _make_compass_modify_expression(void *set_inst,
                                      bool is_init,
                                      void *compass_type,
                                      int line,
                                      int column);

void* _make_compass_init_list_expression(void* init_list, int num_elems,
		void* compass_type, int line, int column);

void* _make_compass_function_address_expression(char* fn_name, void* fn_sig,
		void* compass_type, int line, int column);

void* _make_compass_block_expression(void* compass_block, void* var_decl,
		void* compass_type, int line, int column);

void* _make_compass_builtin_expression(compass_builtin_type bt, void* args,
		int num_args, void* compass_type, int line, int column);

void *_make_compass_throw_expression(void *throw_exp_type,
                                     void *full_throw_exp,
                                     void *compass_type,
                                     int line,
                                     int column);

void _assert_no_unresolved_types(void);


void* _make_compass_var_exp(void *var, int line, int column);



void* _get_compass_node(tree gcc_id);
void* _get_compass_type(tree gcc_id);

void* _get_variable_from_variable_decl(void *v);
void* _get_int_type();



void _compass_set_variable_scope(void *vd, void *b);



void* hashmap_create();


void hashmap_delete(void * _hm);

void hashmap_insert(void * _hm, unsigned long int key);

bool hashmap_contains(void * _hm, unsigned long int key);


bool node_exists(tree gcc_id);





void* hashmap_create2();


void hashmap_insert2(void * _hm, unsigned long int key, char *s);

bool hashmap_contains2(void * _hm, unsigned long int key);

char* hashmap_find2(void * _hm, unsigned long int key);




/*
 * While parsing GCC transforms any higher-level loop structure into
 * goto statements. The functions below allow us to recover the original
 * structure of the source code lost during parsing.
 *
 * For parsing a new language, we need to call _add_loop_mapping
 * at the parsing stage when the original information is available.
 *
 * When converting gcc information to Compass data structures,
 * we need to check whether a given statement ought to be ignored using
 * _ignore_statement. If the statement is part of a loop,
 * _get_loop_from_node gives the corresponding loop information.
 *
 * Thus, for Compass statement processing, we need to check:
 * if(_ignore_statement(stmt)) return;
 * if(_get_loop_from_node(stmt)!=NULL) process_loop();
 *
 */
void _add_loop_mapping(int line, int column, tree init_exp, tree cond,
		tree incr, tree body, tree break_label, tree continue_label,
		bool cond_is_first, bool is_for, tree t, tree top, tree c_label,
		tree b_label, tree entry, tree exit);

bool _ignore_statement(tree stmt);

struct loop_info;

struct loop_info* _get_loop_from_node(tree node);

bool type_exists(tree gcc_id);
bool type_exists_as_incomplete_struct(tree gcc_id);

void print_type(tree gcc_id);

/********************************************************************/
void _add_case_label(tree switch_id, void* il_label);

/* returns a C++ vector* */
void* _get_case_labels(tree switch_id);

void _add_label_to_equivalence_class(tree switch_id,
		void *_representative, void* _l);

void  start_compass_function(void);

void _compass_enable_regressions(void);


typedef void* hashmap_t;


struct base_class {
	void *type;
	long int offset;
};

#endif
