/*
 * Store.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 *
 *  A store instruction: *v1 = v2
 */

#ifndef STORE_H_
#define STORE_H_

#include "Instruction.h"

#include <string>
using namespace std;

namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

/**
 * \brief A store instruction is of the form *s1.f1...fn = s2.
 * Field selectors are optional and only appear if ALLOW_OFFSETS
 * is enabled.
 */
 /**
 * The field selectors in this instruction are optional; if ALLOW_OFFSETS in Function.cpp
 * is set to false, SAIL generates only Store instructions with no
 * field selectors. Note that disabling ALLOW_OFFSETS will
 * cause stores with offsets to generate instructions that involve pointer
 * arithmetic.
 */
class Store: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & v1;
        ar & v2;
        ar & offset;
        ar & name;
    }

private:
	Symbol* v1;
	Symbol* v2;
	int offset;
	string name;

public:
	Store(Symbol* v1, Symbol* v2, int offset, string name,
			il::node* original, int line);



	//haiyan added
	Store(Symbol* v1, Symbol* v2, int offset, string name, int line);
	//haiyan ended


	Store(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	//haiyan added 5.31
	//virtual string to_str(bool pretty_print) const;
	//haiyan ended 5.31
	Symbol* get_rhs();
	Symbol* get_lhs();

	void set_lhs(Symbol* s1);
	void set_rhs(Symbol* s2);

	int get_offset();
	string & get_field_name();
	virtual bool is_removable();
	virtual ~Store();
};

}

#endif /* STORE_H_ */
