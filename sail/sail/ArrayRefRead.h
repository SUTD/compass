/*
 * ArrayRefRead.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  v1 = v2[index];
 */

#ifndef ARRAYREFREAD_H_
#define ARRAYREFREAD_H_

#include "SaveInstruction.h"

#include <string>
using namespace std;


namespace il{class node;}

namespace sail {
class Variable;
class Symbol;

/**
 * \brief v1 = v2[index].(f1 ... .fk); field selectors are optional -- this
 * instruction includes field selectors only if ALLOW_OFFSETS is enabled.
 */

/**
 * Here, v2 is always a non-pointer array.
 * The sequence of field selectors is represented by the offset and
 * the ivar called name can be used for pretty printing the fields.
 * The offset in this instruction is optional; if ALLOW_OFFSETS in Function.cpp
 * is set to false, SAIL generates only ArrayRead instructions with no
 * field selectors.
 */
class ArrayRefRead: public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v1;
        ar & v2;
        ar & index;
        ar & offset;
        ar & name;
    }
private:
	Variable* v1;
	Variable* v2;
	Symbol* index;
	int offset; /**<Represents the offset of the aggregate field selectors*/
	string name; /**<Can be used to pretty-print the field selectors*/


public:
	ArrayRefRead(Variable* v1, Variable* v2, Symbol* index, int offset,
			string name, il::node* original, int line);
	ArrayRefRead(){};

	/**
	 * @return the offset of the aggregate field selectors.
	 */
	int get_offset();
	/**
	 * @return a string that can be used to pretty-print the field selectors that are
	 * represented by the offset.
	 */
	string get_field_name();
	virtual string to_string() const;

	/**
	 * @return the variable storing result of the array read.
	 */
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	/**
	 *
	 * @return the array that is read from
	 */
	Variable* get_rhs();

	/**
	 * @return the index
	 */
	Symbol* get_index();
	virtual bool is_removable();
	virtual string to_string(bool pretty_print) const;
	virtual ~ArrayRefRead();
};

}

#endif /* ARRAYREFREAD_H_ */
