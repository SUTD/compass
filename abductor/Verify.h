/*
 * Verify.h
 *
 *  Created on: May 23, 2012
 *      Author: boyang
 */

#ifndef VERIFY_H_
#define VERIFY_H_

#include"Function.h"
#include "SuperBlock.h"
#include "POE.h"
#include "LoopTree.h"
#include <queue>
#include <stack>
#include "ConstRenaming.h"

using namespace std;
using namespace sail;
using namespace il;

class Verify {


private:

	/*
	 * vector of all functions
	 */
	vector<Function *> func_vector;

	/*
	 * the entry function
	 */
	Function *entry_func;

	/*
	 * constant renaming information
	 */
	ConstRenaming* entry_cr;

	POE* original_poe;

	int level;

	LoopTree loopTree;

	static int num_tries;


	bool do_verification_helper(POE* poe, set<SuperBlock*>& to_verify);

	/*
	 *  Case 1: strengthen the precondition
	 */
	bool strengthen_pre(POEKey* b_while, ProofObligation* po_while,
			map<POEKey* , map<int, Constraint*>* > & invariant_map);


	/*
	 * Case 2: showing invariant
	 */
	bool show_inv(POEKey* w_poekey, ProofObligation* po_while,
				Constraint body_wp, map<POEKey* , map<int, Constraint*>* > & invariant_map);


	/*
	 * Case 3: strengthen the invariant
	 */
	bool strengthen_inv(POEKey * w_poekey, ProofObligation* po_while,
			map<POEKey* , map<int, Constraint*>* > & invariant_map);


	/*
	 * get previous super blocks for the block sb_bottom
	 */
	void get_previous_sb(SuperBlock* sb_bottom, set<SuperBlock*> & set_sb);


	/*
	 * the abduce function.  (psi & f)->phi
	 * return psi
	 */
	Constraint abduce(Constraint f, Constraint phi, set<Constraint>& thita);


	/*
	 * interface for querying the client
	 */
	bool query_client(SuperBlock* b_while, ProofObligation* po_while);


	/*
	 * pretty_print the block b_p.
	 */
	void print_out_block(Block * b_p);

	Function * function_finder(string name);

	void computeLT(BasicBlock * b_entry);

	bool DirectoryExists( const char* pzPath);

	Constraint get_unrolled_psi(Constraint c,
			const set<map<VariableTerm*, VariableTerm*> >& renamings);


	double to_time(int ticks);

	/*
	 * new logic checking since we added constant knowledge
	 */
	bool implies(const Constraint& c1, const Constraint& c2);
	bool unsat(const Constraint& c);
	bool valid(const Constraint& c);

public:

	//Verify(Function * f);

	Verify(Function* func, POE* original_poe, vector<Function *> func_vector ,
					int level, LoopTree& loopTree, ConstRenaming*  entry_cr);

	Verify(char* folder_name, char* entry_name);

	~Verify();

	/*
	 * Start to do the verification. list_loop_id lists loop should be verified.
	 */
	//bool do_verification(list<POEKey*>& list_loop_id, map<POEKey*,
	//			Constraint*>& invariant_map);
	bool do_verification(list<POEKey*>& list_loop_id,
			map<POEKey* , map<int, Constraint*>* > & invariant_map);


	void get_previous_sb_id(POEKey* sb_bottom, list<POEKey*>& list_sb_id);


	SuperBlock* find_superblock(int block_id , BasicBlock* func_entry);

	void getLoopTree(LoopTree& reLT);

	int get_num_tries();

	int get_init_po_time();
	int get_loop_unroll_time();

};

#endif /* VERIFY_H_ */
