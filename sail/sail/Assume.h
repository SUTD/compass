/*
 * Assume.h
 *
 *  Created on: Apr 10, 2009
 *      Author: tdillig
 */

#ifndef ASSUME_H_
#define ASSUME_H_

#include "Instruction.h"

#define ASSUME_ID "assume"

namespace sail {

/**
 * \brief  assume(predicate) serves as an
 * annotation for static analysis.
 */
class Assume: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & assume_predicate;
    }

private:
	Symbol* assume_predicate;

public:
	Assume();
	Assume(Symbol*  arg, il::node* original, int line);
	virtual ~Assume();
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	il::node* get_original_node();

	virtual bool is_save_instruction();

	bool is_synthetic();

	Symbol* get_predicate();

	virtual bool is_removable();
};

}

#endif /* ASSUME_H_ */
