#ifndef BINOP_EXPRESSION_H_
#define BINOP_EXPRESSION_H_

#include "expression.h"

namespace il
{


/*
 * If you change this enum type, you also have to change the definition
 * in il/il-c.c!!!!
 */
/**
 * \brief An enum for different binops
 */
enum binop_type{
		_PLUS, _MINUS, _MULTIPLY,_DIV, _MOD,
		_POINTER_PLUS,
		_LT, _LEQ, _GT, _GEQ, /* <, <=, >, >= */
		_EQ, _NEQ, /* =, != */
		_REAL_DIV,
		_LEFT_SHIFT, _RIGHT_SHIFT, /* <<, >>*/
		_BITWISE_OR, _BITWISE_AND, _BITWISE_XOR, /* |, &, ^ */
		_LOGICAL_AND, _LOGICAL_OR, /* &&, || (shortcircuit semantics) */
		_LOGICAL_AND_NO_SHORTCIRCUIT, _LOGICAL_OR_NO_SHORTCIRCUIT,
		_VEC_BINOP /* For now, we don't reason about vector binops */
	};


/**
 * \brief An expression of the form exp1 binop exp2
 */
class binop_expression : public il::expression
{

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & exp1;
        ar & exp2;
        ar & op;
    }


private:
	expression* exp1;
	expression* exp2;
	binop_type op;
public:
	binop_expression();
	binop_expression(expression* exp1, expression* exp2, binop_type op,
			type* t, location loc);
	virtual string to_string() const;
	virtual binop_type get_operator();
	void set_first_operand(expression* op1);
	void set_second_operand(expression* op2);
	virtual expression* get_first_operand();
	virtual expression* get_second_operand();
	virtual bool is_vector_binop();
	static string binop_to_string(binop_type op);
	virtual ~binop_expression();

	static bool is_predicate_binop(il::binop_type bt);

};

}

#endif /*BINOP_EXPRESSION_H_*/
