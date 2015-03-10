#ifndef CONST_EXPRESSION_H_
#define CONST_EXPRESSION_H_

#include "expression.h"
#include <assert.h>

namespace il
{

/**
 * \brief An abstract class representing integer, real or string constants
 */
class const_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    }

public:
	const_expression();
	virtual string to_string() const = 0;
	virtual ~const_expression();
};

}

#endif /*CONST_EXPRESSION_H_*/
