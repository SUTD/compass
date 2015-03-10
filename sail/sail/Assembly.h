/*
 * Assembly.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef SAIL_ASSEMBLY_H_
#define SAIL_ASSEMBLY_H_

#include "Instruction.h"

namespace il{
	class assembly;
}

namespace sail {

/**
 * \brief Represents inline assembly.
 */

/**
 * All the information is stored in the high-level representation.
 * Use get_original() to access details of the assembly instruction.
 */
class Assembly: public sail::Instruction {

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
    }
private:
public:
	Assembly(il::assembly* c_asm, int line);
	Assembly(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
	virtual ~Assembly();
};

}

#endif /* SAIL_ASSEMBLY_H_ */
