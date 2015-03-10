#ifndef FIELD_REF_EXPRESSION_H_
#define FIELD_REF_EXPRESSION_H_

#include "expression.h"
#include <string>

namespace il
{
struct record_info;


/**
 * \brief A field reference expression of the form exp.field
 * The field is represented as a record_info.
 */
class field_ref_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & inner_exp;
    	ar & field_info;
    }

private:
	expression* inner_exp;
	record_info* field_info;
public:
	field_ref_expression();
	field_ref_expression(expression* inner_exp, record_info* field_info,
			type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_inner_expression();
	virtual record_info* get_field_info();
	virtual ~field_ref_expression();
};

}

#endif /*FIELD_REF_EXPRESSION_H_*/
