#ifndef IF_STATEMENT_H_
#define IF_STATEMENT_H_

#include "control_statement.h"


namespace il
{

/**
 * \brief The representation of an if statement
 */
class if_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & test;
    	ar & if_branch;
    	ar & else_branch;
    }
private:
	expression* test;
	statement* if_branch;
	statement* else_branch;
public:
	if_statement(expression* test, statement* if_branch,
			statement* else_branch, location loc);
	if_statement();
	void print();
	expression* get_test();
	statement* get_if_statement();
	statement* get_else_statement();
	virtual ~if_statement();
	virtual string to_string() const;

};

}

#endif /*IF_STATEMENT_H_*/
