/*
 * TypeInference.h
 *
 *  Created on: Oct 3, 2012
 *      Author: boyang
 */

#ifndef TYPEINFERENCE_H_
#define TYPEINFERENCE_H_


#include "PreciseTypeInfer.h"
#include "ReachingDefInfer.h"
#include "LivenessInfer.h"
#include "BasicBlock.h"
#include "Function.h"
#include <map>
#include <time.h>
#include <stdio.h>
#include "Loop.h"

#define JAVA_LANG_OBJECT "Ljava/lang/Object;"

namespace sail {

class Key
{
public:
	Key(std::string s, type * t)
	{
		this->s = s;
		this->t = t;
	}
	string s;
	type*  t;

	bool operator<(const Key& k) const
	{
		int s_cmp = this->s.compare(k.s);
		if(s_cmp == 0)
		{
			string s_this = t->to_string();
			string s_other = k.t->to_string(); // change?
			s_cmp = s_this.compare(s_other);
			return s_cmp < 0;
		}
		return s_cmp < 0;

	}

	string to_string() const
	{
		string res;
		res += s + ";" + t->to_string();
		return res;
	}
};


class TypeInference {

private:

	/*
	 * The functions entry and exit blocks
	 */

	Function* f;
	BasicBlock* entry_block;
	BasicBlock* exit_block;
	BasicBlock* exception_block;


	//typedef pair<string, type *> Key;

	/*************
	 * for the purpose that first time do not renaming;
	 */
	//set<string> used_var_names;

	//map<Key, Variable*> mapvp;


	/*
	 * an oracle which can help us find the precise type.
	 */
	PreciseTypeInfer* pti;

	/*
	 * an oracle which can help us find the reaching definition.
	 */
	ReachingDefInfer* rdi;



	LivenessInfer* lni;

	bool is_changing;


public:

	TypeInference(Function* f , BasicBlock* entry_b,
			BasicBlock* exit_b,
			BasicBlock* exception_b);

	~TypeInference();


	/*
	 * using fixed-point algorithm to assign type
	 */
	void assign_new_type();


private:

	void basic_flow_analysis();
	void Liveness_reachingdef_analysis();
	void compute_miniReaching();
	void compute_renaming();
	void update_livessness();
	void precise_type_analysis(const bool& isFirst);
	bool type_infer_analysis(const bool& isFirst, const int& round);
	/*
	 * assign type
	 */
	bool assign_new_type_inst(Instruction* inst, bool isFirst, const int& round);





	void set_symbols_inst(Instruction* inst, vector<Symbol*>& vect_syms);



	//void naming_map_generate();
//	void naming_map_generate_inst(Instruction* inst);
	type* get_new_def_type_inst(Instruction* inst, const bool& isFirst);
	bool vect_syms_replace(vector<Symbol*>& vect_syms, Variable* var_ori,
			Variable* rename_var,bool ignore_first);

};


#if 0
/*
 * compute LUB of type 1 and type 2.
 */
type* compute_lub(type* type1, type* type2);
#endif

type* lub_base_base(base_type* b1, base_type* b2);

record_type* base_to_record(base_type* b);

void build_type_hierarchy_for_lib_record_type(record_type* rt);

record_type* lub_record_record(record_type* rt1, record_type* rt2);

type* lub_compute(type* t1, type* t2, int& indicate);


record_type* get_base(record_type* rt_cur);

bool alreadyInDerivatives(il::record_type* base, il::record_type* deriv);


/*
 * get variable which has been assigned by the instruction.
 * return NULL if nothing has been defined.
 */
Variable* get_defined_variable(Instruction* inst);

void get_symbols_inst(Instruction* inst, vector<Symbol*>& vect_syms);


//double to_time(int ticks);





}




#endif /* TYPEINFERENCE_H_ */
