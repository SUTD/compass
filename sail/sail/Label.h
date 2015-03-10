/*
 * Label.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef SAIL_LABEL_H_
#define SAIL_LABEL_H_
#include "Instruction.h"

namespace il{class node;}

#include<string>
using namespace std;

namespace sail {
class Label;

/**
 * \brief A label represented by the string label_name
 */
class Label: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & label_name;
    }
private:
	string label_name;


public:
	Label(string label_name, il::node* original);
	Label(){};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	string get_label_name();
	virtual bool is_removable();
	virtual ~Label();
};

}

#endif /* SAIL_LABEL_H_ */
