#ifndef FOR_LOOP_H_
#define FOR_LOOP_H_

#include "control_statement.h"
#include <assert.h>


namespace il
{
class expression;
class statement;

/**
 * \brief A for loop construct
 */
class for_loop : public il::control_statement
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::control_statement>(*this);
        ar & init;
        ar & continue_cond;
        ar & update;
        ar & body;
    }
private:
	expression* init;
	expression* continue_cond;
	expression* update;
	statement* body;


public:
	for_loop();
	for_loop(expression* init, expression* continue_cond, expression* update,
			statement* body, location loc);

	/**
	 *
	 * @return the expression representing the continuation condition of the loop
	 */
	expression* get_continuation_cond();

	/**
	 *
	 * @return update expression such as i++
	 */
	expression* get_update_expression();

	/**
	 *
	 * @return the actual body of the loop
	 */
	statement* get_body();
	void print();
	virtual ~for_loop();
	virtual string to_string() const;
};

}

#endif /*FOR_LOOP_H_*/
