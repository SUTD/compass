/*
 * builtin_expression.h
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 *
 *      These are "built-in" gcc expressions such as:
 *      va_arg() (Note: va_start and va_end are modeled as function calls.)
 *
 */


#ifndef BUILTIN_EXPRESSION_H_
#define BUILTIN_EXPRESSION_H_

#include "expression.h"

namespace il {
class type;


/**
 * If you change this, make sure you also change type in il_c.c!!!!
 */
enum compass_builtin_type {_VA_ARG};

/**
 *  \brief Used to model  "built-in" gcc expressions such as va_arg()
 */
class builtin_expression: public il::expression {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & bt;
        ar & args;
    }

private:
	compass_builtin_type bt;
	vector<expression*> args;


public:
	builtin_expression();
	builtin_expression(compass_builtin_type bt, vector<expression*> &args,
			type* t, location loc);
	virtual string to_string() const;
	virtual void print(){};
	virtual compass_builtin_type get_builtin_type();
	virtual vector<expression*> & get_arguments();
	virtual ~builtin_expression();
	static string builtin_type_to_string(compass_builtin_type b);

};

}

#endif /* BUILTIN_EXPRESSION_H_ */
