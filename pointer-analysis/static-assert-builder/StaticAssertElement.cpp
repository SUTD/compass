#include "StaticAssertElement.h"
#include "AssertInstProcessing.h"
#include "GlobalAssert.h"
//#include "MtdInstsProcessor.h"
#include "StaticAssertBuilder.h"



StaticAssertElement::StaticAssertElement(int index, StaticAssertBuilder* ab) {
	this->between_insts = new vector<sail::Instruction*>();
	this->block_ids = new vector<int>();
	begin_hit = false;
	this->idx = index;
	//mg = new MemGraph();
	assi.first_assi = NULL;
	assi.second_assi = NULL;

	////added by haiyan
	a_b = ab;
}



StaticAssertElement::~StaticAssertElement() {
	delete this->between_insts;
	delete this->block_ids;
}


vector<sail::Instruction*>*& StaticAssertElement::get_between_insts() {
	return this->between_insts;
}



sail::FunctionCall* StaticAssertElement::get_begin() const {
	return this->begin;
}



void StaticAssertElement::set_begin(sail::FunctionCall* begin) {
	this->begin = begin;
}



sail::StaticAssert* StaticAssertElement::get_end() const {
	return this->end;
}



void StaticAssertElement::set_end(sail::StaticAssert* end) {
	this->end = end;
}



void StaticAssertElement::set_begin_hit() {
	cout << "CHECK here set the begin hit as true++++++++++++++++++ " << endl;
	this->begin_hit = true;
}



bool StaticAssertElement::get_begin_hit() const {
	cout << "Begin hit ??  " << this->begin_hit << endl;
	return this->begin_hit;
}



void StaticAssertElement::set_block_ids(vector<int>*& b_ids) {
	this->block_ids = b_ids;
}




vector<int>* StaticAssertElement::get_block_ids() const {
	return this->block_ids;
}



void StaticAssertElement::test_static_assert_boundary() {
	cout << "the boundary begin inst is " << begin->to_string()
			<< "    & the boundary end inst is " << end->to_string() << endl;
}



void StaticAssertElement::add_instruction(sail::Instruction* inst) {
	this->between_insts->push_back(inst);
}




int StaticAssertElement::get_first_block_id() {
	return this->start_bb->get_block_id();
}




int StaticAssertElement::get_last_block_id() {
	return this->end_bb->get_block_id();
}





void StaticAssertElement::build_must_before_bbs() {
	set<sail::BasicBlock*>* must_before_bbs = new set<sail::BasicBlock*>();
	collect_must_before_block_ids_for_static_assert_ele(start_bb,
			must_before_bbs);
	set_as_must_before_bbs(must_before_bbs);
}






bool StaticAssertElement::find_check_var_assignement(sail::Assignment* assi) {
	return ((this->assi.first_assi == assi)
			|| (this->assi.second_assi == assi));
}




void StaticAssertElement::collect_check_var_assignement(sail::Assignment* assi, sail::Constant* target_value) {
	if(this->assi.first_assi == NULL){
		this->assi.first_assi = assi;
		this->assi.first_target_value = target_value;
	}else if(this->assi.second_assi == NULL){
		this->assi.second_assi = assi;
		this->assi.second_target_value = target_value;
	}
}




sail::Constant* StaticAssertElement::get_check_var_assignment_target_value(
		sail::Assignment* assi) {
	if (this->assi.first_assi == assi)
		return this->assi.first_target_value;
	else if (this->assi.second_assi == assi)
		return this->assi.second_target_value;
	else{
		assert(false);
		return 0;
	}
}





void StaticAssertElement::collect_must_before_block_ids_for_static_assert_ele(
		sail::Block* block, set<sail::BasicBlock*>*& bbs) {
	cout << "block id is " << block->get_block_id() << endl;
	set<sail::CfgEdge*> pred = block->get_predecessors();
	for (set<sail::CfgEdge*>::iterator it = pred.begin(); it != pred.end();
			it++) {
		sail::Block* cur_block = (*it)->get_source();
		bbs->insert((sail::BasicBlock*) cur_block);
		collect_must_before_block_ids_for_static_assert_ele(cur_block, bbs);
	}

}




void StaticAssertElement::collect_own_block_ids() {
	assert(begin != NULL);
	assert(end != NULL);
	vector<int>* block_ids = new vector<int>();
	build_connect_blockids_from_begin_end_ins(*block_ids);
	set_block_ids(block_ids);
}





void StaticAssertElement::build_connect_blockids_from_begin_end_ins(
		vector<int>& block_ids) {
	sail::BasicBlock* b_start = this->begin->get_basic_block();
	block_ids.push_back(b_start->get_block_id());
	sail::BasicBlock* b_end = this->end->get_basic_block();
	//assert(b_start != b_end);
	get_connect_blockids(b_start, b_end, block_ids);
	block_ids.push_back(b_end->get_block_id());
	set_start_block(b_start);
	set_end_block(b_end);
}





string StaticAssertElement::must_before_bb_id_print() const {
	string str = "must before blocks :: ";
	str += "\n";
	for (set<sail::BasicBlock*>::iterator it = must_before_bbs->begin();
			it != must_before_bbs->end(); it++) {
		str += int_to_string((*it)->get_block_id());
		str += "\n";
	}
	return str;
}





void StaticAssertElement::static_assert_insts_processing() {
	assert(between_insts != NULL);
	if (1) {
		cout << endl<<"<<<<<<<<<<<--------- << ==  "<< endl << "check the instructions before processing the assert blocks!"
				<< endl;
		for (unsigned int i = 0; i < between_insts->size(); i++) {
			cout << "INS  == " << between_insts->at(i)->to_string()
					<< "        [" << "ADDRESS :" << between_insts->at(i)
					<< " ]" << endl;
		}
		cout << endl << endl;
	}

	if (0) {
		il::namespace_context ns;
		vector<il::type*>* para_list = new vector<il::type*>();
		il::function_type* fn_signature = il::function_type::make(NULL,
				*para_list, false);
		//set<sail::Variable*> args;
		sail::Function* f = new sail::Function("", "", ns, fn_signature,
				between_insts, false, false, false, false /*, args*/);
		//cout << "static_assert_fun :: " << endl << f->to_string();
	}

	sail::BasicBlock* bb_entry = get_start_block();
	target original_check_const_set;
	set<sail::Constant*> final_check_const_set;
	process_static_assert_block(bb_entry, NULL, original_check_const_set,
			final_check_const_set, true);

	//check if the value of final_check_var_set
	if (1) {
		cout << "Testing the final result of static assert predicate !!!!"
				<< endl;
		if (final_check_const_set.size() == 0) {
			cout << "is EMPTY !! " << endl;
			assert(false);
		} else {
			for (set<sail::Constant*>::iterator it = final_check_const_set.begin();
					it != final_check_const_set.end(); it++) {
				cout << (*it)->to_string() << endl;
			}
		}
		cout << endl << endl;
	}
	assert(final_check_const_set.size() != 0);
	build_ele_in_main_graph(final_check_const_set);
}






void StaticAssertElement::build_ele_in_main_graph(
		set<sail::Constant*>& final_check_const_set) {
	set<sail::Assignment*> assert_assi;
	for (set<sail::Constant*>::iterator it = final_check_const_set.begin();
			it != final_check_const_set.end(); it++) {
		sail::Constant* cons = *it;
		sail::Variable* var =
				((sail::Variable*) this->end->get_predicate())->clone();
		var->set_name(
				var->get_var_name() + "(static_assert:" + int_to_string(idx)
						+ ")");

		il::location lo(-1, -1);
		il::string_const_exp* temp = new il::string_const_exp("",
				var->get_type(), lo);

		sail::Assignment* ass = new sail::Assignment(var, cons, temp,
				end->get_line_number());

		assert_assi.insert(ass);
	}

	for (set<sail::Assignment*>::iterator it = assert_assi.begin();
			it != assert_assi.end(); it++) {
		if ((*it)->is_reachable()) {
			InsMemAnalysis ima(*it, this->a_b->get_method_insts_processor());
			ima.inprecise_ins_analysis(BASIC);
		}
	}
}






void StaticAssertElement::process_static_assert_block(sail::BasicBlock* bb,
		Constraints* con, target& original_check_var_t,
		set<sail::Constant*>& final_check_var_set,
		bool entry_block_flag) {

	sail::BasicBlock* cur_bb = bb;
	int cur_bb_id = cur_bb->get_block_id();
	if (0) {
		cout << "cur block id " << cur_bb_id << endl;
		cout << "start block id " << this->get_first_block_id() << endl;
		cout << "last block id " << this->get_last_block_id() << endl;
	}

	TwoBranchConstraints* two_branch_cons = NULL;
	sail::Symbol* then_branch = NULL;
	sail::Symbol* else_branch = NULL;
	vector<sail::Instruction*> ins = cur_bb->get_statements();
	//those instructions are built for the static_assert function,
	//so the block ids are not the same as original block id;
	for (unsigned int u = 0; u < ins.size(); u++) {
		ins.at(u)->set_inside_basic_block(bb);
	}

	//only has one block
	if(get_first_block_id() == get_last_block_id()){
		for(unsigned int i =0; i < ins.size(); i++){
			sail::Instruction* cur_ins = ins.at(i);
			cout << "inst $$$$ :: " << cur_ins->to_string() << endl;

			//assert(AssertInstProcessing::get_global_assert() != NULL);
			assert(a_b->get_global_assert() != NULL);

//			if (AssertInstProcessing::get_global_assert()->get_inst_related_ele(
//					cur_ins) == this) { //if the instruction really inside the ele in_between ins
			if(a_b->get_global_assert()->get_inst_related_ele(cur_ins) == this){
				AssertInstProcessing aip(cur_ins, this);
				aip.process_static_assert_ins(true, con, two_branch_cons,
										then_branch, else_branch, original_check_var_t);
				if (cur_ins->get_instruction_id() == sail::STATIC_ASSERT)
					break;
			}
		}
		final_check_var_set.insert(*(original_check_var_t.get_constants().begin()));
		cout << "out of Path !! " << endl;
		return;
	}

	if ((get_first_block_id() != get_last_block_id()) &&(cur_bb_id == get_last_block_id())) {
		cout << "original check var is " << original_check_var_t.to_string() << endl;
		assert(original_check_var_t.has_constant());
		assert(!original_check_var_t.has_other());

		//assert(original_check_var_set.size() <= 1);
		cout << "inserting value is " << (*(original_check_var_t.get_constants().begin()))->get_integer_value()
				<< endl;
		//final_check_var_set.insert(*(original_check_var_set.begin()));
		final_check_var_set.insert(*(original_check_var_t.get_constants().begin()));
		cout << " out of one path !!! ------------------>>>>>><<<<<< " << endl
				<< endl << endl << endl << endl;
		return;
	} else {
		for (unsigned int i = 0; i < ins.size(); i++) {
			//if inside the begin_block and it may needs to jump
			sail::Instruction* cur_ins = ins.at(i);
			cout << "before processing == >>>> check original check_var value " << original_check_var_t.to_string() << endl;
			cout << "inst $$$$ :: " << cur_ins->to_string() << endl;
//			if( (cur_bb_id == get_first_block_id())&& (AssertInstProcessing::get_global_assert()->get_inst_related_ele(
//					cur_ins) != this))
			if( (cur_bb_id == get_first_block_id())&& (a_b->get_global_assert()->get_inst_related_ele(
									cur_ins) != this))
				continue;

			AssertInstProcessing aip(ins.at(i), this);
			aip.process_static_assert_ins(entry_block_flag, con, two_branch_cons, then_branch, else_branch, original_check_var_t);
			cout << "after processing == >> check original check_var value " << original_check_var_t.to_string() << endl;
			target else_original_check_var_t = original_check_var_t;

			if (i == (ins.size() - 1)) {
				if (ins.at(i)->get_instruction_id() == sail::ASSIGNMENT) { //no compare, just have an unique successor
					cout << "last instruction is assignment !! " << endl;
					assert(bb->get_successors().size() ==1);
					sail::Block* target_b =
							(*(bb->get_successors().begin()))->get_target();
					process_static_assert_block((sail::BasicBlock*) target_b,
							con, original_check_var_t, final_check_var_set,
							false);
				} else {
					map<sail::Symbol*, sail::CfgEdge*> m =
							bb->get_successors_map();
					assert(two_branch_cons != NULL);
					if (two_branch_cons->then_branch_has_constraints()) {
						Constraints* con_then =
								two_branch_cons->get_then_branch_constraints();
						cout << endl
								<< " ----------------- processing then branch is whose condition variable is :: "
								<< then_branch->to_string() << endl;
						//cout << "whose value set is " << dis_joint_value_set.first_set_to_string() << endl << endl;
						assert(m.count(then_branch) > 0);
						sail::Block* then_block = m[then_branch]->get_target();
						process_static_assert_block(
								(sail::BasicBlock*) then_block, con_then,
								original_check_var_t, final_check_var_set,
								false);
					}
					if (two_branch_cons->else_branch_has_constraints()) { //go to the next block
						assert(m.count(else_branch) > 0);
						sail::Block* else_block = m[else_branch]->get_target();
						cout << " ------------- processing else_block is whose condition variable is :: "
								<< else_branch->to_string() << endl;
						Constraints* con_else =
								two_branch_cons->get_else_branch_constraints();
						process_static_assert_block(
								(sail::BasicBlock*) else_block, con_else,
								else_original_check_var_t,
								final_check_var_set, false);
					}
				}
			}

		}
	}
}






string StaticAssertElement::to_string() const {
	string str;
	str += "Index :: " ;
	str += int_to_string(idx);
	str += "  -->";
	str += "begin is ::";
	str += this->begin->to_string();
	str += "\n";
	str += "end is ::";
	str += this->end->to_string();
	str += "\n";
	str += "in between instructions :: ";
	str += "\n";
	for (unsigned int i = 0; i < this->between_insts->size(); i++) {
		str += between_insts->at(i)->to_string();
		if (i != this->between_insts->size() - 1)
			str += "\n";
	}
	return str;
}
