/*
 * AddressVar.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 *
 *  v = &symbol
 *  Note: The rhs is a symbol because it's legal to take the address of
 *  a constant, e.g. &NULL. Hence, the rhs can be either a var or constant.
 */

#ifndef ADDRESSVAR_H_
#define ADDRESSVAR_H_

#include "SaveInstruction.h"

namespace il{class node;}

#include <string>
using namespace std;

namespace sail {

class Variable;
class Symbol;

/**
 * \brief v = &s. Note that the left hand side is a symbol
 * rather than a variable because taking the address of constants is legal in C.
 */
class AddressVar:public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & s;
    }
private:
	Variable* v;
	Symbol* s;
public:
	AddressVar(Variable* v, Symbol* s, il::node* original, int line);
	AddressVar(){};
	virtual string to_string() const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);
	Symbol* get_rhs();
	virtual ~AddressVar();
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
};

}

#endif /* ADDRESSVAR_H_ */
