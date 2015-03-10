/*
 * block_expression.h
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 */

#ifndef BLOCK_EXPRESSION_H_
#define BLOCK_EXPRESSION_H_

#include "expression.h"


namespace il {
class block;
class variable_declaration;

/**
 *  \brief This is used to model a gcc extension where you can write code like
 *  a = ({args[1]+1;args[2];});
 *  ({args[1]+1;args[2];}) evaluates to the result of the last statement
 *  in the block.
 */
class block_expression: public il::expression {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & b;
    	ar & var_decl;
    }
private:
	block* b;
	variable_declaration* var_decl;
public:
	block_expression();
	block_expression(block* b, variable_declaration* var_decl,
			type* t, location loc);
	virtual string to_string() const;
	virtual block* get_block();
	virtual variable_declaration* get_vardecl();
	virtual ~block_expression();

};

}

#endif /* BLOCK_EXPRESSION_H_ */
