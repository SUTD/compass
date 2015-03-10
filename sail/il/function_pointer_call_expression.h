#ifndef FUNCTION_POINTER_CALL_EXPRESSION_H_
#define FUNCTION_POINTER_CALL_EXPRESSION_H_

#include "expression.h"
#include <vector>

using namespace std;


namespace il
{
/**
 * \brief Represents a function pointer invocation.
 */
class function_pointer_call_expression : public il::expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::expression>(*this);
    	ar & fn_ptr;
    	ar & args;
    }
private:
	expression* fn_ptr;
	vector<expression*> args;


public:
	function_pointer_call_expression();
	function_pointer_call_expression(expression* fn_ptr,
			vector<expression*> args, type* t, location loc);
	virtual string to_string() const;
	virtual expression* get_function_pointer();
	bool has_return();
	virtual vector<expression*>& get_arguments();
	virtual ~function_pointer_call_expression();
};

}

#endif /*FUNCTION_POINTER_CALL_EXPRESSION_H_*/
