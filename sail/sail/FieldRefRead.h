/*
 * FieldRefRead.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  v1 = v2.f
 *  We represent field f as an integer offset.
 *
 */

#ifndef FIELDREFREAD_H_
#define FIELDREFREAD_H_

#include "SaveInstruction.h"

#include <string>
using namespace std;

namespace il{class node;
			 class record_info;
			}

namespace sail {

class Variable;

/**
 * \brief Represents a field read instruction of the form
 * v1 = v2.f1. ... .fk.
 */
/**
 * The field selectors are represented by their aggregate offset.
 */
class FieldRefRead: public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v1;
        ar & v2;
        ar & offset;
        ar & field_name;
    }

private:
	Variable* v1;
	Variable* v2;
	int offset;
	string field_name;
public:
	FieldRefRead(Variable* v1, Variable* v2, int offset, il::node* original,
			int line);
	FieldRefRead(Variable* v1, Variable* v2, int offset, string field_name,
			il::node* original, int line);
	FieldRefRead(Variable* v1, Variable* v2, il::record_info* ri,
			il::node* original, int line);
	FieldRefRead(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	/**
	 * @return the struct whose fields are being read.
	 */
	Variable* get_rhs();

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
	virtual ~FieldRefRead();
};

}

#endif /* FIELDREFREAD_H_ */
