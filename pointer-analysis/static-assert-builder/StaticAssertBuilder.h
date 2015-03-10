#ifndef _STATIC_ASSERT_BUILDER_H
#define _STATIC_ASSERT_BUILDER_H
#include "sail.h"
#include "GlobalAssert.h"
#include "InsMemAnalysis.h"
#include "AssertInstProcessing.h"

class MtdInstsProcessor;

class StaticAssertBuilder {

private:
	sail::Instruction* ins; //important information

	////\\\\static GlobalAssert* g_a; //auto generated
	GlobalAssert* g_a;
	/**
	 * if static_assert_begin hit ?
	 */
	////\\\\static bool _collect_static_assert_inst;
	bool _collect_static_assert_inst;
	/***
	 * falg that indicate there is static_assert
	 */
	////\\\\static bool _has_static_assert;
	bool _has_static_assert;
	int count;

	MtdInstsProcessor* mp;


public:
	StaticAssertBuilder(MtdInstsProcessor* mp, sail::Instruction* ins = NULL);

	~StaticAssertBuilder();

	////\\\\static bool has_static_assert() ;
	bool has_static_assert();

	void collect_static_assert_boundary();
	void test_static_assert_boundary();

	////\\\\static GlobalAssert* get_global_assert();
	GlobalAssert* get_global_assert();

	MtdInstsProcessor* get_method_insts_processor(){
		return mp;
	}

	//is this necessary?
	void set_instruction(sail::Instruction* ins);
	vector<StaticAssertElement*>*& get_static_assert_list();
	map<sail::FunctionCall*, StaticAssertElement*>*& get_static_assert_ele_map();

	void buildup_static_assert_element(vector<sail::Instruction*>*& body);
	void process_static_assert_element(StaticAssertElement* ele);

private:
	bool collect_static_assert_start_enable();
	void assert_inst_start_check();
	void assert_inst_end_check();
	void enable_collect_static_assert_start();
	void disable_collect_static_assert_inst();
	void build_element_static_assert(bool begin_flag);
	void remove_from_static_assert_elements(StaticAssertElement* ele);
	void process_must_before_bbs_ins(StaticAssertElement* ele);

	//void collect_blockids();
	bool inst_reachable_between_start_end(sail::Instruction* cur,
			StaticAssertElement* ele, unsigned int index,
			vector<sail::Instruction*>*& body, int level);

	void collect_blocks();
	void collect_inst_for_static_assert_body(
				vector<sail::Instruction*>*& body);
};
#endif
