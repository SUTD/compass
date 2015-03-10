#ifndef CONDITIONAL_EXPRESSION_H_
#define CONDITIONAL_EXPRESSION_H_

#include "expression.h"
#include "statement.h"

namespace il
{
class type;

/**
 * \brief Conditional expression of the form exp ? a :b
 */
class conditional_expression:public expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & cond;
        ar & then_clause;
        ar & else_clause;
    }


private:
	expression* cond;
	expression* then_clause;
	expression* else_clause;

public:
	conditional_expression();
	conditional_expression(expression* cond, expression* then_clause,
			expression* else_clause, type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_conditional();
	virtual expression* get_then_clause();
	virtual expression* get_else_clause();
	virtual ~conditional_expression();
};

}

#endif /*CONDITIONAL_EXPRESSION_H_*/
