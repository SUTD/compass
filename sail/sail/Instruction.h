/*
 * Instruction.h
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#ifndef SAIL_INSTRUCTION_H_
#define SAIL_INSTRUCTION_H_

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/set.hpp>

#include <set>
#include <string>
#include <sstream>
#include <BasicBlock.h>
using namespace std;

#define SET_INS_TO_BLOCK  true
#define SET_REACHABLE true

namespace il{class node;}

namespace sail {

class Variable;
class Symbol;

enum instruction_type
{
	ADDRESS_LABEL,
	ADDRESS_VAR,
	ADDRESS_STRING,
	ARRAY_REF_READ,
	ARRAY_REF_WRITE,
	SAIL_ASSEMBLY,
	ASSIGNMENT,
	BINOP,
	BRANCH,
	CAST,
	FIELD_REF_READ,
	FIELD_REF_WRITE,
	FUNCTION_CALL,
	FUNCTION_POINTER_CALL,
	JUMP,
	SAIL_LABEL,
	LOAD,
	STORE,
	UNOP,
	LOOP_INVOCATION,
	DROP_TEMPORARY,
	STATIC_ASSERT,
	ASSUME,
	ASSUME_SIZE,
	INSTANCEOF,
	EXCEPTION_RETURN
};

/**
 * \brief All low-level instructions defined by SAIL inherit from
 * Instruction.
 */

class Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & original;
        ar & inst_id;
        ar & line;

        //added by haiyan;
        ar & block;
        ar & inside_static_assert;
        ar & reachable;
        ar & sum_for_alloc;
        ar & return_inst;
        //ended for haiyan;
    }
protected:
	il::node* original;
	instruction_type inst_id;

	//added by haiyan
	//not critical to serialize;
	BasicBlock* block;
	bool inside_static_assert;
	//used to detect if the instruction is reachable
	bool reachable;
	//used to detect if summarized instruction is
	//because of introduced alloc location;
	bool sum_for_alloc;
public:
	int line;
	bool return_inst;

public:
	Instruction();
	virtual string to_string() const = 0;
	virtual string to_string(bool pretty_print) const = 0;

	/**
	 * This function provides the mapping between the low and the high-level
	 * representations.
	 * @return the corresponding expression or statement in the higher-level
	 * language that this instruction was generated from.
	 */
	il::node* get_original_node();

	virtual bool is_save_instruction();

	/**
	 *
	 * @return What type of instruction of this? (ASSIGNMENT, BRANCH,
	 * LOAD etc.)
	 */
	virtual instruction_type get_instruction_id();


	/**
	 * An instruction is removable if it's not present in the original
	 * source code, but introduced in the translation from the
	 * high-level language to the low-level language. For instance,
	 * if **x is an expression used in the original code,
	 * the low-level language will introduce a temporary t=*x; such
	 * instructions are "removable" for printing purposes.
	 */
	virtual bool is_removable() = 0;


	virtual ~Instruction();

	/*
	 * Gives the symbols used in the rhs of an instruction.
	 */
	void get_rhs_symbols(set<Symbol*>& syms);

	/*********
	 * hyz added, care about the order;
	 */
	void get_rhs_symbols(vector<Symbol*>& syms);

	/*******
	 * get in_use symbols(which not defined by the instruction), not the same as get_rhs_symbols(),
	 * because "store" have 1 rhs, but have 2 in_use symbols
	 */
	void get_in_use_symbols(vector<Symbol*>& syms);

	sail::Variable* get_lhs();

	void set_lhs(Variable* lhs);

/**********
 * given new_rhs (replace it with its clone), use to replace the corresponding orig with new_rhs;
 */
	bool replace_in_use_symbol_name(const string& orig_name,
			const string& new_name);

	/*
	 * Gives the symbols used in the rhs of an instruction.
	 */
	void get_symbols(set<Symbol*>& syms);


	//haiyan added 8.26
	int get_line_number(){return line;}
	void set_line_number(int line){this->line = line;}
	//haiyan ended 8.26

	//haiyan added 4.12.2013
	void set_inside_basic_block(BasicBlock* b)
	{
		this->block = b;
	}

	void set_as_unreachable()
	{
		reachable = false;
	}

	bool is_reachable()
	{
		return reachable;
	}

	void set_inside_static_assert()
	{
		inside_static_assert = true;
	}

	bool is_inside_static_assert()
	{
		return inside_static_assert;
	}

	BasicBlock*& get_basic_block()
	{
		return block;
	}


	void set_as_return_instruction()
	{

		return_inst = true;
	}

	//haiyan added 7/27/2013
	//used in type-inference phase
	bool is_return_inst() const
	{
		return this->return_inst;
	}

	void set_as_sum_inst_for_alloc()
	{
		this->sum_for_alloc = true;
	}

	const bool is_sum_inst_for_alloc()const
	{
		return this->sum_for_alloc;
	}

};

}

#endif /* SAIL_INSTRUCTION_H_ */
