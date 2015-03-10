#ifndef _ASSERT_PROCESSOR_H
#define _ASSERT_PROCESSOR_H

#define STATIC_ASSERT_BEGIN "compass_static_assert_start"
#include "StaticAssertElement.h"
#include <map>
#include <vector>
#include "InsMemAnalysis.h"
#include "sail.h"

using namespace std;
class StaticAssertElement;
class GlobalAssert{

	friend class StaticAssertBuilder;
	friend class InsMemAnalysis;

private:
	map<sail::FunctionCall*, StaticAssertElement*>* _static_assert_map;

	vector<StaticAssertElement*>* _static_assert_list;

	map<sail::Instruction*, StaticAssertElement*>* _inst_ele_map;
public:
	StaticAssertElement* get_inst_related_ele(sail::Instruction* inst);
	StaticAssertElement* get_static_assert_ele(sail::FunctionCall*);
private:

	GlobalAssert();
	~GlobalAssert();

	void check_must_before_static_ins(sail::Instruction* ins);

	map<sail::FunctionCall*, StaticAssertElement*>*& get_static_assert_ele_map();

	vector<StaticAssertElement*>*& get_static_assert_list() ;


	StaticAssertElement*& get_lastest_assert_ele();

	void add_to_assert_ele_list(StaticAssertElement* ele);
	void add_to_assert_ele_map(sail::FunctionCall* fc, StaticAssertElement* a_e);
	void add_to_inst_ele_map(sail::Instruction* inst, StaticAssertElement* ele);

	void set_to_latest_assert_ele_end_ins(sail::StaticAssert* sa);


	void test_static_assert_boundary();

	void remove_from_static_assert_elements(StaticAssertElement* ele);
	bool is_static_assert_begin_hit(StaticAssertElement* ele);
	void check_and_set_begin_hit(sail::FunctionCall* begin);

};
#endif
