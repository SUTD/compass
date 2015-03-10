/*
 * fixed_const_exp.h
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#ifndef FIXED_CONST_EXP_H_
#define FIXED_CONST_EXP_H_

#include "const_expression.h"

namespace il {

class type;

/**
 * \brief Decimal constant representation
 */
class fixed_const_exp: public il::const_expression {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    }
public:
	fixed_const_exp();
	fixed_const_exp(type* t, location loc);
	virtual string to_string() const;
	virtual ~fixed_const_exp();
};

}

#endif /* FIXED_CONST_EXP_H_ */
