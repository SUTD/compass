#ifndef EXPRESSION_INSTRUCTION_H_
#define EXPRESSION_INSTRUCTION_H_

#include "instruction.h"


namespace il
{
class expression;

/**
 * \brief Expression used as an instruction such as i++;
 */
class expression_instruction : public il::instruction
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::instruction>(*this);
        ar & exp;
    }

private:
	expression* exp;

public:
	expression_instruction();
	expression_instruction(expression* exp);
	virtual string to_string() const;
	expression* get_expression();
	virtual void print();
	virtual ~expression_instruction();
};

}

#endif /*EXPRESSION_INSTRUCTION_H_*/
