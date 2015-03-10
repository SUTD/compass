/*
 * vector_const_exp.h
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#ifndef VECTOR_CONST_EXP_H_
#define VECTOR_CONST_EXP_H_

#include "const_expression.h"

namespace il {

/**
 * \brief A vector constant expression
 */
class vector_const_exp: public il::const_expression {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    }
public:
	vector_const_exp();
	vector_const_exp(type* t, location loc);
	virtual string to_string() const;
	virtual ~vector_const_exp();
};

}

#endif /* VECTOR_CONST_EXP_H_ */
