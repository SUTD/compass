#ifndef _STATIC_ASSERT_ELE_H
#define _STATIC_ASSERT_ELE_H
#include "DexToSail.h"
#include "Auxiliary.h"
#include "TwoBranchConstraints.h"
#include "MemGraph.h"
#include "StaticAssertBuilder.h"
/********************************
 * the purpose of build check_var_assignement is because
 * we won't to process same [check_var = sth].
 * multiple times because this block has multiple entries
 *************************************/
struct check_var_assignment{

	sail::Instruction* first_assi;
	sail::Constant* first_target_value;

	sail::Instruction* second_assi;
	sail::Constant* second_target_value;
};



class target;
class MemGraph;
class AssertInstProcessing;
//class MtdInstsProcessor;
//class StaticAssertBuilder;


class StaticAssertElement {
	//friend class MemGraph;
	friend class GlobalAssert;

private:

	StaticAssertBuilder* a_b;

	int idx;
	sail::FunctionCall* begin;
	bool begin_hit;
	sail::StaticAssert* end;
	/**
	 * instruction list for the staticAssertElement which needs
	 * to be precisely processed.
	 */
	vector<sail::Instruction*>* between_insts;
	vector<int>* block_ids;
	sail::BasicBlock* start_bb;
	sail::BasicBlock* end_bb;
	set<sail::BasicBlock*>* must_before_bbs;

	check_var_assignment assi;


public:

	//StaticAssertElement(int idx);

	StaticAssertElement(int idx, StaticAssertBuilder* ab);

	~StaticAssertElement();

	StaticAssertBuilder* get_static_assert_builder(){
		return this->a_b;
	}



	sail::FunctionCall* get_begin() const;
	sail::StaticAssert* get_end() const;
	bool get_begin_hit() const;
	void set_block_ids(vector<int>*& b_ids);
	vector<int>* get_block_ids()const;
	vector<sail::Instruction*>*& get_between_insts();

	void add_instruction(sail::Instruction* inst);
	void add_to_inst_map(unsigned int, sail::Instruction* inst);
	bool inside_between_inst();

	int get_last_block_id();
	int get_first_block_id();

	inline unsigned int get_blockids_size() const{
		assert(block_ids != NULL);
		return block_ids->size();
	}

	inline sail::Symbol* get_check_variable() const{
		return end->get_predicate();
	}

	inline void set_start_block(sail::BasicBlock* bb){
		this->start_bb = bb;
	}
	inline void set_end_block(sail::BasicBlock* bb){
		this->end_bb = bb;
	}

	inline sail::BasicBlock* get_start_block(){
		assert(this->start_bb != NULL);
		return this->start_bb;
	}

	inline sail::BasicBlock* get_end_block(){
		assert(this->end_bb != NULL);
		return this->end_bb;
	}
	inline void set_as_must_before_bbs(set<sail::BasicBlock*>*& bb_ids){
		this->must_before_bbs = bb_ids;
	}

	inline set<sail::BasicBlock*>*& get_must_before_bbs(){
		return this->must_before_bbs;
	}

	inline void delete_from_must_bbs(sail::BasicBlock* bb){
		this->must_before_bbs->erase(bb);
	}

	inline bool has_must_before_bbs_left(){
		return (this->must_before_bbs->size() != 0);
	}

	/**
	 * delete the must before basic blocks after processing the element
	 */
	void delete_must_before_bbs();

	void build_must_before_bbs ();
	void collect_own_block_ids ();
	string must_before_bb_id_print() const;
	string to_string() const;
	/************************
	 **** cfg order matters processing the value of static_assert_check_variable
	 **** where original_check_var_t tracking the changes of check_var;
	 ****************************/
	void process_static_assert_block(sail::BasicBlock* bb,
			Constraints* con,	target& original_check_var_t,
			set<sail::Constant*>& final_check_var_set,
			bool entry_block_flag);
	void static_assert_insts_processing();
	void set_begin(sail::FunctionCall* f);
	bool find_check_var_assignement(sail::Assignment*);
	void collect_check_var_assignement(sail::Assignment*, sail::Constant* target_value);
	sail::Constant* get_check_var_assignment_target_value(sail::Assignment* assi);
	void disable_begin_hit(){begin_hit = false;}
	int get_idx(){return this->idx;}
	void build_ele_in_main_graph(
			set<sail::Constant*>& final_check_const_set);

private:

	void set_end(sail::StaticAssert* end);
	void set_begin_hit();
	void test_static_assert_boundary();
	void collect_must_before_block_ids_for_static_assert_ele(
				sail::Block* block,
				set<sail::BasicBlock*>*& bbs);
	void build_connect_blockids_from_begin_end_ins(
				vector<int>& block_ids);
};
#endif
