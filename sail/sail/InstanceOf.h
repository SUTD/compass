/*
 * InstanceOf.h
 *
 *  Created on: July 28, 2012
 *      Author: tdillig
 *
 * Implements the instanceof operator in Java. InstanceOf takes a Symbol S
 * and a type T and returns value R. R is 1 if S is an instance of T, 0
 * otherwise.
 */

#ifndef INSTANCEOF_H_
#define INSTANCEOF_H_

#include "SaveInstruction.h"

#include <string>
using namespace std;

namespace il{class node; class type;}

namespace sail {

class Symbol;
class Variable;

/**
 * \brief v = instanceof(s, t)
 */
class InstanceOf: public SaveInstruction {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & s;
        ar & t;
    }
private:
	Variable* v;
	Symbol* s;
	il::type* t;


public:
	InstanceOf(Variable* v, Symbol* s, il::type* t,
			il::node* original, int line);
	InstanceOf(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);
	Symbol* get_rhs();
	virtual void set_rhs(Symbol* s);

	il::type* get_instance_type();
	virtual bool is_removable();

	/*
	 * Note: this can return NULL
	 */
	il::node* get_original_node();
	virtual ~InstanceOf();
};

}

#endif /* INSTANCEOF_H_ */
