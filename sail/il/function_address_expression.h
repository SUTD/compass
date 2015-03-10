/*
 * function_address_expression.h
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 */

#ifndef FUNCTION_ADDRESS_EXPRESSION_H_
#define FUNCTION_ADDRESS_EXPRESSION_H_

#include "expression.h"


namespace il {

/*
 * Address of function, e.g.
 * int foo(int (*fooxx)(int)){return 3;};
 * int bar(int a){return 4;};
 * int b = foo(bar);
 */
/**
 * \brief Result of taking the address of a function
 */
class function_address_expression: public il::expression {
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & fn_name;
        ar & fn_signature;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & fn_name;
        ar & fn_signature;
        type::register_loaded_typeref(&fn_signature);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	string fn_name;
	type* fn_signature;
public:
	function_address_expression();
	function_address_expression(string fn_name, type* fn_signature,
			type* t, location loc);
	virtual string to_string() const;
	virtual void print(){};
	virtual string get_function_name();
	virtual type* get_function_signature();
	virtual ~function_address_expression();
};

}

#endif /* FUNCTION_ADDRESS_EXPRESSION_H_ */
