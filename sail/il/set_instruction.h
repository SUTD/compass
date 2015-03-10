#ifndef SET_INSTRUCTION_H_
#define SET_INSTRUCTION_H_

#include "instruction.h"


namespace il
{

/**
 * \brief lhs = rhs;
 */
class set_instruction : public il::instruction
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::instruction>(*this);
    	ar & lvalue;
    	ar & rhs;
    	ar & is_declaration;
    }
private:
	expression* lvalue;
	expression* rhs;
	/* Is this set a declaration in the source? */
	bool is_declaration;



public:
	set_instruction();
	set_instruction(expression* lvalue, expression* rhs, location loc,
			bool declaration);
	expression* get_lvalue();
	expression* get_rhs();
	bool is_declaration_inst();
	void print();
	virtual string to_string() const;
	virtual ~set_instruction();
};

}

#endif /*SET_INSTRUCTION_H_*/
