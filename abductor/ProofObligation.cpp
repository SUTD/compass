/*
 * ProofObligation.cpp
 *
 *  Created on: May 20, 2012
 *      Author: boyang
 */

#include "ProofObligation.h"
#include <assert.h>

using namespace std;
using namespace sail;
using namespace il;

//namespace coop {

ProofObligation::ProofObligation() {
	Constraint f(false);
	Constraint t(true);

	this->precondition = f;
	this->postcondition = t;
	this->known_inv = t;
	this->inv_to_show = t;
	this->while_condition = t;
	this->wp_before = t;
}


ProofObligation::ProofObligation(const ProofObligation& other)
{
	this->precondition = other.precondition;
	this->postcondition = other.postcondition;
	this->known_inv = other.known_inv;
	this->inv_to_show = other.inv_to_show;
	this->while_condition = other.while_condition;
	this->observed = other.observed;
	this->wp_before = other.wp_before;
	this->unrolled_sp = other.unrolled_sp;
	this->unroll_renaming = other.unroll_renaming;
}


void ProofObligation::add_constraint(Constraint c, bool is_forward){

	if(is_forward)
	{
		this->precondition = this->precondition|c;
		this->precondition.sat();
	}
	else
	{
		this->postcondition = this->postcondition&c;
		this->postcondition.sat();
	}
}


Constraint ProofObligation::getInv_to_show()
{
	return inv_to_show;
}

Constraint ProofObligation::getKnown_inv()
{
	return known_inv;
}

Constraint ProofObligation::getPostcondition()
{
	return postcondition;
}

Constraint ProofObligation::getPrecondition()
{
	return precondition;
}

Constraint ProofObligation::get_unrolled_sp()
{
	return this->unrolled_sp;
}

void ProofObligation::setInv_to_show(Constraint inv_to_show)
{
	this->inv_to_show = inv_to_show;
}

void ProofObligation::setKnown_inv(Constraint known_inv)
{
	this->known_inv = known_inv;
}

void ProofObligation::setPostcondition(Constraint postcondition)
{
	this->postcondition = postcondition;
}

void ProofObligation::setPrecondition(Constraint precondition)
{
	this->precondition = precondition;
}

void ProofObligation::set_unrolled_sp(Constraint unrolled_sp)
{
	this->unrolled_sp = unrolled_sp;
}


Constraint ProofObligation::getWhile_condition()
{
	return while_condition;
}

void ProofObligation::setWhile_condition(Constraint while_condition)
{
	this->while_condition = while_condition;
}


Constraint ProofObligation::getWp_before()
{
	return wp_before;
}


void ProofObligation::setWp_before(Constraint wp_before)
{
	this->wp_before = wp_before;
}

Constraint ProofObligation::getSp_after()
{
	return sp_after;
}


void ProofObligation::setSp_after(Constraint sp_after)
{
	this->sp_after = sp_after;
}



void ProofObligation::getObserved(set<Constraint>& re_Observed)
{
	re_Observed = this->observed;
}

void ProofObligation::setObserved(set<Constraint> observed)
{
	this->observed = observed;
}

void ProofObligation::add_unroll_renaming(
		map<VariableTerm*, VariableTerm*>  & val)
{
	this->unroll_renaming.insert(val);
}


const set<map<VariableTerm*, VariableTerm*> >&
	ProofObligation::get_unroll_renamings()
{
	return this->unroll_renaming;
}


