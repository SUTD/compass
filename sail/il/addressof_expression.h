#ifndef ADDRESSOF_EXPRESSION_H_
#define ADDRESSOF_EXPRESSION_H_

#include "expression.h"
#include "location.h"
#include <string>
using namespace std;



namespace il
{

class type;

/**
 * \brief &exp
 */
class addressof_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
        ar & inner_exp;
    }


private:
	expression* inner_exp;
public:
	addressof_expression(expression* inner_exp, type* t, location loc);
	addressof_expression();
	virtual expression* get_inner_expression();
	virtual string to_string() const;
	virtual ~addressof_expression();

};

}

#endif /*ADDRESSOF_EXPRESSION_H_*/
