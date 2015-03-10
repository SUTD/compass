/*
 * FieldRefWrite.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  v1.f = v2
 */

#ifndef FIELDREFWRITE_H_
#define FIELDREFWRITE_H_

#include "Instruction.h"

#include <string>
using namespace std;

namespace il{class node;
			 class record_info;}

namespace sail {

class Variable;
class Symbol;

/**
 * \brief Represents a field write of the form v.f1...fk = s
 */
class FieldRefWrite: public Instruction{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & v1;
        ar & v2;
        ar & offset;
        ar & field_name;
    }
private:
	Variable* v1;
	Symbol* v2;
	int offset;
	string field_name;
public:
	FieldRefWrite(Variable* v1, Symbol* v2, int offset, il::node* original,
			int line);
	FieldRefWrite(Variable* v1, Symbol* v2,
			il::record_info* ri, il::node* original, int line);
	FieldRefWrite(Variable* v1, Symbol* v2,
				string & name, int offset, il::node* original, int line);
	FieldRefWrite(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;

	/**
	 *
	 * @return the struct whose fields are written to
	 */
	Variable* get_lhs();

	/**
	 *
	 * @return the value that is written
	 */
	Symbol* get_rhs();

	/**
	 * @return the aggregate offset of the field selectors
	 *
	 */
	int get_offset();

	/**
	 *
	 * @return a string representing the sequence of field selectors
	 * for pretty printing.
	 */
	string get_field_name();
	virtual bool is_removable();
	virtual ~FieldRefWrite();
};

}

#endif /* FIELDREFWRITE_H_ */
