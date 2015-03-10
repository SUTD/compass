#ifndef VARIABLE_EXPRESSION_H_
#define VARIABLE_EXPRESSION_H_

#include "expression.h"


namespace il
{
class variable;

/**
 * \brief A variable expression
 */
class variable_expression: public expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & var;
    }
private:
	variable *var;
public:
	variable_expression();
	variable_expression(variable *v,  location loc);
	virtual ~variable_expression();
	virtual string to_string() const;
	variable *get_variable();
};

}

#endif /*VARIABLE_EXPRESSION_H_*/
