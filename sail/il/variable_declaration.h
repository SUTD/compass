

#ifndef VARIABLE_DECLARATION_H_
#define VARIABLE_DECLARATION_H_


#include "declaration.h"
#include "namespace_context.h"
#include <iostream>



namespace il
{
class variable;
class expression;

/**
 * \brief Represents a variable declaration, possibly containing an initialization
 * expression
 */
class variable_declaration : public il::declaration
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::declaration>(*this);
    	ar & var;
    	ar & ns;
    	ar & init_exp;
    }
private:
	 variable *var;
	 expression *init_exp;
	 namespace_context ns;
public:
	friend ostream& operator <<(ostream &os, const variable_declaration *obj);
	variable_declaration(variable * v, namespace_context ns, expression *init,
			location loc);
	variable_declaration();
	virtual ~variable_declaration();
	void set_initializer(expression* init);
	variable* get_variable();
	bool has_initializer();
	expression *get_initializer();
	const namespace_context& get_namespace() const;
	void print();
	node **get_init_ref()
	{
		return (node**)(&init_exp);
	};
	virtual string to_string() const;
	virtual bool is_fun_decl();
	virtual bool is_var_decl();


};

}

#endif /*VARIABLE_DECLARATION_H_*/
