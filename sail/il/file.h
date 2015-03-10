#ifndef FILE_H_
#define FILE_H_


#include "node.h"
#include <string>


namespace il
{

class function_declaration;
class declaration;
class assembly;
/**
 * \brief File representation
 */
class file:public node
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<il::node>(*this);
        ar & file_name;
        ar & declarations;
        ar & init_func;
        ar & glob_asm;
    }
private:
	/*
	 * Full name & path of source file
	 */
	string file_name;

	/*
	 * All declarations in this file
	 */
	vector<declaration*> declarations;

	/*
	 * Initialization function
	 */
	function_declaration* init_func;

	/*
	 * Global ASM statement
	 */
	assembly* glob_asm;



public:
	file();
	file(string file_name);
	/*
	 * Takes ownership of declaration
	 */
	void add_declaration(declaration * decl);
	void set_initfunc(function_declaration* init_func);
	void set_globasm(assembly* glob_asm);

	/**
	 *
	 * @return All the declarations in this file
	 */
	vector<declaration*> & get_declarations();

	/**
	 *
	 * @return An initializer function that may be invoked to initialize
	 * globals
	 */
	function_declaration * get_initfunc();

	/**
	 * @return Global asm directives
	 */
	assembly *get_globasm();
	virtual string to_string() const;
	string get_name();

	virtual ~file();
	virtual void print();
};

}

#endif /*FILE_H_*/
