/*
 * ProofObligation.h
 *
 *  Created on: May 20, 2012
 *      Author: boyang
 */

#ifndef PROOFOBLIGATION_H_
#define PROOFOBLIGATION_H_

#include "Constraint.h"


/**
 * \brief A label represented by the string label_name
 */
class ProofObligation {

private:

	Constraint precondition;	//P
	Constraint postcondition;	//Q
	Constraint inv_to_show;		//R
	Constraint known_inv;		//I
	Constraint while_condition;	//C
	Constraint wp_before;		//wp right before the block
	Constraint sp_after;  		//sp after the block;
	set<Constraint> observed;
	/*
	 * Formula representing k unrollings of the loop body starting from
	 * precondition
	 */
	Constraint unrolled_sp;
	set<map<VariableTerm*, VariableTerm*> > unroll_renaming;


public:
	ProofObligation();

	ProofObligation(const ProofObligation& other);


	/*
	 * update information
	 */
	void add_constraint(Constraint c, bool is_forward);


	Constraint getInv_to_show();
	Constraint getKnown_inv();
	Constraint getPostcondition();
	Constraint getPrecondition();
	Constraint get_unrolled_sp();

	const set<map<VariableTerm*, VariableTerm*> >& get_unroll_renamings();

	void setInv_to_show(Constraint inv_to_show);
	void setKnown_inv(Constraint known_inv);
	void setPostcondition(Constraint postcondition);
	void setPrecondition(Constraint precondition);
	void set_unrolled_sp(Constraint unrolled_sp);
	void add_unroll_renaming(map<VariableTerm*, VariableTerm*> & val);


	bool getis_super_block();
	void setis_super_block(bool is_super_block);


	Constraint getWhile_condition();
	void setWhile_condition(Constraint while_condition);

	Constraint getWp_before();
	void setWp_before(Constraint wp_before);

	void getObserved(set<Constraint>& re_Observed);
	void setObserved(set<Constraint> observed);


	Constraint getSp_after();
	void setSp_after(Constraint sp_after);

};



#endif /* PROOFOBLIGATION_H_ */
