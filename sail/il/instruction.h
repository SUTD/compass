#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include "statement.h"
#include <assert.h>

namespace il
{

class instruction: public il::statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::statement>(*this);
    }
public:
	instruction();
	virtual string to_string() const = 0;
	virtual ~instruction();
};

}

#endif /*INSTRUCTION_H_*/
