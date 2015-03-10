/*
 * FuncInfo.h
 *
 *  Created on: Nov 27, 2012
 *      Author: boyang
 */

#ifndef CONSTRENAMING_H_
#define CONSTRENAMING_H_


#include <map>
#include <set>
#include <vector>
#include "Instruction.h"
#include "Assignment.h"
#include "Binop.h"
#include "Constraint.h"

namespace sail{
	class BasicBlock;
	class Block;
	class Variable;
	class Function;
	class Constant;
}

using namespace std;
using namespace sail;

class ConstRenaming {

private:

	map<int , Variable*> int_var_map;

	int name_counter;

	int temp_name_counter;

	Constraint knowlege;

	string var_prefix;



	/*
	 * fill constant info of the block into the map
	 */
	void Fill_map_by_block(BasicBlock* bb_cur);

	/*
	 *	rename constants
	 *	if instruction is assignment or logic binop
	 */
	void cons_inst_renaming(BasicBlock* bb_cur,
			Instruction* inst_cur,  Instruction* preInst);


	/*
	 * rename constants of logic binop.
	 */
	void cons_of_binop_renaming(BasicBlock* bb_cur, Binop* binop,
			Instruction* preInst);

	/*
	 * compute constant assignment constraint
	 */
	void compute_knowlege();


	/*
	 * insert a constant into the map and give a unique variable
	 * do nothing if the constant is already in the map.
	 */
	void insert_to_map(int i_insert);


	/*
	 * getting variable of the giving constant.
	 */
	Variable* get_variable_of_con(int i_insert, int& i_case);


	void assignment_renaming(BasicBlock* bb_cur, Variable* v_cn,
				Assignment* inst_assignment, int i_case);


	void binop_renaming_rhs1(BasicBlock* bb_cur, Variable* v_cn,
			Binop* inst_binop, int i_case);

	void binop_renaming_rhs2(BasicBlock* bb_cur, Variable* v_cn,
			Binop* inst_binop, int i_case);

	/*
	 * rename the variable name to make it unique
	 */
	void rename_unique_temp(Variable* var);

public:

	ConstRenaming();

	ConstRenaming(Function* f);

	~ConstRenaming();

	/*
	 * print out the map int_var_map
	 */
	void print_map();

	Constraint get_knowledge();


};

#endif /* CONSTRENAMING_H_ */
