/*
 * Binop.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  v = s1 BINOP s2
 */

#ifndef BINOP_H_
#define BINOP_H_

#include "SaveInstruction.h"

namespace il{
	class node;
}
#include "../il/binop_expression.h"

/*
 * IMPORTANT: In sail, all logical connectives have been converted to have non
 * short-circuit connectives. SO, we can treated LOGICAL_AND and
 * LOGICAL_AND_NO_SHORT_CIRCUIT in exactly the same way. Short circuit
 * semantics is captured by modifying the control flow.
 */

namespace sail {

class Variable;
class Symbol;

/**
 * \brief An instruction of the form v = s1 binop s2.
 */
/**
 * Different binop's are identified by the enum il::binop_type
 * defined in il::binop_expression. In the low-level representation,
 * all logical connectives have been converted to have non
 * short-circuit connectives. So, LOGICAL_AND and
 * LOGICAL_AND_NO_SHORT_CIRCUIT can be treated in the same way. Short circuit
 * semantics is captured by modifying the control flow.
 */
class Binop:public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & s1;
        ar & s2;
        ar & binop;
        ar & v;
    }

private:

	Symbol* s1;
	Symbol* s2;
	il::binop_type binop;
	Variable* v;

public:

	Binop(Variable* v, Symbol* s1, Symbol* s2, il::binop_type binop,
			il::node* original, int line);
	Binop(Variable* v, Symbol* s1, Symbol* s2, il::binop_type binop, int line);
	Binop(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;

	/**
	 * @return the variable holding the result of the binop
	 */
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	virtual void set_rhs1(Symbol* s1);
	virtual void set_rhs2(Symbol* s2);

	/**
	 *
	 * @return first operand of binop
	 */
	Symbol* get_first_operand();

	/**
	 *
	 * @return the second operand of binop
	 */
	Symbol* get_second_operand();

	/**
	 *
	 * @return the type of the binop, e.g. +, % etc.
	 */
	il::binop_type get_binop();
	virtual bool is_removable();
	virtual ~Binop();
};

}

#endif /* BINOP_H_ */
