#ifndef EXPR_LIST_EXPRESSION_H_
#define EXPR_LIST_EXPRESSION_H_

#include "expression.h"
#include <vector>

using namespace std;

namespace il
{
class type;
/**
 * \brief An expression list expression consists of comma separated
 * expressions as in i++, j++ in a for loop.
 */

class expr_list_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & exprs;
    }

private:
	vector<expression*> exprs;

public:
	expr_list_expression();
	expr_list_expression(vector<expression*> & exprs, type* t, location loc);
	vector<expression*>& get_inner_expressions();
	virtual string to_string() const;
	virtual ~expr_list_expression();
};

}

#endif /*EXPR_LIST_EXPRESSION_H_*/
