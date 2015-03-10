#include "SummaryUnit.h"
#include "MemGraph.h"
#include "MemNode.h"
#include "CallManager.h"
#include "MtdInstsProcessor.h"
#include "AllocContextController.h"

#define DEBUG_INFO false
#define ARITHMETIC_NODE false

#define CHECK_ALLOC_NODE false
#define REMOVE_REDUNDANCY_MAX_SIZE 100

/***********************************
 * SummaryUnit class
 *******************************/

int SummaryUnit::temp_num = 0;

sail::Variable* SummaryUnit::get_summary_unit_var() {

	sail::Variable* var = NULL;
	string str;
	str = "sum_temp_" + int_to_string(temp_num++);
	//return str;
	var = new sail::Variable(str, il::get_integer_type(), false, false, -1,
			false);

	return var;
}

SummaryUnit::SummaryUnit(MtdInstsProcessor* mdp) {
	this->mtdp = mdp;
	this->has_return_var = false;
	this->return_node = NULL;
}

SummaryUnit::~SummaryUnit() {

}

void SummaryUnit::set_return_var_flag(bool return_flag) {
	this->has_return_var = return_flag;
}

void SummaryUnit::collect_reachable_nodes(MemGraph& mg) {

	//for alloc nodes, collect its reachable node;
	if (this->mtdp->has_allocations()) {
		map<MemNode*, AllocContext*>& allocs =
				mtdp->get_alloc_ctx_controller()->get_allocs_map();
		for (auto it = allocs.begin(); it != allocs.end(); it++) {
			MemNode* allocn = it->first;
			AccessPath* lhs =
					mtdp->get_alloc_ctx_controller()->get_alloc_lhs_ap(allocn);
			assert(allocn != NULL);
			assert(lhs != NULL);
			collect_alloc_related_nodes(mg, lhs, allocn);
		}
	}

	//for those directly reachable nodes that by different argument;
	map<AccessPath*, MemNode*> arg_locs = mg.get_argument_location_map();
	map<AccessPath*, MemNode*>::iterator it = arg_locs.begin();

	//for each argument, collect its reachable nodes;
	for (; it != arg_locs.end(); it++) {

		MemNode* start_arg_node = (*it).second;
		assert(start_arg_node != NULL);

		if (start_arg_node != NULL) {
			if (0)
				cout << endl << endl << "collecting related node for node "
						<< start_arg_node->to_string() << endl;
			collect_args_related_nodes(mg, start_arg_node);
		}
	}

	//return variable process separately
	if (has_return_var) {

		if (mg.get_return_var_ap() != NULL) {

			MemNode* return_node = mg.get_return_var_node();

			if (return_node == NULL)
				return;
			collect_return_related_nodes(mg, return_node);
		}
	}
}

void SummaryUnit::build_equivalent_insts(MemGraph mg) {

	collect_reachable_nodes(mg);

	//collected nodes that shared by argument and return var
	process_node_sets();

	//arithmetic node that are not reachable directly by argument or return value but still have flows

	collect_arithvalue_insts();

	order_insts();

	//check instruction set
	if (1) {
		string str = to_string();
		cout << str << endl;
	}
	cout << "end of build_equivalent_insts ! " << endl;
}

void SummaryUnit::collect_alloc_related_nodes(MemGraph& mg, AccessPath* lhs_ap,
		MemNode* node) {
	set<MemNode*>* res = new set<MemNode*>();
	mg.collect_reachable_locations(node, res);
	simplify_nodes(mg, res);

	node_set* ns = new node_set();
	//deref of alloc
	ns->var = lhs_ap;
	ns->nodes = res;
	alloc_nodes_set.push_back(ns);
}

void SummaryUnit::collect_args_related_nodes(MemGraph& mg, MemNode* node) {

	set<MemNode*>* res = new set<MemNode*>();
	mg.collect_reachable_locations(node, res);

	simplify_nodes(mg, res);

	//node_set ns;
	node_set* ns = new node_set();
	ns->var = node->get_representative();
	ns->nodes = res;
	arg_return_nodes_set.push_back(*ns);
}

void SummaryUnit::collect_return_related_nodes(MemGraph& mg, MemNode* node) {

	set<MemNode*>* res = new set<MemNode*>();
	mg.collect_reachable_locations(node, res);

	simplify_nodes(mg, res);

	//node_set ns;
	node_set* ns = new node_set();
	ns->var = node->get_representative();
	ns->nodes = res;
	arg_return_nodes_set.push_back(*ns);
	return_node = ns;
}

void SummaryUnit::simplify_nodes(MemGraph& mg, set<MemNode*>* res) {
	for (set<MemNode*>::iterator it = res->begin(); it != res->end(); it++) {
		MemNode* cur_loc = *it;

		if (cur_loc->is_terminal()
				&& (cur_loc->get_representative()->get_ap_type() == AP_DEREF)) {

			map<int, set<MemEdge*>*>* preds = cur_loc->get_preds();

			assert(preds->size() == 1);

			set<MemEdge*>* edges = preds->begin()->second;

			if (edges->size() == 1) {
				assert(cur_loc->is_default_node());
				assert(cur_loc->get_inner_ap()->get_ap_type() != AP_ALLOC);
				res->erase(cur_loc);
			}
		}
	}
}

void SummaryUnit::process_alloc_nodes() {

	cout << "#########################boundary 1 process "
			"alloc flow begin #####################" << endl;

	if (mtdp->has_allocations()) {

		if (CHECK_ALLOC_NODE) {
			cout << "Print out alloc_node set !! " << endl;
			alloc_node_set_test();
		}

		for (auto it = alloc_nodes_set.begin(); it != alloc_nodes_set.end();
				it++) {
			//每次都传入一个不同的alloc_lhs_ap;
			AccessPath* alloc_lhs_ap = (*it)->var;

			set<MemNode*>* alloc_related_nodes = (*it)->nodes;

			for (int i = 0; i < (int) arg_return_nodes_set.size(); i++) {

				node_set cur_node_set = arg_return_nodes_set.at(i);

				AccessPath* other_ap = cur_node_set.var;
				set<MemNode*>* othern = cur_node_set.nodes;
				assert(other_ap != NULL);
				assert(othern != NULL);
				assert(other_ap->get_ap_type() == AP_VARIABLE);

				process_different_node_set_for_alloc(alloc_related_nodes,
						othern, alloc_lhs_ap, other_ap);

			}
		}
	}

	cout << "#########################boundary 1 process alloc "
			"flow end #####################" << endl;
}

void SummaryUnit::process_arguments_nodes() {

	cout << "#########################boundary 2 process arg "
			"to arg flow begin #####################" << endl;

	for (vector<node_set>::iterator it = arg_return_nodes_set.begin();
			it != arg_return_nodes_set.end(); it++) {

		set<MemNode*>* first_level_node = (*it).nodes;
		if (first_level_node == NULL)
			continue;
		//processing argument to argument flow

		for (vector<node_set>::iterator it1 = arg_return_nodes_set.begin();
				it1 != arg_return_nodes_set.end(); it1++) {
			if (*it == *it1)
				continue;

			//guarantee that return_node is not NULL
			if (this->has_return_var) {
				if (*it1 == *return_node)
					continue;
			}

			if (this->has_return_var) {
				if (*it == *return_node)
					continue;
			}

			//guarantee that we only process argument->argument flows
			set<MemNode*>* second_level_node = (*it1).nodes;

			if (second_level_node != NULL) {
				//process_different_node_sets(first_level_node, second_level_node);
				process_different_node_set_for_args(first_level_node,
						second_level_node);
			}

		}
	}
	cout << "#########################boundary 2 process arg "
			"to arg flow end #####################" << endl;
}

/**********
 ******processing return nodes;
 ****/

void SummaryUnit::process_return_nodes() {

	cout << "#########################boundary 3 process arg to return "
			"flow begin #####################" << endl;

	if (!this->has_return_var)
		return;

	if(this->return_node == NULL)
		return;

//	if(this->return_node->var == NULL)
//		return;

	assert(this->return_node->var != NULL);

	set<MemNode*>* first_level_node = return_node->nodes;

	for (vector<node_set>::iterator it = arg_return_nodes_set.begin();
			it != arg_return_nodes_set.end(); it++) {

		set<MemNode*>* second_level_node = (*it).nodes;

		if (*it == *return_node)
			continue;

		//for test purpose
		if (first_level_node == NULL)
			return;

		if (second_level_node == NULL)
			return;

		assert(first_level_node != NULL);
		assert(second_level_node != NULL);

		if ((first_level_node->size() == 0) || (second_level_node->size() == 0))
			return;

		//process_different_node_sets(first_level_node, second_level_node, true);
		process_different_node_set_for_return(first_level_node,
				second_level_node);
	}
	cout << "#########################boundary 3 process arg "
			"to return flow end #####################" << endl;
}

void SummaryUnit::arg_node_sets_test() {

}

void SummaryUnit::return_node_set_test() {

}

void SummaryUnit::alloc_node_set_test() {

	auto it = this->alloc_nodes_set.begin();
	if (this->alloc_nodes_set.size() == 0)
		cout << "EMPTY!!" << endl;

	int i = 0;
	for (; it != alloc_nodes_set.end(); it++) {
		cout << i++ << "	LHS AP :: " << (*it)->var->to_string() << endl;
		set<MemNode*>* nodes = (*it)->nodes;

		for (auto iter = nodes->begin(); iter != nodes->end(); iter++) {
			cout << " reaching node " << (*iter)->get_time_stamp() << endl;
		}
		cout << endl;
	}
}

void SummaryUnit::process_node_sets() {

	process_alloc_nodes();

	process_arguments_nodes();

	process_return_nodes();

}

void SummaryUnit::add_to_eq_insts(sail::Instruction* inst) {

	assert(inst != NULL);
	string str = inst->to_string();

	if (unique_insts_string.count(str) > 0) {
		//delete inst;
		inst = NULL;
		return;
	}

	eq_ins.push_back(inst);
	unique_insts_string.insert(str);
}

void SummaryUnit::add_to_return_insts(sail::Instruction* inst) {
	assert(inst != NULL);

	string str = inst->to_string();

	if (unique_insts_string.count(str) > 0) {
		//delete inst;
		inst = NULL;
		return;
	}

	return_ins.push_back(inst);
	unique_insts_string.insert(str);
}

AccessPath* SummaryUnit::collect_alloc_insts_from_shared_nodes(
		AccessPath* lhs_ap) {

	AccessPath* deref_of_alloc =
			mtdp->get_alloc_ctx_controller()->get_alloc_deref_ap(lhs_ap);

	MemNode* alloc_deref_node = mtdp->get_mem_graph()->get_memory_node(
			deref_of_alloc);

	assert(mtdp->get_alloc_ctx_controller()->has_node(alloc_deref_node));

	//collect vi = alloc_sth only once;
	if (!mtdp->is_pointed_by_var(alloc_deref_node)) {
		//collect alloc

		sail::FunctionCall* alloc_inst = NULL;

		alloc_inst = mtdp->get_alloc_ctx_controller()->get_alloc_ins(
				alloc_deref_node);

		assert(alloc_inst != NULL);

		sail::Assignment* assi = size_of_alloc_inst(alloc_inst,
				alloc_deref_node);

		if (assi != NULL)
			add_to_eq_insts(assi);

		//add the alloc_inst;
		add_to_eq_insts(alloc_inst);

		sail::Variable* lhs = alloc_inst->get_lhs();

		//added to map so that won't be processed next time;
		mtdp->put_to_alloc_pointed_by_var_map(alloc_deref_node, lhs);

		if (0)
			cout << mtdp->alloc_mem_pointed_by_var_to_string() << endl;
	}

	return deref_of_alloc;

}

//(can't guess the rhs based on default edge (it could be local variable, have no effect))
//others are ret/argument ap(it should be Variable type)
void SummaryUnit::process_different_node_set_for_alloc(
		set<MemNode*>* alloc_relateds, set<MemNode*>* others,
		AccessPath* alloc_lhs_ap, AccessPath* other_ap) {



	cout << endl;
	cout << " == alloc ap && other ap is " << endl << alloc_lhs_ap->to_string()
			<< "\t" << other_ap->to_string() << endl;

	//this one will add sth = alloc_sth to instruction set;
	AccessPath* deref_of_alloc = collect_alloc_insts_from_shared_nodes(
			alloc_lhs_ap);

	assert(deref_of_alloc != NULL);

	//this one used to decide if sth = alloc_sth instruction should be delete from summaryunit;
//	int before_size = this->eq_ins.size();
//	cout << "BBBBBBBBefore check " << this->to_string() << endl << endl;
	set<MemNode*> shared_nodes;
//	cout << "size of alloc_relateds " << alloc_relateds->size() << endl;
	for (auto it = alloc_relateds->begin();
			it != alloc_relateds->end(); it++) {

		if (others->count(*it) > 0) {
			cout << "Collected Shared Nodes is " << (*it)->to_string() << endl;
			shared_nodes.insert(*it);
		}

		//collect arithmetic node of alloc;
		if ((*it)->get_representative()->get_ap_type() == AP_DEREF) {
			AccessPath* inner_ap = (*it)->get_inner_ap();

			if ((inner_ap->get_ap_type() == AP_ARITHMETIC)
					|| (inner_ap->get_ap_type() == AP_FUNCTION)) {
				cout << "processing arithmetic is  " << (*it)->to_string()
						<< endl;
				process_arithmetic_fun_deref_node(*it);

				if (ARITHMETIC_NODE) //this one could probably called inside process_arithmetic_fun_deref_node
					cout << " --------------------node " << (*it)->to_string()
							<< "finished! " << endl;
				processed_node.insert(*it);

			}
		}
	}
	//cout << "111111111111111111111111 " << endl;
	//no shared nodes found
	if (shared_nodes.size() == 0)
		return;

	//each pair denotes two accesspath point to same memnode;
	//first ele of pair belongs to alloc, second belongs to other

	set<pair<AccessPath*, AccessPath*>> collect_pairs;
	int ret_code = collect_relation_pairs_for_alloc(shared_nodes, alloc_lhs_ap,
			deref_of_alloc, other_ap, collect_pairs);
	//cout << "2222222222222222222222222 " << endl;
	//check out all possible relations(how many relation it has?)
	cout << endl << "alloc ap is " << cout << endl
			<< "------size of collect_pairs is " << collect_pairs.size() << endl
			<< " && other ap has default is ?? " << ret_code << endl;

	for (auto it_p = collect_pairs.begin(); it_p != collect_pairs.end();
			it_p++) {
		//based on the two source to generate instructions;
		AccessPath* ap1 = (*it_p).first; //field selection for alloc, or point to alloc
		AccessPath* ap2 = (*it_p).second; //other

		//process alloc side;
		assert((ap1 == alloc_lhs_ap) || (ap1->get_ap_type() == AP_FIELD));
		//assert(ap1->get_ap_type() == AP_FIELD);

		//get the deref of alloc;
		MemNode* alloc_node = mtdp->get_mem_graph()->get_memory_node(
				deref_of_alloc);

		sail::Variable* alloc_var =
				mtdp->get_alloc_ctx_controller()->get_alloc_ins_lhs_var(
						alloc_node);

		variable_info alloc_var_info;
		int alloc_side_offset = IMPOSSIBLE_OFFSET;
		//init
		if (ap1 == alloc_lhs_ap) {
			alloc_var_info.var = alloc_var;
		} else if (ap1->get_ap_type() == AP_FIELD) {
			FieldSelection* alloc_fs = static_cast<FieldSelection*>(ap1);
			alloc_side_offset = alloc_fs->get_field_offset();
			alloc_var_info.var = alloc_var;
			alloc_var_info.fn = alloc_fs->get_field_name();
			alloc_var_info.ft = alloc_fs->get_type();
		} else {
			assert(false);
		}

		//process other side;
		//must be argument;
		variable_info other_side;
		int other_offset = IMPOSSIBLE_OFFSET;
		//init
		if (ap2->get_ap_type() == AP_VARIABLE) {
			Variable* other_var = static_cast<Variable*>(ap2);
			other_side.var = other_var->get_original_var();
		} else if (ap2->get_ap_type() == AP_FIELD) {
			FieldSelection* other_fs = static_cast<FieldSelection*>(ap2);
			other_side.var = ((Variable*) other_ap)->get_original_var();
			other_side.fn = other_fs->get_field_name();
			other_side.ft = other_fs->get_type();
			other_offset = other_fs->get_field_offset();
		} else {
			assert(false);
		}

		if (ret_code == 1) { //other is rhs, and must be argument

			if (!((Variable*) other_ap)->is_argument()) {
				cerr
						<< "if ret_code  is 1 means that default edge belongs to argument! "
								"(only argument has default edge!)!" << endl;
				assert(false);
			}
			produce_assignment_ins(alloc_var_info, alloc_side_offset,
					other_side, other_offset, false);

		} else { //other(arg/ret) in lhs;

			//add temp = alloc_sth() to summary;
			Variable* other_var_ap = static_cast<Variable*>(other_ap);
			bool ret = false;
			if (other_var_ap->is_return_var())
				ret = true;
			produce_assignment_ins(other_side, other_offset, alloc_var_info,
					alloc_side_offset, ret);
		}

	}

//	int after_size = this->eq_ins.size();

//	//last one should be sth = alloc_sth; and decide if it needs to delete from summary-unit?
//	if(before_size == after_size){
//		cout << "After check  " << this->to_string() << endl;
//		sail::Instruction* inst = this->eq_ins.back();
//
//		if(inst->get_instruction_id() == sail::FUNCTION_CALL){
//			sail::FunctionCall* fc = static_cast<sail::FunctionCall*>(inst);
//
//			if(fc->get_lhs()->is_argument())
//				return;
//			if(fc->get_lhs()->is_return_variable())
//				return;
//
//			if(fc->get_arguments()->size() == 0){
//				this->eq_ins.pop_back();
//			}else{
//				sail::Instruction* size_inst = this->eq_ins.back();
//				assert(size_inst->get_instruction_id() == sail::ASSIGNMENT);
//				this->eq_ins.pop_back();
//			}
//
//		}else{
//			cerr << "Should be alloc instruction ! " << endl;
//			cout << this->to_string();
//			assert(false);
//		}
//	}
}

sail::Assignment* SummaryUnit::size_of_alloc_inst(sail::FunctionCall* allo_fc,
		MemNode* deref_of_alloc) {

	vector<sail::Symbol*>* args = allo_fc->get_arguments();

	if (args->size() == 1) {

		if(args->at(0)->is_constant()){
			return NULL;
		}
		assert(args->at(0)->is_variable());
		sail::Variable* size_s = (sail::Variable*) args->at(0);

		if (size_s->is_argument())
			return NULL;

		//find the target of deref_of_alloc's offset -4;
		set<MemNode*>* size_targets = deref_of_alloc->get_targets_from_offset(
				-4);
		if (size_targets->size() == 1) {

			MemNode* size_target = *(size_targets->begin());

			AccessPath* inner_ap = size_target->get_inner_ap();

			if (inner_ap->get_ap_type() != AP_CONSTANT) {
				cout << "alloc_fc " << allo_fc->to_string() << endl;
				print_ap_type(inner_ap);
				return NULL;
				//assert(false);
			}

			ConstantValue* const_ap = static_cast<ConstantValue*>(inner_ap);
			sail::Constant* const_value = new sail::Constant(
					const_ap->get_constant(), true, 32);

			sail::Assignment* assi = new sail::Assignment(size_s, const_value,
			NULL, -1);
			return assi;

		}

		//otherwise, otherwise to find the size of alloc;

		AccessPath* size_ap = get_ap_from_symbol(size_s, this->mtdp);
		MemNode* size_var_node = mtdp->get_mem_graph()->get_memory_node(
				size_ap);

		set<MemNode*>* deref_const_ns = size_var_node->get_targets_from_offset(
				0);

		//make sure that it contain target node;
		if (deref_const_ns == NULL) {

			cerr << " || {{ size_var_node " << size_var_node->to_string()
					<< " do not contain its target! " << endl;

			assert(false);
		}

		if (deref_const_ns->size() == 0)
			return NULL;

		if (deref_const_ns->size() == 1) {
			MemNode* const_deref_n = *(deref_const_ns->begin());

			AccessPath* deref_const = const_deref_n->get_representative();
			assert(deref_const->get_ap_type() == AP_DEREF);

			if(
					((Deref* )deref_const)->get_inner()->get_ap_type()
							!= AP_CONSTANT){
				cerr << "processing size of alloc, if it is not constant, just ignore! " << endl;
				print_ap_type(((Deref*)deref_const)->get_inner());
				return NULL;
			}
//			assert(
//					((Deref* )deref_const)->get_inner()->get_ap_type()
//							== AP_CONSTANT);

			ConstantValue* const_ap =
					(ConstantValue*) (((Deref*) deref_const)->get_inner());
			sail::Constant* const_value = new sail::Constant(
					const_ap->get_constant(), true, 32);

			sail::Assignment* assi = new sail::Assignment(size_s, const_value,
			NULL, -1);
			return assi;
		} else {
			cout << "it has " << deref_const_ns->size() << " target !" << endl;
			return NULL;
		}
	}

	return NULL;

}

int SummaryUnit::collect_relation_pairs_for_alloc(set<MemNode*>& shared_nodes,
		AccessPath* alloc_lhs_ap, AccessPath* alloc_deref, AccessPath* other_ap,
		set<pair<AccessPath*, AccessPath*>>& relation_pairs) {
	int which_default = -1;
	for (auto it = shared_nodes.begin(); it != shared_nodes.end(); it++) {

		//to each shared node, find its preds;
		cout << "Shared node is " << (*it)->to_string() << endl;
		map<int, set<MemEdge*>*>* preds = (*it)->get_preds();

		//size of entry point is 1;
		assert(preds->size() <= 1);

		if (preds->size() == 1) {

			//to each node, collect if two of them existing;
			pair<AccessPath*, AccessPath*> p;

			set<MemEdge*>* pred_edges = preds->begin()->second;

			for (auto iter = pred_edges->begin(); iter != pred_edges->end();
					iter++) {

				MemEdge* pred_e = *iter;
				//is this a default edge?
				bool default_e = pred_e->is_default_edge();

				//default edge belongs to whom? set the code;
				int collect_which_default = -1;

				if (0) {
					cout << " alloc_lhs_ap is " << alloc_lhs_ap->to_string()
							<< endl;
					cout << " other ap is " << other_ap->to_string() << endl;
					cout << " source representative is "
							<< pred_e->get_source_node()->get_representative()->to_string()
							<< endl;
				}

				//one edge build one pair at most;
				//alloc_ap is the deref of alloc_ap;
				if (pred_e->get_source_node()->get_representative()
						== alloc_deref) {
					//so far impossible have assignment to size offset;
					if (pred_e->get_source_offset() == SIZE_OFFSET)
						continue;
					cout << "edge timestamp is " << pred_e->get_time_stamp() << endl;
					cout << "edge is " << pred_e->to_string() << endl;
					AccessPath* s_ap = pred_e->get_source_ap(
							pred_e->get_source_offset());
					p.first = s_ap;

					if (default_e)
						collect_which_default = 0;

				} else if (pred_e->get_source_node()->get_representative()
						== alloc_lhs_ap) { //it is ap that point to deref of alloc

					AccessPath* s_ap = pred_e->get_source_ap(
							pred_e->get_source_offset());

					//rhs
					p.first = s_ap;

					//impossible is a default edge;
					assert(!default_e);

				} else if (pred_e->get_source_node()->get_representative()
						== other_ap) {

					AccessPath* s_ap = pred_e->get_source_ap(
							pred_e->get_source_offset());
					assert(s_ap != NULL);
					p.second = s_ap;

					if (default_e)
						collect_which_default = 1;

				}

				//it is probably that some edge point to either if or else if(cases)
				if ((p.first != NULL) && (p.second != NULL)) {
					if (relation_pairs.count(p) == 0) {
						//set which one it belongs to?
						if ((default_e) && (collect_which_default != -1))
							which_default = collect_which_default;
						cout << " Shared Node " << (*it)->to_string()
								<< "	generate relation ! " << endl;
						relation_pairs.insert(p);
					}
				}

			}

		}
	}
	return which_default;
}

void SummaryUnit::process_different_node_set_for_args(set<MemNode*>* ns1,
		set<MemNode*>* ns2) {
	for (set<MemNode*>::iterator it_node = ns1->begin(); it_node != ns1->end();
			it_node++) {
		for (set<MemNode*>::iterator it_node1 = ns2->begin();
				it_node1 != ns2->end(); it_node1++) {

			MemNode* node1 = *it_node;
			MemNode* node2 = *it_node1;

			if (1) {
				cout << endl;
				cout << " == == node1 " << node1->to_string() << endl;
				cout << " == == node2 " << node2->to_string() << endl;
			}

			if (processed_node.find(node1) == processed_node.end()) {

				if (0)
					cout << "ARGUMENT => ARGUMENT FLOW !!" << endl;

				if (node1 == node2) {

					collect_assignment_insts(node1);

					processed_node.insert(node1);

				} else if (node1->get_representative()->get_ap_type()
						== AP_DEREF) {

					if (processed_node.find(node1) == processed_node.end()) {

						if (ARITHMETIC_NODE)
							cout << endl
									<< "---------------------------- Deref node 1 ?? :) "
									<< node1->to_string() << endl;

						process_arithmetic_fun_deref_node(node1);

						if (ARITHMETIC_NODE)
							cout << " --------------------node "
									<< node1->to_string() << "finished! "
									<< endl;
						processed_node.insert(node1);
					}
				}

			}
		}
	}
}

void SummaryUnit::process_different_node_set_for_return(set<MemNode*>* ns1,
		set<MemNode*>* ns2) {
	for (set<MemNode*>::iterator it_node = ns1->begin(); it_node != ns1->end();
			it_node++) {
		for (set<MemNode*>::iterator it_node1 = ns2->begin();
				it_node1 != ns2->end(); it_node1++) {

			MemNode* node1 = *it_node;
			MemNode* node2 = *it_node1;

			if (1) {
				cout << endl;
				cout << " == == node1 " << node1->to_string() << endl;
				cout << " == == node2 " << node2->to_string() << endl;
			}

			//process those return var shared un_arithmetic_value nodes with other argument;
			if (1)
				cout << "ARGUMENT  => Return FLOW!! " << endl;

			if (processed_return_node.find(node2)
					== processed_return_node.end()) {

				if (node1 == node2) {
					cout << "	**	processing node 1 " << node1->to_string()
							<< endl;
					collect_assignment_for_return_var(node1);
					processed_return_node.insert(node2);

				} else if (node1->get_representative()->get_ap_type()
						== AP_DEREF) { //process if it is an arithmetic node

					//because it will be put into processed_node after processed;
					if (processed_node.find(node1) == processed_node.end()) {
						process_arithmetic_fun_deref_node(node1);

						if (ARITHMETIC_NODE) //this one could probably called inside process_arithmetic_fun_deref_node
							cout << " --------------------node "
									<< node1->to_string() << "finished! "
									<< endl;
						processed_node.insert(node1);

					}
				}

			}

		}
	}

}

void SummaryUnit::process_arithmetic_fun_deref_node(MemNode* arith_deref) {

	//ArithMetic or Function ap;
	AccessPath* inner_ap = arith_deref->get_inner_ap();
	if ((inner_ap->get_ap_type() == AP_ARITHMETIC)
			|| (inner_ap->get_ap_type() == AP_FUNCTION)) {
		//first if the node's inner ap itself contains the argument;
		if (1)
			cout << "ENcountering arith/fun value " << endl;

		//arith_deref contain argument or alloc(return is impossible)
		bool arg_or_alloc_flag;
		vector<AccessPath*> aps;
		collect_arith_fun_aps(inner_ap, aps, arg_or_alloc_flag);

		int arith_ap_size = aps.size();
		if (arg_or_alloc_flag) {

			//to find if its source is an argument or return value(is pointed by ?)
			//step by step to process its source node
			set<MemEdge*>* pre_edges = arith_deref->get_preds()->begin()->second;
			for (set<MemEdge*>::iterator edge_it = pre_edges->begin();
					edge_it != pre_edges->end(); edge_it++) {

				//process each source node;
				cout << "EEEEEEEEdge timestamp is " << (*edge_it)->get_time_stamp() << endl;
				cout << "	EEEEEEdge content is " << (*edge_it)->to_string() << endl;
				MemNode* s_arg_node = (*edge_it)->get_source_node();

				int s_offset = (*edge_it)->get_source_offset();

				AccessPath* s_ap = (*edge_it)->get_source_ap(s_offset);
				AccessPath* s_arg_rep = s_arg_node->get_representative();

				if (s_arg_rep->get_ap_type() == AP_VARIABLE) { //be directly pointed by ...
					Variable* s_var = (Variable*) s_arg_rep;
					if (s_var->is_argument()) {
						process_argument_node_with_arith_value_target(
								s_arg_node, s_var, s_offset, arith_ap_size);
					} else if (s_var->is_return_var()) { //collect return var
						process_argument_node_with_arith_value_target(
								s_arg_node, s_var, s_offset, arith_ap_size);
					}
				} else if (s_arg_rep->get_ap_type() == AP_DEREF) { //store to field (  be indirectly pointed by ...)

					if(s_ap->get_ap_type() != AP_FIELD){
						cerr <<"processing arith deref node is "
								<< arith_deref->to_string() << endl;
						print_ap_type(s_ap);
						if(s_ap->get_ap_type() == AP_DEREF){
							cerr << "deref of whom ? " << endl;
							print_ap_type(s_ap->get_inner());
						}

						//assert(s_ap->get_ap_type() == AP_FIELD);
						assert(false);
					}

					FieldSelection* fs = (FieldSelection*) s_ap;
					cout << "at least here ! " << endl;
					if (fs->get_inner()->get_inner()->get_ap_type()
							== AP_VARIABLE) {
						Variable* var_s =
								(Variable*) (fs->get_inner()->get_inner());

						if (var_s->is_argument())
							process_argument_node_with_arith_value_target(
									s_arg_node, fs, s_offset, arith_ap_size);
						if (var_s->is_return_var()) {
							cout
									<< "processing return var whose target is arithmetic value !!! FIXME !"
									<< endl;
							assert(false);
						}

					}

					//this is the only case that (arg => ap_alloc) in arithmetic operations;
					if (fs->get_inner()->get_inner()->get_ap_type()
							== AP_ALLOC) {
						cout << "catch here ! " << endl;
						process_argument_node_with_arith_value_target(
								s_arg_node, fs, s_offset, arith_ap_size);

					}
				}
			}

		}

		//if anything is wrong , please do not comment out this lines, and delete something outside the call;
//		if (ARITHMETIC_NODE)
//			cout << " --------------------node " << arith_deref->to_string()
//					<< "finished! " << endl;
//		processed_node.insert(arith_deref);

	}
}

//av_target contain argument ap and its representative  target aps;
void SummaryUnit::update_arg_arith_target_for_arg(AccessPath* arg_ap,
		set<AccessPath*>& target_aps) {
	set<AccessPath*> copy_targets = target_aps;

	assert(arg_arith_target.count(arg_ap));
	av_target* avt = arg_arith_target[arg_ap];
	set<AccessPath*> av_t = avt->av_target;

	set<AccessPath*>::iterator max_it = av_t.begin();
	for (; max_it != av_t.end(); max_it++) {

		if (0)
			cout << endl << " -- -- biggest aps :: " << (*max_it)->to_string()
					<< endl;
		//max arithmaticValue/funValue ap consists of max_apo(not terminals);prepared to compare for other target aps;
		vector<ap_with_operator> max_apo;
		collect_sub_terms((*max_it)->to_term(), max_apo);

		int k = 0;
		//target_aps belongs to targets of argument/ret ap
		for (set<AccessPath*>::iterator t_ap_it = target_aps.begin();
				t_ap_it != target_aps.end(); t_ap_it++) {
			AccessPath* tar_ap = *t_ap_it;
			if (0)
				cout << "tar [ " << k++ << "]:  " << tar_ap->to_string()
						<< endl;
			if (*max_it == tar_ap) {
				copy_targets.erase(tar_ap);
				continue;
			}

			bool flag = belong_to_parts_of_terms(tar_ap, max_apo);

			if (flag) { //update target_aps, directly find
				assert(copy_targets.count(tar_ap));
				copy_targets.erase(tar_ap);

			} else if (!is_terminal_term(tar_ap)) {
				if (ap_strip_term_deletable(tar_ap, arg_ap->to_term(),
						max_apo)) {
					copy_targets.erase(tar_ap);
					if (av_t.count(tar_ap)) {
						avt->delete_ap_target(tar_ap);
					}
				} else
					cout << "strip didn't find !" << endl;
			} else {
				cout << " can't find it " << endl;
			}
		}
		//check if something left;
		if (DEBUG_INFO)
			cout << "check left *************** copy_targets "
					<< copy_targets.size() << endl;
		target_aps.clear();

		//updating avt's final representative target ap;
		for (set<AccessPath*>::iterator it_copy_t = copy_targets.begin();
				it_copy_t != copy_targets.end(); it_copy_t++) {
			if (DEBUG_INFO)
				cout << "INserting  " << (*it_copy_t)->to_string() << endl;
			avt->insert_ap_target(*it_copy_t);
			//avt->av_target.insert(*it_copy_t);
		}

	}
}

//representative targets are built for argument/ret source;
//correct arg_arith_target are built
void SummaryUnit::process_argument_node_with_arith_value_target(
		MemNode* arg_node, AccessPath* var, int offset, unsigned int ap_size) {
	if (!arg_arith_target.count(var)) { //haven't built, then build it

		// a set of target node for the argument in memory graph
		set<AccessPath*> target_aps;
		//would change "arg_arith_target" with max ap settle for arg_node;
		process_arithvalue_source_node(arg_node, ap_size, target_aps, offset);

		set<AccessPath*> copy_targets = target_aps;

		//check if other ones need to added into it

		if (0) {
			if (var->get_ap_type() == AP_VARIABLE)
				cout << "source ap is :: " << var->to_string() << endl;
		}

		store_into_argument_and_target_map(var, target_aps);

		//until now the max arithmetic target has been selected;
		update_arg_arith_target_for_arg(var, target_aps);

	}

}

/********************************************************
 * MemNode* source is argument node whose ap type is variable;
 * ************
 * *** find the maximum accesspaths who contain argument;
 * *** and collect other target except the biggest;
 ********************************************************/
void SummaryUnit::process_arithvalue_source_node(MemNode* source,
		int cur_target_ap_size, set<AccessPath*>& target_aps, int offset) {
	cout << "process_arithvalue_source_node " << endl;
	cout << "Source Node is  " << source->to_string() << endl;
	AccessPath* rep = source->get_representative();
	AccessPath* s_ap = NULL;
	if (rep->get_ap_type() == AP_VARIABLE) {
		Variable* var_ap = (Variable*) rep;
		s_ap = var_ap;
	} else if (rep->get_ap_type() == AP_DEREF) {
		AccessPath* ap = source->get_access_path(offset);
		if (ap->get_ap_type() == AP_FIELD) {
			FieldSelection* fs = (FieldSelection*) ap;
			s_ap = fs;
		}
	}

	//cout << "s_ap " << s_ap->to_string() << endl;
	if (DEBUG_INFO)
		cout << "=== argument node is " << source->to_string() << endl;
	int max = cur_target_ap_size;
	set<MemEdge*>* t = NULL;
	if (rep->get_ap_type() == AP_VARIABLE)
		t = source->get_succs()->begin()->second;
	else if (rep->get_ap_type() == AP_DEREF)
		t = source->get_succ_from_offset(offset);

	if (DEBUG_INFO)
		cout << "size of target  ********** " << t->size() << endl;

	for (set<MemEdge*>::iterator it = t->begin(); it != t->end(); it++) {
		MemNode* target = (*it)->get_target_node();
		//cout << "Tar node " << target->to_string() << endl;
		AccessPath* inner_ap = target->get_inner_ap();

		if (/*1*/DEBUG_INFO)
			cout << endl << "**********check target's inner ap***********  "
					<< inner_ap->to_string() << endl;

		if ((inner_ap->get_ap_type() != AP_ARITHMETIC)
				&& (inner_ap->get_ap_type() != AP_FUNCTION)) {
			continue;
		}

		//cout << "inserting inner ap " << inner_ap->to_string() << endl;
		target_aps.insert(inner_ap);

		bool arg_flag;

		// a set of termial aps;
		vector<AccessPath*> aps;
		collect_arith_fun_aps(inner_ap, aps, arg_flag);

//		cout << "max " << max << endl;
//		cout << "aps size " << aps.size() <<endl;
		if (arg_flag) { //containing argument
			if (aps.size() >= max) {
				max = aps.size();
				// rep= source_ap, inner_ap = target_ap;
				cout << "called here ! " << endl;
				update_arg_max_arithvalue_target(s_ap, inner_ap, aps.size(),
						offset);
			}
		}
	}

}

//based on the represenative target stored in "arg_arith_target";
void SummaryUnit::collect_arithvalue_insts() {

	cout
			<< "######################### boundary 4 process arithvalue flow #####################"
			<< endl;
	map<AccessPath*, av_target*>::iterator it = arg_arith_target.begin();
	int i = 0;

	for (; it != arg_arith_target.end(); it++) {
		AccessPath* arg = (*it).first;

		av_target* av_t = (*it).second;

		//if (DEBUG_INFO) {
		if (1) {
			cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% " << endl;
			cout << " [ " << i++ << "] ";
			cout << "  Arithmetic lhs {" << arg->to_string() << "}";
			cout << "\t" << "  av_t  [" << av_t->to_string() << "]";
		}

		//testing the build ap->symbols
		if (DEBUG_INFO) {
			cout << "HHHHHHHHHHHHHH ------------------ " << endl;
			cout << solved_ap_symbols_to_string() << endl;
		}

		set<AccessPath*> tar = av_t->av_target;

		for (set<AccessPath*>::iterator it1 = tar.begin(); it1 != tar.end();
				it1++) {
			//target ap which need to be solved
			AccessPath* ap = *it1;

			sail::Variable* sail_var = get_sail_variable(arg);

			if (sail_var == NULL) {
				if (arg->get_ap_type() == AP_FIELD) {
					if (arg->get_inner()->get_ap_type() == AP_DEREF) {
						if (((Deref*) (arg->get_inner()))->get_inner()->get_ap_type()
								== AP_ALLOC) {
							MemNode* alloc_deref_node =
									mtdp->get_mem_graph()->get_memory_node(
											arg->get_inner());
							sail_var =
									mtdp->get_alloc_ctx_controller()->get_alloc_ins_lhs_var(
											alloc_deref_node);
							cout << "SAIL VAR is  " << sail_var->to_string()
									<< endl;
						}
					}
				}

			}
			assert(sail_var != NULL);

			//in case that different tree share same sub-expressions
			//if the checked one belongs to part of the solved symbols
			if (solved_ap_symbols.count(*it1)) {
				//generate an assignment;
				sail::Symbol* sym = solved_ap_symbols[*it1];

				if (arg->get_ap_type() == AP_VARIABLE) {
					sail::Assignment* ass = new sail::Assignment(sail_var, sym);
					arithmetic_ins.push_back(ass);
					update_track_include(ass, sail_var->is_return_variable());

				} else if (arg->get_ap_type() == AP_FIELD) {
					FieldSelection* fs = (FieldSelection*) arg;
					sail::Store* s = new sail::Store(sail_var, sym,
							fs->get_field_offset(), fs->get_field_name(), -1);

					arithmetic_ins.push_back(s);
					update_track_include(s, sail_var->is_return_variable());

				} else {
					cout << "ap type is " << arg->get_ap_type() << endl;
					assert(false);
				}

				continue;
			}

			ApTree apt;
			if (arg->get_ap_type() == AP_VARIABLE) {
				apt = ApTree(this, sail_var, arg, *it1);
			} else if (arg->get_ap_type() == AP_FIELD)
				apt = ApTree(this, sail_var, arg, *it1, true);

			apt.produce_arith_fun_insts(ap);

		}
	}
}

/*********
 * ap is the ap we need to check, t is the argument itself
 */
bool SummaryUnit::ap_strip_term_deletable(AccessPath*& ap, Term* strip_t,
		vector<ap_with_operator>& refer_ap_terms) {

	AccessPath* orig = ap;
	string orig_str = orig->to_string();

	//this function should changed the object itself;
	Term* after_t = ap->to_term()->strip_term(strip_t);
	AccessPath* after_ap = AccessPath::to_ap(after_t);
	//cout << "strip_t " << strip_t->to_string() << endl;
	//cout << "orig ap " << orig_str << endl;
	//cout << "striped ap " <<after_ap->to_string() << endl;
	if (orig_str == after_ap->to_string()) {
		return false;
	}

	return belong_to_parts_of_terms(after_ap, refer_ap_terms);
}

void SummaryUnit::update_arg_max_arithvalue_target(AccessPath* arg,
		AccessPath* target, int size, int offset) {
	if (arg_arith_target.count(arg) > 0) {
		av_target* av_t = (arg_arith_target)[arg];
		if (av_t->max_aps_size < size) {
			av_t->av_target.clear();
			av_t->max_aps_size = size;
		}
		av_t->av_target.insert(target);
	} else {
		av_target* t = new av_target();
		t->av_target.insert(target);
		//t->s_offset = offset;
		t->max_aps_size = size;
		(arg_arith_target)[arg] = t;
	}
}

void SummaryUnit::collect_assignment_for_return_var(MemNode* node) {

	if (0) {
		cout << "*******************???????????????? "
				"inside collect assignment for return var " << endl;
		cout << "processing node " << node->to_string() << endl;
	}

	map<int, set<MemEdge*>*>* preds = node->get_preds();
	set<MemEdge*>* edges = preds->begin()->second;
	bool lhs_ret_f = false;
	bool rhs_arg_f = false;
	variable_info ret_var;
	variable_info rhs_var;
	int ret_var_offset = IMPOSSIBLE_OFFSET;
	int rhs_offset = IMPOSSIBLE_OFFSET;

	AccessPath* rhs_variable_ap = NULL;

	for (set<MemEdge*>::iterator it = edges->begin(); it != edges->end();
			it++) {

		int s_offset = (*it)->get_source_offset();
		AccessPath* s_ap = (*it)->get_source_ap(s_offset);
		if (0) {
			cout << " = = == =  = check edge !! " << (*it)->to_string() << endl
					<< " 		source_offset " << s_offset << endl
					<< "				&& source_ap " << s_ap->to_string() << endl;
		}

		//to non_alloc rhs
		if ((*it)->is_default_edge()) { //rhs

			if (0) {
				cout << " &&&&&&&&&&& default edge RHS" << (*it)->to_string()
						<< endl;
				cout << "ap type " << s_ap->get_ap_type() << endl;
			}

			if ((s_ap->get_ap_type() == AP_ARITHMETIC)
					|| (s_ap->get_ap_type() == AP_FUNCTION)) {
				vector<AccessPath*> aps;
				bool arg_flag;
				//cout << "^^^^^^^^^^^^ collecting arith fun aps " << endl;
				collect_arith_fun_aps(s_ap, aps, arg_flag);

				if (arg_flag) {
//					cout << "collecting " << s_ap->to_string()
//							<< endl;
					return_targets_in_memory_graph.insert(s_ap);
				}

				continue;
			}

			//if not arithvalue type of node
			//cout << "###### call build_ins_assi_side " << endl;
			build_ins_assi_side(s_ap, rhs_var, rhs_arg_f);
			if (rhs_arg_f) {
				if (s_ap->get_ap_type() == AP_FIELD)
					rhs_offset = s_offset;

				rhs_variable_ap = s_ap;
			}
		} else { //lhs must be return variable;
			//cout << "LHS " << s_ap->to_string() << endl;
			//cout << "~~~~~~~~~~~call build_ret_var_side " << endl;
			build_ret_var_side(s_ap, ret_var, lhs_ret_f);

			if (lhs_ret_f) {
				if (s_ap->get_ap_type() == AP_FIELD)
					ret_var_offset = s_offset;
			}
		}

//		cout << "lhs_ret_f " << lhs_ret_f << endl;
//		cout << "rhs_arg_f" << rhs_arg_f << endl;
		if (lhs_ret_f && rhs_arg_f) {
			//true means it is  a return var
			//cout << "$$$$$$$$$$$$$$ producing instruction !! " << endl;
			produce_assignment_ins(ret_var, ret_var_offset, rhs_var, rhs_offset,
					true);

			continue;
		}

	}
}

void SummaryUnit::collect_assignment_insts(MemNode* node) {

	if (0)
		cout << "== inside collect_assignment_insts == for node"
				<< node->to_string() << endl;

	map<int, set<MemEdge*>*>* preds = node->get_preds();
	assert(preds->size() == 1);
	set<MemEdge*>* edges = preds->begin()->second;
	bool lhs_arg_f = false;
	bool rhs_arg_f = false;

	variable_info lhs_var;
	variable_info rhs_var;

	int lhs_offset = IMPOSSIBLE_OFFSET;
	int rhs_offset = IMPOSSIBLE_OFFSET;

	for (set<MemEdge*>::iterator it = edges->begin(); it != edges->end();
			it++) {
		//cout << "edge is " << (*it)->to_string() << endl;
		int s_offset = (*it)->get_source_offset();
		//cout << "<<<<< processing source offset is " << s_offset << endl;
		AccessPath* s_ap = (*it)->get_source_ap(s_offset);

		if (0) {
			cout << "hyz|| == s_ap " << s_ap->to_string() << endl;
			cout << "hyz|| == s_ap type " << s_ap->get_ap_type() << endl;
			cout << "hyz|| == s_ap' variable type "
					<< s_ap->get_type()->to_string() << endl;
		}

		if ((*it)->is_default_edge()) { //rhs
			//cout << "default edge RHS== " << (*it)->to_string() << endl;
//			cout << "s_ap " << s_ap->to_string() << endl;

			build_ins_assi_side(s_ap, rhs_var, rhs_arg_f);
			if (rhs_arg_f) {
				if (s_ap->get_ap_type() == AP_FIELD)
					rhs_offset = s_offset;
			}
		} else { //lhs
			//cout << "LHS " << endl;
			build_ins_assi_side(s_ap, lhs_var, lhs_arg_f);

			if (lhs_arg_f) {
				if (s_ap->get_ap_type() == AP_FIELD)
					lhs_offset = s_offset;
			}
		}

		if (0) {
			cout << "== || rhs_arg " << rhs_arg_f << endl;
			cout << "== || lhs_arg " << lhs_arg_f << endl;
		}

		if (lhs_arg_f && rhs_arg_f) {
			produce_assignment_ins(lhs_var, lhs_offset, rhs_var, rhs_offset);

			lhs_arg_f = false;
			lhs_offset = IMPOSSIBLE_OFFSET;
		}
	}
}

void SummaryUnit::build_alloc_var_side(AccessPath* s_ap,
		variable_info& alloc_side) {

	if (s_ap->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = static_cast<FieldSelection*>(s_ap);
		alloc_side.fn = fs->get_field_name();
		alloc_side.ft = fs->get_type();

		AccessPath* inner = fs->get_inner();
		assert(inner->get_ap_type() == AP_DEREF);

		AccessPath* deref_inner = ((Deref*) inner)->get_inner();

		assert(deref_inner->get_ap_type() == AP_ALLOC);

		MemNode* deref_alloc_node = mtdp->get_mem_graph()->get_memory_node(
				inner);
		sail::Variable* sail_var = mtdp->get_point_to_alloc_var(
				deref_alloc_node);
		if (sail_var == NULL)
			return;

		if (0) {
			cout << "rhs variable of allocation is " << sail_var->to_string()
					<< endl;
			cout << "is sail_var a return variable ? "
					<< sail_var->is_return_variable() << endl;
		}
		//it is possible that even if the sail_var(temp/argument)
		//is not return, its alias to return var

		alloc_side.var = sail_var;
		return;

	}

}

void SummaryUnit::build_ret_var_side(AccessPath* s_ap, variable_info& ret_side,
		bool& ret_flag) {
	ret_flag = false;
	if (s_ap->get_ap_type() == AP_VARIABLE) {
		Variable* ap_var = (Variable*) s_ap;
		if (!ap_var->is_return_var())
			return;
		else
			ret_flag = true;
		ret_side.var = ap_var->get_original_var();
		return;
	}

	if (s_ap->get_ap_type() == AP_FIELD) { //5
		FieldSelection* fs = static_cast<FieldSelection*>(s_ap);
		ret_side.fn = fs->get_field_name();
		ret_side.ft = fs->get_type();

		AccessPath* inner = fs->get_inner();

		//it could be alloc ap; assert couldn't hold;
		if (inner->get_inner()->get_ap_type() == AP_ALLOC)
			return;

		if (0) {

			if (inner->get_inner()->get_ap_type() == AP_ALLOC) {

				MemNode* deref_alloc_node =
						mtdp->get_mem_graph()->get_memory_node(inner);
				sail::Variable* sail_var = mtdp->get_point_to_alloc_var(
						deref_alloc_node);
				if (sail_var == NULL)
					return;
				if (0) {
					cout << "rhs variable of allocation is "
							<< sail_var->to_string() << endl;
					cout << "is sail_var a return variable ? "
							<< sail_var->is_return_variable() << endl;
				}
				//it is possible that even if the sail_var(temp/argument)
				//is not return, its alias to return var

				ret_flag = true;
				ret_side.var = sail_var;
				return;
			}
		}

		assert(inner->get_inner()->get_ap_type() == AP_VARIABLE);
		Variable* var = (Variable*) inner->get_inner();
		if (!var->is_return_var())
			return;

		ret_flag = true;
		ret_side.var = var->get_original_var();
		return;
	}

	assert(false);
}

void SummaryUnit::build_ins_assi_side(AccessPath* s_ap, variable_info& side,
		bool& flag) {
	flag = false;
	if (s_ap->get_ap_type() == AP_VARIABLE) { //7
		Variable* ap_var = static_cast<Variable*>(s_ap);
		if ((!ap_var->is_argument()) && (!ap_var->is_return_var())) {
			return;
		}
		if (ap_var->is_argument()) {
			flag = true;
		}
		//cout << "check original var before it assigned to side.var " <<
		side.var = ap_var->get_original_var();

		if (0) {
			cout << "ap variable type " << ap_var->get_type()->to_string()
					<< endl;
			cout << "Variable ==  " << side.to_string() << endl;
		}
		return;
	}

	if (s_ap->get_ap_type() == AP_FIELD) { //5
		FieldSelection* fs = static_cast<FieldSelection*>(s_ap);
		side.fn = fs->get_field_name();
		//cout << "set side's type is " << fs->get_type()->to_string() << endl;
		side.ft = fs->get_type();

		AccessPath* inner = fs->get_inner();
		//cout << "inner is " << inner->to_string() << endl;

		if (inner->get_inner()->get_ap_type() == AP_ALLOC) {
			//cout << "Encountering return Alloc " << endl;
			//find the allocation instruction;
			MemNode* deref_alloc_node = mtdp->get_mem_graph()->get_memory_node(
					inner);
			sail::Variable* sail_var = mtdp->get_point_to_alloc_var(
					deref_alloc_node);

			if (sail_var == NULL)
				return;

			if ((!sail_var->is_argument()) && (!sail_var->is_return_variable()))
				return;

			flag = true;
			side.var = sail_var;
			return;
		}

		if (inner->get_inner()->get_ap_type() == AP_VARIABLE) {

			Variable* var = (Variable*) inner->get_inner();

			if ((!var->is_argument()) && (!var->is_return_var()))
				return;

			flag = true;
			//cout << "&&&& side var is " << var->get_original_var()->to_string() << endl;
			side.var = var->get_original_var();
			return;
		}

	}

}

void SummaryUnit::delete_ins_from_eq_ins(sail::Instruction* ins) {
	list<sail::Instruction*>::iterator it = eq_ins.begin();

	for (; it != eq_ins.end(); it++) {

		if (ins == *it) {
			cout << "Deleting " << (*it)->to_string() << endl;
			eq_ins.erase(it);
			return;
		}
	}
}

//void SummaryUnit::delete_ins_from_return_ins(sail::Instruction* ins){
//	vector<sail::Instruction*>::iterator it = return_ins.begin();
//
//	for(; it != return_ins.end(); it++){
//
//		if(ins == *it){
//			cout << "Deleting " << (*it)->to_string() << endl;
//			return_ins.erase(it);
//			return;
//		}
//	}
//}

string SummaryUnit::track_lhs_sides_to_string() {

	string str = " [track_lhs_sides_to_string]     ";

	set<side_ele*>::iterator it = track_lhs_sides.begin();

	for (; it != track_lhs_sides.end(); it++) {
		str += (*it)->to_string();
	}

	return str;
}

void SummaryUnit::update_track_include(sail::Instruction* ins,
		bool return_flag) {

	ins_two_side* two_sides = make_complete_two_sides(ins, true);

	instruction_side* lhs_s = two_sides->lhs;

	side_ele* lhs_ele = make_side_ele(lhs_s, return_flag, ins);

	track_lhs_sides.insert(lhs_ele);

}

void SummaryUnit::produce_assignment_ins(variable_info& lhs, int lhs_offset,
		variable_info& rhs, int rhs_offset, bool return_var_f) {
//	cout << "HYZ lhs is " << lhs.to_string() << endl;
//	cout << "HYZ rhs is " << rhs.to_string() << endl;
	if (0) {
		cout << "in testing -------------- " << endl;
		cout << "tracking lhs side ---->>>>>>>>>>>" << endl
				<< track_lhs_sides_to_string() << endl;

	}

	type_of_ins ti;

	if ((lhs_offset == IMPOSSIBLE_OFFSET)
			&& (rhs_offset == IMPOSSIBLE_OFFSET)) {

		ti = ASSIGNMENT;

	} else if ((lhs_offset != IMPOSSIBLE_OFFSET) && (rhs_offset =
	IMPOSSIBLE_OFFSET)) {

		ti = STORE;

	} else if ((lhs_offset == IMPOSSIBLE_OFFSET)
			&& (rhs_offset != IMPOSSIBLE_OFFSET)) {

		ti = LOAD;

	} else {

		ti = LOAD_AND_STORE;
	}
	il::location lo(-1, -1);

	switch (ti) {

	case ASSIGNMENT: {
		il::string_const_exp* temp = new il::string_const_exp("",
				lhs.var->get_type(), lo);
		sail::Assignment* assi = new sail::Assignment(lhs.var, rhs.var, temp,
				-1);

		cout << "HZHU |||  **************** generate assi " << assi->to_string()
				<< endl << endl;

		if (!return_var_f)
			add_to_eq_insts(assi);
		else
			add_to_return_insts(assi);

		//used to unique instruction;
		update_track_include(assi, return_var_f);
	}
		break;
	case STORE: {

		assert(lhs.ft != NULL);
		il::string_const_exp* temp = new il::string_const_exp("", lhs.ft, lo);

		sail::Store* s = new sail::Store(lhs.var, rhs.var, lhs_offset, lhs.fn,
				temp, -1);

		cout << "HZHU ||| *********************  generate store "
				<< s->to_string() << endl << endl;

		if (!return_var_f)
			add_to_eq_insts(s);
		else
			add_to_return_insts(s);
		if (s == NULL)
			cout << "instruction is NULL " << endl;
		if (s != NULL)
			update_track_include(s, return_var_f);
	}
		break;
	case LOAD: {
		assert(rhs.ft != NULL);
		il::string_const_exp* temp = new il::string_const_exp("", lhs.ft, lo);
		sail::Load* l = new sail::Load(lhs.var, rhs.var, rhs_offset, rhs.fn,
				temp, -1);

		cout << "HZHU ||| ********************* generate load "
				<< l->to_string() << endl << endl;

		if (!return_var_f)
			add_to_eq_insts(l);
		else
			add_to_return_insts(l);
		if (l != NULL)
			update_track_include(l, return_var_f);

	}
		break;
	case LOAD_AND_STORE: {

		sail::Variable* temp_var = new sail::Variable(
				sail::Variable::get_temp_name(), lhs.ft);
		il::string_const_exp* temp = new il::string_const_exp("", lhs.ft, lo);
		sail::Load* l = new sail::Load(temp_var, rhs.var, rhs_offset, rhs.fn,
				temp, -1);

		sail::Store* s = new sail::Store(lhs.var, temp_var, lhs_offset, lhs.fn,
				temp, -1);

		cout << "HZHU ||| **************** generate load " << l->to_string()
				<< endl << "& store " << s->to_string() << endl << endl;

		if (!return_var_f) {
			add_to_eq_insts(l);
			add_to_eq_insts(s);

		} else {
			add_to_return_insts(l);
			add_to_return_insts(s);

		}

		if (l != NULL)
			update_track_include(l, return_var_f);
		if (s != NULL)
			update_track_include(s, return_var_f);
	}
		break;
	default:
		assert(false);
	}

	if (0) {
		cout << "out testing------------ " << endl;
		cout << track_lhs_sides_to_string() << endl;
	}
}

bool SummaryUnit::inside_return_var_target_in_memory_graph(AccessPath* tar) {

	return (return_targets_in_memory_graph.count(tar) > 0);
}

bool SummaryUnit::inside_argument_target_in_memory_graph(AccessPath* tar,
		AccessPath* arg) {

	if (argument_ap_targets_in_memory_graph.find(arg)
			== argument_ap_targets_in_memory_graph.end())
		assert(false);

	set<AccessPath*> tars = argument_ap_targets_in_memory_graph[arg];

	if (tars.count(tar) > 0) {
		return true;
	}

	return false;
}

void SummaryUnit::testing_argument_ap_targets_in_memory_graph() {
	map<AccessPath*, set<AccessPath*> >::iterator it =
			argument_ap_targets_in_memory_graph.begin();

	for (; it != argument_ap_targets_in_memory_graph.end(); it++) {

		cout << "Arg :: " << it->first->to_string() << endl;
		set<AccessPath*>::iterator it1 = it->second.begin();

		for (; it1 != it->second.end(); it1++) {

			cout << "   -- t " << (*it1)->to_string() << endl;
		}

	}

}

string SummaryUnit::sub_ap_with_operators_to_string(
		vector<ap_with_operator>& sub_terms) {
	string str;

	vector<ap_with_operator>::iterator it = sub_terms.begin();

	for (; it != sub_terms.end(); it++) {

		str += (*it).to_string();
		str += "\n";
	}
	return str;
}

/*****************************************************************
 * for eg.(v1/v2)%v3 +1 would be collected as [+1, %3, /v2 and v1]
 *****************************************************************/

void SummaryUnit::collect_sub_terms(Term* t,
		vector<ap_with_operator>& sub_terms, bool collect_only_terminals) {
	switch (t->get_term_type()) {
	case CONSTANT_TERM:
	case VARIABLE_TERM: {
	}
		break;
	case FUNCTION_TERM: {

		FunctionValue* fv = (FunctionValue*) t;
		FunctionTerm* fn = fv;
		long int cur_op_code = fv->get_fun_type();
		vector<Term*> terms = fn->get_args();
		if (terms.size() != 2) {
			cout << "Function term is " << fv->to_string() << endl;
			cout << "Function_term size is " << terms.size() << endl;
		}
		//assert(terms.size() == 2);
		int i = 0;
		for (vector<Term*>::iterator it = terms.begin(); it != terms.end();
				it++) {

			Term* sub_term = *it;
			ap_with_operator apo;
			apo.ap = AccessPath::to_ap(sub_term);

			if (i++ == 0) {
				if (reflexible_operator(cur_op_code))
					apo.op = cur_op_code;

			} else {

				apo.op = cur_op_code;
			}

			if (DEBUG_INFO)
				cout << "|||apo is " << apo.to_string() << endl;
			if (collect_only_terminals) {

				if (is_terminal_term(sub_term))
					sub_terms.push_back(apo);

			} else
				sub_terms.push_back(apo);

			collect_sub_terms(sub_term, sub_terms, collect_only_terminals);
		}

	}
		break;
	case ARITHMETIC_TERM: {

		ArithmeticTerm* at = (ArithmeticTerm*) t;
		map<Term*, long int> elems = at->get_elems();
		map<Term*, long int>::iterator it = elems.begin();

		for (; it != elems.end(); it++) {
			ap_with_operator apo;
			apo.op = it->second;
			apo.ap = AccessPath::to_ap(it->first);

			if (DEBUG_INFO)
				cout << "||| ||| apo is " << apo.to_string() << endl;

			if (collect_only_terminals) {

				if (is_terminal_term(it->first))
					sub_terms.push_back(apo);

			} else
				sub_terms.push_back(apo);

			collect_sub_terms(it->first, sub_terms, collect_only_terminals);
		}

		long int constant = at->get_constant();
		if (constant != 0) {

			ConstantValue* const_value = ConstantValue::make(abs(constant));
			ConstantTerm* ct = const_value;

			int op = 0;
			if (constant > 0)
				op = 1;
			else
				op = -1;

			ap_with_operator apo;
			apo.ap = AccessPath::to_ap(ct);
			apo.op = op;

			if (DEBUG_INFO)
				cout << "||| ||| apo is " << apo.to_string() << endl;

			sub_terms.push_back(apo);
			collect_sub_terms(ct, sub_terms, collect_only_terminals);
		}
	}
		break;
	default:
		return;
	}
}

bool SummaryUnit::is_sub_ap(AccessPath* cur_ap,
		vector<ap_with_operator>& refer_ap_terms) {
	vector<ap_with_operator>::iterator it = refer_ap_terms.begin();

	for (; it != refer_ap_terms.end(); it++) {

		if (cur_ap == (*it).ap)
			return true;
	}
	return false;
}

bool SummaryUnit::is_sub_ap_with_operator(ap_with_operator& cur_ap,
		vector<ap_with_operator>& refer_sub_terms) {
	if (DEBUG_INFO)
		cout << "+++++++++++++ cur_ap is " << cur_ap.to_string() << endl;

	vector<ap_with_operator>::iterator it = refer_sub_terms.begin();
	for (; it != refer_sub_terms.end(); it++) {

		if (cur_ap == *it)
			return true;
	}

	return false;
}

bool SummaryUnit::belong_to_parts_of_terms(AccessPath* ap,
		vector<ap_with_operator>& biggest_ap_sub_terms) {

	if (is_sub_ap(ap, biggest_ap_sub_terms)) {
		cout << "directly find! " << endl;
		return true;
	}

	//otherwise split it all , check if every term inside the all_sub_terms;
	vector<ap_with_operator> sub_terms;
	collect_sub_terms(ap->to_term(), sub_terms, true);

	vector<ap_with_operator>::iterator it = sub_terms.begin();
	for (; it != sub_terms.end(); it++) {

		if (!is_sub_ap_with_operator(*it, biggest_ap_sub_terms)) {
			return false;
		}
	}

	return true;

}

void SummaryUnit::order_insts() {

	test_arithmetic_instruction_for_argument();

	list<sail::Instruction*>::iterator it = eq_ins.begin();

	eq_ins.insert(it, arithmetic_ins.begin(), arithmetic_ins.end());

	it = eq_ins.end();

	eq_ins.insert(it, return_ins.begin(), return_ins.end());

	remove_redundant_instruction();



	//collect_argument_and_return_var();
}



void SummaryUnit::collect_same_lhs_side_eles_for_diff_rhs(set<side_ele*>& s,
		one_side* one_side) {

	set<side_ele*>::iterator it = track_lhs_sides.begin();

	for (; it != track_lhs_sides.end(); it++) {
		side_ele* s_e = *it;
		if (s_e->ins_side->var == one_side->var) {
			if (s_e->ins_side->offset == one_side->offset) {

				s.insert(s_e);
			}
		}
	}
}

void SummaryUnit::check_rhs_and_delete_instructions(one_side* one_side) {

	set<side_ele*> set_contain_one_side;

	collect_same_lhs_side_eles_for_diff_rhs(set_contain_one_side, one_side);

	for (set<side_ele*>::iterator it = set_contain_one_side.begin();
			it != set_contain_one_side.end(); it++) {

		for (set<side_ele*>::iterator it1 = set_contain_one_side.begin();
				it1 != set_contain_one_side.end(); it1++) {

			if (*it == *it1)
				continue;

			string out_layer_str =
					(*it)->ins_side->other_side->basic_info_to_string();

			string inner_layer_str =
					(*it1)->ins_side->other_side->basic_info_to_string();

			for (set<side_ele*>::iterator it_lhs_track =
					this->track_lhs_sides.begin();
					it_lhs_track != this->track_lhs_sides.end();
					it_lhs_track++) {

				side_ele* cur_e = *it_lhs_track;
				string lhs_side_str = cur_e->ins_side->basic_info_to_string();
				string rhs_side_str =
						cur_e->ins_side->other_side->basic_info_to_string();

				if ((lhs_side_str == out_layer_str)
						&& (rhs_side_str == inner_layer_str)) {

					delete_ins_from_eq_ins((*it1)->consist_ins);

					break;

				} else if ((lhs_side_str == inner_layer_str)
						&& (rhs_side_str == out_layer_str)) {

					delete_ins_from_eq_ins((*it)->consist_ins);

					break;

				} else
					continue;

			}

		}
	}

}

void SummaryUnit::remove_redundant_instruction() {
	cout << "size of lhs is " << lhs_time.size() << endl;
	int idx = 0;
	for (map<one_side*, int>::iterator it = lhs_time.begin();
			it != lhs_time.end(); it++) {
		cout << "IDX ==> " << idx++ << endl;
		cout << "side :: " << (*it).first->to_string() << endl;

		cout << "Times :: " << it->second << endl << endl << endl;

		if ((it->second > 1)&&(it->second < REMOVE_REDUNDANCY_MAX_SIZE)) { //find the corresponding rhs
			check_rhs_and_delete_instructions(it->first);
		}

	}

}

void SummaryUnit::update_lhs_times_map(sail::Instruction* ins) {
	one_side* side = get_lhs_side(ins);

	if (lhs_time[side] > 0)
		lhs_time[side] = lhs_time[side] + 1;
	else
		lhs_time[side] = 1;
}

ins_two_side* SummaryUnit::make_complete_two_sides(sail::Instruction* ins,
		bool lhs) {
	ins_two_side* two_side = new ins_two_side();
	instruction_side* ins_one_side = new instruction_side(ins, lhs);
	if (lhs) {

		update_lhs_times_map(ins);
	}

	instruction_side* ins_another_side = new instruction_side(ins, !lhs);

	ins_one_side->other_side = ins_another_side;
	ins_another_side->other_side = ins_one_side;

	if (lhs) {

		two_side->lhs = ins_one_side;
		two_side->rhs = ins_another_side;

	} else {

		two_side->lhs = ins_another_side;
		two_side->rhs = ins_one_side;
	}

	return two_side;
}

one_side* SummaryUnit::get_lhs_side(sail::Instruction* ins) {

	sail::Symbol* s = NULL;
	int offset = IMPOSSIBLE_OFFSET;
	one_side* side;
	switch (ins->get_instruction_id()) {
	case sail::ASSIGNMENT: {
		s = ((sail::Assignment*) ins)->get_lhs();
	}
		break;
	case sail::LOAD: {
		s = ((sail::Load*) ins)->get_lhs();
	}
		break;
	case sail::STORE: {
		s = ((sail::Store*) ins)->get_lhs();
		offset = ((sail::Store*) ins)->get_offset();
	}
		break;
	default:
		assert(false);
	}
	side = new one_side(s, offset);

	one_side* ge_side;

	if (lhs_sides.count(side->to_string()) > 0) {

		ge_side = lhs_sides[side->to_string()];
		delete side;

	} else {

		ge_side = side;
		lhs_sides[ge_side->to_string()] = ge_side;
	}

	return ge_side;
}

side_ele* SummaryUnit::make_side_ele(instruction_side* instruction_side,
		bool is_return, sail::Instruction* ins) {

	side_ele* ele = new side_ele(instruction_side, is_return, ins);
	string str = ele->to_string();

	if (side_ele_map.count(str) > 0) {
		return side_ele_map[str];

		delete ele;

	} else {
		side_ele_map[str] = ele;
		return ele;
	}

}

//void SummaryUnit::set_alloc_context_controller(AllocContextController* ac){
//	allocc = ac;
//	this->has_alloc = true;
//}

MtdInstsProcessor* SummaryUnit::get_mtd_insts_processor() {
	return this->mtdp;
}

const string SummaryUnit::to_string() const {

	string str;
	int i = 0;
	list<sail::Instruction*>::const_iterator it = eq_ins.begin();
	for (; it != eq_ins.end(); it++) {
		str += '[';
		str += int_to_string(i++);
		str += "] : ";
		str += (*it)->to_string(true);

		str += "\n";
	}

//	str += "ALLOC  ==   ";
//	vector<sail::Instruction*>::const_iterator it_1 = alloc_ins.begin();
//	for(; it_1 != alloc_ins.end(); it_1 ++){
//		str += (*it_1)->to_string();
//		str += "\n";
//	}

	return str;
}

void SummaryUnit::test_arithmetic_instruction_for_argument() {

	cout << "==== arithmetic instruction for argument ! ---- " << endl;

	vector<sail::Instruction*>::iterator it = arithmetic_ins.begin();
	cout << "really  ?? " << endl;
	for (; it != arithmetic_ins.end(); it++) {
		cout << (*it)->to_string() << endl;
	}
	cout << " ------------ " << endl;
}

string SummaryUnit::solved_ap_symbols_to_string() {
	string str;
	map<AccessPath*, sail::Symbol*>::iterator it = solved_ap_symbols.begin();
	for (; it != solved_ap_symbols.end(); it++) {
		str += "ap :: ";
		str += it->first->to_string();
		str += "  symbol:: ";
		str += it->second->to_string();
		str += "\n";
	}
	return str;
}

//list<sail::Instruction*>& SummaryUnit::get_equivalent_insts() {
//	return this->eq_ins;
//}
