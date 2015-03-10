#ifndef THROW_EXPRESSION_H_
#define THROW_EXPRESSION_H_

#include "expression.h"
#include "location.h"
#include <string>
using namespace std;

namespace il
{

class type;

/**
 * \brief &exp
 */
class throw_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & thrown_type;
        ar & full_throw_expr;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & thrown_type;
        type::register_loaded_typeref(&thrown_type);
        ar & full_throw_expr;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
private:
	type *thrown_type;
	expression *full_throw_expr;

public:
	throw_expression(type *thrown_type, expression *full_throw_expr, type *t, location loc);
	throw_expression();
	virtual ~throw_expression();

	virtual type *get_thrown_type();
	virtual expression *get_full_throw_expression();

	virtual string to_string() const;
};

}

#endif /* THROW_EXPRESSION_H_ */
