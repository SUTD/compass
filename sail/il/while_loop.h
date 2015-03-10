#ifndef WHILE_LOOP_H_
#define WHILE_LOOP_H_

#include "control_statement.h"


namespace il
{

/**
 * \brief A while loop construct
 */
/**
 * Since GCC converts all looping constructs to goto's during parsing,
 * this is currently not used.
 */
class while_loop : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
    	ar & continue_cond;
    	ar & body;
    }
private:
	expression* continue_cond;
	statement* body;

public:
	while_loop();
	while_loop(expression* continue_cond, statement* body);
	expression* get_continuation_cond();
	statement* get_body();
	void print();
	virtual ~while_loop();
};

}

#endif /*WHILE_LOOP_H_*/
