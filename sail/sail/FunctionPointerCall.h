/*
 * FunctionPointerCall.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *
 *  A function pointer call: v1 = v2(args)
 */

#ifndef FUNCTIONPOINTERCALL_H_
#define FUNCTIONPOINTERCALL_H_

#include "SaveInstruction.h"

#include<string>
#include<vector>
using namespace std;

namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

/**
 * \brief A function pointer call instruction is of the form
 * ret = (*fn_ptr)(s1, ... sk)
 */
class FunctionPointerCall: public SaveInstruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & ret;
        ar & fn_ptr;
        ar & args;
    }

private:
	Variable* ret; // can be null
	Variable* fn_ptr;
	vector<Symbol*>* args;

public:
	FunctionPointerCall(Variable* ret, Variable* fn_ptr, vector<Symbol*>* args,
			il::node* original, int line);
	FunctionPointerCall(){args = NULL;};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;

	/**
	 * @return Is the return value captured?
	 */
	bool has_return();

	/**
	 *
	 * @return the variable used to capture the return value; can be NULL.
	 */
	Variable* get_return_variable();
	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);

	/**
	 * @return the function pointer that's invoked
	 */
	Variable* get_function_pointer();


	/**
	 * @return Arguments used in this call.
	 */
	vector<Symbol*>* get_arguments();
	virtual bool is_removable();
	virtual ~FunctionPointerCall();
};

}

#endif /* FUNCTIONPOINTERCALL_H_ */
