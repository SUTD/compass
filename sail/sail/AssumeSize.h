/*
 * AssumeSize.h
 *
 *  Created on: Apr 10, 2009
 *      Author: tdillig
 */

#ifndef ASSUMESIZE_H_
#define ASSUMESIZE_H_

#include "Instruction.h"

#define ASSUME_SIZE_ID "assume_size"


namespace sail {

/**
 * \brief A convenience instruction for annotating buffer sizes.
 * assume_size(b, s) indicates size of buffer b is s (in bytes).
 */
class AssumeSize: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & buffer;
        ar & size;
    }

private:
	Symbol* buffer;
	Symbol* size;

public:
	AssumeSize();
	AssumeSize(Symbol*  buffer, Symbol*  size, il::node* original, int line);
	virtual ~AssumeSize();
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	il::node* get_original_node();

	virtual bool is_save_instruction();

	bool is_synthetic();
	Symbol* get_buffer();
	Symbol* get_size();

	virtual bool is_removable();
};

}

#endif /* ASSUMESIZE_H_ */
