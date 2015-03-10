#ifndef _ASSERT_INST_PROCESSING_H
#define _ASSERT_INST_PROCESSING_H
#include "sail.h"
//#include "StaticAssertElement.h"
#include "MemGraph.h"
#include "InsMemAnalysis.h"
#include <assert.h>

class StaticAssertElement;
class StaticAssertBuilder;
class target;

class AssertInstProcessing{
private:
	sail::Instruction* ins;

	StaticAssertElement* sae;


public:
	AssertInstProcessing(sail::Instruction* ins, StaticAssertElement* sae);
	~AssertInstProcessing(){}



	StaticAssertElement* get_assert_ele();
	void process_static_assert_ins(bool entry_block,
				Constraints* con,
				TwoBranchConstraints*& two_branch_cons,
				sail::Symbol*& then_branch,
				sail::Symbol*& else_branch,
				target& t);

private:
	void update_check_var_for_assignment(
			target& original_var_t,
			sail::Constant*& c,
			sail::Assignment* assi);

	void non_check_var_collect_rhs_var_const(
			sail::Constant*& c,
			sail::Assignment* assi);

	void check_original_value_set(sail::Symbol* check_var,
			target& t);

	void collect_value_set_for_rhs_operands(sail::Binop* ins,
			TwoBranchConstraints*& two_branch_cons,
			Constraints* con,
			target& orig_check_var_t);
};
#endif
