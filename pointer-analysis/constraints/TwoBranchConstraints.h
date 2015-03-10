#ifndef _TWO_BRANCH_CONSTRAINTS_H
#define _TWO_BRANCH_CONSTRAINTS_H
#include "Constraints.h"

class TwoBranchConstraints {
private:
	Constraints* then_branch_constraints;
	Constraints* else_branch_constraints;
public:
	TwoBranchConstraints();
	~TwoBranchConstraints();
	Constraints* get_then_branch_constraints();
	Constraints* get_else_branch_constraints();
	void add_to_then_branch(ConstraintItem* item);
	void add_to_else_branch(ConstraintItem* item);
	bool then_branch_has_constraints();
	bool else_branch_has_constraints();
};
#endif
