/*
 * Assignment.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 * A variable assignment assigns a symbol (variable or const) to  a variable.
 *
 * Note: In C, one is not allowed to assigned a variable corresponding to
 * static array to another variable. In SAIL, the variables can correspond to
 * arrays, and the semantics is a element-wise copy, similar to
 * assignments from one struct to another.
 */

#ifndef ASSIGNMENT_H_
#define ASSIGNMENT_H_

#include "SaveInstruction.h"

#include <string>
using namespace std;

namespace il{class node;}

namespace sail {

class Symbol;
class Variable;

/**
 * \brief v = s
 */
class Assignment: public SaveInstruction {

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
	Assignment(Variable* v, Symbol* s, il::node* original, int line);
	Assignment(){};


	//added by haiyan
	Assignment(Variable*v, Symbol*s = NULL, int line = -1);
	//end of haiyan

	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	virtual void set_rhs(Symbol* s);

	Symbol* get_rhs();
	virtual bool is_removable();

	/*
	 * Note: this can return NULL if the assignment is fabricated
	 * to introduce a temporary.
	 */
	il::node* get_original_node();
	virtual ~Assignment();
};

}

#endif /* ASSIGNMENT_H_ */
