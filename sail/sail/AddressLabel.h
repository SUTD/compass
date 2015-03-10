/**
 * AddressLabel.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *   v = &label;
 */

#ifndef ADDRESSLABEL_H_
#define ADDRESSLABEL_H_

#include "SaveInstruction.h"


#include<string>
using namespace std;

namespace il{
	class node;
	class type;
}

namespace sail {

class Variable;

/**
 * \brief  v = &label.
 * This is used for modeling a GNU C extension that
 * allows taking address of labels.
 */
class AddressLabel:public SaveInstruction {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & v;
        ar & label;
        ar & is_function;
    }
private:
	Variable* v;
	string label;
	bool is_function;
public:
	AddressLabel(Variable* v, string label, bool is_function,
			il::node* original, int line);
	AddressLabel(){};
	virtual ~AddressLabel();
	virtual string to_string() const;
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);
	string get_label();
	bool is_function_label();
	il::type* get_signature();
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
};

}

#endif /* ADDRESSLABEL_H_ */
