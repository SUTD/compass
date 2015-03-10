#ifndef FUNCTION_CALL_EXPRESSION_H_
#define FUNCTION_CALL_EXPRESSION_H_

#include "expression.h"
#include <vector>

using namespace std;

namespace il
{

/**
 * \brief A function call
 */
class function_call_expression : public expression
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & fn_name;
        ar & ns;
        ar & fn_signature;
        ar & args;
        ar & is_no_return;
        ar & is_alloc;
        ar & operator_new;
        ar & is_dealloc;
        ar & constructor;
        ar & destructor;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::expression>(*this);
        ar & fn_name;
        ar & ns;
        ar & fn_signature;
        type::register_loaded_typeref(&fn_signature);
        ar & args;
        ar & is_no_return;
        ar & is_alloc;
        ar & operator_new;
        ar & is_dealloc;
        ar & constructor;
        ar & destructor;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	string fn_name;
	namespace_context ns;
	type* fn_signature;
	vector<expression*> args;
	bool is_no_return;
	bool is_alloc;
	bool operator_new;
	bool is_dealloc;
	bool constructor;
	bool destructor;

	
	/**
	 * If this call is a virtual call, then this value represents the
	 * expression that is evaluated in order to obtain the address of
	 * the function to call.  NULL iff and only if this call is not
	 * virtual.
	 */
	expression *vtable_lookup_expr;
	
	/**
	 * If this call is a virtual call, then this value represents the
	 * offset into the vtable at which the function address can be
	 * found.
	 */
	long int vtable_index;


public:
	function_call_expression();
	function_call_expression(string fn_name,
							 namespace_context ns,
	                         type *fn_signature,
	                         vector<expression *> &args,
	                         type *t,
	                         bool is_no_return,
	                         bool is_alloc,
	                         bool is_operator_new,
	                         bool is_dealloc,
	                         bool is_constructor,
	                         bool is_destructor,
	                         expression *vtable_lookup_expr,
	                         long int vtable_index,
	                         location loc);
	
	virtual string to_string() const;

	/**
	 * @return name of the called function
	 */
	virtual string get_function();

	/**
	 *
	 * @return signature of the called function
	 */
	virtual type* get_signature();

	/**
	 *
	 * @ return the namespace associated with the called function.
	 */
	const namespace_context & get_namespace();

	/**
	 *
	 * @return arguments used in the function call
	 */
	virtual vector<expression*>& get_arguments();

	/**
	 * @return Does the called function abort execution?
	 */
	bool is_exit_function();

	/**
	 * @return Is this a memory allocator?
	 */
	bool is_allocator();
	
	/**
	 * @return Is this a call to operator new?
	 */
	bool is_operator_new();

	/**
	 * Is this a call to a memory deallocator?
	 */
	bool is_deallocator();

	/**
	 * Is this a call to a contructor?
	 */
	bool is_constructor();


	/**
	 * Is this a call to a destructor?
	 */
	bool is_destructor();


	bool is_call_to_virtual() const;

	/**
	 *
	 * @return Does the called function have a return value according to its
	 * signature?
	 */
	bool has_return();

	expression *get_vtable_lookup_expr();

	int get_vtable_index();

	virtual ~function_call_expression();
};

}

#endif /*FUNCTION_CALL_EXPRESSION_H_*/
