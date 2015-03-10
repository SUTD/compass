#ifndef CAST_EXPRESSION_H_
#define CAST_EXPRESSION_H_

#include "expression.h"
#include "location.h"

namespace il
{

/**
 * \brief Represents a cast (t) exp
 */
class cast_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & exp;
    }

private:
	expression* exp;

public:
	cast_expression();
	cast_expression(expression* exp, type* t, location loc);
	virtual string to_string() const;
	virtual ~cast_expression();
	virtual expression* get_inner_expression();
};

}

#endif /*CAST_EXPRESSION_H_*/
