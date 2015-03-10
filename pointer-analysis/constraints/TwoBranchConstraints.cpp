#include "TwoBranchConstraints.h"

TwoBranchConstraints::TwoBranchConstraints() {
	then_branch_constraints = new Constraints();
	else_branch_constraints = new Constraints();
}



Constraints* TwoBranchConstraints::get_then_branch_constraints() {
	return this->then_branch_constraints;
}



Constraints* TwoBranchConstraints::get_else_branch_constraints() {
	return this->else_branch_constraints;
}



void TwoBranchConstraints::add_to_then_branch(ConstraintItem* item) {
	this->then_branch_constraints->get_constraints_content().insert(item);
}



void TwoBranchConstraints::add_to_else_branch(ConstraintItem* item) {
	this->else_branch_constraints->get_constraints_content().insert(item);
}



bool TwoBranchConstraints::then_branch_has_constraints() {
	return this->then_branch_constraints->has_constraints();
}



bool TwoBranchConstraints::else_branch_has_constraints() {
	return this->else_branch_constraints->has_constraints();
}

