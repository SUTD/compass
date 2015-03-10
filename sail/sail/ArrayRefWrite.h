/*
 * ArrayRefWrite.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *   v1[index] = v2;
 */

#ifndef ARRAYREFWRITE_H_
#define ARRAYREFWRITE_H_

#include "Instruction.h"

#include <string>
using namespace std;

namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

/**
 *
 * \brief  v1[index].(f1...fk) = s. Field selectors are optional
 * and appear only if ALLOW_OFFSETS is enabled.
 */

/**
 * Here, v1 is always a non-pointer array.
 * The sequence of field selectors is represented by the ivar called offset and
 * the "name" can be used for pretty printing the fields.
 * The offset in this instruction is optional; if ALLOW_OFFSETS in Function.cpp
 * is set to false, SAIL generates only ArrayWrite instructions with no
 * field selectors.
 */
class ArrayRefWrite:public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & v1;
        ar & v2;
        ar & index;
        ar & offset;
        ar & name;
    }

private:
	Variable* v1;
	Symbol* v2;
	Symbol* index;
	int offset;
	string name;
public:
	ArrayRefWrite(Variable* v1, Symbol* v2, Symbol* index,
			int offset, string name, il::node* original, int line);
	ArrayRefWrite(){};
	virtual string to_string() const;

	/**
	 * @return the array that is written to
	 */
	Variable* get_lhs();

	/**
	 * @return the value that is written
	 */
	Symbol* get_rhs();

	/**
	 *
	 * @return the index of the array
	 */
	Symbol* get_index();

	/**
	 * @return offset of the aggregate field selectors f1, ..., fk.
	 */
	int get_offset();

	/**
	 * @return string representing the field selectors that can be used for
	 * pretty-printing the offset
	 */
	string get_field_name();
	virtual bool is_removable();
	virtual string to_string(bool pretty_print) const;
	virtual ~ArrayRefWrite();
};

}

#endif /* ARRAYREFWRITE_H_ */
