#ifndef DEREF_EXPRESSION_H_
#define DEREF_EXPRESSION_H_

#include "expression.h"

namespace il
{
class type;

/**
 * \brief A pointer dereference expression of the form *exp
 */
class deref_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & inner_exp;
    }

private:
	expression* inner_exp;
public:
	deref_expression();
	deref_expression(expression* inner_exp, type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_inner_expression();
	virtual ~deref_expression();
};

}

#endif /*DEREF_EXPRESSION_H_*/
