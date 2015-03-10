/*
 * FunctionCall.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 *  ret = fn_name(args)
 */

#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include "SaveInstruction.h"
#include <string>
#include <vector>
#include <set>
#include "type.h"
#include "call_id.h"
using namespace std;
namespace il{class type;}

namespace sail {
class Variable;
class Symbol;

/**
 * \brief Represents a function call instruction of the form ret = f(s1, ... ,sn)
 */
/**
 * The variable representing the return value can be NULL if either
 * the function does not have a return value or if the return value
 * is not captured.
 */
class FunctionCall: public SaveInstruction  {
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & ret;
        ar & fn_name;
        ar & fn_signature;
        ar & args;
        ar & virtual_dispatch;
        ar & ns;
        ar & is_allocator_flag;
        ar & is_constructor_flag;
        ar & is_super_flag;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<sail::SaveInstruction>(*this);
        ar & ret;
        ar & fn_name;
        ar & fn_signature;
        il::type::register_loaded_typeref(&fn_signature);
        ar & args;
        ar & virtual_dispatch;
        ar & ns;
        ar & is_allocator_flag;
        ar & is_constructor_flag;
        ar & is_super_flag;

    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	Variable* ret; // this can be null if function doesn't have a return value
				   // or if the return value is not captured. Use has_return()
				   // to check.
	string fn_name;

	/**
	 * Signature can be null for builtin functions like va_args
	 * or macros we might potentially turn into function.
	 */
	il::type* fn_signature;
	vector<Symbol*>* args;
	il::namespace_context ns;

	bool virtual_dispatch;
	bool is_allocator_flag;
	bool is_constructor_flag;
	bool is_super_flag;



public:
	FunctionCall(Variable* ret, string fn_name, il::namespace_context ns,
			il::type* fn_signature,
			vector<Symbol*>* args, bool virtual_dispatch,
			bool is_allocator, bool is_constructor, bool is_super,
			il::node* original,
			int line);

	FunctionCall(Variable* ret, string fn_string, il::namespace_context ns,
			il::type* fn_signature, vector<Symbol*>* args,
			bool virtual_dispatch, bool is_allocator, bool is_constructor,
			bool is_super, int line);

	FunctionCall(){args = NULL;};
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;

	/**
	 * @return name of the function that is called
	 */
	string get_function_name();

	/**
	 * @return namespace associated with the called function.
	 */
	il::namespace_context get_namespace() const;

	/**
	 * @return Signature of the function being called, this can return NULL!!!
	 */
	il::type* get_signature();

	/**
	 * @return the arguments used in the function call.
	 */
	vector<Symbol*>* get_arguments();

	/**
	 * @return Does this function have a return value that is captured?
	 */
	bool has_return();

	/**
	 * @return the variable capturing the result of the function call.
	 */
	Variable* get_return_variable();


	/**
	 * @return Is this a virtual method call?
	 */
	bool is_virtual_call();



	/**
	 * @return Returns the set of all call_id's that can be the target
	 * of this call based on the type hierarchy.
	 */
	void get_virtual_call_targets(set<call_id>& targets);

	virtual Variable* get_lhs();
	virtual void set_lhs(Variable* v);
	virtual bool is_removable();

	virtual void set_rhs(Symbol* s, int pos);


	/**
	 * @return Does this function abort program execution? (e.g., a call
	 * to exit())
	 */
	bool is_exit_function() const;

	/**
	 * @return Is this function a memory allocator?
	 */
	bool is_allocator() const;

	/**
	 * @return Is this a call to a method in the superclass?
	 */
	bool is_super() const;

	/**
	 *@return Is this a call to operator new?
	 */
	bool is_operator_new() const;


	/**
	 * @return Is this a call to a memory deallocator?
	 */
	bool is_deallocator() const;


	/**
	 * @return Is this a call to a constructor?
	 */
	bool is_constructor();


	/**
	 * @return Is this a call to a destructor?
	 */
	bool is_destructor();

	virtual ~FunctionCall();


	/*
	 * This is a helper function for get_virtual_call_targets.
	 * Finds a method with the matching name and signature declared in class t.
	 * In comparing the signatures, the type of the first argument is ignored
	 * since it corresponds to the type of the this pointer.
	 */
	il::method_info* find_matching_method(il::record_type* t,
			const string& name, il::type* signature);
};

}

#endif /* FUNCTIONCALL_H_ */
