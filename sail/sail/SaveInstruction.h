/*
 * SaveInstruction.h
 *
 *  Created on: Jul 5, 2008
 *      Author: isil
 *
 *  An abstract superclass of instructions that have a variable
 *  on the left hand side.
 */

#ifndef SAVEINSTRUCTION_H_
#define SAVEINSTRUCTION_H_

#include "Instruction.h"
//haiyan added
#include <sstream>

namespace sail {

class SaveInstruction: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
    }
public:
	SaveInstruction();
	virtual void set_lhs(Variable* v) = 0;
	virtual Variable* get_lhs() = 0;
	virtual ~SaveInstruction();
};

}

#endif /* SAVEINSTRUCTION_H_ */
