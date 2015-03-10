#include "GlobalAssert.h"
#include "StaticAssertElement.h"

GlobalAssert::GlobalAssert() {

	_static_assert_list = new vector<StaticAssertElement*>();
	_static_assert_map = new map<sail::FunctionCall*, StaticAssertElement*>();
	_inst_ele_map = new map<sail::Instruction*, StaticAssertElement*>();
}



GlobalAssert::~GlobalAssert() {

}



map<sail::FunctionCall*, StaticAssertElement*>*&
GlobalAssert::get_static_assert_ele_map() {

	return _static_assert_map;
}



vector<StaticAssertElement*>*& GlobalAssert::get_static_assert_list() {

	return this->_static_assert_list;
}


StaticAssertElement* GlobalAssert::get_static_assert_ele(
		sail::FunctionCall* begin) {

	assert(this->_static_assert_map != NULL);
	assert(this->_static_assert_map->count(begin) >0);
	return (*_static_assert_map)[begin];

}



void GlobalAssert::add_to_assert_ele_list(StaticAssertElement* ele) {
	assert(ele != NULL);
	_static_assert_list->push_back(ele);
}



StaticAssertElement*& GlobalAssert::get_lastest_assert_ele() {
	StaticAssertElement* ele = _static_assert_list->back();
	assert(ele != NULL);
	return ele;
}



void GlobalAssert::set_to_latest_assert_ele_end_ins(sail::StaticAssert* end) {
	assert(end != NULL);
	StaticAssertElement* ele = get_lastest_assert_ele();
	ele->set_end(end);
}



void GlobalAssert::add_to_assert_ele_map(sail::FunctionCall* fc,
		StaticAssertElement* a_e) {

	assert(_static_assert_map != NULL);
	assert(fc != NULL);
	assert(a_e != NULL);

	(*_static_assert_map)[fc] = a_e;
}



void GlobalAssert::add_to_inst_ele_map(sail::Instruction* inst,
		StaticAssertElement* ele) {

	assert(inst != NULL);
	assert(ele != NULL);
	(*_inst_ele_map)[inst] = ele;
}



StaticAssertElement* GlobalAssert::get_inst_related_ele(
		sail::Instruction* inst) {

	assert(inst != NULL);
	StaticAssertElement* ele = NULL;
	assert(_inst_ele_map != NULL);
	if (_inst_ele_map->count(inst) > 0) {

		ele = (*_inst_ele_map)[inst];
		assert(ele != NULL);
	}
	return ele;
}



void GlobalAssert::check_must_before_static_ins(sail::Instruction* ins) {

	sail::BasicBlock* bb = ins->get_basic_block();

	vector<StaticAssertElement*>::iterator it = _static_assert_list->begin();

	for (; it != _static_assert_list->end(); it++) {

		StaticAssertElement* ele = *it;

		set<sail::BasicBlock*>* must_before_bbs = ele->get_must_before_bbs();

		if (must_before_bbs != NULL) {

			if (must_before_bbs->find(bb) != must_before_bbs->end()) { //inside must before

				ele->delete_from_must_bbs(bb);
			}
		}
	}
}



void GlobalAssert::test_static_assert_boundary() {

	assert(_static_assert_list != NULL);
	assert(_static_assert_list->size() != 0);

	unsigned int i = 0;

	for (; i < _static_assert_list->size(); i++) {

		assert(_static_assert_list->at(i) != NULL);

		_static_assert_list->at(i)->test_static_assert_boundary();
	}
}



void GlobalAssert::check_and_set_begin_hit(sail::FunctionCall* begin) {

	assert(begin != NULL);
	assert((*_static_assert_map)[begin] != NULL);

	(*_static_assert_map)[begin]->set_begin_hit();
}



void GlobalAssert::remove_from_static_assert_elements(
		StaticAssertElement* ele) {
	assert(ele != NULL);
	assert(_static_assert_list != NULL);
	vector<StaticAssertElement*>::iterator find_iter =
			_static_assert_list->begin();

	vector<int>* block_ids = (*find_iter)->get_block_ids();

	if (_static_assert_list->size() > 1) {

		for (vector<StaticAssertElement*>::iterator it =
				_static_assert_list->begin(); it != _static_assert_list->end();
				it++) {

			if (it == _static_assert_list->begin()) {
				assert(*find_iter == ele);

			} else {

				set<sail::BasicBlock*>* must_before_bbs =
						(*it)->get_must_before_bbs();

				for (unsigned int i = 0; i < block_ids->size(); i++) {
					for (set<sail::BasicBlock*>::iterator it_bb =
							must_before_bbs->begin();
							it_bb != must_before_bbs->end(); it_bb++) {

						if ((*it_bb)->get_block_id() == block_ids->at(i)) { //inside it, then delete it

							cout << " %%%%%%%%%%%%% delete must before bb "
									<< (*it_bb)->to_string() << " for "
									<< (*it)->to_string() << endl;
							(*it)->delete_from_must_bbs(*it_bb);
						}
					}
				}
			}
		}

	}

	_static_assert_list->erase(find_iter);
}



bool GlobalAssert::is_static_assert_begin_hit(StaticAssertElement* ele) {
	return (ele->get_begin_hit());
}
