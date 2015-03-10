/*
 * complex_const_exp.h
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#ifndef COMPLEX_CONST_EXP_H_
#define COMPLEX_CONST_EXP_H_

#include "const_expression.h"


namespace il {

/**
 * \brief Complex number representation
 */
class complex_const_exp: public il::const_expression {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    }

public:
	complex_const_exp();
	complex_const_exp(type* t, location loc);
	virtual string to_string() const;
	virtual ~complex_const_exp();
};

}

#endif /* COMPLEX_CONST_EXP_H_ */
