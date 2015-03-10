#ifndef RETURN_STATEMENT_H_
#define RETURN_STATEMENT_H_

#include "control_statement.h"

namespace il
{
class expression;

class expression;

/**
 * \brief return exp;
 */
class return_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & exp;
    }
private:
	expression* exp;
public:
	return_statement(expression* e, location loc);
	return_statement(){};
	expression* get_ret_expression();
	bool has_return_value();
	void print();
	virtual string to_string() const;
	virtual ~return_statement();
};

}

#endif /*RETURN_STATEMENT_H_*/
