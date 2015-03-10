#ifndef UNOP_EXPRESSION_H_
#define UNOP_EXPRESSION_H_

#include "expression.h"

namespace il
{

/*
 * If you change this enum type, you also have to change the definition
 * in il/il-c.c!!!!
 */
enum unop_type{
	_NEGATE, /* e.g. -a*/
	_CONJUGATE, /* complex conjugate */
	_PREDECREMENT, _PREINCREMENT,  /* --i, ++i */
	_POSTDECREMENT, _POSTINCREMENT, /* i--, i++ */
	_BITWISE_NOT, /* ~x */
	_LOGICAL_NOT, /* !x */
	_VEC_UNOP /* For now, we don't reason about vector unops */
	};

/**
 * \brief An expression of the form (unop exp)
 */
class unop_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & inner_exp;
    	ar & unop;
    }
private:
	expression* inner_exp;
	unop_type unop;
public:
	unop_expression();
	unop_expression(expression* inner_exp, unop_type unop,
			type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_operand();
	virtual unop_type get_operator();
	virtual bool is_vector_unop();
	virtual bool is_postop() const; /* only for i++ and i-- */
	static string unop_to_string(unop_type op, bool* need_paren);
	virtual bool has_side_effect();
	virtual ~unop_expression();
};

}

#endif /*UNOP_EXPRESSION_H_*/
