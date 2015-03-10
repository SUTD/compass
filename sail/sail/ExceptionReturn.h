/*
 * Label.h
 *
 *  Created on: July 28, 2012
 *      Author: tdillig
 *
 *  Indicates that control flows to the exception block.
 */

#ifndef EXCEPTION_RETURN_H_
#define EXCEPTION_RETURN_H_
#include "Instruction.h"

namespace il{class node;}

#include<string>
using namespace std;

namespace sail {
class ExceptionReturn;

/**
 * \brief A label represented by the string label_name
 */
class ExceptionReturn: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
    }


public:
    ExceptionReturn();
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
	virtual ~ExceptionReturn();
};

}

#endif /* EXCEPTION_RETURN_H_ */
