/*
 * initializer_list_expression.h
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#ifndef INITIALIZER_LIST_EXPRESSION_H_
#define INITIALIZER_LIST_EXPRESSION_H_

#include "expression.h"

namespace il {

/**
 * \brief An initializer list expression such as
 * struct point {int x; int y;} p = {1, 2};
 */
class initializer_list_expression: public il::expression {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & init_list;
    }
private:
	vector<expression*> init_list;
public:
	initializer_list_expression();
	initializer_list_expression(vector<expression*>& init_list,
			type* t, location loc);
	virtual vector<expression*> & get_init_list();
	virtual string to_string() const;
	virtual ~initializer_list_expression();
};

}

#endif /* INITIALIZER_LIST_EXPRESSION_H_ */
