/*
 * Unop.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  v = UNOP s
 */

#ifndef UNOP_H_
#define UNOP_H_

#include "SaveInstruction.h"

#include "../il/unop_expression.h"
namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

/**
 * \brief An instruction of the form v = (unop) s.
 */

/**
 * The unop is defined in il::unop_expression, but the low-level
 * language only contains a subset of the unops defined in il::unop_expression.
 * For instance, pre and post increments are not allowed to appear at the
 * low-level representation.
 */
class Unop: public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & s;
        ar & unop;
    }
private:
	Variable* v;
	Symbol* s;
	il::unop_type unop;
public:
	Unop(Variable* v, Symbol* s, il::unop_type unop,
			il::node* original, int line);

	Unop(Variable* v, Symbol* s, il::unop_type unop, int line);
	Unop(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	virtual void set_rhs(Symbol* s);

	Symbol* get_operand();
	il::unop_type get_unop();
	virtual bool is_removable();
	virtual ~Unop();
};

}

#endif /* UNOP_H_ */
