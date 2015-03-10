#ifndef FUNCTION_DECLARATION_H_
#define FUNCTION_DECLARATION_H_

#include "declaration.h"
#include <vector>
#include <string>

using namespace std;

#include "location.h"
#include <vector>
#include "variable.h"
#include "namespace_context.h"

namespace il
{
class block;
class variable_declaration;
class function_declaration;


/**
 * \brief Represents a function declaration
 */
class function_declaration : public il::declaration
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & boost::serialization::base_object<il::declaration>(*this);
        ar & qt;
        ar & name;
        ar & ns;
        ar & ret_type;
        ar & arguments;
        ar & body;
        ar & is_alloc;
        ar & is_dealloc;
        ar & constructor;
        ar & destructor;
        ar & is_inline;
        ar & vararg;
        ar & member_function;
        ar & static_member_function;
        ar & is_virtual;
        ar & is_abstract;
        ar & signature;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<il::declaration>(*this);
        ar & qt;
        ar & name;
        ar & ns;
        ar & ret_type;
        type::register_loaded_typeref(&ret_type);
        ar & arguments;
        ar & body;
        ar & is_alloc;
        ar & is_dealloc;
        ar & constructor;
        ar & destructor;
        ar & is_inline;
        ar & vararg;
        ar & member_function;
        ar & static_member_function;
        ar & is_virtual;
        ar & is_abstract;
        ar & signature;
        il::type::register_loaded_typeref((il::type**)&signature);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
	qualifier_type qt;
	string name;
	namespace_context ns;
	type *ret_type;
	vector<variable_declaration*> arguments;
	block* body;
	bool is_alloc;
	bool is_dealloc;
	bool constructor;
	bool destructor;
	bool is_inline;
	bool vararg;
	bool member_function;
	bool static_member_function;
	bool is_virtual;
	bool is_abstract;
	function_type* signature;

public:
	function_declaration();
	function_declaration(string name,
	                     namespace_context ns,
	                     qualifier_type qt,
	                     type *ret_type,
	                     vector<variable_declaration *> &args,
	                     block *body,
	                     bool is_alloc,
	                     bool is_dealloc,
	                     bool is_constructor,
	                     bool is_destructor,
	                     bool is_inline,
	                     bool is_vararg,
	                     bool is_member_function,
	                     bool is_static_member_function,
	                     bool is_virtual,
	                     bool is_abstract,
	                     function_type *sig);

	friend ostream &operator<<(ostream &os, const function_declaration &obj);

	/**
	 * @return name of the declared function
	 */
	string get_name();

	/**
	 *
	 * @return number of arguments of this function
	 */
	int get_num_args();



	/**
	 * @return Is this function a method (i.e., a member function) of a class?
	 */
	bool is_method();

	/**
	 * @return Is this function declared as virtual?
	 */
	bool is_virtual_method();


	/**
	 * @return Is this function declared to be an abstract virtual method?
	 */
	bool is_abstract_virtual_method();

	/**
	 * @return Is this a memeber function declared as static?
	 */
	bool is_static_method();

	/**
	 * @return Is this function a memory allocator?
	 */
	bool is_allocator();

	/**
	 * @return Is this function a memory deallocator?
	 */
	bool is_deallocator();

	/**
	 * @return Is this a constructor?
	 */
	bool is_constructor();

	/**
	 * @return Is this a destructor?
	 */
	bool is_destructor();



	/**
	 *
	 * @return the function body
	 */
	block* get_body();

	/**
	 * @param Body of the function
	 */
	void set_body(block *b);

	/**
	 *
	 * @return arguments of function
	 */
	const vector<variable_declaration*> & get_arguments();

	/**
	 *
	 * @return return type of the function
	 */
	type* get_ret_type();

	/**
	 * @return the namespace in which this function is declared
	 */
	const il::namespace_context & get_namespace();


	/**
	 * @return Is this a vararg function?
	 */
	bool is_vararg();

	/**
	 * @return Is this function declared as "inline"?
	 */
	bool is_inlined();
	virtual ~function_declaration();
	virtual void print();
	virtual string to_string() const;
	virtual bool is_fun_decl();
	virtual bool is_var_decl();

	/**
	 * @return signature of this function
	 */
	virtual function_type *get_signature();
};

}

#endif /*FUNCTION_DECLARATION_H_*/
