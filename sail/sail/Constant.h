/*
 * Constant.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#ifndef CONSTANT_H_
#define CONSTANT_H_

#include "Symbol.h"
#include <string>
using namespace std;


namespace il{class const_expression;}

namespace sail {

/**
 * \brief Represents a constant.
 */
class Constant:public Symbol {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Symbol>(*this);
        ar & constant_exp;
        ar & constant;
        ar & _is_signed;
        ar & size;
    }

private:
	/*
	 * The last three fields only exist if
	 * constant_exp is NULL, i.e. for SAIL
	 * fabricated constants.
	 */
	il::const_expression* constant_exp;
	long constant;
	bool _is_signed;
	short size;


public:
	Constant(il::const_expression* constant_exp);
	Constant(long constant, bool is_signed, short size);
	Constant();
	virtual string to_string() const;
	virtual string to_string(bool print_original) const;

	//add by haiyan for the purpose of clone a constant value;
	Constant* clone(){
		Constant* new_cons = new Constant();
		new_cons->constant_exp = constant_exp;
		new_cons->constant = constant;
		new_cons->_is_signed = _is_signed;
		new_cons->size = size;
		return new_cons;
	}

	/**
	 * This can return NULL if we made up the constant, e.g., for pointer
	 * arithmetic that did not exist in the original source code.
	 */
	virtual il::node* get_original_node();

	/**
	 * Implements the abstract is_constant() method of Symbol.
	 */
	virtual bool is_constant();

	/**
	 * Implements the abstract is_variable() method of Symbol.
	 */
	virtual bool is_variable();
	virtual il::type* get_type();

	/**
	 * Is this an integer constant?
	 */
	bool is_integer();

	/**
	 * Is this a string constant?
	 */
	bool is_string_constant();


	/**
	 * This function can only be called if this is an integer constant,
	 * so call is_integer() before calling this function.
	 */
	long int get_integer();

	/**
	 * Calling this function is only valid for string constants.
	 */
	string get_string_constant();

	/**
	 * Is this constant signed?
	 */
	bool is_signed();

	/**
	 * @return the size (in bits!), so divide by 8 to get size in bytes.
	 */
	int get_size();
	virtual ~Constant();

	//added by haiyan
	long int get_integer_value(){
		return constant;
	}
	//added by haiyan
};

}

#endif /* CONSTANT_H_ */
