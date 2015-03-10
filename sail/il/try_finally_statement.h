#ifndef TRY_FINALLY_STATEMENT_H_
#define TRY_FINALLY_STATEMENT_H_

#include "control_statement.h"

#include <vector>

namespace il
{

/**
 * \brief The representation of a try-finally statement
 */
class try_finally_statement : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & try_statement;
    	ar & exit_statement;
    }
private:
	statement *try_statement;
	statement *exit_statement;

public:
	try_finally_statement(statement *try_stmt,
	                      statement *exit_stmt,
	                      location loc);
	try_finally_statement();
	virtual ~try_finally_statement();
	void print();
	
	statement *get_try_statement();
	statement *get_exit_statement();
	
	virtual string to_string() const;

};

}

#endif /* TRY_FINALLY_STATEMENT_H_ */
