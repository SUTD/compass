/*
 * modify_expression.h
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#ifndef MODIFY_EXPRESSION_H_
#define MODIFY_EXPRESSION_H_

#include "expression.h"


namespace il {
class set_expression;
class set_instruction;
/**
 * \brief Used to model C feature where set statements can be used as expressions,
 * e.g., c = (a=b)
 */
class modify_expression: public il::expression {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & s;
    	ar & is_initalizing;
    }
private:
	set_instruction* s;
	bool is_initalizing;
public:
	modify_expression();
	modify_expression(set_instruction* s, bool is_initalizing, location loc);
	virtual string to_string() const;
	virtual set_instruction* get_set_instruction();
	virtual ~modify_expression();
	bool is_initalizing_exp();
};

}

#endif /* MODIFY_EXPRESSION_H_ */
