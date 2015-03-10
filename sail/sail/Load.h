/*
 * Load.h
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 *
 *  A load instruction: v1 = *v2
 */

#ifndef LOAD_H_
#define LOAD_H_

#include "SaveInstruction.h"
#include <string>
using namespace std;

namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

/**
 * \brief A load instruction of of the form  v1 = *s.(f1...f_k)
 * The field selectors are optional and only generated if ALLOW_OFFSETS
 * enabled.
 */
/**
 * The offset represents the aggregate offset of the field selectors.
 * Note: In C, constants can be dereferenced, e.g., for DMA, thus,
 * v2 is a symbol rather than a variable.
 */
class Load: public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v1;
        ar & v2;
        ar & offset;
        ar & name;
    }
private:
	Variable* v1;
	Symbol* v2;
	int offset;
	string name;
public:
	Load(Variable* v1, Symbol* v2, int offset, string name,
			il::node* original, int line);

	Load(Variable* v1, Symbol* v2, int offset, string name, int line);
	Load(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	//haiyan added for load
	//virtual string to_str(bool pretty_print) const;
	//haiyan end for load
	virtual Variable* get_lhs(); // returns v1
	virtual void set_lhs(Variable* v);

	virtual void set_rhs(Symbol* s);


	Symbol* get_rhs(); //returns v2
	int get_offset();
	string& get_field_name();
	virtual bool is_removable();
	virtual ~Load();
};

}

#endif /* LOAD_H_ */
