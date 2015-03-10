#ifndef DO_WHILE_LOOP_H_
#define DO_WHILE_LOOP_H_

#include "control_statement.h"


using namespace std;


#include "do_while_loop.h"
#include <assert.h>

namespace il
{
class expression;
class statement;

/**
 * \brief A do while loop construct
 */

/**
 * Since GCC replaces looping constructs with goto's during parsing,
 * this instruction currently does  not appear in SAIL.
 */
class do_while_loop : public il::control_statement
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
	do_while_loop();
	do_while_loop(expression* continue_cond, statement* body);
	expression* get_continuation_cond();
	statement* get_body();
	void print();
	virtual ~do_while_loop();
};

}

#endif /*DO_WHILE_LOOP_H_*/
