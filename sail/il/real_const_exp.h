#ifndef REAL_CONST_EXP_H_
#define REAL_CONST_EXP_H_

#include "const_expression.h"

namespace il
{

/**
 * \brief A real constant expression
 */
class real_const_exp : public il::const_expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::const_expression>(*this);
    }
public:
	real_const_exp(){};
	real_const_exp(type* t, location loc);
	virtual ~real_const_exp();
	virtual string to_string() const;
};

}

#endif /*REAL_CONST_EXP_H_*/
