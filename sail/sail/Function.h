/*
 * Function.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef FUNCTION_H_
#define FUNCTION_H_
#include<vector>
#include<string>
#include<map>
#include<list>
#include "SummaryUnit.h"
#include "Serializable.h"
#include "namespace_context.h"
#include "Annotation.h"



/**
 * If ALLOW_OFFSETS is false, offsets are not permitted in Load, Store,
 * ArrayRefRead, and ArrayRefWrite instructions.
 */
#define ALLOW_OFFSETS true


/**
 * If this is set to true, SAIL generates DropVariable instructions that mark
 * when temporaries are no longer needed.
 */
#define ADD_DROP_TEMP_INSTRUCTIONS false


using namespace std;

namespace il{
	class function_declaration;
	class expression;
	class statement;
	class label;
	class case_label;
	class unop_expression;
	//class block;
	class type;
	class initializer_list_expression;
	class file;
	class function_type;
	class node;
	class variable_declaration;
	class binop_expression;
}

namespace sail {
class Instruction;
class Symbol;
class Variable;
class Label;
class Cfg;
class TranslationUnit;


/**
 * \brief Representation of a function in the low-level language.
 * Most of the work of translating the high-level language
 * to the low-level language is done here.
 */
class Function:  public Serializable, public SummaryUnit{

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<Serializable>(*this);
    	ar & boost::serialization::base_object<sail::SummaryUnit>(*this);

    	ar & body;
        ar & original;
        ar & id;
        ar & has_return;
        ar & cfg;
        ar & first_line;
        ar & cpp;
        ar & local_vars;
    	ar & is_method_flag;
    	ar &  is_virtual_method_flag;
    	ar &  is_abstract_virtual_method_flag;
    	ar &  is_constructor_flag;
    	ar &  is_destructor_flag;
    	ar &  annotations;
    }
private:
    vector<Annotation*> annotations;
	vector<Instruction*>* body;
	il::function_declaration* original;

	Identifier id;

	map<il::label*, Label*> label_map;
	map<il::case_label*, Label*> case_label_map;
	// Map from each label to its equivalence class representative.
	map<Label*, Label*> label_eqv_map;
	int label_counter;
	int default_counter;
	Label* return_label;
	bool has_return;
	Cfg* cfg;
	int cur_line;
	int first_line;
	bool cpp;
	bool is_method_flag;
	bool is_virtual_method_flag;
	bool is_abstract_virtual_method_flag;
	bool is_constructor_flag;
	bool is_destructor_flag;



	/*
	 * Special g++ guard variables. According
	 * to C++ ABI 3.0 any variable starting with
	 * _ZGV is assumed to start at 0, so we
	 * need to add this to the global
	 * vars to be initialized.
	 */
	map<string, Variable*> cxx_zgv_vars;



	/*
	 * The list of local variables, sorted according to their
	 * declaration order.
	 */
	list<sail::Variable*> local_vars;





// ---------------------------------------
public:

	/*
	 * Constructor for normal functions declared in the program
	 */
	Function(il::function_declaration* original, il::file* f, bool cpp);
    // contructor for function's instructions for dex file
	// File is absolute path of source file where method is defined
	Function(const string& name, const string & file,
			il::namespace_context ns,
			il::function_type* signature, vector<Instruction*>* instruction,
			bool is_virtual_method, bool is_abstract_method,
					bool is_constructor, bool is_destructor);

	/*
	 * Constructor for the special init functions to initialize globals.
	 */
	Function(vector<il::variable_declaration*>& global_decls, string file,
			bool cpp, vector<Variable*> & zvg_vars);



	Function()
	{
		body = NULL;
		original = NULL;
		has_return = false;
		cfg = NULL;
		cur_line = -1;
		first_line = -1;
	};
	virtual ~Function();
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	
	/**
	 * @return the representation of this function in the high-level language.
	 * Important: This function can return NULL for made-up
	 * functions that are used to initialize globals in a given file!
	 */
	il::function_declaration* get_original_declaration();
	
	/**
	 *
	 * @return instructions forming the body of this function
	 */
	vector<Instruction*>* get_body();
	
	/**
	 * Add an annotation to this function declaration
	 */
	void add_annotation(Annotation* a);
	/**
	* Add annotations to this function
	*/
	void add_annotations(vector<Annotation *>& a); //added by haiyan

	/**
	 * @return Annotations associated with this function
	 */
	const vector<Annotation*> & get_annotations();

	
	/**
	 *
	 * @return Control flow graph of this function.
	 */
	Cfg* get_cfg();
	il::function_type* get_signature();

	/**
	 * Fills the passed in instructions vector with the instructions
	 * forming the body of this function.
	 * @param instructions
	 */
	void get_instructions(vector<Instruction*>& instructions);

	/**
	 * @return Identifier of the function
	 */
	virtual Identifier get_identifier();

	/**
	 * @return namespace in which the function is declared
	 */
	il::namespace_context get_namespace();


	/**
	 * @return Is this a member function of some class?
	 */
	bool is_method();

	/**
	 * @return The class in which this method is declared if this is a method,
	 * otherwise NULL.
	 */
	il::record_type* get_defining_class();

	/**
	 * @return Is this a virtual method?
	 */
	bool is_virtual_method();


	/**
	 * @return Is this an abstract virtual method?
	 */
	bool is_abstract_virtual_method();

	/**
	 * @return Is this a constructor?
	 */
	bool is_constructor();

	/**
	 * @return Is this a destructor?
	 */
	bool is_destructor();


	map<string, Variable*>& get_cxx_zvg_vars();



	/***************************************
	 * Summary-unit related functions.
	 ****************************************/

	/**
	 * Implements the abstract is_function() defined by SummaryUnit.
	 */
	virtual bool is_function();

	/**
	 * Implements the abstract is_superblock() defined by SummaryUnit.
	 */
	virtual bool is_superblock();



	/**
	 *
	 * @return entry_block of this function
	 */
	virtual BasicBlock* get_entry_block();

	/**
	 * @return exit block of this function
	 */
	virtual BasicBlock* get_exit_block();

	/**
	 *
	 * @return exception block of this function (see Cfg for what this means)
	 */
	virtual BasicBlock* get_exception_block();

	/**
	 * @return The first line this function is defined on.
	 */
	int get_first_line();
	
	
	/**
	 * Is this function introduced to initialize globals?
	 */
	bool is_init_function();

	/**
	 * Gives the return type of this function
	 */
	il::type* get_return_type();

	/*******
	 * haiyan added
	 * real return type that passed to function signature;
	 */
	il::type* get_return_t();

	string get_file();

	/**
	 * Gives the set of all local variables declared in this function,
	 * sorted by their declaration order.
	 */
	const list<sail::Variable*>& get_local_vars();

private:
	void make_function_body();
	Symbol* process_expression(il::expression* exp);
	Symbol* process_statement(il::statement* stmt, bool return_needed);
	Label* find_or_create_label(il::label* l);
	Label* find_or_create_label(il::case_label* l);
	Label* create_default_label();
	bool case_label_exists(il::case_label* l);
	Label* get_fresh_label();
	Label* get_return_label();
	void process_if_statement(il::statement* stmt);
	void process_return_statement(il::statement* stmt);
	Symbol* process_set_instruction(il::statement* stmt, bool return_needed,
			il::node* original = NULL);
	void process_switch_statement(il::statement* stmt);
	Symbol* process_conditional_expression(il::expression* exp);
	Symbol* process_unop_side_effect(il::unop_expression* unop);
	Symbol* process_pre_unop(il::unop_expression* unop);
	Symbol* process_post_unop(il::unop_expression* unop);
	Symbol* generate_condition_from_ranges(vector<pair<long, long> >* ranges,
			Symbol *switch_var, il::type* t );
	Variable* process_function_call(il::expression* exp);
	Variable* process_function_pointer_call(il::expression* exp);
	Variable* process_initializer_list(il::expression* exp);
	Variable* process_record_initializer_list(il::initializer_list_expression*
			init_exp);
	Variable* process_array_initializer_list(il::initializer_list_expression*
			init_exp);
	void remove_redundant_temporaries();
	void remove_redundant_labels();
	void compute_label_equivalences();
	void remove_null_instructions();
	il::expression* strip_casts(il::expression* e);
	void add_drop_temp(Symbol* s);
	void add_drop_ref(Symbol* s);
	Symbol* process_complex_store(il::expression* lhs_exp, Symbol* rhs,
			il::statement* original, bool return_needed);
	Symbol* calculate_complex_address(il::expression* lhs_exp,
			il::node* stmt);
	Variable* make_multiply_binop(Symbol* s1, Symbol* s2);
	Variable* make_add_binop(Symbol* s1, Symbol* s2);

	/*
	 * Checks if the expression if of the form x & (2^c-1) = 0, if so
	 * makes up a x%2^c =0 expression since gcc converts all such
	 * expressions into bitwise and which we can't understand.
	 * Returns NULL if the expression is not of this form.
	 */
	Symbol* convert_to_mod_expression(il::binop_expression* exp);

	/*
	 * Ensures that if this binop has any constant operands,
	 * the first operand is guaranteed to be a constant.
	 */
	void make_first_op_constant(il::binop_expression* exp);

};

}

#endif /* FUNCTION_H_ */
