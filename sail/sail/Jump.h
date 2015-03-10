/*
 * Jump.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef JUMP_H_
#define JUMP_H_

#include "Instruction.h"

#include <string>
using namespace std;

namespace il{class node;}

namespace sail {
class Label;

/**
 * \brief Jump instruction to some label.
 */
class Jump: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & l;
    }
private:
	Label* l;
public:
	Jump(Label* l, il::node* original);
	Jump(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	void set_label(Label* l);
	/**
	 * @return the label that is target of this jump.
	 */
	Label* get_label();
	virtual bool is_removable();
	virtual ~Jump();
};

}

#endif /* JUMP_H_ */
