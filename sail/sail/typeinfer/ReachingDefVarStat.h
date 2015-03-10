/*
 * ReachingDefVarStat.h
 *
 *  Created on: Sep 22, 2012
 *      Author: boyang
 */

#ifndef ReachingDefVarStat_H_
#define ReachingDefVarStat_H_
#include <set>

#include "Instruction.h"


namespace sail {


class ReachingDefVarStat {
private:

	/*
	 * the variable of ReachingDefVarStat
	 */
	Variable* var;

	/*
	 * A set of Instruction indicates the reaching definition state for current variable
	 */
	set<Instruction*> set_reaching_inst;



public:
	ReachingDefVarStat();

	ReachingDefVarStat(const ReachingDefVarStat& other);

	ReachingDefVarStat(Variable* var, Instruction* inst);

	~ReachingDefVarStat();


	/*
	 * get variable of the state.
	 */
	Variable* get_var();

	/*******
	 * get the set_of insts
	 */
	set<Instruction*>& get_set_reaching_inst(){
		return this->set_reaching_inst;
	}

	/*
	 * get a set of reaching instruction that defined current variable.
	 */
	void get_set_reaching(set<Instruction*>& set_reaching_inst);


	/*
	 * merge other with current state.
	 */
	void merge(ReachingDefVarStat* other);


	string to_string() const;


	/*
	 * Is inst in the current set?
	 */
	bool find_inst(Instruction* inst);


	/*
	 * return true if all elements are same.
	 */
	bool operator==(const ReachingDefVarStat &  other) const;
};

}

#endif /* ReachingDefVarStat_H_ */
