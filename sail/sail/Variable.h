/*
 * Variable.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#ifndef SAIL_VARIABLE_H_
#define SAIL_VARIABLE_H_

#include "Symbol.h"
#include <string>
#include<map>
using namespace std;
#include "../il/variable.h"
#include "../il/namespace_context.h"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>

namespace il{
	class node;
	class variable_expression;
	class block;
}



namespace sail {

/**
 * \brief Represents a variable, which can be either an actual program variable or a
 * temporary introduced by SAIL.
 *
 */
class Variable:public Symbol {
private:
	/*
	 * The original il expression corresponding to this
	 * (potentially synthetic, i.e. introduced by SAIL) variable.
	 * This can be NULL if the variable is not related
	 * to any existing il expression.
	 * Example: for a switch statement
	 * switch(a)
	 * {
	 * 	case 1:
	 * }
	 * for the first case we introduce a temporary __temp1 = (a==1)
	 * which we use in the conditional. In this case __temp does not have
	 * an original corresponding il node.
	 */
	il::node* original;

	/*
	 * Either the original name of the variable or
	 * the name of the temporary produced by SAIL.
	 */
	string name;

	il::namespace_context ns;

	/**
	 * Name we would like to use for human-readable printing.
	 */
	string alias_name;

	bool isTemp;

	bool isRemovableTemp;

	/*
	 * Is this variable introduced to mark which exit point was taken to
	 * leave a loop?
	 */
	bool is_loop_exit;



	/*
	 * Is this a designated return variable?
	 */
	bool is_retvar;

	il::type* var_type;

	bool local;
	bool arg;
	int arg_num;
	bool global;
	bool is_static_var;


	bool is_point_to_alloc;

	friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
    	ar & boost::serialization::base_object<sail::Symbol>(*this);
		ar & name;
		ar & ns;
		ar & alias_name;
		ar & isTemp;
		ar & isRemovableTemp;
		ar & is_loop_exit;
		ar & is_retvar;
		ar & var_type;
		ar & local;
		ar & arg;
		ar & arg_num;
		ar & global;
		ar & is_static_var;
		ar & original;

		//added by haiyan
		ar & is_point_to_alloc;
		//ended by haiyan

	}
	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<sail::Symbol>(*this);
		ar & name;
		ar & ns;
		ar & alias_name;
		ar & isTemp;
		ar & isRemovableTemp;
		ar & is_loop_exit;
		ar & is_retvar;
		ar & var_type;
		il::type::register_loaded_typeref(&var_type);
		ar & local;
		ar & arg;
		ar & arg_num;
		ar & global;
		ar & is_static_var;
		ar & original;

		//added by haiyan
		ar & is_point_to_alloc;
		//added by haiyan

	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()





// --------------------------------

public:

	/*
	 * The argument to the constructor can be one of three conceptually
	 * different things:
	 * 1) If we are introducing a Variable that corresponds to an
	 * actual program variable, the original node is the node associated
	 * with the il::variable_expression.
	 *
	 * 2) If we are introducing a temporary variable to "decompose" a
	 * side effect free expression (e.g. t = a+b), then the original
	 * node corresponds to the node for the side-effect free il expression,
	 * such as a+b. The idea here is to use a+b instead of t for printing
	 * purposes.
	 *
	 * 3) If we are introducing a temporary variable to model expressions
	 * with side-effects, the original node provided to the constructor
	 * should be NULL. This means we should print the SAIL-introduced
	 * temporary as an actual temporary.
	 */
	Variable(il::node* original, il::type* t);


	/*
	 * This constructor is used if we want to use alias_name
	 * for pretty printing.
	 */
	Variable(il::node* original, string alias_name, il::type* t);


	/*
	 * Shorthand for Variable(NULL, alias_name)
	 */
	Variable(string alias_name, il::type* t);


	Variable(const sail::Variable& other);
	/*
	 * Constructor for forcing a given temporary name.
	 */
	Variable(string special_temp_name, il::type* var_type, bool unused);

	Variable(string special_temp_name, il::type* var_type, bool mark_temp, bool is_arg,
			int num_arg, bool global);
	Variable(){};

	Variable* clone();

	void set_alias_name(string alias_name);


	////
	//hzhu added 5.25
	void set_type(il::type*);
	void set_name(string name);
	//hzhu end 5.25
	////

	static Variable* get_return_variable(il::type* t);
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual il::node* get_original_node();

	/*
	 * In SAIL, variables are not guaranteed to be unique by address.
	 * Use get_unique_id for adding them to hash maps, sets etc.
	 */
	string get_unique_id();


	/** Is this variable a temporary introduced by SAIL?
	 *
	 */
	/*
	 * Category 2 & 3 above.
	 */
	virtual bool is_temp();

	/**
	 * Category 2 only.
	 */
	virtual bool is_removable_temp();
	virtual void set_removable(bool removable);

	/**
	 * Is this variable introduced to mark which exit point was taken to
	 * leave a loop if super blocks are constructed? (see Cfg documentation)
	 */
	bool is_loop_exit_var();

	/**
	 * Implements abstract method defined by Symbol
	 */
	virtual bool is_constant();
	/**
	 * Implements abstract method defined by Symbol
	 */
	virtual bool is_variable();
	virtual ~Variable();
	static string get_temp_name();
	static void clear_maps();
	il::type* get_type();

	/**
	 * Is this an anonymous variable introduced by il or gcc?
	 */
	bool is_anonymous();

	/**
	 * Is this  a local variable?
	 */
	bool is_local();

	/**
	 * Is this variable a function argument?
	 */
	bool is_argument();

	/**
	 * If this variable is an argument, which argument is it?
	 */
	int get_arg_number();

	//set the variable as local but argument;
	void set_as_local();

	/**
	 * Is this a global variable?
	 */
	bool is_global();

	/**
	 * Does this variable hold the return value? Such variables are
	 * introduced by SAIL.
	 */
	bool is_return_variable();

	/**
	 * @return the name of this variable
	 */
	string get_var_name();

	//haiyan added for purposely set var name as temp name
	void set_var_name_as_temp_name()
	{
		this->name = get_temp_name();
		this->isTemp = true;
	};

	/**
	 * @return namespace associated with this variable
	 */
	const il::namespace_context& get_namespace() const;

	void set_original(il::node* orig);
	bool operator==(const sail::Variable& other);

	il::variable_declaration* get_declaration();

	bool is_static();

	/**
	 * @return Any variable that is not a program variable.
	 */
	bool is_synthesised_var();
	/*
	 * Only to be used for return variables fabricated by SAIL
	 */
	Variable(il::type* t);

	void replace_type(il::type* t);


	bool fixed;

/******
 * haiyan added to disguish the lhs of alloc instruction;
 */
	void set_is_point_to_alloc();
	bool has_point_to_alloc();


protected:
	/*
	 * Only to be used for return variables fabricated by SAIL
	 */
	//Variable(il::type* t);

private:
	string get_name(string orig_name, il::block* b, il::scope_type st);
	void add_block(il::block* b);
	void initialize(il::node* original, il::type* t);
	void set_local_status();
	void set_arg_status();
	void set_global_status();
	void set_is_static();

};

}

#endif /* SAIL_VARIABLE_H_ */
