
#ifndef VARIABLE_H_
#define VARIABLE_H_

#include <string>
#include <iostream>

#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

#include "type.h"
#include "namespace_context.h"

using namespace std;




namespace il
{
class type;
enum qualifier_type {NONE = 0, EXTERN = 1, VOLATILE = 2, STATIC = 4,
			REGISTER = 8, CONSTANT = 16, PRIVATE = 32, PUBLIC=64,
			PROTECTED=128};

string qt_to_string(qualifier_type qt);



enum scope_type {GLOBAL, ARGUMENT, LOCAL};

class variable_declaration;
class block;

/**
 * \brief Representation of a program variable
 */
class variable
{
    friend class boost::serialization::access;

    template<class Archive>
    void save(Archive & ar, const unsigned int version) const
    {
        ar & name;
        ar & ns;
        ar & scope;
        ar & qt;
        ar & decl;
        ar & t;
        ar & st;
        ar & arg_num;
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version)
    {
        ar & name;
        ar & ns;
        ar & scope;
        ar & qt;
        ar & decl;
        ar & t;
        type::register_loaded_typeref(&t);
        ar & st;
        ar & arg_num;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()


private:

    static int unmodeled_counter;
	string name;
	namespace_context ns;
	block *scope;
	qualifier_type qt;
	variable_declaration *decl;
	type* t;
	scope_type st;
	int arg_num;


public:

	static variable* get_unmodeled_var();

	friend ostream& operator <<(ostream &os, const variable &obj);
	variable();
	variable(string name, namespace_context ns,
			block *scope, scope_type st, qualifier_type qt,
			variable_declaration* decl, type* t, int i=-1);

	bool is_global();
	bool is_local();
	bool is_argument();
	void set_declaration(variable_declaration *vd){decl = vd;};
	virtual string to_string() const;
	void set_scope(block *b);
	int get_arg_num();
	variable_declaration* get_declaration(){return decl;}

	/*
	 * Returns NULL for global and argument scopes
	 */
	block *get_scope();
	scope_type get_scope_type();
	type* get_type(){return t;};
	string get_name();
	const namespace_context& get_namespace() const;
	void print();
	bool operator==(const variable & other);
	~variable();
	bool is_extern(){return qt & EXTERN;};
	bool is_volatile(){return qt & VOLATILE;};
	bool is_static(){return qt & STATIC;};
	bool is_register(){return qt & REGISTER;};
	bool is_constant(){return qt & CONSTANT;};
	bool is_private(){return qt & PRIVATE;};
	bool is_protected(){return qt & PROTECTED;};
	bool is_public(){return qt & PUBLIC;};
};

}

#endif /*VARIABLE_H_*/
