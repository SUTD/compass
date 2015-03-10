#include "StaticAssertBuilder.h"
#include "MtdInstsProcessor.h"

////\\\\GlobalAssert* StaticAssertBuilder::g_a = NULL;

////\\\\bool StaticAssertBuilder::_has_static_assert = false;

////\\\\bool StaticAssertBuilder::_collect_static_assert_inst = false;

//StaticAssertBuilder::StaticAssertBuilder(sail::Instruction* ins) {
//	this->ins = ins;
//	count = 0;
//	///////
//	///////added by haiyan
//	_has_static_assert = false;
//	_collect_static_assert_inst = false;
//
//}

StaticAssertBuilder::StaticAssertBuilder(MtdInstsProcessor* mp, sail::Instruction* ins){

	this->ins = ins;
		count = 0;
		///////
		///////added by haiyan
		_has_static_assert = false;
		_collect_static_assert_inst = false;

		this->mp = mp;

		g_a = NULL;

}

StaticAssertBuilder::~StaticAssertBuilder() {

}

void StaticAssertBuilder::set_instruction(sail::Instruction* ins) {
	assert(ins != NULL);

	this->ins = ins;
}

GlobalAssert* StaticAssertBuilder::get_global_assert() {

	return g_a;
}

vector<StaticAssertElement*>*& StaticAssertBuilder::get_static_assert_list() {

	return g_a->get_static_assert_list();
}

map<sail::FunctionCall*, StaticAssertElement*>*& StaticAssertBuilder::get_static_assert_ele_map() {
	return g_a->get_static_assert_ele_map();
}

void StaticAssertBuilder::collect_static_assert_boundary() {
	if (!(collect_static_assert_start_enable())) {
		assert_inst_start_check();
	} else {
		assert_inst_end_check();
	}
}

bool StaticAssertBuilder::has_static_assert() {
	return _has_static_assert;
}

void StaticAssertBuilder::test_static_assert_boundary() {
	if (_has_static_assert) {
		g_a->test_static_assert_boundary();
	}
}

void StaticAssertBuilder::process_static_assert_element(
		StaticAssertElement* ele) {
	cout << "---------------------- <<< processing must "
			<< "before bb insts >>>------------------" << endl;
	process_must_before_bbs_ins(ele);
	cout << "------------------------<<< end processing "
			<< "must before bb insts >>> ----------------" << endl;

	cout << " ---------------------  <<<< before static_assert_insts_processing"
			<< " >>>>> ----------------  " << endl;
	cout << ele->to_string() << endl;
	ele->static_assert_insts_processing();
	cout << " ---------------------  <<<< end static_assert_insts_processing "
			<< ">>>>> ----------------  " << endl;
	remove_from_static_assert_elements(ele);
}

void StaticAssertBuilder::remove_from_static_assert_elements(
		StaticAssertElement* ele) {
	g_a->remove_from_static_assert_elements(ele);
}

bool StaticAssertBuilder::collect_static_assert_start_enable() {

	return _collect_static_assert_inst;
}

void StaticAssertBuilder::assert_inst_start_check() {

	if (ins->get_instruction_id() == sail::FUNCTION_CALL) {

		sail::FunctionCall* fun_call =
				static_cast<sail::FunctionCall*>(this->ins);

		if (fun_call->get_function_name().find(STATIC_ASSERT_BEGIN)
				!= string::npos) {

			enable_collect_static_assert_start(); //set flag for
			build_element_static_assert(true);
		}
	}
}

void StaticAssertBuilder::assert_inst_end_check() {

	assert(this->_has_static_assert);

	if (ins->get_instruction_id() == sail::STATIC_ASSERT) {

		build_element_static_assert(false);
		disable_collect_static_assert_inst();
	}

}

void StaticAssertBuilder::enable_collect_static_assert_start() {

	_collect_static_assert_inst = true;

	if (!_has_static_assert) {
		_has_static_assert = true;
		//when it has assert, set the InsMemAnalysis field
		g_a = new GlobalAssert();

		////\\\\InsMemAnalysis::set_assert_processor(g_a);

		////\\\\AssertInstProcessing::set_global_assert(g_a);
	}
}

void StaticAssertBuilder::disable_collect_static_assert_inst() {

	this->_collect_static_assert_inst = false;
}

//based on instruction order, now we build up StaticAssertProcessor
void StaticAssertBuilder::build_element_static_assert(bool begin_flag) {
	if (begin_flag) {

		////\\\\StaticAssertElement* ele = new StaticAssertElement(this->count++);
		StaticAssertElement* ele = new StaticAssertElement(this->count++, this);
		assert(ins->get_instruction_id() == sail::FUNCTION_CALL);
		ele->set_begin((sail::FunctionCall*) ins);
		assert(g_a != NULL);
		g_a->add_to_assert_ele_list(ele);
		g_a->add_to_assert_ele_map(ele->get_begin(), ele);

	} else {

		assert(ins->get_instruction_id() == sail::STATIC_ASSERT);
		g_a->set_to_latest_assert_ele_end_ins((sail::StaticAssert*) ins);
	}
}

void StaticAssertBuilder::process_must_before_bbs_ins(
		StaticAssertElement* ele) {
	cout
			<< "processing must before bbs ins for static assert elements whose begin block ins are:: "
			<< ele->get_start_block()->to_string() << endl;
	bool flag = ele->has_must_before_bbs_left();
	if (flag) {
		vector<StaticAssertElement*>* s_a_eles = this->get_static_assert_list();
		assert(s_a_eles != NULL);

		set<sail::BasicBlock*>* bbs = ele->get_must_before_bbs();
		cout << ele->must_before_bb_id_print() << endl;
		set<sail::BasicBlock*>::iterator it_bb = bbs->begin();
		while (it_bb != bbs->end()) {
			sail::BasicBlock* cur_bb = *it_bb;
			assert(cur_bb != NULL);
			vector<sail::Instruction*> insts = cur_bb->get_statements();
			for (unsigned int i = 0; i < insts.size(); i++) {
				sail::Instruction* ins = insts.at(i);

				if (ins->is_reachable()) {
					cout << "Inside bb " << cur_bb->get_block_id() << endl
							<< " +=++++=+=+=+++ must before instruction processing "
									"**************  " << ins->to_string()
							<< endl;

					////\\\\InsMemAnalysis ima(ins);
					InsMemAnalysis ima(ins, this->mp);
					//ima.inprecise_ins_analysis(false);
					ima.inprecise_ins_analysis(BASIC);
				}
			}

			//updating the must_before_bbs for each static_assert_element
			for (vector<StaticAssertElement*>::iterator it_ele =
					s_a_eles->begin(); it_ele != s_a_eles->end(); it_ele++) {
				set<sail::BasicBlock*>* must_bb =
						(*it_ele)->get_must_before_bbs();
				if (must_bb->find(*it_bb) != must_bb->end()) {
					(*it_ele)->delete_from_must_bbs(*it_bb);
				}
			}
			it_bb++;
		}

	}
}

void StaticAssertBuilder::collect_inst_for_static_assert_body(
		vector<sail::Instruction*>*& body) {
	vector<StaticAssertElement*>* static_eles =
			this->g_a->get_static_assert_list();

	for (unsigned int i = 0; i < body->size(); i++) {
		sail::Instruction* cur_inst = body->at(i);
		cout << "i == " << i << endl;
		if (cur_inst->get_instruction_id() == sail::FUNCTION_CALL) {
			sail::FunctionCall* fun_call =
					static_cast<sail::FunctionCall*>(cur_inst);
			if (fun_call->get_function_name().find(STATIC_ASSERT_BEGIN)
					!= string::npos) {
				cout << "encountering  " << fun_call->to_string()
						<< "  inside  block "
						<< fun_call->get_basic_block()->get_block_id() << endl;
				this->g_a->check_and_set_begin_hit(fun_call);
				fun_call->set_inside_static_assert();
			}
		} else { //otherwise to check if it resides in any connected part
			for (unsigned int j = 0; j < static_eles->size(); j++) {
				cout << endl << endl << endl << endl;
				StaticAssertElement* ele = static_eles->at(j);
				assert(ele != NULL);
				cout << "-------------------- ||||||||||||||| "
						<< "check if instruction :: " << cur_inst->to_string()
						<< " inside static_assert body ?  in "
						<< ele->to_string();
				if (inst_reachable_between_start_end(cur_inst, ele, i, body,
						0)) {
					cout << "YES !" << endl;
					ele->add_instruction(cur_inst);
					this->g_a->add_to_inst_ele_map(cur_inst, ele);
					cur_inst->set_inside_static_assert();
				}
			}
		}
	}
}

void StaticAssertBuilder::collect_blocks() {
	vector<StaticAssertElement*>* static_eles =
			this->g_a->get_static_assert_list();
	for (unsigned int i = 0; i < static_eles->size(); i++) {
		StaticAssertElement* ele = static_eles->at(i);
		assert(ele != NULL);
		ele->collect_own_block_ids();
		ele->build_must_before_bbs();
	}
}

void StaticAssertBuilder::buildup_static_assert_element(
		vector<sail::Instruction*>*& body) {
	collect_blocks();
	collect_inst_for_static_assert_body(body);
}

bool StaticAssertBuilder::inst_reachable_between_start_end(
		sail::Instruction* cur, StaticAssertElement* ele, unsigned int index,
		vector<sail::Instruction*>*& body, int level) {
	cout << "level is ==>" << level << endl;
	sail::BasicBlock* cur_block = cur->get_basic_block();
	if (cur->get_instruction_id() == sail::BRANCH) {
		//cout << "inside branch !!!" << endl;
		sail::Branch* b = static_cast<sail::Branch*>(cur);
		//check the goto branch
		sail::Label* then_l = b->get_then_label();
		int idx_then = get_index_of_label(body, then_l);
		assert(idx_then != -1);
		sail::Label* else_l = b->get_else_label();
		int idx_else = get_index_of_label(body, else_l);
		assert(idx_else != -1);
		bool flag_then = inst_reachable_between_start_end(then_l, ele, idx_then,
				body, level + 1);
		bool flag_else = inst_reachable_between_start_end(else_l, ele, idx_else,
				body, level + 1);
		if (flag_then && flag_else)
			return true;
	} else if (cur->get_instruction_id() == sail::JUMP) {
		cout << "inside JUMP!!! " << cur->to_string() << endl;
		sail::Jump* jump = static_cast<sail::Jump*>(cur);
		sail::Label* l = jump->get_label();
		int idx = get_index_of_label(body, l);
		assert(idx != -1);
		return inst_reachable_between_start_end(l, ele, idx, body, level + 1);
	} else if (cur->get_instruction_id() == sail::SAIL_LABEL) {
		cout << "inside label !!! " << cur->to_string() << endl;
		sail::Label* l = static_cast<sail::Label*>(cur);
		if (l->get_label_name() == "__return_label")
			return true;
		if (l->get_label_name() == "goto_520")
			cout << "instruction is " << body->at(index + 1)->to_string()
					<< endl;
		bool flag = inst_reachable_between_start_end(body->at(index + 1), ele,
				index + 1, body, level + 1);
		return flag;
	} else {
		assert(cur_block != NULL);
		cout << "non-boundary instruction  == " << cur->to_string() << endl;
		int cur_block_id = cur_block->get_block_id();
		if (1) {
			cout << endl << " cur block id " << cur_block_id << endl;
			cout << "start block id " << ele->get_first_block_id() << endl;
			cout << "end block id " << ele->get_last_block_id() << endl;
		}
		//in case when begin and end in the same block
		if (ele->get_first_block_id() == ele->get_last_block_id()) {
			if (cur_block_id == ele->get_first_block_id()) {
				if (ele->get_begin_hit()) {
					if (cur->get_instruction_id() == sail::STATIC_ASSERT)
						ele->disable_begin_hit();
					return true;
				}
			}
			return false;
		}

		if (ele->get_first_block_id() == cur_block_id) {
			if (level == 2) {
				cout << "jump to entry block " << endl;
				return false;
			}
			cout << ele->get_idx() << " ele begin hit " << ele->get_begin_hit()
					<< endl;
			if ((ele->get_begin_hit())
					&& (cur->get_instruction_id() != sail::FUNCTION_CALL)
					&& (cur->get_instruction_id() != sail::STATIC_ASSERT)) {
				cout << "check instruction  " << cur->to_string() << endl;
				cout << "hit inside the static_assert !!!  " << endl;
				return true;
			} else {
				return false;
			}
		} else {
			for (unsigned int i = 0; i < ele->get_blockids_size(); i++) {
				if (ele->get_block_ids()->at(i) != ele->get_first_block_id()) {
					if (cur_block_id == ele->get_last_block_id()) {
						if (cur->get_instruction_id() == sail::STATIC_ASSERT) {
							cout << "HERE ?  @" << endl;
							ele->disable_begin_hit();
							cout << ele->get_idx() << "  ele->begin hit "
									<< ele->get_begin_hit() << endl;
							return true;
						}
					} else {
						if (ele->get_block_ids()->at(i) == cur_block_id) {
							cout << "HERE ? @@ " << endl;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}
