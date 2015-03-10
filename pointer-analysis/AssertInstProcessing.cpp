#include "AssertInstProcessing.h"
#include "StaticAssertElement.h"
#include "StaticAssertBuilder.h"

////\\\\GlobalAssert* AssertInstProcessing::g_a = NULL;

AssertInstProcessing::AssertInstProcessing(sail::Instruction* ins,
		StaticAssertElement* ele) {
	this->ins = ins;
	sae = ele;
}

StaticAssertElement* AssertInstProcessing::get_assert_ele() {
	return sae;
}

////\\\\void AssertInstProcessing::set_global_assert(GlobalAssert* ga) {
////\\\\	g_a = ga;
////\\\\}
////\\\\
////\\\\GlobalAssert* AssertInstProcessing::get_global_assert() {
////\\\\	return g_a;
////\\\\}

void AssertInstProcessing::process_static_assert_ins(bool entry_block,
		Constraints* con, TwoBranchConstraints*& two_branch_cons,
		sail::Symbol*& then_branch, sail::Symbol*& else_branch,
		target& original_var_t) {
	cout << "inside processing static_assert_ins@@@@@@@@@@@@@@" << endl;
	switch (ins->get_instruction_id()) {
	case sail::ASSIGNMENT: {
		print_inst(ins);
		sail::Assignment* assi = static_cast<sail::Assignment*>(ins);
		if (entry_block) { //imprecisely doing the analysis, updating graph for the main graph
			if (this->sae->get_first_block_id()
					== this->sae->get_last_block_id()) {
				cout << "entry and exit block are the same, "
						<< "but there are instructions between them, "
						<< "please do precisely analysis for it ! " << endl;
				assert(false);
			} else {
				////\\\\InsMemAnalysis ima(ins);
				if (ins->is_reachable()) {
					InsMemAnalysis ima(ins,
							this->sae->get_static_assert_builder()->get_method_insts_processor());
					//ima.inprecise_ins_analysis(false);
					ima.inprecise_ins_analysis(BASIC);
				}
			}
		} else { //not entry block
			sail::Constant* constant = NULL;
			if ((sail::Symbol*) assi->get_lhs()
					== this->sae->get_check_variable())
				update_check_var_for_assignment(original_var_t, constant, assi);
			else
				non_check_var_collect_rhs_var_const(constant, assi);

			//precisely update lhs
			long int target_value = constant->get_integer_value();
			if ((con != NULL) && (con->contain_symbol(assi->get_lhs()))) {
				set<ConstraintItem*> cons_item = con->get_constraints_content();
				for (set<ConstraintItem*>::iterator it = cons_item.begin();
						it != cons_item.end(); it++) {
					(*it)->set_value(assi->get_lhs(), target_value);
				}
			} else { //add to con
				assert(con != NULL);
				set<ConstraintItem*> cons_item = con->get_constraints_content();
				for (set<ConstraintItem*>::iterator it = cons_item.begin();
						it != cons_item.end(); it++) {
					constraint_ele* c_ele = new constraint_ele(assi->get_lhs(),
							target_value);
					(*it)->add_to_contraint(c_ele);
				}

			}
		}
	}
		break;
	case sail::LOAD: { //weak update
		print_inst(ins);
		if (ins->is_reachable()) {
			InsMemAnalysis ima(ins,
					sae->get_static_assert_builder()->get_method_insts_processor());
			//ima.inprecise_ins_analysis(false);
			ima.inprecise_ins_analysis(BASIC);
		}
	}
		break;
	case sail::BINOP: {
		print_inst(ins);
		sail::Binop* binop = (sail::Binop*) ins;
		il::binop_type bt = binop->get_binop();
		bool need_to_collect_branch_value = true;
		bool array = false;
		bool record = false;
		il::type* il_t =
				((sail::Variable*) binop->get_first_operand())->get_type();
		if (il_t->is_pointer_type()) {
			if (il_t->get_inner_type()->is_record_type()) { //record_type
				record = true;
			} else { //array
				array = true;
			}
		}

		if (entry_block) {
			if (comparable_bt(bt)) { // bt = plus && operand1 base type
				cout
						<< "collecting original value set because of the entry block!"
						<< endl;
				check_original_value_set(this->sae->get_check_variable(),
						original_var_t);
				cout << "size after collecting of constant::  "
						<< original_var_t.get_constants().size() << endl;

			} else { //may related two binop instruction on array [tempi = vi*4; tempj = vj + tempi(vj is array type)]
				need_to_collect_branch_value = false;

				if (ins->is_reachable()) {
					InsMemAnalysis ima(ins,
							sae->get_static_assert_builder()->get_method_insts_processor());
					//ima.inprecise_ins_analysis(false);
					ima.inprecise_ins_analysis(BASIC);
				}
			}
		} else { //not entry block
			if (!comparable_bt(bt)) {
				need_to_collect_branch_value = false;
				if (ins->is_reachable()) {
					InsMemAnalysis ima(ins,
							sae->get_static_assert_builder()->get_method_insts_processor());
					//ima.inprecise_ins_analysis(false);
					ima.inprecise_ins_analysis(BASIC);
				}
			}
		}

		if ((con != NULL) && (need_to_collect_branch_value)) {
			cout << endl
					<< " <<<< [[[ check condition before processing binop ]]] >>>> "
					<< ins->to_string() << endl;
			cout << con->to_string() << endl;
		}
		//tempi = vi == vj (next inst :: tempj != tempi)
		if (need_to_collect_branch_value) {
			cout << "ZZZ check inside here !!! " << endl;
			collect_value_set_for_rhs_operands((sail::Binop*) ins,
					two_branch_cons, con, original_var_t);
			then_branch = ((sail::Binop*) ins)->get_lhs();
		}
	}
		break;
	case sail::UNOP: { //tempj != tempi
		print_inst(ins);
		((sail::Unop*) ins)->get_unop();
		if (((sail::Unop*) ins)->get_unop() == il::_LOGICAL_NOT) {
			else_branch = ((sail::Unop*) ins)->get_lhs();
			return;
		}
		assert(false);
	}
		break;
	case sail::SAIL_LABEL: {
		print_inst(ins);
		assert(false);
		return;
	}
		break;
	case sail::JUMP: {
		print_inst(ins);
		assert(false);
		return;
	}
		break;
	case sail::BRANCH: {
		print_inst(ins);
		assert(false);
		return;
	}
		break;
	case sail::FUNCTION_CALL: { //do nothing
		print_inst(ins);
	}
		break;
	case sail::STATIC_ASSERT: { // when only has static_assert() inside the element instruction
		print_inst(ins);

		if (this->sae->get_first_block_id() == this->sae->get_last_block_id()) {
			check_original_value_set(this->sae->get_check_variable(),
					original_var_t);
		} else {
			cout << "do not processing the it!! " << endl;
		}
	}
		break;
	default: {
		print_inst(ins);
		assert(false);
	}
	}
}

void AssertInstProcessing::update_check_var_for_assignment(
		target& original_var_t, sail::Constant*& c, sail::Assignment* assi) {
	if (this->sae->find_check_var_assignement(assi)) {
		c = this->sae->get_check_var_assignment_target_value(assi);
	} else {
		if (assi->get_rhs()->is_constant())
			c = (sail::Constant*) assi->get_rhs();
		else {
			target rhs_t;
			check_original_value_set(assi->get_rhs(), rhs_t);
			assert(rhs_t.has_constant());
			assert(!rhs_t.has_other());
			assert(rhs_t.get_constants().size() == 1);
			c = *(rhs_t.get_constants().begin());
		}
		this->sae->collect_check_var_assignement(assi, c);
	}
	original_var_t.clear();
	original_var_t.get_constants().insert(c);
}

void AssertInstProcessing::non_check_var_collect_rhs_var_const(
		sail::Constant*& c, sail::Assignment* assi) {
	if (assi->get_rhs()->is_constant()) {
		c = (sail::Constant*) assi->get_rhs();
	} else {
		target rhs_var_t;
		check_original_value_set(assi->get_rhs(), rhs_var_t);
		assert( rhs_var_t.has_constant() && (!rhs_var_t.has_other()));
		assert(rhs_var_t.get_constants().size() == 1);
		c = *(rhs_var_t.get_constants().begin());
	}
}

void AssertInstProcessing::check_original_value_set(
		sail::Symbol* check_variable, target& target) {

	AccessPath* ap = get_ap_from_symbol(check_variable,
			sae->get_static_assert_builder()->get_method_insts_processor());

//	AccessPath* ap = AccessPath::get_ap_from_symbol(check_variable);
	MemNode* check_var_node =
			sae->get_static_assert_builder()->get_method_insts_processor()->get_mem_graph()->get_memory_node(
					ap);
	//cout << "~~~~~~~~~~~~~ 11111" << endl;
	check_var_node->collect_target_value(target);
	//cout << "^^^^^ value of check variable " << endl << target.to_string()
	//		<< endl;
}

void AssertInstProcessing::collect_value_set_for_rhs_operands(sail::Binop* ins,
		TwoBranchConstraints*& two_branch_cons, Constraints* con,
		target& original_var_t) {
	target oper1_t;
	target oper2_t;
	sail::Symbol* oper1_s = ((sail::Binop*) ins)->get_first_operand();
	if (oper1_s == this->sae->get_check_variable()) {
		oper1_t = original_var_t;
	} else {
		//AccessPath* oper1_ap = AccessPath::get_ap_from_symbol(oper1_s);
		AccessPath* oper1_ap = get_ap_from_symbol(oper1_s,
				sae->get_static_assert_builder()->get_method_insts_processor());
		assert(oper1_ap->get_ap_type() == AP_VARIABLE);
		////\\\\MemNode* oper1_node = InsMemAnalysis::get_primary_mg()->get_memory_node(
		////\\\\		oper1_ap);

		MemNode * oper1_node =
				sae->get_static_assert_builder()->get_method_insts_processor()->get_mem_graph()->get_memory_node(
						oper1_ap);
		//cout << "~~~~~~~~~~~~~ 2222222" << endl;
		oper1_node->collect_target_value(oper1_t);
	}

	sail::Symbol* oper2_s = ((sail::Binop*) ins)->get_second_operand();
	if (oper2_s == this->sae->get_check_variable()) {
		oper2_t = original_var_t;
	} else {
		//AccessPath* oper2_ap = AccessPath::get_ap_from_symbol(oper2_s);
		AccessPath* oper2_ap = get_ap_from_symbol(oper2_s,
				sae->get_static_assert_builder()->get_method_insts_processor());

		assert(oper2_ap->get_ap_type() == AP_VARIABLE);

		MemNode * oper2_node =
				sae->get_static_assert_builder()->get_method_insts_processor()->get_mem_graph()->get_memory_node(
						oper2_ap);
		//cout << "~~~~~~~~~~~~~ 3333333" << endl;
		oper2_node->collect_target_value(oper2_t);
	}

	il::binop_type bt = ins->get_binop();
	if (con == NULL) { //build the constraints
		adding_two_new_constraints(oper1_s, oper2_s, oper1_t, oper2_t, bt,
				two_branch_cons, true, NULL);
	} else { //already has constraints, need to process based on it
		update_existing_constraints(con, oper1_s, oper2_s, oper1_t, oper2_t, bt,
				two_branch_cons);
	}
}
