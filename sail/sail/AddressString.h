/*
 * AddressString.h
 *
 *  Created on: Sep 22, 2008
 *      Author: tdillig
 */


#ifndef ADDRESSSTRING_H_
#define ADDRESSSTRING_H_


#include "SaveInstruction.h"

namespace il{class node;}

#include <string>
using namespace std;

namespace il{
class string_const_exp;
class node;
}
namespace sail {

class Variable;
class Symbol;

/**
 * \brief v = &str_constant.
 */
class AddressString:public SaveInstruction {
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
		ar & ce;
		ar & v;
	}
private:
	il::string_const_exp* ce;
	Variable* v;
public:
	AddressString(){};
	AddressString(Variable* v, il::string_const_exp* ce,
			il::node* original, int line);
	virtual ~AddressString();
	virtual string to_string() const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	virtual il::string_const_exp* get_rhs();

	string get_string();
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
};

}
#endif /* ADDRESSSTRING_H_ */
