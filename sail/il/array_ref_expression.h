#ifndef ARRAY_REF_EXPRESSION_H_
#define ARRAY_REF_EXPRESSION_H_

#include "expression.h"

namespace il
{

/** \brief Represents a reference to any non-pointer array
 */

class array_ref_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & array_exp;
        ar & index_exp;
    }



private:
	expression* array_exp;
	expression* index_exp;

public:
	array_ref_expression();
	array_ref_expression(expression* array_exp, expression* index_exp,
			type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_array_expression();
	virtual expression* get_index_expression();
	virtual ~array_ref_expression();

};

}

#endif /*ARRAY_REF_EXPRESSION_H_*/
