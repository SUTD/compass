#ifndef NODE_H_
#define NODE_H_

#include <string>
#include "location.h"

#include <iostream>
#include <assert.h>

enum node_type {
	ADDRESSOF_EXP, //0
	ARRAY_REF_EXP, //1
	ASSEMBLY, //2
	BINOP_EXP, //3
	BLOCK, //4
	BLOCK_EXP, //5
	BREAK_STMT, //6
	BUILTIN_EXP, //7
	CASE_LABEL, //8
	CAST_EXP, //9
	COMPLEX_CONST_EXP, //10
	CONDITIONAL_EXP, //11
	CONTINUE_STMT, //12
	DEREF_EXP, //13
	DO_WHILE_LOOP, //14
	EXPR_LIST_EXP, //15
	EXPR_INSTRUCTION, //16
	FIELD_REF_EXP, //17
	FILE_NODE, //18
	FIXED_CONST_EXP, //19
	FOR_LOOP, //20
	FUNCTION_ADDRESS_EXP, //21
	FUNCTION_CALL_EXP, //22
	FUNCTION_DECL, //23
	FUNCTION_PTR_CALL_EXP, //24
	GOTO_STMT, //25
	IF_STMT, //26
	INITIALIZER_LIST_EXP, //27
	INTEGER_CONST_EXP, //28
	LABEL, //29
	MODIFY_EXP, //30
	NOOP_INSTRUCTION, //31
	REAL_CONST_EXP, //32
	RETURN_STMT, //33
	SET_INSTRUCTION, //34
	STRING_CONST_EXP, //35
	SWITCH_STMT, //36
	UNOP_EXP, //37
	VARIABLE_DECL, //38
	VARIABLE_EXP, //39
	VECTOR_CONST_EXP, //40
	WHILE_LOOP, //41
	TRY_CATCH_STMT, // 42
	TRY_FINALLY_STMT, // 43
	THROW_EXP // 44
};

/*
 * Abstract class representing a generic AST node
 */

#include "type.h"
namespace il
{
class type;

/**
 * \brief Represents a generic AST node, such as a statement, expression, declaration etc.
 */
class node
{
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & loc;
    	ar & node_type_id;
    }
protected:

	location loc;
public:
	short int node_type_id;
	node();
	virtual ~node();
	virtual string to_string() const {return "";};//= 0;
	virtual location get_location();
	bool is_expression();
	void assert_expression();
	virtual void print(){};

	node* substitute(node* (*sub_func)(node*));

};

}

#endif /*NODE_H_*/
