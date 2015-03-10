/*
 * StaticAssert.h
 *
 *  Created on: Oct 10, 2008
 *      Author: tdillig
 */

#ifndef STATICASSERT_H_
#define STATICASSERT_H_
#include "Instruction.h"

#define STATIC_ASSERT_ID "static_assert"

namespace sail {

/**
 * \brief A static assert instruction of the form
 * static_assert(predicate) tells the static analysis to check
 * whether this predicate holds.
 */
class StaticAssert: public Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & assert_predicate;
    }
private:
	Symbol* assert_predicate;
public:
	StaticAssert();
	StaticAssert(Symbol*  arg, il::node* original, int line);
	virtual ~StaticAssert();

	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	il::node* get_original_node();

	virtual bool is_save_instruction();

	/*
	 * is_synthetic() returns true if this instruction does not
	 * have a valid mapping in the original AST.
	 * E.g. is_synthetic is true for label statements artficially introduced
	 * by SAIL.
	 */
	bool is_synthetic();
	Symbol* get_predicate();
	void set_predicate(Symbol* s);

	virtual bool is_removable();
};
}
#endif /* STATICASSERT_H_ */
