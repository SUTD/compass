#ifndef NOOP_INSTRUCTION_H_
#define NOOP_INSTRUCTION_H_

#include "instruction.h"

namespace il
{

/**
 * \brief A noop instruction
 */
class noop_instruction:public instruction
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::instruction>(*this);
    }
public:
	noop_instruction();
	virtual ~noop_instruction();
	virtual string to_string() const {return "noop";};
	virtual void print(){cout << "noop"<<endl;};
};

}

#endif /*NOOP_INSTRUCTION_H_*/
